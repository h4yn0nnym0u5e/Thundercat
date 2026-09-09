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

LittleFS_SPIFram FRAMfs;

//----------------------------------------------------------------------------
//
//    888 d8b 888    888    888          8888888888 .d8888b.  
//    888 Y8P 888    888    888          888       d88P  Y88b 
//    888     888    888    888          888       Y88b.      
//    888 888 888888 888888 888  .d88b.  8888888    "Y888b.   
//    888 888 888    888    888 d8P  Y8b 888           "Y88b. 
//    888 888 888    888    888 88888888 888             "888 
//    888 888 Y88b.  Y88b.  888 Y8b.     888       Y88b  d88P 
//    888 888  "Y888  "Y888 888  "Y8888  888        "Y8888P"  
//
bool MainLCDtask::initFS(void)
{
  bool ok;

  pinMode(MRAM_CS, arduino::OUTPUT);
  digitalWriteFast(MRAM_CS, arduino::HIGH);
  vTaskDelay(5);

  if ((ok = FRAMfs.begin(MRAM_CS, SPIflex, true))) // use FlexIOSPI, configured above
  {
    const size_t uidsz = 19;
    uint8_t buffer[uidsz];

    FRAMfs.getUniqueID(buffer,uidsz);

    Serial.printf("MRAM mfr ID: %02X %02X; unique ID: ", buffer[3], buffer[4]);
    for (size_t i=5;i<uidsz;i++)
      Serial.printf("%02X ", buffer[i]);
    Serial.println();      
    Serial.printf("\n%u Storage list initialized.\n", millis());
  }//  MTP.addFilesystem(FRAMfs, FRAMfs.name());
  else
    Serial.printf("\nStorage not added for pin %d", MRAM_CS);
    
  return ok;    
}
 
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
  uint32_t SPIclk = SPIflex.getSCKrate(); // normally 60'000'000 ... but
  const float pixelsToMicros = 0.3f * 60'000'000 / SPIclk;
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

// record and restore last-used settings
static const char* settingsRecord = "lastSettings.txt";
bool MainLCDtask::recordLastSetting(const char* fileName)
{
  bool result = false;
taskENTER_CRITICAL();
  File f = FRAMfs.open(settingsRecord, FILE_WRITE_BEGIN);
taskEXIT_CRITICAL();
  if (f)
  {
    size_t nameLen = strlen(fileName);
taskENTER_CRITICAL();
    f.write(fileName, nameLen);
    f.close();
taskEXIT_CRITICAL();
    Serial.printf("Wrote '%s' to '%s'\n", fileName, settingsRecord);
    result = true;
  }
  return result;
}

bool MainLCDtask::restoreLastSetting(void)
{
  bool result = false;
taskENTER_CRITICAL();
  File f = FRAMfs.open(settingsRecord, FILE_READ);
taskEXIT_CRITICAL();
  if (f)
  {
    char buf[40];
taskENTER_CRITICAL();
    size_t len = f.read(buf, sizeof buf-1);
    f.close();
taskEXIT_CRITICAL();
    if (len > 0)
    {
      buf[len+1] = 0;
   
      loadWasManual = false;
      result = doLoadSettings(buf) == InterTaskRequest::Result::done;
    }
    else
      Serial.printf("Failed to read from %s\n", settingsRecord);
  }

  return result;

}

// save settings to CSV file
InterTaskRequest::Result MainLCDtask::doSaveSettings(void* _fileName)
{
  const char* fileName = (char*) _fileName;
  InterTaskRequest::Result result = InterTaskRequest::Result::failed;

  Serial.printf("Save to '%s'\n", fileName);

taskENTER_CRITICAL();
  File f = FRAMfs.open(fileName,FILE_WRITE_BEGIN); // overwrite
taskEXIT_CRITICAL();
  if (f)
  {
    Settings::save(f);
taskENTER_CRITICAL();
    f.close();
taskEXIT_CRITICAL();
    result = InterTaskRequest::Result::done;

    recordLastSetting(fileName);
  }

  return result;
}

// load settings from CSV file
InterTaskRequest::Result MainLCDtask::doLoadSettings(void* _fileName)
{
  const char* fileName = (char*) _fileName;
  InterTaskRequest::Result result = InterTaskRequest::Result::failed;

  Serial.printf("Load from '%s'\n", fileName);

taskENTER_CRITICAL();
  File f = FRAMfs.open(fileName,FILE_READ);
taskEXIT_CRITICAL();
  if (f)
  {
    Settings::load(f, faderMonsterSettings);
taskENTER_CRITICAL();
    f.close();
taskEXIT_CRITICAL();
    result = InterTaskRequest::Result::done;

    if (loadWasManual)
      recordLastSetting(fileName);
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
// functions called by client task to queue a request
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


InterTaskRequest& MainLCDtask::sendPayload(InterTaskRequest& req, requestPayload& payload, TickType_t timeout)
{
    RequestQueue<MainLCDtask, requestPayload>::queueEntry entry{&req,payload};
    reqQueue.request(entry, timeout);      // queue the request, if inactive
    return req;
}


InterTaskRequest& MainLCDtask::saveSettings(InterTaskRequest& req,  // request to be filled in
                                            char* fileName,         // name for file
                                            TickType_t timeout)
{
    requestPayload payload{&MainLCDtask::doSaveSettings, fileName};
    opIsSave = true;
    return sendPayload(req, payload, timeout);
}


InterTaskRequest& MainLCDtask::loadSettings(InterTaskRequest& req,  // request to be filled in
                                            char* fileName,         // name for file
                                            TickType_t timeout)
{
    requestPayload payload{&MainLCDtask::doLoadSettings, fileName};
    opIsSave = false;
    loadWasManual = true;
    return sendPayload(req, payload, timeout);
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
  // Start the filesystem
  if (initFS())
  {
    // quick test - dump a pre-existing file
    int dumped = 999; // don't dump!
    vTaskDelay(2);

    for (int i=0;i<10 && dumped < 1;i++)
    {
      //File f = FRAMfs.open("log.txt", FILE_READ);
taskENTER_CRITICAL();
      File f = FRAMfs.open("scene-1.csv", FILE_READ);
taskEXIT_CRITICAL();
      if (f)
      {
        int fch, idx = 0;
        char buf[50];
        Serial.println("=======================");
        do
        {
          // seems to need a critical section
taskENTER_CRITICAL();
          fch = f.readBytes(buf,sizeof buf - 1);
taskEXIT_CRITICAL();
          if (fch > 0)
          {
            buf[fch] = 0;
            Serial.print(buf);
          }
          else 
            fch = -1;
        } while (fch >= 0);
        buf[idx] = 0;
        
        
        Serial.print(buf);
taskENTER_CRITICAL();
        f.close();
taskEXIT_CRITICAL();
        Serial.println("=======================");
        dumped++;
      }
      else
      {
        Serial.printf("try %d failed; wait %d... ", i+1, i*10);
        vTaskDelay(i*10);
      }
    }
    restoreLastSetting();
  }
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

MainLCDtask mainLCDtask{"mainLCD", 768, nullptr, 
                        1,            // display updates are a fairly low priority
                        tft, sprite,  // actual tft and sprite objects
                        NUM_POTS      // allow for one request per strip
                       }; 