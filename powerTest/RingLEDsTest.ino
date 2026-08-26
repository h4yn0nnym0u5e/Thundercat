#include "headers.h"
#include "hardware.h"

void initRings() 
{
  while (!Serial)
    ;

  //pinMode(SCRIBBLE_BL, OUTPUT);
  //digitalWriteFast(SCRIBBLE_BL, LOW);

  Serial.println("\n\nstarted");

  // initialise hardware
  initLEDs();
}

elapsedMillis em;
bool updateRings(int ch)
{
  bool result = false; // assume we absorb the character

  if (em >= 250)
  {
    em = 0;
    updateLEDs();
  }

  // int ch = Serial.read();
  switch (ch)
  {
    default:
      result = true; // didn't absorb character
      break;

    case '#':
      bright = 0;
      Serial.println("off");
      break; 

    case '0':
      bright = -1;
      Serial.println("brightness: max");
      break; 

    case '1' ... '9':
      bright = 9.0f * powf(1.45f,ch - '1'); // 9 to 175, geometric scale
      Serial.printf("brightness: %d (level %d, %.1f%%)\n", bright, ch - '0', (float) bright / 2.55f);
      break;
  }

  return result;
}
