/* 
 Simple ring LED support

 We have 8 rings of 20 LEDs, which for historical
 reasons go clockwise from 216° ("north" being 0°),
 and the first ring is on pot 8, i.e. the
 rightmost one.

 Derived from WS2812Serial BasicTest Example
 code which is in the public domain. 
*/

#include "header.h"

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
    0xFF0000, 0xF00800, 0xE02000, 0xD03000,
    0xC04000, 0xB05000, 0xA06000, 0x908000,
    0x80A000, 0x60C000, 0x20D000, 0x00FF00,
    0x00F020, 0x008040, 0x004080, 0x0000FF,
    0x2000F0, 
    0x4000E0, 0x6000A0, 0x8000A0
  };
int cold2hot1[LEDS_PER_RING]   = {
    0xA00000, 0xC00000, 0xFF0000, 0xF02020,
    0xF04040, 0xC06060, 0xA08080, 0x808080,
    0x606080, 0x404090, 0x4040C0, 0x2020F0,
    0x2020FF, 0x0000FF, 0x2000F0, 0x2000C0,
    0x4000B0, 
    0x001000, 0x001000, 0x001000
  };
int rainbow[LEDS_PER_RING], cold2hot[LEDS_PER_RING];

//*
#define xRED    0xFF0000
#define xORANGE 0xC02000
#define xYELLOW 0xB09000
#define xGREEN  0x00A000 // be a bit conservative here
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

//int colours[]{RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, PINK, WHITE};
//int colours2[]{xRED, xORANGE, xYELLOW, xGREEN, xBLUE, xPURPLE, xPINK, xWHITE};
#define BLACK  0x000000
#define PATTERN -1
//int ringColours[]{RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, PATTERN, PATTERN};


struct ringConfig_t
{
  int  colour;
  int* pattern;
} ringConfigs[NUM_POTS]
{
  {xRED,    nullptr},
  {xORANGE, nullptr},
  {xYELLOW, nullptr},
  {xGREEN,  nullptr},
  {xBLUE,   nullptr},
  {xPURPLE, nullptr},
  {xPINK,   cold2hot},
  {xWHITE,  rainbow}
};


/*
 Draw an arc from the start to 
 the given value (which should have a range of ±1.0)
 */
//int bright = 39; // level 5

/*
 * This task just updates the rings periodically
 * If necessary we could trigger it only if there's a change.
 */
void RingLEDsTask::run(void)
{
  // set up multicoloured patterns
  for (int i=0;i<LEDS_PER_RING;i++)
  {
    int dst = 8-i;
    if (dst < 0) dst += LEDS_PER_RING;
    rainbow[dst] = rainbow1[i];
    cold2hot[dst]= cold2hot1[i];
  }

  // this would normally be a configuration / scene option:
  for (int i=0;i<LEDS_PER_RING;i++)
  {
    faderMonsterSettings.stripsConfig.colours[6].ringLEDs.pattern[i] = cold2hot[i];
    faderMonsterSettings.stripsConfig.colours[7].ringLEDs.pattern[i] = rainbow[i];
  }

  // starting colours
  rings.begin();
  rings.clear();
  for (int i=0;i<NUM_POTS; i++)
    rings.setPixel(i,10,ringConfigs[i].colour);
  rings.show();

  while (1)
  {
    rings.show();
    vTaskDelay(10); // every 10 ticks - should be triggered?
  }
}

RingLEDsTask ringLEDsTask{"RingLEDs", 512, nullptr, 2};
