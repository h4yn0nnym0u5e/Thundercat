#include "header.h"

//========================================================================
//
//             888             888    d8b          
//             888             888    Y8P          
//             888             888                 
//    .d8888b  888888  8888b.  888888 888  .d8888b 
//    88K      888        "88b 888    888 d88P"    
//    "Y8888b. 888    .d888888 888    888 888      
//         X88 Y88b.  888  888 Y88b.  888 Y88b.    
//     88888P'  "Y888 "Y888888  "Y888 888  "Y8888P 
//
FlexIOSPI SPIflex(MAINLCD_SPI_PINS, -1); // Setup on (int mosiPin, int misoPin, int sckPin, int csPin=-1) :

static TFT_eSPI tft = TFT_eSPI(240,320,SPIflex,MAINLCD_CS);
static TFT_eSprite sprite{&tft}; // sprite for off-screen rendering
 
//----------------------------------------------------------------------------
//
//                      888 888 888                        888      
//                      888 888 888                        888      
//                      888 888 888                        888      
//     .d8888b  8888b.  888 888 88888b.   8888b.   .d8888b 888  888 
//    d88P"        "88b 888 888 888 "88b     "88b d88P"    888 .88P 
//    888      .d888888 888 888 888  888 .d888888 888      888888K  
//    Y88b.    888  888 888 888 888 d88P 888  888 Y88b.    888 "88b 
//     "Y8888P "Y888888 888 888 88888P"  "Y888888  "Y8888P 888  888 
//
// callback executed within ISR when DMA SPI transfer completes
static void TFTdmaDoneCB(FlexIOSPI* pFlex)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE; 
//  bool inISR = (SCB_ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;

  xTaskNotifyFromISR(mainLCDtask.handle, 
                     1, eSetBits,
                     &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );  
}

//========================================================================
//
//                     d8b                   888            
//                     Y8P                   888            
//                                           888            
//    88888b.  888d888 888 888  888  8888b.  888888 .d88b.  
//    888 "88b 888P"   888 888  888     "88b 888   d8P  Y8b 
//    888  888 888     888 Y88  88P .d888888 888   88888888 
//    888 d88P 888     888  Y8bd8P  888  888 Y88b. Y8b.     
//    88888P"  888     888   Y88P   "Y888888  "Y888 "Y8888  
//    888                                                   
//    888                                                   
//    888                                                   
//
// Task-level code: block until DMA is complete
int timeoutCount, stallCount;
bool MainLCDtask::TFTdmaWait(int pixels)
{
  bool timedOut = false;
  uint32_t notifiedValue;
  const float pixelsToMicros = 0.3f;
  int ticks = 2 + (int)(pixels * pixelsToMicros / 1000.0f);
  // wait for notification from async TFT_eSPI library
  digitalWriteFast(DBG1, arduino::HIGH);
  xTaskNotifyStateClear(nullptr);
//Serial.printf(" wait for %d ticks ", ticks);
  timedOut = pdPASS != xTaskNotifyWait(0, UINT32_MAX, 
                                       &notifiedValue,
                                       ticks /* portMAX_DELAY */);
  digitalWriteFast(DBG1, arduino::LOW);
//Serial.print("notified ");
  if (timedOut)
  {
    Serial.print("********** timeout *********** ");
    SPIflex.killTransfer();
    timeoutCount++;
  }

  bool stalled = tft.dmaBusy();
  for (int i=0;i<5 && stalled;i++)
  {
    vTaskDelay(1);
    stalled = tft.dmaBusy();
  }

  if (stalled && !timedOut)
  {
    Serial.print("********** stalled *********** ");
    timedOut |= stalled;
    stallCount++;
  }

  // now we can...
  tft.dmaWait(!timedOut); // ...tidy up...
  tft.endWrite();         // ...and release the SPI bus

  return timedOut;
}


//----------------------------------------------------------------------------
// actual method used to do the update
int updateCount;
InterTaskRequest::Result MainLCDtask::doUpdateDirty(void* pDisplay)
{
    TFT_eSprite& display = *((TFT_eSprite*) pDisplay);

    InterTaskRequest::Result result = InterTaskRequest::Result::done;
    int32_t x,y,w,h;
    uint16_t* src = (uint16_t*) display.getPointer();
    int sw = display.width();
    
    // this has to be atomic, just in 
    // case the caller is badly-behaved - 
    // it shouldn't queue another update
    // until this one is complete
    taskENTER_CRITICAL();
    bool isDirty = display.getDirtyArea(x,y,w,h);
    display.clearDirtyArea();
    taskEXIT_CRITICAL();
    //Serial.printf("Dirty area: %dx%d @ %d,%d (%s)\n", w,h,x,y, isDirty?"dirty":"clean");

    // need a valid sprite and buffer, and sprite has to need updating
    if (nullptr != src && nullptr != DMAbuffer && isDirty)
    {
        int pixels = w*h;
#if 1
// Doesn't seem to be needed with FlexIOSPI:
// wrong: 1x1 foxes it!
        if (pixels < 2) // stupidly small!
        {
          // Serial.printf("fudged %dx%d\n", w,h);
          int fudge = 1;
          w += fudge;
          if (x+w > display.width())
              x -= fudge;
          h += fudge;           
          if (y+h > display.height())
              y -= fudge;
        }
#endif // 0
        uint16_t* dst = DMAbuffer;
        src += y*sw + x;
        for (int i=0;i<h;i++)
        {
            memcpy(dst, src, w*sizeof *src);
            dst += w;
            src += sw;
        }

        bool pushNeeded = true;
        while (pushNeeded) 
        {
          display.startWrite();
          updateCount++;
          display.pushImageDMA(x,y,w,h,DMAbuffer);
          pushNeeded = TFTdmaWait(w*h); // suspend until DMA completes, then tidy up
#if 1          
          if (pushNeeded)
          {
            // pauseOutput = true;
  Serial.printf("%d : %d : %dx%d @ %d,%d (%d)\n", timeoutCount, updateCount, w,h,x,y, w*h);
            //tft.fillRect(0,0,tft.width(), tft.height() - 25, TFT_RED);
            tft.fillRect(1+(timeoutCount-1)*10,221,8,18, TFT_BLUE);
            tft.fillRect(320 - stallCount*10 + 1,221,8,18, TFT_GREEN);
          }
#endif // including debug code          
        } 
    }

    return result;
}

//========================================================================
//
//                      888      888 d8b          
//                      888      888 Y8P          
//                      888      888              
//    88888b.  888  888 88888b.  888 888  .d8888b 
//    888 "88b 888  888 888 "88b 888 888 d88P"    
//    888  888 888  888 888  888 888 888 888      
//    888 d88P Y88b 888 888 d88P 888 888 Y88b.    
//    88888P"   "Y88888 88888P"  888 888  "Y8888P 
//    888                                         
//    888                                         
//    888                                         
//
// function called by client task to queue the request
// returns reference to the request, so we can interrogate it
// for success immediately
InterTaskRequest& MainLCDtask::updateDirty(InterTaskRequest& req,  // request to be filled in
                                            TFT_eSprite& display,  // sprite requiring update to display
                                            TickType_t timeout)
{
    requestPayload payload{&MainLCDtask::doUpdateDirty, &display};
    RequestQueue<MainLCDtask, requestPayload>::queueEntry entry{&req,payload};

    if (display.isDirty())
      reqQueue.request(entry, timeout);      // queue the request, if inactive
    else 
      req.status = InterTaskRequest::Result::done; // nothing to do, so it's done!      

    return req;
}

//========================================================================
//
//    d8b          d8b 888    
//    Y8P          Y8P 888    
//                     888    
//    888 88888b.  888 888888 
//    888 888 "88b 888 888    
//    888 888  888 888 888    
//    888 888  888 888 Y88b.  
//    888 888  888 888  "Y888 
//
//----------------------------------------------------------------------------
void MainLCDtask::initDisplayPins(void)
{
  pinMode(MAINLCD_BL,arduino::OUTPUT);
  pinMode(MAINLCD_CS,arduino::OUTPUT);

  // turn backlight off
  digitalWriteFast(MAINLCD_BL, arduino::LOW);
}


// elapsedMicros eu;
bool MainLCDtask::doAphase(int& phase)
{
  int newPhase = tft.phasedInit(0, phase);
  if (newPhase != phase)
  {
//    Serial.printf("%d: tft #%d -> phase %d\n", (int) eu, i, newPhase);
    phase = newPhase;    
  }

  return newPhase < 0;
}


void MainLCDtask::phasedInit(void)
{
  elapsedMillis em = 0;
  int phase{0};
  bool finished = false;

  while (!finished)
  {
    finished = true;
    finished &= doAphase(phase);
    vTaskDelay(1);
  }
  Serial.printf("[%d] Main LCD phased init - took %dms\n", micros(), (int) em);

  // FlexIOSPI-specific stuff --------------------------------------------
  // This gives us a base clock of 120MHz:
  SPIflex.flexIOHandler()->setClock(120'000'000.0f);

  uint32_t clk = SPIflex.flexIOHandler()->computeClockRate();
  Serial.printf("Updated Flex IO speed: %u; SPI clock will be an integer division of %u\n", clk, clk/2);
  SPIflex.setTransferCallback(TFTdmaDoneCB);
  Serial.printf("SPI clock is %.1fMHz\n", (float) SPIflex.getSCKrate() / 1000000.0f);
  // ---------------------------------------------------------------------

  tft.fillScreen(TFT_DARKGREY);
}

//========================================================================
//
//    888                      888      
//    888                      888      
//    888                      888      
//    888888  8888b.  .d8888b  888  888 
//    888        "88b 88K      888 .88P 
//    888    .d888888 "Y8888b. 888888K  
//    Y88b.  888  888      X88 888 "88b 
//     "Y888 "Y888888  88888P' 888  888 
//
/*
 * This is the one task that's allowed to 
 * access the main LCD's hardware
 */
void MainLCDtask::run(void)
{
  Serial.printf("[%d]: main display task: init pins ...\n", micros());
  initDisplayPins();

  // LCD and GT911 share a reset signal, and it requires 
  // specific timings to set the GT911 I²C address. Hence
  // we wait for the touch code to do the reset before we
  // initialise the display.
  while (!touchTask.touchReady)
  {
    //Serial.print('!');
    vTaskDelay(50);
  }

  // ---------------------------------------------------------------------
  phasedInit();  // does phased init then initial screen fill

  // set backlights to full power (640ms)
  Serial.printf("[%d] main backlight ...\n", micros());
  for (int i=1;i<256;i+=2)
  {
      analogWrite(MAINLCD_BL,i);
      vTaskDelay(5);
  }
  // this will depend on your hardware!
  tft.setRotation(MAIN_TFT_ROTATION);
  tft.invertDisplay(true);

  //tft.setSPISpeed(60'000'000);  // done in phasedInit()
  tft.initDMA();
  // ---------------------------------------------------------------------

  // create the DMA buffer
  setDMAbuffer(allocateDMAbuffer(tft.width(), tft.height()));
  //setDMAcompletionISR(sssss);
  // ---------------------------------------------------------------------
  // create sprite buffer in PSRAM
  int w, h;
  sprite.getTFTarea(w,h);
  Serial.printf("Main LCD is %dx%d\n",w,h);
  sprite.createInPSRAM(true);
  taskENTER_CRITICAL();
  sprite.createSprite(w,h);
  taskEXIT_CRITICAL();
                
  // basic settings
  sprite.setSpriteSwapBytes(true);
  // ---------------------------------------------------------------------


  Serial.printf("\n[%d]: ready\n", micros());
  initComplete = true;

  //int colour = 0;
  //elapsedMillis em = 0;
  while (1)
  {
      reqQueue.executeRequest(*this, 10);
      vTaskDelay(1);

      if (zapScreen)
      {
        zapScreen = false;
        tft.fillScreen(TFT_LIGHTGREY);
        stallCount = 0;
        timeoutCount = 0;
      }

      // check whether this task is running
      //cycleLED(em, colour, 0);
  }
}

MainLCDtask mainLCDtask{"mainLCD", 512, nullptr, 
                        1,            // display updates are a fairly low priority
                        tft, sprite,  // actual tft and sprite objects
                        NUM_POTS      // allow for one request per strip
                       }; 