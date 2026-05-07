/* 
 Simple ring LED support

 We have 8 rings of 20 LEDs, which for historical
 reasons go clockwise from 216° ("north" being 0°),
 and the first ring is on pot 8, i.e. the
 rightmost one.

 Derived from WS2812Serial BasicTest Example
 code which is in the public domain. 
*/

//#include <WS2812Serial.h>
#include "RingLEDs.h"
#include "contPot.h"

const int numled = LEDS_PER_RING*NUM_POTS + 20;
const int pin = LED_DRIVE_PIN;

// Usable pins:
//   Teensy LC:   1, 4, 5, 24
//   Teensy 3.2:  1, 5, 8, 10, 31   (overclock to 120 MHz for pin 8)
//   Teensy 3.5:  1, 5, 8, 10, 26, 32, 33, 48
//   Teensy 3.6:  1, 5, 8, 10, 26, 32, 33
//   Teensy 4.0:  1, 8, 14, 17, 20, 24, 29, 39
//   Teensy 4.1:  1, 8, 14, 17, 20, 24, 29, 35, 47, 53

byte drawingMemory[numled*3];         //  3 bytes per LED
DMAMEM byte displayMemory[numled*12]; // 12 bytes per LED

WS2812Serial leds(numled, displayMemory, drawingMemory, pin, WS2812_GRB);
RingLEDs rings(leds, drawingMemory, 8,20,8);

//*
#define xRED    0xFF0000
#define xORANGE 0xC02000
#define xYELLOW 0xC0A000
#define xGREEN  0x00FF00
#define xBLUE   0x0000FF
#define xPURPLE 0x2000C0
#define xPINK   0xC00060
#define xWHITE  0xC0C0C0

// Less intense...
// */
#define RED    0x070000
#define ORANGE 0x060100
#define YELLOW 0x060500
#define GREEN  0x000700
#define BLUE   0x000007
#define PURPLE 0x010006
#define PINK   0x060003
#define WHITE  0x060606
//*/

int colours[]{RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, PINK, WHITE};
int colours2[]{xRED, xORANGE, xYELLOW, xGREEN, xBLUE, xPURPLE, xPINK, xWHITE};
#define BLACK  0x000000
uint32_t ringColours[]{RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, PINK, WHITE};


void initLEDs(void) 
{
  leds.begin();
  for (int i=0;i<numled;i++)
    leds.setPixel(i, BLACK);
  for (int i=0;i<NUM_POTS; i++)
    leds.setPixel((NUM_POTS - 1 - i)*LEDS_PER_RING+18,ringColours[i]);
  leds.show();
}


/*
 Simplest possible UI - a dot at the nearest position to 
 the given value (which should have a range of ±1.0)
 */
extern uint8_t keyStatuses[NUM_POTS];
void setDotx(int ringNum, float value, uint32_t colour)
{
//Serial.printf("%3d:%06X  ", base+offi, colour);
  if (value < -1.0f) value = -1.0f;
  if (value > +1.0f) value = +1.0f;

  value = value*8*18; // angle
  if (value < 0.0f)
    value += 360.0f;
  rings.clear(ringNum);
  rings.setPixel(ringNum, value, colour);

  if (keyStatuses[ringNum])
      rings.setPixel(ringNum, 10, WHITE);
}

void setDot(int ringNum, float value, uint32_t colour)
{
  const int firstLED = 12;

  if (value < -1.0f) value = -1.0f;
  if (value > +1.0f) value = +1.0f;

  value = value*(17*18 / 2 - 0.02f); // angle: ±152.8
  if (value < 0.0f)
    value += 360.0f; // 207.2 minimum
  rings.clear(ringNum);

  if (echoOnce && 0 == ringNum)
  {
    rings.debug = true;
    echoOnce = false;
  }

  rings.setArc(ringNum, 18.0f*firstLED - 8.99f, value, colour);
  rings.ensurePixelVisible(ringNum,firstLED,colour);
  if (rings.debug)
    Serial.println();
  rings.debug = false;

  if (keyStatuses[ringNum])
      rings.setPixel(ringNum, 10, WHITE);
}

extern ContinuousPot allPots[NUM_POTS];
void updateLEDs(void)
{
    static elapsedMillis em = 0;

    if (em >= 5)
    {
        em = 0;
        for (int i=0;i<NUM_POTS;i++)
            setDot(i, allPots[i].getCurrent(), ringColours[i]);
        rings.show();
//Serial.println();        
    }
}
