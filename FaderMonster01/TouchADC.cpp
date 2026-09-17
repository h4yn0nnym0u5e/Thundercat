
#include "header.h"

// AT42QT2120 touch sensor on fader caps:
static AT42QT2120_Wire fadersWire{FADERS_TOUCH_I2C, FADERS_TOUCH_ADDR};
static touchChipDriverWire fadersTouch{fadersWire, 0xFF0};
TouchStatus TouchADCtask::keyStatuses[NUM_POTS];

static void isrTouch(void);

static ADC* adc = new ADC();
const uint32_t buffer_size = 64;
DMAMEM static volatile uint16_t __attribute__((aligned(32))) dma_adc_buff1[buffer_size];
AnalogBufferDMA abdma1(dma_adc_buff1, buffer_size);

static int _readADCsum(AnalogBufferDMA& abdma)
{
    int result = INT32_MIN;

    if (abdma.interrupted())
    {
        volatile uint16_t *pbuffer = abdma.bufferLastISRFilled();
        int samples = abdma.bufferCountLastISRFilled(); // expect 64 here!

        // THIS IS IMPORTANT!
        if ((uint32_t)pbuffer >= 0x20200000u)  
            arm_dcache_delete((void*)pbuffer, samples * sizeof *pbuffer);

        int sum = 0;
        for (int i=0;i<samples;i++)
            sum += pbuffer[i];
        
        result = sum;

        // run again
        abdma.clearInterrupt();
        abdma.clearCompletion(); 
    }

    return result;
}   

static int readADCsum(void)
{
    return _readADCsum(abdma1);
}



/**
 * ExpressionPedal instance to inject into touchTask
 */
static ExpressionPedal exprPedal{
    EXPR_PED_I2C, EXPR_PED_MCP4018_RES,
    //[](void){ return analogRead(EXPR_PED_ADC)*256;}, 4095*256,
    [](void){ return readADCsum(); }, 4095*buffer_size,
    [](bool trctl) { SET_BIT(PEDAL_TRCTRL, trctl); },
    [](bool pull_up_rs_in) { SET_PULLUP(PEDAL_RSIN, pull_up_rs_in); },
    [](void){ return GET_BIT(PEDAL_RSIN);},
    [](void){ return GET_BIT(PEDAL_DET);},
    abdma1
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

//=========================================================================
//                           888          888 
//                           888          888 
//                           888          888 
//    88888b.   .d88b.   .d88888  8888b.  888 
//    888 "88b d8P  Y8b d88" 888     "88b 888 
//    888  888 88888888 888  888 .d888888 888 
//    888 d88P Y8b.     Y88b 888 888  888 888 
//    88888P"   "Y8888   "Y88888 "Y888888 888 
//    888                                     
//    888                                     
//    888                                     
//
/**
 * Get pedal reading, scaled to ±1.0f
 */
float ExpressionPedal::setValue(void)
{
    raw = (*ADCread)();
    float newValue = (float) raw/ADCmax2 - 1.0f;

    rar.update(raw);
    lastResponsiveValue = (float) rar.getValue()/ADCmax2 - 1.0f;

    if (fabs(newValue - lastValue) > 0.01f) // big jump, act quickly
        lastValue = newValue;
    else 
    {        
        lastValue = lastValue * (1.0f - smooth) + newValue * smooth;
    }
    return lastValue;
}

float ExpressionPedal::getStableValue(int n, int d)
{
    float newVal, oldVal = UNSTABLE;
    bool ok = false;
    while (n--)
    {
        newVal = setValue();
        if (raw < 0) // no new reading
        {
            vTaskDelay(1);
            continue;
        }

        newVal = raw / ADCmax2 - 1.0f; // use raw value for this, scaled one my be sluggish
        if (fabs(newVal - oldVal) <= THRESHOLD)
        {
            ok = true;
            break;
        }
        oldVal = newVal;
        vTaskDelay(d);            
    }
Serial.printf("Value is %.3f at gain of %d; %d tries left\n", newVal, getGain(), n);
    return ok?newVal:UNSTABLE;
}

/**
 * Find out if one of the fottswitches is pressed.
 * The numbers are 1 and 2 - using musician-speak here!
 * \return true if pedal is correct type and switch is pressed
 */
bool ExpressionPedal::isPressed(int footSwitch)
{
    bool result = false;

    switch (footSwitch)
    {
        default:
            break;

        case 1:
            if (eType::dualSwitch   == type
             || eType::singleSwitch == type)
                result = getValue() < 0.0f;
            break;

        case 2:
            if (eType::dualSwitch == type)
                result = !getRS_IN();
            break;                
    }

    return result;
}

/**
 * Seek a gain which gets the pedal value within a range
 */
int ExpressionPedal::gainSeek(float lower, float upper)
{
    float newVal;
    int gain = 63, gainStep = 64, result = -1;

    do 
    {    
        setGain(gain);
        newVal = getStableValue(20,10);
        if (UNSTABLE == newVal)
        {
            gain = -1;
            break;
        }
        if (newVal > lower && newVal < upper)
        {
            result = gain;
            break;
        }
        gainStep /= 2;
        gain += newVal > (upper + lower) / 2
                        ?gainStep
                        :-gainStep;
    } while (gainStep > 1);

    return result;
}

/**
 * Auto-detect pedal type.
 * This should be called, after a suitable delay, when a pedal has been plugged in.
 */
ExpressionPedal::eType ExpressionPedal::autoDetect(void)
{
    eType result = eType::none;
    float oldSmooth = smooth;
    smooth = 0.75f; // don't really smooth

    do 
    {
        if (!isPresent())
            break;

        setExprMode(true); // probe using Expression mode

        // try to get it well inside the analogue range
        if (gainSeek(EXPR_MIN, EXPR_MAX) >= 0)
            result = eType::expression;
        setGain(63); // back to mid-range for now

        // if in range, it's an expression pedal
        if (eType::none != result)
            break;

        // not expression, must be a switch
        setExprMode(false);
        vTaskDelay(20);

        if (isPressed(2)) // does switch 2 appear to be pressed?
            result = eType::singleSwitch; // shorted - it's a single switch
        else
            result = eType::dualSwitch;            

    } while (0);

    type = result; // save for later
    smooth = oldSmooth;

    return result;
}

/**
 * Seek a gain level that achieves the target reading.
 * Should be called when an expression pedal is plugged in and
 * set to maximum.
 * 
 * The MCP6001 can reach within about 25mV of the rail, which is
 * 4096 * 0.025 / 3.3 = 31 counts, or ±0.984 on our standard ±1.0
 * analogue range. Thus a target of 0.980 or so is probably sensible!
 */
int ExpressionPedal::autoCalibrate(float target, float range)
{
    int result = gainSeek(target - range, target + range);
    if (result < 0) setGain(63); // failed - set safe gain

    return result;
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
    elapsedMillis em, detectEm;

    /*
    // set up analogue to suit our purposes
    analogReadRes(12);        // 12-bit, 0..4095
    analogReadAveraging(16);   // do some inbuilt averaging

    // do some dummy reads
    analogRead(EXPR_PED_ADC);
    analogRead(LT_SENS_ADC);
    //*/

    adc = new ADC();
    adc->adc0->setAveraging(8); // set number of averages
    adc->adc0->setResolution(12); // set bits of resolution
    abdma1.init(adc, ADC_0);

    // Start the dma operation..
    adc->adc0->startSingleRead(EXPR_PED_ADC); // call this to setup everything before the Timer starts, differential is also possible
    adc->adc0->startTimer(buffer_size*1000);  // frequency in Hz: try to get a buffer every 1ms

    //*/
    // start the expression pedal
    expressionPedal.begin();
    //*
    expressionPedal.setExprMode(true); // set to Expression mode (rather than Switch)
    /*/
    expressionPedal.setExprMode(false); // set to Switch mode (rather than Expression)
    //*/
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
            if (pedalPresent)
                detectEm = 0;
            else 
                Serial.println("Pedal unplugged");                
        }

        if (detectEm > 1000 && detectEm < 2000)
        {
            const char* pedalType = "No";

            detectEm = 2000;
            expressionPedal.autoDetect();
            switch (expressionPedal.type)
            {
                default:
                    pedalType = "None (?)";
                    break;
                
                case ExpressionPedal::eType::expression:
                    pedalType = "Expression";
                    break;
                
                case ExpressionPedal::eType::singleSwitch:
                    pedalType = "Single switch";
                    break;
                
                case ExpressionPedal::eType::dualSwitch:
                    pedalType = "Dual switch";
                    break;
            }
            Serial.printf("%s pedal is present\n", pedalType);
        }

        if (expressionPedal && abdma1.interrupted())
        {
            static float raw;
            static int updateCount = 0;
            raw = expressionPedal.setValue();
            updateCount++;

            if (em >= 250)    
            {
                em = 0;
                if (enablePedalPrint > 0)
                {
                    Serial.printf("%d, Expr: %.3f; gain %d\n", updateCount, raw, expressionPedal.getGain());
                    enablePedalPrint--;
                    if (0 == enablePedalPrint)
                        Serial.println("Pedal print stopped");
                }
            }
        }

        // Hacky way of setting expression pedal gain
        if (3 == smartKnobTask.whichConfig)
        {
            static int lastSKpos = 127;
            if (smartKnobTask.last_position != lastSKpos)
            {
                lastSKpos = smartKnobTask.last_position;
                expressionPedal.setGain(lastSKpos / 2);
                // Serial.printf("Set expr pot to 0x%02X\n", lastSKpos / 2);
            }
        }

        if (countBits)
        {
            static int count = 50;
            if (--count <0 )
            {
                count = 50;
                bits ^= 2;
                //*
                Serial.printf("%d, %.4f, %.4f, %.4f\n", 
                    ADCcount++,
                /*/                    
                Serial.printf("raw:%.5f, smoothed:%.5f, responsive:%.5f\n", 
                //*/
                    expressionPedal.getRaw()/2047.0f/64.0f - 1.0f, 
                    expressionPedal.getValue(),
                    expressionPedal.getResponsiveValue()
                );
            }
        }
    }
}


TouchADCtask touchADCtask{"TouchADC", 512, nullptr, 6, 1, fadersTouch, exprPedal};
