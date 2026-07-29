
#include "header.h"

//================================================================
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
        if (InterTaskRequest::Result::failed == touchTask.requestCalibration(&touchCalibrationRequest))
        {
          if (touchCalibrationRequest.isBusy())
            Serial.println("Calibration request pending!");
          else
            Serial.println("Calibration request failed (queue full?)");
        }          
        break;

      case 'q':
        {
          InterTaskRequest* preq;
          if (pdPASS == xQueuePeek(touchTask.queue, &preq, 0))
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

  touchTask.create(); // creates task - doesn't start it
  superTask.create();

  vTaskStartScheduler();
}

void loop() {}

// Run a FaderMonsterTask
void taskRoot(void* pfmt)
{
  FaderMonsterTask& fmt = *((FaderMonsterTask*) pfmt);
  fmt.run();
}
