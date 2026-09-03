
#include "header.h"

//================================================================
//                                                             
//                                                             
//    888d888 .d88b.  88888b.d88b.   .d88b.  888  888  .d88b.  
//    888P"  d8P  Y8b 888 "888 "88b d88""88b 888  888 d8P  Y8b 
//    888    88888888 888  888  888 888  888 Y88  88P 88888888 
//    888    Y8b.     888  888  888 Y88..88P  Y8bd8P  Y8b.     
//    888     "Y8888  888  888  888  "Y88P"    Y88P    "Y8888  
//
ContinuousPot allPots[NUM_POTS]
  {
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f}
  };

  extern DPex U3, U5;

//================================================================
// some default settings
FaderMonsterSettings faderMonsterSettings
{
  .stripsConfig = 
  {
    { .ringLEDs = {xRED,    {0}}, .scribble = {{ .fg = TFT_RED }}},
    { .ringLEDs = {xORANGE, {0}}, .scribble = {{ .fg = TFT_ORANGE2 }}},
    { .ringLEDs = {xYELLOW, {0}}, .scribble = {{ .fg = TFT_YELLOW }}},
    { .ringLEDs = {xGREEN,  {0}}, .scribble = {{ .fg = TFT_GREEN }}},
    { .ringLEDs = {xBLUE,   {0}}, .scribble = {{ .fg = TFT_CYAN }}},
    { .ringLEDs = {xPURPLE, {0}}, .scribble = {{ .fg = TFT_BLUE }}},
    { .ringLEDs = {xPINK,   {0}}, .scribble = {{ .fg = TFT_MAGENTA }}},
    { .ringLEDs = {xWHITE,  {0}}, .scribble = {{ .fg = TFT_VIOLET }}}
  }
};
//================================================================
// one source of truth on where / how to allocate a 
// buffer used for DMA transfer of sprite image data
// to a display
uint16_t* allocateDMAbuffer(int w, int h)
{
  size_t sz = w*h               // number of pixels
            * sizeof(uint16_t); // pixels take this space each
  
  taskENTER_CRITICAL();
  uint16_t* result = (uint16_t*) malloc(sz);
  taskEXIT_CRITICAL();

  Serial.printf("Allocate %dx%d @ %08X\n", w, h, (uint32_t) result);
  return result;
}

//================================================================
void cycleLED(elapsedMillis& em, int& colour, int ring, int led)
{
  if (em >= 250)
  {
      em = 0;
      rings.setPixel(ring,led, faderMonsterSettings.stripsConfig[colour].ringLEDs.colour, 10);
      if (++colour >= NUM_POTS)
          colour = 0;
  }
}

//================================================================
TaskHandle_t* handles[]{nullptr, nullptr,    // 0-1
                        &scribbleTask.handle, &mainLCDtask.handle, // 2-3
                        nullptr, nullptr, // 4-5
                        &superTask.handle,  &ringLEDsTask.handle, // 6-7
                        &potsTask.handle, &touchTask.handle, &midiTask.handle}; // 8-10
void printTaskStates(void)
{
  TaskHandle_t handleIdle = xTaskGetIdleTaskHandle();
  handles[0] = &handleIdle;
  handles[1] = &freertos::g_yield_task;
  handles[4] = &StripTask::getStripTask(0).handle;
  handles[5] = &StripTask::getStripTask(3).handle;
  configRUN_TIME_COUNTER_TYPE idlePercent = ulTaskGetIdleRunTimePercent(),
                              idleCount = ulTaskGetIdleRunTimeCounter();
  float pct = idleCount * 100.0f / idlePercent; // 100% of counts to date
  Serial.println();
  for (int i = 0;i < COUNT_OF(handles);i++)
  {
    TaskStatus_t s;
    vTaskGetInfo(*(handles[i]), &s, pdTRUE, eInvalid);
    Serial.printf("Name '%s'; priority: %d; unused stack: %d; runtime %d (%.3f%%)\n",
              s.pcTaskName,
              s.uxCurrentPriority,
              s.usStackHighWaterMark,
              s.ulRunTimeCounter,
              (float) s.ulRunTimeCounter / pct * 100.0f
            );
  }
  Serial.printf("ADC updates take %uµs; DMA channel bits: %08X\n", 
                ADCupdateMicros,
                dma_channel_allocated_mask);
  freertos::print_ram_usage();
}

//================================================================
char dbgBuffer[200];
bool dbgWritten;

bool enableADCprint;

TouchStatus powerButton;
void pollPowerButton(void)
{
  powerButton = !GET_BIT(SOFT_POWER);
  if (powerButton.isChangedStatus())
  {
    static bool hadLongPress = false, canPowerOff = false;
    TouchStatus::eStatus e = powerButton.getExtendedStatus();
    Serial.printf("Power button status is %d\n", e);

    switch (e)
    {
      default:
        break;

      case TouchStatus::eStatus::LONG:
        if (canPowerOff)
        {
          hadLongPress = true;
          SET_BIT(POWER_LED,0); // clear power LED B.1 output
          Serial.println("Release to power off");
        }
        break;

      case TouchStatus::eStatus::OFF: // released after power-up etc.
        canPowerOff = true;
        if (hadLongPress)
        {
          Serial.print("Off ... ");
          vTaskDelay(1000);

          Serial.print("6V ... ");
          digitalWriteFast(EN_6V, arduino::LOW); // 6V supply off
          vTaskDelay(1000);

          Serial.println("shutdown!");
          SET_BIT(TOGGLE_POWER, 1); // shutdown!
          for (int i=0;i<100;i++)
          {
            Serial.print('.');
            vTaskDelay(10);
          }
        }
        else
          SET_BIT(POWER_LED,1); // set power LED B.1 output

        break;
    }
  }
}
void SuperTask::loopFn(void)
{
  {
    static elapsedMillis em;
    if (em >= 250)
    {
      em = 0;
      if (enableADCprint)
        printADCs();
/*
      static bool pwrLED;
      pwrLED = !pwrLED;
      SET_BIT(POWER_LED, pwrLED);
*/      
      //SET_BIT(POWER_LED, 1);
    }
  }

  if (dbgWritten)
  {
    dbgWritten = false;
    Serial.print(dbgBuffer);
  }

#if 0  
  {
    static uint16_t lastU5;
    const uint16_t mask = 0b1011'1111'0010'1100;
    uint16_t u5 = U5.getGPIO() & mask;
    if (lastU5 != u5)
    {
      lastU5 = u5;
      Serial.printf("U5: %04hX; pwr: %s\n", u5 ^ mask, GET_BIT(SOFT_POWER)?"released":"pressed");
    }
  }
#endif // 0

  pollPowerButton();

  // deal with a string of commands all in one go,
  // unless an unrecognised commands is given
  while (1)
  {
    bool exitWhile = false;
    int ch = Serial.read();
    switch (ch)
    {
      default:
        exitWhile = true;
        break; 

      case '0':
        StripTask::globalBright = -1;
        Serial.println("brightness: max");
        break; 

      case '1' ... '9':
        StripTask::globalBright = 9.0f * powf(1.45f,ch - '1'); // 9 to 175, geometric scale
        Serial.printf("brightness: %d (level %d, %.1f%%)\n", StripTask::globalBright, ch - '0', (float) StripTask::globalBright / 2.55f);
        break;        

      case 'c':
        if (InterTaskRequest::Result::failed == touchTask.requestCalibration(touchCalibrationRequest))
        {
          if (touchCalibrationRequest.isBusy())
            Serial.println("Calibration request pending!");
          else
            Serial.println("Calibration request failed (queue full?)");
        }          
        break;

      case 'q':
        {
          if (0 != touchTask.messagesWaiting())
            Serial.println("Touch task queue has item(s) pending");
          else
            Serial.println("Touch task queue is empty");
        }
        break;

      case 'x':
        touchCalibrationRequest.setInactive();
        break;

      case ' ':
        Serial.println();
        break;

      case 'd':
        vTaskDelay(5);
        break;

      case 'p':
        enableADCprint = !enableADCprint;
        break;

      case 't':
        printTaskStates();
        break;        
    }
    if (exitWhile)
      break;
  }

  if (touchCalibrationRequest.isFinished())
  {
    Serial.printf("Touch calibration %s after %uus; execution time was %uus\n",
            InterTaskRequest::Result::failed == touchCalibrationRequest.status
                ?"failed"
                :"done",
            touchCalibrationRequest.overallTime(),
            touchCalibrationRequest.executionTime());
    touchCalibrationRequest.setInactive();            
  }

}


SuperTask superTask{"Super", 512, nullptr, 2};

uint8_t bits;
void SuperTask::run(void)
{
  Serial.printf("\n\n[%d]: started supervisor task\n", micros());

  /*** REMOVE THIS LATER ! ****/
  // dummy allocation to ensure we don't over-allocate RAM2
  // allocateDMAbuffer(320,240);

  while (1)
  {
    loopFn();
    bits++;
    vTaskDelay(20);
  }
}

//================================================================
//                      888                      
//                      888                      
//                      888                      
//    .d8888b   .d88b.  888888 888  888 88888b.  
//    88K      d8P  Y8b 888    888  888 888 "88b 
//    "Y8888b. 88888888 888    888  888 888  888 
//         X88 Y8b.     Y88b.  Y88b 888 888 d88P 
//     88888P'  "Y8888   "Y888  "Y88888 88888P"  
//                                      888      
//                                      888      
//                                      888      
//
void setup() 
{
  while (!Serial)
    ;

  // enable 6V    
  pinMode(EN_6V, arduino::OUTPUT);
  digitalWriteFast(EN_6V, arduino::HIGH);

  delay(100); // wait for it to stabilise (?)

  // some startup things are on the port expanders:
  initDPEX();
  ADCsReset();

  // don't do this: the GT911 needs a specific sequence!
  // scribbleReset();

  // switch display backlights off
  pinMode(SCRIBBLE_BL, arduino::OUTPUT);
  digitalWriteFast(SCRIBBLE_BL, arduino::LOW);
  pinMode(MAINLCD_BL, arduino::OUTPUT);
  digitalWriteFast(MAINLCD_BL, arduino::LOW);
  

  // hardware "server" tasks - independent of one another
  touchTask.create(); // creates task - doesn't start it
  // fadersTask.create(); // just touch - potsTask deals with analogue
  ringLEDsTask.create();
  scribbleTask.create();
  mainLCDtask.create();
  // mainLCDtask.create();
  potsTask.create(); // need to be before...

  // "client" tasks
  StripTask::CreateTasks(); // ...the strip...
  midiTask.create(); // ...and MIDI tasks
  superTask.create();

  vTaskStartScheduler(); // start all tasks - the mayhem begins!
}
//================================================================
void loop() {} // keep Arduino happy

// Run a FaderMonsterTask
void taskRoot(void* pfmt)
{
  FaderMonsterTask& fmt = *((FaderMonsterTask*) pfmt);
  fmt.run();
}
