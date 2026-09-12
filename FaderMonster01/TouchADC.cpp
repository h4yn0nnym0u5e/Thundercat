
#include "header.h"

// AT42QT2120 touch sensor on fader caps:
static AT42QT2120_Wire fadersWire{FADERS_TOUCH_I2C, FADERS_TOUCH_ADDR};
static touchChipDriverWire fadersTouch{fadersWire, 0xFF0};
TouchStatus TouchADCtask::keyStatuses[NUM_POTS];

static void isrTouch(void);

static ExpressionPedal expressionPedal{
    EXPR_PED_I2C, EXPR_PED_MCP4018_RES,
    [](void){ return analogRead(EXPR_PED_ADC);}, 4095,
    [](bool trctl) { SET_BIT(PEDAL_TRCTRL, trctl); },
    [](bool pull_up_rs_in) { SET_PULLUP(PEDAL_RSIN, pull_up_rs_in); },
    [](void){ return GET_BIT(PEDAL_RSIN);},
    [](void){ return GET_BIT(PEDAL_DET);},
};

//**************************************************************************
//**************************************************************************
//**************************************************************************
//
//    888 888       .d888 d8b          888b     d888               888 888 
//    888 888      d88P"  Y8P          8888b   d8888               888 888 
//    888 888      888                 88888b.d88888               888 888 
//    888 888      888888 888 888  888 888Y88888P888  .d88b.       888 888 
//    888 888      888    888 `Y8bd8P' 888 Y888P 888 d8P  Y8b      888 888 
//    Y8P Y8P      888    888   X88K   888  Y8P  888 88888888      Y8P Y8P 
//     "   "       888    888 .d8""8b. 888   "   888 Y8b.           "   "  
//    888 888      888    888 888  888 888       888  "Y8888       888 888 
// 
// This is copypasta from touch.cpp, and should be made into a proper class
// covering both sync and async AT42Q2120 drivers.
//**************************************************************************
//**************************************************************************
//**************************************************************************

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
  BaseType_t xHigherPriorityTaskWoken = pdFALSE; 

  touchADCtask.checkChange = true;
  xTaskNotifyFromISR(touchADCtask.handle,    // notify touch task ...
                     TouchADCtask::touchFlag, eSetBits, // ...setting the touch flag
                     &xHigherPriorityTaskWoken);
  
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


void TouchADCtask::updateKeyStatuses(touchChipDriverWire& touch)
{
  int keyNum;
  bool state;

  while (touch.getChangedKey(keyNum, state))
  {
    switch (keyNum)
    {
#define CASE(b,n) case b: keyStatuses[n-1] = state; break
// NOTE: different mapping to pots touch chip!
      CASE( 4,1);
      CASE( 5,2);
      CASE( 6,3);
      CASE( 7,4);
      CASE( 8,5);
      CASE( 9,6);
      CASE(10,7);
      CASE(11,8);
      default:
        break;
    }
  }
}


InterTaskRequest::Result TouchADCtask::doCalibrateTouch(void* context)
{
  Serial.println("Calibrate touch");

  touchChip.calibrate();

  return InterTaskRequest::Result::done;
}

void TouchADCtask::updateTouch() 
{
  if (checkChange)
  {
    touchChip.prepReadKeys();
    touchChip.readKeys();
    updateKeyStatuses(touchADCtask.touchChip);
    checkChange = false;
  }
}


void TouchADCtask::pollTouch(void)
{
  for (int i=0;i<COUNT_OF(keyStatuses);i++)
  {
    if (keyStatuses[i].isChangedStatus())
    {
      //int status = (int) keyStatuses[i].getExtendedStatus();
      //Serial.printf("Touch %d: status %d\n", i+1, status);
      StripTask::getStripTask(i).touchChanged(keyStatuses[i]);
    }
  }
}


void TouchADCtask::initTouch(void)
{
  // access touch chip via I²C
  while (1)
  {
    if (touchChip.probe(400'000))
      break;
    Serial.println("Waiting for 6V supply...");
    vTaskDelay(500);
  }

  touchChip.setTouchRecalDelay(0); // set TRD register so continuous touch doesn't time out

  // set up change interrupt
  pinMode(FADERS_TOUCH_INT,arduino::INPUT_PULLUP);
  attachInterrupt(FADERS_TOUCH_INT, isrTouch, arduino::FALLING);

  // initial update() seems to be necessary
  updateTouch();
}

/**
 * Get pedal reading, scaled to ±1.0f
 */
float ExpressionPedal::getValue(void)
{
    float raw = (*ADCread)();
    return raw/ADCmax2 - 1.0f;
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
void TouchADCtask::run(void)
{
  uint32_t whichISR = 0;
  vTaskDelay(1500);
  initTouch();
  bool pedalPresent{false};
  elapsedMillis em;

  // set up analogue to suit our purposes
  analogReadRes(12);        // 12-bit, 0..4095
  analogReadAveraging(4);   // do some inbuilt averaging

  // do some dummy reads
  analogRead(EXPR_PED_ADC);
  analogRead(LT_SENS_ADC);

  // start the expression pedal
  expressionPedal.begin();
  expressionPedal.setExprMode(true); // set to Expression mode (rather than Switch)

  while (1)
  {
    reqQueue.executeRequest(*this, 0); // execute any pending requests (calibration)

    if (pdTRUE == xTaskNotifyWait(0UL, UINT32_MAX, &whichISR, 2)) // wait for notification from touch ISR
    {
      // Fader touch chip triggered?
      if (0 != (whichISR & touchFlag))
        updateTouch(); // only does I²C if ISR fired
    }

    pollTouch(); // generate state outputs, e.g. time long presses

    if (expressionPedal.isPresent() != pedalPresent)
    {
        pedalPresent = !pedalPresent;
        Serial.printf("Pedal is %spresent\n", pedalPresent?"":"not ");
    }

    if (expressionPedal)
    {
        static float raw;
        raw = expressionPedal.getValue();

        if (em >= 250)    
        {
            em = 0;
            if (enablePedalPrint)
                Serial.printf("Expr: %.3f\n", raw);
        }
    }

    if (3 == smartKnobTask.whichConfig)
    {
        static int lastSKpos = 127;
        if (smartKnobTask.last_position != lastSKpos)
        {
            lastSKpos = smartKnobTask.last_position;
            expressionPedal.setGain(lastSKpos / 2);
            Serial.printf("Set expr pot to 0x%02X\n", lastSKpos / 2);
        }
    }
  }
}


TouchADCtask touchADCtask{"TouchADC", 512, nullptr, 7, 1, fadersTouch};
