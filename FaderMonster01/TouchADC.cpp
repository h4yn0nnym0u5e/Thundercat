
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
      superTask.touchChanged(keyStatuses[i]);
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
 * Get pedal reading, in range ±1.0f but NOT scaled
 * Updates raw, lastValue and lastResponsiveValue
 */
float ExpressionPedal::setValue(void)
{
    // get a new value, and scale it
    raw = (*ADCread)();
    float newValue = rawToFloat(raw); // float version

    // feed the ResponsiveAnalogRead class, and use it to get a scaled value
    ResponsiveAnalogRead::update(raw);
    lastResponsiveValue = rawToFloat(ResponsiveAnalogRead::getValue());

    // also do our own smoothing, in case we don't like ResponsiveAnalogRead
    if (fabs(newValue - lastValue) > 0.01f) // big jump, act quickly
        lastValue = newValue;
    else 
    {        
        lastValue = lastValue * (1.0f - smooth) + newValue * smooth;
    }
    return lastValue;
}


/**
 * Get a stable pedal value, e.g. after gain has changed
 */
float ExpressionPedal::getStableValue(int n, int d)
{
    elapsedMillis em=0;
    float newVal, oldVal = UNSTABLE;
    bool ok = false;
    int okCount = 0;

    while (n--)
    {
        newVal = setValue();
        if (raw < 0) // no new reading
        {
            vTaskDelay(1);
            continue;
        }

        newVal = rawToFloat(raw); // use raw value for this, smoothed one my be sluggish
Serial.printf("%d: %d / %.3f ", (int) em, raw, newVal);        
        if (fabs(newVal - oldVal) <= THRESHOLD)
        {
            okCount++;
            if (okCount > 5)
            {
                ok = true;
                break;
            }
        }
        else 
            okCount = 0;
        oldVal = newVal;
        vTaskDelay(d);            
    }
Serial.printf("[%u] Value is %.3f at gain of %d; %d tries left\n", micros(), newVal, getGain(), n);
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
        newVal = getStableValue(30,10);
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
        gain += (newVal > (upper + lower) / 2)
                        ?gainStep
                        :-gainStep;
    } while (gainStep > 1);

    return result;
}
/*
From https://scotttroyer.com/2019/06/alesis-adat-hd24-remote-control-pin-out/
-----------------------------------------------------------------------------
                    Measured
Switch 	Button 	    Value (R) 	PCB Label
N/A 	N/A 	    1406 	    (value read with no buttons pressed)
S1 	    Auto Loop 	126 	    Auto Input
S2 	    Locate 0 	279 	    Locate 1
S3 	    Loop Start 	228 	    Locate 2
S4 	    Loop End 	177 	    Auto 2>1
S5 	    Rehearse 	76 	        All Input
S6 	    Set Locate 	432 	    Set Locate
S7 	    Punch In 	330 	    Locate 0
S8 	    Punch Out 	381 	    Auto Play
S9 	    Rewind 	    1107 	    Rewind
S10 	F Fwd 	    1207 	    F Fwd
S11 	Stop 	    1008 	    Stop
S12 	Play 	    899 	    Play
S13 	Record 	    1307 	    Rec
*/
/**
 * Auto-detect pedal type.
 * This should be called, after a suitable delay, when a pedal has been plugged in.
 */
ExpressionPedal::eType ExpressionPedal::autoDetect(void)
{
    eType result = eType::none;
    int   oldGain = gain;
    float oldSmooth = smooth;
    smooth = 0.75f; // don't really smooth

    do 
    {
        if (!isPresent())
            break;
        // quick hack...
        setExprMode(false); // probe using switch mode
        setGain(ALESIS_GAIN);
        float newVal = getStableValue(30,10);
        if (newVal > 0.8f && newVal < +0.9f)
        {
            result = eType::alesisLRC;
            calcLRCpullup();
            break;
        }

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
    setGain(oldGain);
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
void TouchADCtask::pollPedal(void)
{
    // detect change of pedal
    if (expressionPedal.isPresent() != pedalPresent)
    {
        pedalPresent = !pedalPresent;
        if (pedalPresent)
            detectEm = 0;
        else 
            Serial.println("Pedal unplugged");                
    }

    // detect newly plugged-in pedal
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
            
            case ExpressionPedal::eType::alesisLRC:
                pedalType = "Alesis LRC";
                expressionPedal.setAlesisLRCgain();
                expressionPedal.setExprMode(false);
                Serial.printf("Rpullup = %.3fk\n", expressionPedal.getLRCpullup() / 1000.0f);
                break;
        }
        Serial.printf("[%u] %s pedal is present\n", micros(), pedalType);
    }

    // get new value, if any, and process it
    if (expressionPedal && abdma1.interrupted())
    {
        static float raw;
        static int updateCount = 0;
        raw = expressionPedal.setValue();
        updateCount++;
        midiTask.sendRaw(expressionPedal.getScaled(), pedalMIDIreq, -1, CTL_PEDAL);

        // debug: print pedal value occasionally
        if (em >= 250)    
        {
            em = 0;
            if (enablePedalPrint > 0)
            {
                Serial.printf("[%u] %d, Expr: %.3f; gain %d\n", micros(), updateCount, raw, expressionPedal.getGain());
                enablePedalPrint--;
                if (0 == enablePedalPrint)
                    Serial.println("Pedal print stopped");
            }
        }

        // debug: process LRC button change
        if (ExpressionPedal::eType::alesisLRC == expressionPedal.type)
        {
            float newRes = expressionPedal.getLRCresistance();

            if (expressionPedal.isNewLRCbutton())
            {
                const LRC::sButton& button = LRC::getButton(newRes);
                if (LRC::eButton::INVALID != button.button)
                {
                    Serial.printf("LRC resistance is %.1f (raw = %d); button is '%s'\n", 
                                    newRes, 
                                    expressionPedal.getRaw(),
                                    button.name
                                );
                }
            }
        }
    }

    // Hacky way of setting expression pedal gain
    if (3 == smartKnobTask.whichConfig && tuningPedal)
    {
        static int lastSKpos = 127;
        if (smartKnobTask.last_position != lastSKpos)
        {
            lastSKpos = smartKnobTask.last_position;
            expressionPedal.setGain(lastSKpos / 2);
            // Serial.printf("Set expr pot to 0x%02X\n", lastSKpos / 2);
        }
    }
}

void TouchADCtask::run(void)
{
    uint32_t whichISR = 0;
    vTaskDelay(150);
    initTouch();

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

        // Fader touch chip triggered?
        if (pdTRUE == xTaskNotifyWait(0UL, UINT32_MAX, &whichISR, 2)) // wait for notification from touch ISR
        {
            if (0 != (whichISR & touchFlag))
                updateTouch(); // only does I²C if ISR fired
        }

        pollTouch(); // generate state outputs, e.g. time long presses
        mainBacklight.update();
        scribbleBacklight.update();
        pollPedal();

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
