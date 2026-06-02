#include "headers.h"
#include "arduino_freertos.h"

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

static void mainLoop(void*);

TaskHandle_t handleSuper;
void setup() 
{
  while (!Serial)
    ;
  Serial.println("\n\nstarted");
  freertos::print_ram_usage();

  // initialise hardware
  doReset();
  initLEDs();
  initTouch();
  initADCs();
  initScribble();

//Serial.printf("Create Super task: \n");    
  xTaskCreate(mainLoop, "Super", 512, nullptr, 2, &handleSuper);

  delay(1000);

  vTaskStartScheduler();
}

/*
// Useful if you instrument pvPortMalloc() etc.
void print_malloc(uint32_t xSize, uint32_t where)
{
    Serial.printf("  * allocated %u at 0x%08X\n", xSize, where);
}

void* mymalloc(size_t xSize)
{
  return ::malloc(xSize);
}

void myfree(void* p)
{
  ::free(p);
}
*/

elapsedMillis em;
bool echoOnce, enableADCprint, enablePrintTouches;
;

void loop() // dummy to keep Arduino happy
{
}

extern TaskHandle_t handleSuper, handleADCs, handleRing0, handleTouch, handleScribble;
extern TaskHandle_t handlesRings[];
TaskHandle_t* handles[]{nullptr, &handleSuper, &handleADCs, &handleRing0, &handleTouch, handlesRings+1, &handleScribble};
void printTaskStates(void)
{
  TaskHandle_t handleIdle = xTaskGetIdleTaskHandle();
  handles[0] = &handleIdle;
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

static void loopFn(void)
{
  if (em >= 250)
  {
    em = 0;
    if (enableADCprint)
      printADCs();
  }

  if (enablePrintTouches)
  {
    static int last[NUM_POTS]{0};
    bool changed = false;
    for (int i=0;i<NUM_POTS;i++)
    {
      int t = (int) keyStatuses[i].getExtendedStatus();
      if (last[i] != t)
        changed = true;
      last[i] = t;
    }
    if (changed)
      printTouches();
  }
  int ch = Serial.read();

  switch (ch)
  {
    case '0':
      bright = -1;
      Serial.println("brightness: max");
      break; 

    case '1' ... '9':
      bright = 9.0f * powf(1.45f,ch - '1'); // 9 to 175, geometric scale
      Serial.printf("brightness: %d (level %d, %.1f%%)\n", bright, ch - '0', (float) bright / 2.55f);
      break;

    case 'c':
      calibrateTouch();
      break;

    case 'e':
      echoOnce = true;
      break;

    case 'p':
      enableADCprint = !enableADCprint;
      break;

    case 'q':
      enablePrintTouches = !enablePrintTouches;
      break;

    case 'r':
      doReset();
      initADCs();
      initTouch();
      break;

    case 't':
      printTaskStates();
      break;

    case 'y':
      potsToRaw();
      break;

    case 'z':
      zeroPots();
      break;
  }
}

extern uint8_t bits;
static void mainLoop(void*)
{
  while (1)
  {
    loopFn();
    bits++;
    vTaskDelay(20);
  }
}

extern "C"
void startup_middle_hook(void)
{
  pinMode(TFT_BLK,arduino::OUTPUT);
  digitalWriteFast(TFT_BLK, arduino::LOW);
}
