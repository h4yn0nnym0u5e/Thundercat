//#include <TeensyDebug.h>
#include "headers.h"

//=====================================================
void setup() 
{
  // Teensy USB serial ports
  Serial.begin(0);
  SerialUSB1.begin(0);

  initSmartKnob(Serial1);
  initSupervisor();
  initMainLCD();
  initGT911touch();

  //halt_cpu();
  SER_TERM.println("\n\nStarting");
  vTaskStartScheduler();
}

void loop() {} // keep Arduino happy

//=====================================================
#define TASK_LIST_ENTRY(tsk) extern TaskHandle_t handle##tsk;
TASK_LIST
#undef TASK_LIST_ENTRY

#define TASK_LIST_ENTRY(tsk) , &handle##tsk
TaskHandle_t* handles[]
  { nullptr
    TASK_LIST
  };
#undef TASK_LIST_ENTRY

void printTaskStates(void)
{
  TaskHandle_t handleIdle = xTaskGetIdleTaskHandle();
  handles[0] = &handleIdle;
  configRUN_TIME_COUNTER_TYPE idlePercent = ulTaskGetIdleRunTimePercent(),
                              idleCount = ulTaskGetIdleRunTimeCounter();
  float pct = idleCount * 100.0f / idlePercent; // 100% of counts to date
  SER_TERM.println();
  for (int i = 0;i < COUNT_OF(handles);i++)
  {
    TaskStatus_t s;
    vTaskGetInfo(*(handles[i]), &s, pdTRUE, eInvalid);
    SER_TERM.printf("Name '%s'; priority: %d; unused stack: %d; runtime %d (%.3f%%)\n",
              s.pcTaskName,
              s.uxCurrentPriority,
              s.usStackHighWaterMark,
              s.ulRunTimeCounter,
              (float) s.ulRunTimeCounter / pct * 100.0f
            );
  }
  freertos::print_ram_usage();
}
//=====================================================
static void updateSupervisor() 
{
  int ch;

  ch = SER_TERM.read();
  switch (ch)
  {
    case -1:
    default: 
      break;

    case '1' ... '5':
      cmdSK = ch;
      break;

    case 't':
      printTaskStates();
      break;      
  }

  if (buttonReleased >= 0)
  {
    //SER_TERM.printf("Released button %d\n", buttonReleased);
    cmdSK = '1' + buttonReleased;
    buttonReleased = -1;
  }
  //updateSmartKnob();

  if (positionUpdated)
  {
    positionUpdated = false;
    SER_TERM.printf("Position:%s", positionText);
    SER_TERM.println();
  }
}


TaskHandle_t handleSupervisor;
static void taskSupervisor(void* params)
{
  while (1)
  {
    updateSupervisor();
    vTaskDelay(2);
  }
}


void initSupervisor(void)
{
  xTaskCreate(taskSupervisor, "Supervisr", 512, nullptr, 2, &handleSupervisor);
}

