#include "config.h"
#include "headers.h"
#include "arduino_freertos.h"

//namespace arduino {
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

void setup() 
{
  while (!Serial)
    ;
  Serial.println("\n\nstarted");

  // initialise hardware
  doReset();
  initLEDs();
  while (0 != initTouch())
  {
    Serial.println("Waiting for 6V supply...");
    delay(500);
  }
  initADCs();

  xTaskCreate(mainLoop, "Super", 1024, nullptr, 2, nullptr);

  delay(1000);

  vTaskStartScheduler();
}



elapsedMillis em;
bool echoOnce, enableADCprint;

void loop() // dummy to keep Arduino happy
{
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

    case 'y':
      potsToRaw();
      break;

    case 'z':
      zeroPots();
      break;
  }
}

static void mainLoop(void*)
{
  while (1)
  {
    loopFn();
  }
}


//} // namespace