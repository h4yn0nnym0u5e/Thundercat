/*
 * Test USB+Power board I/O
 * Uses "Waveshare" Pico Zero
 * 
 * This uses the second core to run the onboard WS2812 output, so
 * we have the whole capacity of the first core to run the algorithms.
 */

#include <Adafruit_NeoPixel.h>

#include "legacy.h"

#define COUNT_OF(a) ((int)(sizeof a / sizeof a[0]))
//int litLEDs[] = {20,21,22}; // lit by default, due to button hrdware - panic not!

#if defined(LED_BUILTIN)
  int led = LED_BUILTIN; // the PWM pin the LED is attached to
#else
  #if defined(ARDUINO_WAVESHARE_RP2040_ZERO)
    int led = 21; // the PWM pin the LED is not attached to - it's an RP2040 Zero...
  #endif
#endif // defined(LED_BUILTIN)
  

//====================================================================================
// the setup routine runs once when you press reset:
void setup() {
  // declare pin to be an output:
  pinMode(led, OUTPUT);
  pinMode(SOFT_POWER, INPUT_PULLUP);

  Serial.begin(115200);
  while (!Serial && millis() < 5000)
  {
    digitalWrite(led,1);
    delay(50);
    digitalWrite(led,0);
    delay(200);
  }
    
  Serial.println("==============\n"
                 "=== Reboot ===");
                 
  setupLights();
}

//====================================================================================
extern Adafruit_NeoPixel pixels;
void updateNeoPixels(void)
{
  for (int i=0;i<NLEDS;i++)
    pixels.setPixelColor(i,pixels.Color(colours[i].r,colours[i].g,colours[i].b));
  // the other core will send the update    
}

//====================================================================================
// Assert or negate power control pins.
// All hardware has pull-downs as we expect to be off, or in input
// mode if just powered up. So we only switch between tri-state and high.
bool power, smartknob, powLED, en6V, softSwitch, ssActive;
void assertPin(int pin, bool state)
{
  if (state)
  {
    pinMode(pin,OUTPUT);
    digitalWrite(pin,1);
  }
  else
    pinMode(pin,INPUT);
}
//====================================================================================
// the loop routine runs over and over again forever:
void loop() 
{
  static bool ssLast;

  // Deal with serial commands to toggle I/O
  char ch = Serial.read();
  if (ch > 0)
  {
    switch (ch)
    {
#define ACASE(c,v,p) case c:  v = !v; Serial.printf("Set " #p " %s\nk", v?"asserted":"negated"); Serial.flush(); delay(100); assertPin(p,v); break; 
      ACASE('k',power,TOGGLE_POWER)
      ACASE('s',smartknob,SK_EN)
      ACASE('l',powLED,POWER_LED)
      ACASE('6',en6V,EN_6V)
      case 'a':
        ssActive = !ssActive;
        Serial.printf("Soft switch %sactive\n", ssActive?"":"not ");
        break;
      default: break;
    }
  }

  // Deal with soft switch
  bool ssNow = !digitalRead(SOFT_POWER); // active low
  if (ssNow != ssLast) // changed state
  {
    Serial.printf("Soft switch %s", ssNow?"pressed":"released");
    Serial.flush();

    // if 'a' command has made it active, turn off
    if (ssActive && ssNow)
    {
      // fundamentally the switch turns the system on, so
      // wait for release before we take the turn-off action
      while (!digitalRead(SOFT_POWER))
      {
        delay(250);
        Serial.print('.');
      }
      Serial.print("\nShutdown");
      Serial.flush();
      delay(100);
      assertPin(TOGGLE_POWER,HIGH); // die die die !!!
    }
    Serial.println();
    ssLast = ssNow;
  }
  
  for (int i=0;i<3;i++)
  {
    delay(10);
    updateLights();
    updateNeoPixels();
  }
}
