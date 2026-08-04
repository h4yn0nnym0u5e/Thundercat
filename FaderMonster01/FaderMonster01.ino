
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
  uint16_t* result = (uint16_t*) extmem_malloc(sz);
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
TaskHandle_t* handles[]{nullptr, nullptr, &superTask.handle, &touchTask.handle, // 0-2
                        &scribbleTask.handle, &potsTask.handle, &ringLEDsTask.handle, // 3-5
                        nullptr, nullptr}; // 6+7
void printTaskStates(void)
{
  TaskHandle_t handleIdle = xTaskGetIdleTaskHandle();
  handles[0] = &handleIdle;
  handles[1] = &freertos::g_yield_task;
  handles[7] = &StripTask::getStripTask(0).handle;
  handles[8] = &StripTask::getStripTask(3).handle;
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

void SuperTask::loopFn(void)
{
  {
    static elapsedMillis em;
    if (em >= 250)
    {
      em = 0;
      if (enableADCprint)
        printADCs();
    }
  }

  if (dbgWritten)
  {
    dbgWritten = false;
    Serial.print(dbgBuffer);
  }

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
void doReset()
{
  // reset ADCs and touch chip
  pinMode(RST_PIN,arduino::OUTPUT);
  digitalWrite(RST_PIN,arduino::HIGH);
  delay(1);
  digitalWrite(RST_PIN,arduino::LOW);
  delay(1);
  digitalWrite(RST_PIN,arduino::HIGH);
  delay(1);
}

void setup() 
{
  while (!Serial)
    ;

  pinMode(TFT_BLK, arduino::OUTPUT);
  digitalWriteFast(TFT_BLK, arduino::LOW);

  doReset();

  // hardware "server" tasks - independent of one another
  touchTask.create(); // creates task - doesn't start it
  ringLEDsTask.create();
  scribbleTask.create();
  // mainLCDtask.create();
  // fadersTask.create();
  // buttonsTask.create();
  potsTask.create(); // need to be before...

  // "client" tasks
  StripTask::CreateTasks(); // ...the strip...
  // midiTask.create(); // ...and MIDI tasks
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
