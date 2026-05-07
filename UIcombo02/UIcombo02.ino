#include "config.h"

void doReset()
{
  // reset ADCs and touch chip
  pinMode(RST_PIN,OUTPUT);
  digitalWrite(RST_PIN,HIGH);
  delay(1);
  digitalWrite(RST_PIN,LOW);
  delay(1);
  digitalWrite(RST_PIN,HIGH);
  delay(1);
}

void setup() 
{
  while (!Serial)
    ;
  Serial.println("started");

  // initialise hardware
  doReset();
  initLEDs();
  while (0 != initTouch())
  {
    Serial.println("Waiting for 6V supply...");
    delay(500);
  }
  initADCs();

  delay(1000);
}



elapsedMillis em;
bool echoOnce, enableADCprint;

void loop() 
{
  if (em >= 250)
  {
    em = 0;
    if (enableADCprint)
      printADCs();
  }
  updateADCs();
  updateTouch();
  updateLEDs();

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
