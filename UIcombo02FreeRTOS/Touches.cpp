/*
 * Quick test of the AT42QT2120 touch sensor
 */
#include "headers.h"
#include <Wire.h>
#include "config.h"

TwoWire& theWire{TOUCH_WIRE};
static TaskHandle_t handleTouch;

static void isrTouch(void);


uint8_t keyStatuses[NUM_POTS];

uint8_t status[6];
void readKeys(void)
{
  uint8_t newKeys[2];

  theWire.beginTransmission(TOUCH_ADDR);
  theWire.write(3);
  theWire.endTransmission();

  const int reqNum = 2;
  theWire.requestFrom(TOUCH_ADDR,reqNum,1);
  newKeys[0] = theWire.read();
  newKeys[1] = theWire.read();
  theWire.endTransmission();

  uint16_t oldK = (status[3] << 8) | status[4];
  uint16_t newK = (newKeys[0] << 8) | newKeys[1];
  uint16_t chg = (newK ^ oldK) & 0xF00F; // ignore unused keys

  while (chg)
  {
    uint16_t lsK = ((~chg) + 1) & chg;
    switch (lsK)
    {
//#define CASE(b,n) case b: Serial.printf("Key %d %s; ", n, (lsK & newK)?"touched":"released"); break
#define CASE(b,n) case b: keyStatuses[n-1] = (lsK & newK)?1:0; break
      CASE(0x0001,1);
      CASE(0x0002,2);
      CASE(0x0004,3);
      CASE(0x0008,4);
      CASE(0x1000,5);
      CASE(0x2000,6);
      CASE(0x4000,7);
      CASE(0x8000,8);
      default:
        //Serial.printf(" *** %04X *** ", lsK);
        chg = 0;
        break;
    }
    chg &= ~lsK;
  }
  status[3] = newKeys[0];
  status[4] = newKeys[1];
}


void calibrateTouch(void)
{
    uint8_t calibrate[]{6,1};
    Serial.println("Calibrate touch");
    theWire.beginTransmission(TOUCH_ADDR);
    theWire.write(calibrate,2);
    theWire.endTransmission();
}


bool checkChange = true;
static void isrTouch(void)
{
  checkChange = true;
  xTaskResumeFromISR(handleTouch);
}

void updateTouch() 
{
  if (checkChange)
  {
    Serial.println("touch");
    //*
    theWire.beginTransmission(TOUCH_ADDR);
    theWire.write(0);
    theWire.endTransmission();

    const int reqNum = 6;
    theWire.requestFrom(TOUCH_ADDR,reqNum,1);

    theWire.endTransmission();
    //*/
    readKeys();
    checkChange = false;
  }
}

static void taskTouch(void*)
{
  while (1)
  {
    updateTouch();
    vTaskSuspend(nullptr);
  }
}

uint8_t initTouch() 
{
  // access touch chip via I²C
  theWire.begin();
  theWire.beginTransmission(TOUCH_ADDR);
  uint8_t result = theWire.endTransmission();

  // change interrupt
  pinMode(CHANGE_PIN,arduino::INPUT_PULLUP);
  attachInterrupt(CHANGE_PIN, isrTouch, arduino::FALLING);

  xTaskCreate(taskTouch, "Touch", 256, nullptr, 2, &handleTouch);

  return result;
}

