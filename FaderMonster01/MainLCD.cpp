#include "header.h"



FlexIOSPI SPIflex(MAINLCD_SPI_PINS, -1); // Setup on (int mosiPin, int misoPin, int sckPin, int csPin=-1) :

static TFT_eSPI tft = TFT_eSPI(240,320,SPIflex,MAINLCD_CS);
static TFT_eSprite sprite{&tft}; // sprite for off-screen rendering
 
//----------------------------------------------------------------------------
// callback executed within ISR when DMA SPI transfer completes
static void TFTdmaDoneCB(FlexIOSPI* pFlex)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE; 

  xTaskNotifyFromISR(mainLCDtask.handle, 
                     1, eSetBits,
                     &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );  
}

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
        if (pixels < 16) // stupidly small!
        {
            int fudge = 2;
            w += fudge;
            if (x+w > display.width())
                x -= fudge;
            h += fudge;           
            if (y+h > display.height())
                y -= fudge;
        }
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
  elapsedMicros eu = 0;        
//  Serial.printf("%d : %dx%d @ %d,%d (%d)", timeoutCount, w,h,x,y, w*h);
          display.startWrite();
          updateCount++;
          display.pushImageDMA(x,y,w,h,DMAbuffer);
//  Serial.print("... ");
          pushNeeded = TFTdmaWait(w*h); // suspend until DMA completes, then tidy up
  uint32_t t = eu;        
  // Serial.printf("done (%dus)\n", t);
          if (pushNeeded)
          {
            // pauseOutput = true;
  Serial.printf("%d : %d : %dx%d @ %d,%d (%d)\n", timeoutCount, updateCount, w,h,x,y, w*h);
            tft.fillRect(0,0,tft.width(), tft.height() - 20, TFT_RED);
            tft.fillRect(1+(timeoutCount-1)*10,221,8,18, TFT_BLUE);
            tft.fillRect(320 - stallCount*10 + 1,221,8,18, TFT_GREEN);
          }
        } 
    }

    return result;
}

void randomRect(TFT_eSPI& tft)
{
  int x,y, w, h;
  w = random(140); h = random(80);
  uint16_t colour = random(65536);

  do
  {
    x = random(tft.width());
    y = random(tft.height());
  } while (x+w > tft.width() || y+h > tft.height() - 20);

  //Serial.printf("%dx%d @ %d,%d; %04hX\n", w,h,x,y,colour);
  //tft.fillRect(x,y,w,h,colour);
  tft.setViewport(x,y,w,h);
  tft.fillScreen(colour);

  char buf[50];
  sprintf(buf,"%dx%d @ %d,%d", w,h,x,y);
  
  tft.setTextColor(~colour);
  tft.setTextWrap(true);
  tft.drawString(buf,1,1);
  tft.resetViewport();
}

// function called by client task to queue the request
// returns reference to the request, so we can interrogate it
// for success immediately
InterTaskRequest& MainLCDtask::updateDirty(InterTaskRequest& req,  // request to be filled in
                                            TFT_eSprite& display,  // sprite requiring update to display
                                            TickType_t timeout)
{
    requestPayload payload{&MainLCDtask::doUpdateDirty, &display};
    RequestQueue<MainLCDtask, requestPayload>::queueEntry entry{&req,payload};

    reqQueue.request(entry, timeout);      // queue the request, if inactive

    return req;
}
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
  // ---------------------------------------------------------------------

  tft.fillScreen(TFT_DARKGREY);
}

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
  sprite.setSpriteSwapBytes(false);
  // ---------------------------------------------------------------------


  Serial.printf("\n[%d]: ready\n", micros());
  initComplete = true;

  int colour = 0;
  elapsedMillis em = 0;
  while (1)
  {
      reqQueue.executeRequest(*this, 10);
      vTaskDelay(10);

      // test code
      if (!pauseOutput)
      {
        randomRect(sprite);
        doUpdateDirty(&sprite);
      }

      if (zapScreen)
      {
        zapScreen = false;
        tft.fillScreen(TFT_LIGHTGREY);
        stallCount = 0;
        timeoutCount = 0;
      }

      // check whether this task is running
      cycleLED(em, colour, 0);
  }
}

MainLCDtask mainLCDtask{"mainLCD", 512, nullptr, 
                        1,            // display updates are a fairly low priority
                        tft, sprite,  // actual tft and sprite objects
                        NUM_POTS      // allow for one request per strip
                       }; 