#include "headers.h"

void setup() 
{
  while (!Serial)
    ;

  pinMode(TFT_BLK, OUTPUT);
  digitalWriteFast(TFT_BLK, LOW);

  Serial.println("\n\nstarted");

  // initialise hardware
  initLEDs();
}

elapsedMillis em;
void loop(void)
{
  if (em >= 250)
  {
    em = 0;
    updateLEDs();
  }

  int ch = Serial.read();
  switch (ch)
  {
    case 'x':
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
}
