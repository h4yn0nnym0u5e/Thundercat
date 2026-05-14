#include "config.h"
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

  // initialise hardware
  doReset();
  initLEDs();
  initTouch();
  initADCs();

  xTaskCreate(mainLoop, "Super", 512, nullptr, 2, &handleSuper);

  delay(1000);

  vTaskStartScheduler();
}



elapsedMillis em;
bool echoOnce, enableADCprint;

void loop() // dummy to keep Arduino happy
{
}

extern TaskHandle_t handleSuper, handleADCs, handleRing0, handleTouch;
extern TaskHandle_t handlesRings[];
TaskHandle_t* handles[]{nullptr, &handleSuper, &handleADCs, &handleRing0, &handleTouch, handlesRings+1};
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
}

static void loopFn(void)
{
  if (em >= 250)
  {
    em = 0;
    if (enableADCprint)
      printADCs();
  }

  int ch = Serial.read();

  switch (ch)
  {
    case 'c':
      calibrateTouch();
      break;

    case 'e':
      echoOnce = true;
      break;

    case 'p':
      enableADCprint = !enableADCprint;
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
