#include "header.h"

/*
 * Use 74LVC138 decoder to provide /CS signal to one of
 * 8 displays, using only 4 Teensy outputs
 */
[[maybe_unused]] void CSfn(int which, bool negate)
{
  if (negate)
    digitalWriteFast(MUX_G,1);
  else
  {
    digitalWriteFast(MUX_A,(which&1)!=0);
    digitalWriteFast(MUX_B,(which&2)!=0);
    digitalWriteFast(MUX_C,(which&4)!=0);
    digitalWriteFast(MUX_G,0);
  }
}


TFT_TYPE ScribbleTask::tft1{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(0, negate); }};
TFT_TYPE ScribbleTask::tft2{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(1, negate); }};
TFT_TYPE ScribbleTask::tft3{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(2, negate); }};
TFT_TYPE ScribbleTask::tft4{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(3, negate); }};
TFT_TYPE ScribbleTask::tft5{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(4, negate); }};
TFT_TYPE ScribbleTask::tft6{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(5, negate); }};
TFT_TYPE ScribbleTask::tft7{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(6, negate); }};
TFT_TYPE ScribbleTask::tft8{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(7, negate); }};

TFT_TYPE* ScribbleTask::scribbles[]
{&ScribbleTask::tft1, &ScribbleTask::tft2, &ScribbleTask::tft3, &ScribbleTask::tft4, 
 &ScribbleTask::tft5, &ScribbleTask::tft6, &ScribbleTask::tft7, &ScribbleTask::tft8};

 bool ScribbleTask::initComplete{false};
//----------------------------------------------------------------------------
// run from ISR when TFT DMA has finished
void DMAcompletionISR(TFT_eSPI& which) 
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE; 

  vTaskNotifyGiveFromISR(scribbleTask.handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

//----------------------------------------------------------------------------
// actual method used to do the update
InterTaskRequest::Result ScribbleTask::doUpdateDirty(void* pScribble)
{
    TFT_eSprite& scribble = *((TFT_eSprite*) pScribble);

    InterTaskRequest::Result result = InterTaskRequest::Result::done;
    int32_t x,y,w,h;
    uint16_t* src = (uint16_t*) scribble.getPointer();
    int sw = scribble.width();
    
    // this has to be atomic, just in 
    // case the caller is badly-behaved - 
    // it shouldn't queue another update
    // until this one is complete
    taskENTER_CRITICAL();
    bool isDirty = scribble.getDirtyArea(x,y,w,h);
    scribble.clearDirtyArea();
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
            if (x+w > scribble.width())
                x -= fudge;
            h += fudge;           
            if (y+h > scribble.height())
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
        scribble.startWrite();
        scribble.pushImageDMA(x,y,w,h,DMAbuffer);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // suspend until DMA completes
        scribble.endWrite();
    }

    return result;
}


// function called by client task to queue the request
// returns reference to the request, so we can interrogate it
// for success immediately
InterTaskRequest& ScribbleTask::updateDirty(InterTaskRequest& req,  // request to be filled in
                                            TFT_eSprite& scribble,  // sprite requiring update to display
                                            TickType_t timeout)
{
    requestPayload payload{&ScribbleTask::doUpdateDirty, &scribble};
    RequestQueue<ScribbleTask, requestPayload>::queueEntry entry{&req,payload};

    reqQueue.request(entry, timeout);      // queue the request, if inactive

    return req;
}
//----------------------------------------------------------------------------
void ScribbleTask::initDisplayPins(void)
{
  // TFT reset is not on a Teensy pin!
#if defined(TFT_RST)
  if (TFT_RST < 0)
    Serial.printf("TFT_RST set to %d - probably an error!\n", TFT_RST);
#endif // defined(TFT_RST)
    
  pinMode(TFT_BLK,arduino::OUTPUT);
  //pinMode(TFT_RST,arduino::OUTPUT);
  pinMode(MUX_A,arduino::OUTPUT);
  pinMode(MUX_B,arduino::OUTPUT);
  pinMode(MUX_C,arduino::OUTPUT);
  pinMode(MUX_G,arduino::OUTPUT);

  // turn backlight off
  digitalWriteFast(TFT_BLK, arduino::LOW);

  // reset display
  /*
  digitalWriteFast(TFT_RST, arduino::HIGH);
  vTaskDelay(1);
  digitalWriteFast(TFT_RST, arduino::LOW);
  vTaskDelay(1);
  digitalWriteFast(TFT_RST, arduino::HIGH);
  */
}


// elapsedMicros eu;
bool ScribbleTask::doAphase(int i, int& phase)
{
  int newPhase = scribbles[i]->phasedInit(0, phase);
  if (newPhase != phase)
  {
//    Serial.printf("%d: tft #%d -> phase %d\n", (int) eu, i, newPhase);
    phase = newPhase;    
  }

  return newPhase < 0;
}

#define ALL_TFTS for (int i=0;i<8;i++) (*scribbles[i])
#define FN_TFTS(fn) for (int i=0;i<8;i++) fn(*scribbles[i],i)

void ScribbleTask::fillUnique(TFT_TYPE& tft, int i)
{
  tft.fillScreen(faderMonsterSettings.stripsConfig[i].scribble.colours.fg);
}

void ScribbleTask::phasedInit(void)
{
  elapsedMillis em = 0;
  int phases[8]{0};
  bool finished = false;

//  eu = 0;
  while (!finished)
  {
    finished = true;
    for (int i=0;i<8;i++)
    {
      finished &= doAphase(i,phases[i]);
    }
    vTaskDelay(1);
  }
  Serial.printf("Phased init - took %dms\n", (int) em);
  FN_TFTS(fillUnique);
}

/*
 * This is the one task that's allowed to 
 * access the scribble displays' hardware
 */
void ScribbleTask::run(void)
{
    Serial.printf("[%d]: scribble task: init pins ...\n", micros());
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

    //Serial.print(" phased init ...");
    phasedInit();  // does phased init then initial screen fill

    // set backlights to half-power (640ms)
    Serial.print(" backlight ...");
    for (int i=0;i<128;i+=1)
    {
        analogWrite(TFT_BLK,i);
        vTaskDelay(5);
    }
    // this will depend on your hardware!
    ALL_TFTS.setRotation(TFT_ROTATION);
    //tft.setSPISpeed(60'000'000);  
    ALL_TFTS.initDMA();

    // assume all scribble TFTs are the same size
    setDMAbuffer(allocateDMAbuffer(tft1.width(), tft1.height()));
    setDMAcompletionISR(DMAcompletionISR);

    Serial.printf("\n[%d]: ready\n", micros());
    initComplete = true;

    //int colour = 0;
    //elapsedMillis em = 0;
    while (1)
    {
        reqQueue.executeRequest(*this, 10);

        // check whether this task is running
        // cycleLED(em, colour, 0);
    }
}

ScribbleTask scribbleTask{"Scribble", 512, nullptr, 
                          1,         // display updates are a fairly low priority
                          NUM_POTS   // allow for one request per strip
                         }; 