#include <Wire.h>
#include "initGT911.h"

// I2C pins/frequency (adjust for your board)
#define I2C_FREQ  400'000


// GT911 I2C address (choose one based on your wiring)
#define TOUCH_ADDR  GT911_I2C_ADDR_BA  // or GT911_I2C_ADDR_28

// GT911 pins
#define INT_PIN   15   // must be interrupt-capable
#define RST_PIN    -1//9   // set to -1 if not connected

// Display resolution (match your panel)
// No point, driver is incredibly buggy - do our own scaling
#define TFT_HOR_RES  960
#define TFT_VER_RES  960

initGT911 Touchscreen(&Wire1, TOUCH_ADDR);

void startGT911touch() {
  // Init I2C
  Wire1.begin();

  // Init GT911 (interrupts are handled INSIDE the library)
  for (int i=0;i<10;i++)
  {
    if (Touchscreen.begin(INT_PIN, RST_PIN, I2C_FREQ)) {
      Serial.println("GT911 initialized (interrupt mode).");
      Touchscreen.setupDisplay(TFT_HOR_RES, TFT_VER_RES, initGT911_ROTATION_0);
      break;
    } else {
      Serial.printf("%d ... ", Touchscreen.beginError);
    }
  }
}


/*
 * Check for touches, and if present read the values
 * If none, then this returns very quickly, otherwise the
 * library puts in a bunch of (unneccessary?) delays.
 */
uint8_t updateGT911touch() {
  // Library checks the internal IRQ flag; no external ISR/attachInterrupt needed
  uint8_t count = Touchscreen.touched(GT911_MODE_INTERRUPT);
  return count;
}

GTPoint lastTouch;
uint32_t lastTouchTime;
int buttonTouch = -1;
int buttonReleased = -1;
void processTouch(int n)
{
  GTPoint p = Touchscreen.getPoint(n);

  // hacky compensation for library bugs:
  int x = p.y/3, y = 240-p.x/4;
  p.x = x; p.y = y;
  p.reserved = 1; // say it's valid
  //Serial.printf("Touch: X=%u, Y=%u; ", p.x, p.y);

  lastTouch = p;
  lastTouchTime = millis();
}


int whichButton(GTPoint p)
{
  int result = -1;
  if (p.y < 50)
  {
    result = ((int) p.x) * 5 / 320;
  }
  //Serial.printf("%d: button %d\n", millis(), result);
  return result;
}

/*
TaskHandle_t handleGT911touch;
static void taskGT911touch(void* params)
{
  startGT911touch();

  while (1)
  {
    if (0 == updateGT911touch())
    {
      if ((millis() - lastTouchTime) > 15)
      {
        if (1 == lastTouch.reserved)
        {
          buttonReleased = whichButton(lastTouch);
          lastTouch.reserved = 0; // dealt with
        }
        buttonTouch = -1;
      }
      vTaskDelay(2);
    }
    else 
    {
      processTouch(0);
      buttonTouch = whichButton(lastTouch);
    }
  }
}


void initGT911touch(void)
{
  xTaskCreate(taskGT911touch, "GT911touch", 256, nullptr, 2, &handleGT911touch);
}
*/ 