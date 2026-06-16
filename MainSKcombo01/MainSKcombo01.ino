//#include <TeensyDebug.h>
#include "headers.h"

void setup() 
{
  // Teensy USB serial ports
  Serial.begin(0);
  SerialUSB1.begin(0);

  initSmartKnob(Serial1);

  //halt_cpu();
}

int last_position;
void loop() 
{
  updateSmartKnob();

  if (last_position != current_position)
  {
    last_position = current_position;
    SER_TERM.printf("Position: %d", current_position);
    SER_TERM.println();
  }
}
