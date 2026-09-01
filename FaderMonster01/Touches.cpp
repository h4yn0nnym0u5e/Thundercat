/*
 * Quick test of the AT42QT2120 touch sensor
 */
#include "header.h"
#include <Wire.h>


AT42QT2120 potsTouch{TOUCH_WIRE, TOUCH_ADDR, 0xFF0};

static void isrTouch(void);

TouchStatus TouchTask::keyStatuses[NUM_POTS];

//===========================================================
//
//     .d8888b. 88888888888 .d8888b.   d888    d888   
//    d88P  Y88b    888    d88P  Y88b d8888   d8888   
//    888    888    888    888    888   888     888   
//    888           888    Y88b. d888   888     888   
//    888  88888    888     "Y888P888   888     888   
//    888    888    888           888   888     888   
//    Y88b  d88P    888    Y88b  d88P   888     888   
//     "Y8888P88    888     "Y8888P"  8888888 8888888 
//   
// see touchGT911.cpp

//===========================================================
void printTouches(void)
{
    static int last[NUM_POTS]{0};
    bool changed = false;
    for (int i=0;i<NUM_POTS;i++)
    {
      int t = (int) TouchTask::keyStatuses[i].getExtendedStatus();
      if (last[i] != t)
        changed = true;
      last[i] = t;
    }

    if (changed)
    {
      Serial.printf("%u: ", millis());
      for (int i=0;i<NUM_POTS;i++)
      {
        int t = (int) TouchTask::keyStatuses[i].getExtendedStatus();
        if (0 != t)
          Serial.printf("%2d ", t);
        else
          Serial.print(" - ");      
      }
      Serial.println();
    }
}

//===========================================================
//
//           d8888 88888888888  d8888   .d8888b.   .d88888b.   .d8888b.   d888    .d8888b.   .d8888b.  
//          d88888     888     d8P888  d88P  Y88b d88P" "Y88b d88P  Y88b d8888   d88P  Y88b d88P  Y88b 
//         d88P888     888    d8P 888         888 888     888        888   888          888 888    888 
//        d88P 888     888   d8P  888       .d88P 888     888      .d88P   888        .d88P 888    888 
//       d88P  888     888  d88   888   .od888P"  888     888  .od888P"    888    .od888P"  888    888 
//      d88P   888     888  8888888888 d88P"      888 Y8b 888 d88P"        888   d88P"      888    888 
//     d8888888888     888        888  888"       Y88b.Y8b88P 888"         888   888"       Y88b  d88P 
//    d88P     888     888        888  888888888   "Y888888"  888888888  8888888 888888888   "Y8888P"  
//                                                       Y8b                                           
//

// see if chip responds
bool AT42QT2120::probe(void)
{
    theWire.begin();
    theWire.beginTransmission(touch_addr);
    return 0 == theWire.endTransmission();
}    


void AT42QT2120::prepReadKeys(void)
{
  theWire.beginTransmission(touch_addr);
  theWire.write(0);
  theWire.endTransmission();

  const int reqNum = 6;
  theWire.requestFrom(touch_addr,reqNum,1);
  theWire.endTransmission();
}


//uint8_t status[6];
void AT42QT2120::readKeys(void)
{
  uint8_t newKeys[2];

  theWire.beginTransmission(touch_addr);
  theWire.write(3);
  theWire.endTransmission();

  const int reqNum = 2;
  theWire.requestFrom(touch_addr,reqNum,1);
  newKeys[0] = theWire.read();
  newKeys[1] = theWire.read();
  theWire.endTransmission();

  oldK = (status[4]  << 8) | status[3];
  newK = (newKeys[1] << 8) | newKeys[0];
  chg = (newK ^ oldK) & mask; // ignore unimplemented bits

  status[3] = newKeys[0];
  status[4] = newKeys[1];
}

void AT42QT2120::calibrate(void)
{
    uint8_t calibrate[]{6,1};
    theWire.beginTransmission(touch_addr);
    theWire.write(calibrate,2);
    theWire.endTransmission();
}


void AT42QT2120::setTouchRecalDelay(uint8_t theDelay)
{
    uint8_t cmd[]{12,theDelay};
    theWire.beginTransmission(touch_addr);
    theWire.write(cmd,2);
    theWire.endTransmission();
}


//===========================================================
//
//    888                              888      
//    888                              888      
//    888                              888      
//    888888 .d88b.  888  888  .d8888b 88888b.  
//    888   d88""88b 888  888 d88P"    888 "88b 
//    888   888  888 888  888 888      888  888 
//    Y88b. Y88..88P Y88b 888 Y88b.    888  888 
//     "Y888 "Y88P"   "Y88888  "Y8888P 888  888 
//
static void isrTouch(void)
{
  //xTaskResumeFromISR(handleTouch);
  BaseType_t xHigherPriorityTaskWoken = pdFALSE; 

  touchTask.checkChange = true;
  /*
  vTaskNotifyGiveFromISR(touchTask.handle, &xHigherPriorityTaskWoken);
  /*/
  xTaskNotifyFromISR(touchTask.handle,    // notify touch task ...
                     TouchTask::touchFlag, eSetBits, // ...setting the touch flag
                     &xHigherPriorityTaskWoken);
  //*/                     
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


void TouchTask::updateKeyStatuses(AT42QT2120& touch)
{
  int keyNum;
  bool state;

  while (touch.getChangedKey(keyNum, state))
  {
    switch (keyNum)
    {
//#define CASE(b,n) case b: Serial.printf("Key %d %s; ", n, (lsK & newK)?"touched":"released"); break
//#define CASE(b,n) case b: keyStatuses[n-1] = (lsK & newK)?1:0; break
#define CASE(b,n) case b: keyStatuses[n-1] = state; break
      CASE( 8,1);
      CASE( 9,2);
      CASE(10,3);
      CASE(11,4);
      CASE( 4,5);
      CASE( 5,6);
      CASE( 6,7);
      CASE( 7,8);
      default:
        break;
    }
  }
}


InterTaskRequest::Result TouchTask::doCalibrateTouch(void* context)
{
  Serial.println("Calibrate touch");

  touchChip.calibrate();

  return InterTaskRequest::Result::done;
}

void TouchTask::updateTouch() 
{
  if (0)
  {
    static int dots = 0;
    Serial.print('.');
    dots++;
    if (dots > 30)
    {
      Serial.println();
      dots = 0;
    }
  }

  if (checkChange)
  {
    touchChip.prepReadKeys();
    touchChip.readKeys();
    updateKeyStatuses(touchTask.touchChip);
    checkChange = false;
  }
}


void TouchTask::pollTouch(void)
{
  for (int i=0;i<COUNT_OF(keyStatuses);i++)
  {
    if (keyStatuses[i].isChangedStatus())
    {
      //int status = (int) keyStatuses[i].getExtendedStatus();
      //Serial.printf("Touch %d: status %d\n", i+1, status);
      StripTask::getStripTask(i).touchChanged();
    }
  }
}


void TouchTask::initTouch(void)
{
  // access touch chip via I²C
  while (1)
  {
    if (touchChip.probe())
      break;
    Serial.println("Waiting for 6V supply...");
    vTaskDelay(500);
  }

  supplyValid = true;

  touchChip.setTouchRecalDelay(0); // set TRD register so continuous touch doesn't time out

  // set up change interrupt
  pinMode(CHANGE_PIN,arduino::INPUT_PULLUP);
  attachInterrupt(CHANGE_PIN, isrTouch, arduino::FALLING);

  // initial update() seems to be necessary
  updateTouch();
}

//=========================================================================
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
void TouchTask::run(void)
{
  uint32_t whichISR = 0;
  vTaskDelay(5);
  //*
  initTouch();        // GT911 breaks pots touch :-(
  /*/
  supplyValid = true; // fake it for now
  //*/
  startGT911(xTaskGetCurrentTaskHandle());

  while (1)
  {
    reqQueue.executeRequest(*this, 0); // execute any pending requests (calibration)

    if (pdTRUE == xTaskNotifyWait(0UL, UINT32_MAX, &whichISR, 10)) // wait for notification from touch ISR
    {
      // Pot touch chip triggered?
      if (0 != (whichISR & touchFlag))
        updateTouch(); // only does I²C if ISR fired

      // Main LCD touch screen triggered?
      if (0 != (whichISR & GT911Flag))
      {
        if (0 == updateGT911()) // no touches right now
        {
          // ...deal with being untouched
        }
        else 
        {
          processGT911(0); // just grab first touch point for now
          Serial.printf("[%d]: %d, %d\n", lastTouchTime, lastTouch.x, lastTouch.y);
          //xTaskNotifyGive(handleMainLCD); // wake up Main LCD task to deal with touch
        }
      }
    }

    pollTouch(); // generate state outputs, e.g. time long presses
  }
}


TouchTask touchTask{"Touch", 512, nullptr, 7, 1, potsTouch};
