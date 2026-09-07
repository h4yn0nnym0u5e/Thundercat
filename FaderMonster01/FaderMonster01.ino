
#include "header.h"
#include <usb_names.h>

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
static TFTcolours mainColours{TFT_LIGHTGREY, TFT_DARKGREY, TFT_WHITE};
static StripColours stripColours[8] =
{
    { {xRED,    {0}}, {0}, {{ TFT_RED }}},
    { {xORANGE, {0}}, {0}, {{ TFT_ORANGE2 }}},
    { {xYELLOW, {0}}, {0}, {{ TFT_YELLOW }}},
    { {xGREEN,  {0}}, {0}, {{ TFT_GREEN }}},
    { {xBLUE,   {0}}, {0}, {{ TFT_CYAN }}},
    { {xPURPLE, {0}}, {0}, {{ TFT_BLUE }}},
    { {xPINK,   {0}}, {0}, {{ TFT_MAGENTA }}},
    { {xWHITE,  {0}}, {0}, {{ TFT_VIOLET }}}
  };
static StripControls stripControls[8];  

FaderMonsterSettings faderMonsterSettings{{stripColours,stripControls}, mainColours};
/*
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
  },
  .mainColours = {TFT_LIGHTGREY, TFT_DARKGREY, TFT_WHITE}
};
*/
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
      rings.setPixel(ring,led, faderMonsterSettings.stripsConfig.colours[colour].ringLEDs.colour, 10);
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

//================================================================
//! queue a request for the supervisor task to process a main LCD touch
InterTaskRequest& SuperTask::updateMainTouch(InterTaskRequest& req, GTPoint& touchPoint, TickType_t timeout)
{
  requestPayload payload{&SuperTask::doUpdateMainTouch, &touchPoint};
  RequestQueue<SuperTask, requestPayload>::queueEntry entry{&req,payload};

  reqQueue.request(entry, timeout);

  return req;
}

//! start UI executing a main LCD touch message
InterTaskRequest::Result SuperTask::doUpdateMainTouch(void* pGTPoint)
{
  GTPoint& lastTouch = *((GTPoint*) pGTPoint);
  Trigger trigger{.type    = Trigger::eTriggerType::touchPoint, 
                  .trigger = { .touchPoint = lastTouch }};

  ui.update(trigger);
  //Serial.printf("[%d]: %d, %d (%d)\n", touchTask.lastTouchTime, lastTouch.x, lastTouch.y, lastTouch.reserved);
 
  return InterTaskRequest::Result::done;
}

//================================================================

TouchStatus powerButton;
void pollPowerButton(void)
{
  static bool hadLongPress = false, canPowerOff = false;
  powerButton = !GET_BIT(SOFT_POWER);
  bool isChanged = powerButton.isChangedStatus();
  TouchStatus::eStatus e = powerButton.getExtendedStatus();

  if (isChanged)
  {
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

      case TouchStatus::eStatus::JUST_OFF:
        superTask.flipUI();
        break;

      case TouchStatus::eStatus::OFF: // released after power-up etc.
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

        break;
    }
  }

  // not entirely certain if button comes up 
  // in "off" state, so we have to poll this 
  if (!canPowerOff && TouchStatus::eStatus::OFF == e)
  {
    canPowerOff = true;
    SET_BIT(POWER_LED,1); // set power LED B.1 output
  }
}

/**
 * Process the supervisor task's UI (main LCD).
 * \return true if extra dealy might be wanted, e.g. if screen update was requested
 */
bool SuperTask::processUI(void)
{
  bool wait = true;

  // test of changing UI classes on the fly
  if (flipui) // set by power button brief press
  {
    flipui = false;       // this is kinda important!
    whichUI = 1-whichUI;  // flip to the other UI
    switch (whichUI)
    {
      case 0:
        new(_ui.space) MainColourPicker; // placement new
        break;

      case 1:
        new(_ui.space) MainTestRects; 
        break;
    }
    ui.begin(mainLCDtask.getSprite(), faderMonsterSettings.mainColours);
  }

  // we're responsible solely for the UI - real-time MIDI etc.
  // is dealt with separately by a high-priority task
  [[maybe_unused]] uint32_t pollInterval = ui.poll(); // allow UI to do internally-timed stuff
  switch (ui.state)
  {
      case UIclass::State::done: // ready for a new trigger
        // poll for queued requests
        if (InterTaskRequest::Result::inactive == reqQueue.executeRequest(*this, 10))
            wait = false; // already waited 10 ticks
        break;

      case UIclass::State::next: // can do next phase, if any
        ui.update({Trigger::eTriggerType::nextPhase});
        wait = false; // have probably changed state - loop quickly
        break;

      case UIclass::State::push:
        if (ui.writeToDisplay().isInactive()) // will change state for us, or not
            wait = false; // active - wait for display task to finish
        break;

      case UIclass::State::busy:
        if (ui.writeFinished())
            wait = false;
        break;
  }
  return wait;
}

extern FlexIOSPI SPIflex;
extern int stallCount;
static char fileName[30];
void SuperTask::loopFn(void)
{
  { // debug print of ADCs
    static elapsedMillis em;
    if (em >= 250)
    {
      em = 0;
      if (enableADCprint)
        printADCs();
    }
  }

  if (dbgWritten) // other task requested Serial output
  {
    dbgWritten = false;
    Serial.print(dbgBuffer);
  }

  pollPowerButton();
  if (processUI())
    vTaskDelay(2);

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
        {
          DMAChannel& dma = SPIflex.getDMArx();
          Serial.printf("CSR: %08X\n", dma.TCD->CSR);
          Serial.printf("CR:  %08X\n", DMA_CR);
          Serial.printf("ES:  %08X\n", DMA_ES);
        }
        break;

      case 'g':
        Serial.println();
        mainLCDtask.pauseOutput = false;
        stallCount++;
        break;

      case 'z':
        mainLCDtask.zapScreen = true;
        break;

      case 'p':
        enableADCprint = !enableADCprint;
        break;

      case 's':
      {
        char n = Serial.read();
        if (n >= '0' && n <= '9')
        {
          sprintf(fileName, "scene-%c.csv", n);
          mainLCDtask.saveSettings(saveSettingsRequest, fileName);
        }
        else
          dumpSettings(Serial);
      }
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

  if (saveSettingsRequest.isFinished())
  {
    Serial.printf("Save settings to '%s' %s after %uus; execution time was %uus\n",
            fileName,
            InterTaskRequest::Result::failed == saveSettingsRequest.status
                ?"failed"
                :"done",
            saveSettingsRequest.overallTime(),
            saveSettingsRequest.executionTime());
    saveSettingsRequest.setInactive();            
  }

}


SuperTask superTask{"Super", 512, nullptr, 2, 
                    8 /* plenty of requests (?) */};

uint8_t bits;
void SuperTask::run(void)
{
  Serial.printf("\n\n[%d]: started supervisor task\n", micros());

  while (!mainLCDtask.tftInitComplete())
    vTaskDelay(10);

    // set the initial UI presentation on the display
  Serial.println("Init supervisor UI");
  new(_ui.space) MainColourPicker; // placement new
  ui.begin(mainLCDtask.getSprite(), faderMonsterSettings.mainColours);

  while (1)
  {
    static elapsedMillis em = 0;
    loopFn(); // includes some delay, unless UI is being drawn

    if (em >= 20)
    {
      em = 0;
      bits++;
    }
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
static void getSerialNumber(char* sernum);
void setup() 
{
  while (!Serial)
    ;

  Serial.print("\n\n*************************************************************");
  Serial.printf("\n" __FILE_NAME__ "; Teensyduino %.2f; " __DATE__ " " __TIME__ "\n", (float) TEENSYDUINO / 100.0f);
  {
    char buf[11];
    getSerialNumber(buf);
    Serial.printf("Teensy serial number is: %s\n", buf);
  }
  // debug pins for scope:
  pinMode(DBG1, arduino::OUTPUT);
  pinMode(DBG2, arduino::OUTPUT);
  pinMode(DBG3, arduino::OUTPUT);
  
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

static void getSerialNumber(char* sernum)
{
  //char sernum[10];
  for (size_t i = 0; i < 10; i++)
    sernum[i] = usb_string_serial_number.wString[i];
  sernum[10] = 0;    
}
//================================================================
void loop() {} // keep Arduino happy

// Run a FaderMonsterTask
void taskRoot(void* pfmt)
{
  FaderMonsterTask& fmt = *((FaderMonsterTask*) pfmt);
  fmt.run();
}
