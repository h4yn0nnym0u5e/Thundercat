/* 
 Simple ring LED support

 We have 8 rings of 20 LEDs, which for historical
 reasons go clockwise from 216° ("north" being 0°),
 and the first ring is on pot 8, i.e. the
 rightmost one.

 Derived from WS2812Serial BasicTest Example
 code which is in the public domain. 
*/

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
RingLEDs<NUM_POTS> rings(leds, drawingMemory, LEDS_PER_RING,LED_TOP_OFFSET);

int rainbow1[LEDS_PER_RING] = {
    // straight order: first entry is highest value
    0x070000, 0x080100, 0x070200, 0x060200,
    0x060200, 0x060300, 0x060400, 0x060500,
    0x050500, 0x030600, 0x010600, 0x000700,
    0x000601, 0x000402, 0x000204, 0x000007,
    0x010007, 
    0x020007, 0x030005, 0x040005
  };
int cold2hot1[LEDS_PER_RING]   = {
    0x050000, 0x060000, 0x070000, 0x070101,
    0x070202, 0x060303, 0x050404, 0x040404,
    0x030304, 0x020204, 0x020206, 0x010108,
    0x010109, 0x000007, 0x010009, 0x010006,
    0x020006, 
    0x000100, 0x000100, 0x000100
  };
int rainbow[LEDS_PER_RING], cold2hot[LEDS_PER_RING];

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
#define PATTERN -1
int ringColours[]{RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, PATTERN, PATTERN};


void initLEDs(void) 
{
  // set up multicoloured patterns
  for (int i=0;i<LEDS_PER_RING;i++)
  {
    int dst = 8-i;
    if (dst < 0) dst += LEDS_PER_RING;
    rainbow[dst] = rainbow1[i];
    cold2hot[dst]= cold2hot1[i];
  }

  rings.setPattern(7,rainbow);
  rings.setPattern(6,cold2hot);

  // starting colours
  rings.begin();
  rings.clear();
  for (int i=0;i<NUM_POTS; i++)
    rings.setPixel(i,10,colours[i]);
  rings.show();
}


/*
 Simplest possible UI - a dot at the nearest position to 
 the given value (which should have a range of ±1.0)
 */
extern uint8_t keyStatuses[NUM_POTS];

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
