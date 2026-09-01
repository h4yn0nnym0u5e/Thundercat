#if 1
#include "header.h"

// I2C pins/frequency (adjust for your board)
#define I2C_FREQ  400'000

// GT911 I2C address (choose one based on your wiring)
#define CTP_TOUCH_ADDR  GT911_I2C_ADDR_BA  // or GT911_I2C_ADDR_28

// GT911 pins
#define INT_PIN   CTP_INT  // must be interrupt-capable
#define RST_PIN    -1//9   // set to -1 if not connected

// Display resolution (match your panel)
// No point, driver is incredibly buggy - do our own scaling
#define TFT_HOR_RES  960
#define TFT_VER_RES  960

static initGT911 Touchscreen(&TFT_CTP_I2C, CTP_TOUCH_ADDR);

struct touchWireContext_s
{
  int stuff;
  TaskHandle_t handle;
} touchWireContext{1};


// un-block the task when the touch interrupt fires
static void touchISR(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE; 

  Touchscreen.setIRQflag(true);
  /*
  vTaskNotifyGiveFromISR(touchWireContext.handle, &xHigherPriorityTaskWoken);
  /*/
  xTaskNotifyFromISR(touchWireContext.handle,    // notify touch task ...
                     TouchTask::GT911Flag, eSetBits, // ...setting the touch flag
                     &xHigherPriorityTaskWoken);
  //*/                     
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

// Wait for async I2C transaction to complete.
// When it does, touchWireCallback() will be called
static void touchAsyncWait(void* pctxt)
{
  //touchWireContext_s& context = *((touchWireContext_s*) pctxt);

  // wait for notification from async I2C library
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

// Called from an interrupt in the async I2C 
// library when a transaction is complete
void touchWireCallback(void* pctxt)
{
  touchWireContext_s& context = *((touchWireContext_s*) pctxt);

  context.stuff++;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE; 

  vTaskNotifyGiveFromISR(context.handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

// Replacement for delay() when called from inside GT911 library
void touchDelay(uint32_t ms)
{
  vTaskDelay(pdMS_TO_TICKS(ms));
}

/*
 * Adapted reset function to deal with the fact 
 * that the reset line is on a port expander
 */
static void GT911reset(uint8_t _intPin = CTP_INT, uint8_t _addr = CTP_TOUCH_ADDR)
{
  SET_BIT_NOW(LCD_RESET, arduino::HIGH);
  pinMode(_intPin, arduino::OUTPUT);
  digitalWrite(_intPin, arduino::LOW);
  delay(1);

  SET_BIT_NOW(LCD_RESET, arduino::LOW);
  delay(1);

  digitalWrite(_intPin, _addr == GT911_I2C_ADDR_28);
  delayMicroseconds(110);

  SET_BIT_NOW(LCD_RESET, arduino::HIGH);

  delay(6);
  pinMode(_intPin, arduino::INPUT_PULLUP);
  delay(51);
}

bool touchReady = false;
void TouchTask::startGT911(TaskHandle_t owner) 
{
  touchWireContext.handle = owner;

  // Init I2C
#if defined(IMX_RT1060_I2C_DRIVER_H)
  TFT_CTP_I2C.begin(I2C_FREQ);
#else  
  TFT_CTP_I2C.begin();
#endif // defined(IMX_RT1060_I2C_DRIVER_H)

  // Init GT911 (interrupts are handled INSIDE the library)
  GT911reset(); // special reset, chooses the I2C address
  for (int i=0;i<10;i++)
  {
    if (Touchscreen.begin(INT_PIN, RST_PIN, I2C_FREQ)) 
    {
      Serial.println("GT911 initialized (interrupt mode).");
      Touchscreen.setupDisplay(TFT_HOR_RES, TFT_VER_RES, initGT911_ROTATION_0);
      Touchscreen.getWire().set_callback(touchWireCallback);
      Touchscreen.getWire().set_context(&touchWireContext);
      Touchscreen.setInterruptHandler(touchISR);
      Touchscreen.setAsyncWait(touchAsyncWait);
      //Touchscreen.setContext(&touchWireContext);
      Touchscreen.setDelayFn(touchDelay);
      touchReady = true; // reset has occurred, GT911 is OK
      break;
    } else {
      Serial.printf("%d ... ", i /* Touchscreen.beginError */);
    }
  }
}


/*
 * Check for touches, and if present read the values
 * If none, then this returns very quickly, otherwise the
 * library puts in a bunch of (unneccessary?) delays.
 */
uint8_t TouchTask::updateGT911(void) 
{
  // Library checks the internal IRQ flag; no external ISR/attachInterrupt needed
  uint8_t count = Touchscreen.touched(GT911_MODE_INTERRUPT);
  return count;
}

// GTPoint lastTouch;
// uint32_t lastTouchTime;
int buttonTouch = -1;
int buttonReleased = -1;

void TouchTask::processGT911(int n)
{
  GTPoint p = Touchscreen.getPoint(n);

  // hacky compensation for library bugs:
  int x = p.y/3, y = 240-p.x/4;
  p.x = x; p.y = y;
  p.reserved = 1; // say it's valid
  //Serial.printf("Touch: X=%u, Y=%u, stuff: %d; \n", p.x, p.y, touchWireContext.stuff);

  lastTouch = p;
  lastTouchTime = millis();
}




#if 0
TaskHandle_t handleGT911touch;
static void taskGT911touch(void* params)
{
  startGT911touch();

  while (1)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // wait for notification from touch screen ISR
    if (0 == updateGT911touch())
    {
      /*
      if ((millis() - lastTouchTime) > 15)
      {
        if (1 == lastTouch.reserved)
        {
          buttonReleased = whichButton(lastTouch);
          lastTouch.reserved = 0; // dealt with
        }
        buttonTouch = -1;
      }
      */
      vTaskDelay(2);
    }
    else 
    {
      processGT911(0);
      xTaskNotifyGive(handleMainLCD); // wake up Main LCD task to deal with touch
      vTaskDelay(2);
      //buttonTouch = whichButton(lastTouch);
    }
  }
}
#endif // 0


#endif // 0