#include <Adafruit_NeoPixel.h>
#include "legacy.h"


Adafruit_NeoPixel pixels(NLEDS, PIN, STRIP_SETTINGS);

void setup1(void)
{
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  
  while (!Serial && millis() < 5000)
    ;
  delay(10); // Core 0 gets to output first
  Serial.printf("Core %d started at %lu\n",rp2040.cpuid(),millis());
}

int count;
void loop1(void)
{  
  if (0 == millis() % 10) // do updates when milliseconds are a multiple of 10
  {
    uint32_t now = micros();
    pixels.show();
    now = micros() - now;
  
    count--;
    if (count <= 0)
    {
      count = 100;
      //Serial.printf("show() took %luus\n",now);
    }
  }
}
