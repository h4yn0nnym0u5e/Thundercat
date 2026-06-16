//#include <TeensyDebug.h>
#include "headers.h"

void setup() 
{
  // Teensy USB serial ports
  Serial.begin(0);
  SerialUSB1.begin(0);

  // USART port to SmartKnob
  Serial1.begin(115200);
  
  initSmartKnob();

  //halt_cpu();
}

void loop() 
{
  updateSmartKnob();
}
