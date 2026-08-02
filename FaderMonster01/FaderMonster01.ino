
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

  return (uint16_t*) extmem_malloc(sz);
}

//================================================================
char dbgBuffer[200];
bool dbgWritten;

void SuperTask::loopFn(void)
{
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
  Serial.println("\n\nstarted supervisor task");

  while (1)
  {
    loopFn();
    bits++;
    vTaskDelay(20);
  }
}

//================================================================
void setup() 
{
  pinMode(TFT_BLK, arduino::OUTPUT);
  digitalWriteFast(TFT_BLK, arduino::LOW);

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

void loop() {} // keep Arduino happy

// Run a FaderMonsterTask
void taskRoot(void* pfmt)
{
  FaderMonsterTask& fmt = *((FaderMonsterTask*) pfmt);
  fmt.run();
}
