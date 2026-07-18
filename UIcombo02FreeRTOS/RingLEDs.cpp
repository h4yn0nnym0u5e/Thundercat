/* 
 Simple ring LED support

 We have 8 rings of 20 LEDs, which for historical
 reasons go clockwise from 216° ("north" being 0°),
 and the first ring is on pot 8, i.e. the
 rightmost one.

 Derived from WS2812Serial BasicTest Example
 code which is in the public domain. 
*/

#include "headers.h"
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
  LEDring<NUM_POTS> ring;
  ContinuousPot& myPot;
  int colour;
  int* pattern;
  struct {
    TFT_TYPE& tft;
    uint16_t& colour;
    uint16_t& bkgnd;
    uint16_t& textColour;
  } tft;
} ringConfigs[NUM_POTS]
{
  {{rings, 0}, allPots[0],xRED,    nullptr,  {*tfts[0], colours[0], bkgnds[0], textColours[0]}},
  {{rings, 1}, allPots[1],xORANGE, nullptr,  {*tfts[1], colours[1], bkgnds[1], textColours[1]}},
  {{rings, 2}, allPots[2],xYELLOW, nullptr,  {*tfts[2], colours[2], bkgnds[2], textColours[2]}},
  {{rings, 3}, allPots[3],xGREEN,  nullptr,  {*tfts[3], colours[3], bkgnds[3], textColours[3]}},
  {{rings, 4}, allPots[4],xBLUE,   nullptr,  {*tfts[4], colours[4], bkgnds[4], textColours[4]}},
  {{rings, 5}, allPots[5],xPURPLE, nullptr,  {*tfts[5], colours[5], bkgnds[5], textColours[5]}},
  {{rings, 6}, allPots[6],xPINK,   cold2hot, {*tfts[6], colours[6], bkgnds[6], textColours[6]}},
  {{rings, 7}, allPots[7],xWHITE,  rainbow,  {*tfts[7], colours[7], bkgnds[7], textColours[7]}}
};


/*
 Draw an arc from the start to 
 the given value (which should have a range of ±1.0)
 */
int bright = 39; // level 5

void setDot(LEDring<NUM_POTS>& ring, float value, uint32_t colour)
{
  if (0 == bright)
  {
    ring.clear();
  }
  else
  {
    const int firstLED = 12;

    if (value < -1.0f) value = -1.0f;
    if (value > +1.0f) value = +1.0f;

    value = value*(17*18 / 2 - 0.02f); // angle: ±152.8
    if (value < 0.0f)
      value += 360.0f; // 207.2 minimum
    //rings.clear(ringNum);

    if (echoOnce && 0 == ring.ring)
    {
      ring.debug = true;
      echoOnce = false;
    }

    ring.setArc(18.0f*firstLED - 8.99f, 8*18+8.99f, BLACK);
    ring.setArc(18.0f*firstLED - 8.99f, value, colour, bright);
    ring.ensurePixelVisible(firstLED,colour);
    if (ring.debug)
      Serial.println();
    ring.debug = false;

    ring.setPixel(10, keyStatuses[ring.ring]?xWHITE:BLACK,bright);
  }
}



/*
 * This task will actually end up handling the whole strip UI,
 * not just the rings and touches. We will also need to devise
 * a mechanism for generating the relevant MIDI output in a 
 * timely manner (the UI can get updated afterwards).
 */
TaskHandle_t handlesRings[NUM_POTS];
TaskHandle_t handleRing0;
uint8_t bits = 0;
void taskRing(void* pcfg)
{
  ringConfig_t& cfg = *((ringConfig_t*) pcfg);

  cfg.ring.clear();
  cfg.ring.setPattern(cfg.pattern); // null pointer is OK here

  while (1)
  {
    // Serial.printf("%u: ring task; bits: %02X\n", xTaskGetTickCount(), bits);
    uint8_t mask = bits & (1<<(NUM_POTS - 1 - cfg.ring.ring));
    cfg.ring.setPixel(9,mask?cfg.colour:BLACK,bright);

    setDot(cfg.ring, cfg.myPot.getCurrent(), nullptr == cfg.pattern?cfg.colour:PATTERN);

    if (0 == cfg.ring.ring)
      rings.show();

    vTaskDelay(10);
  }
}


static constexpr size_t STACK_SIZE{512};
//static DMAMEM StackType_t RingStacks[NUM_POTS][STACK_SIZE];
//static DMAMEM StaticTask_t RingTasks[NUM_POTS];
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

  // starting colours
  rings.begin();
  rings.clear();
  for (int i=0;i<NUM_POTS; i++)
    rings.setPixel(i,10,ringConfigs[i].colour);
  rings.show();

  // xTaskCreate(taskRings, "Rings", 1024, nullptr, 2, &handleRings);

  // create one task per ring
  // the task name gets copied, so we can create it dynamically
  for (int i=0;i<NUM_POTS;i++)
  {
    char buf[configMAX_TASK_NAME_LEN+1]; // from FreeRTOSconfig.h
    sprintf(buf,"Ring%d",i);
//Serial.printf("Create %s task: \n", buf);    
    xTaskCreate(taskRing, buf, 512, ringConfigs+i, 2, handlesRings+i);
    //handlesRings[i] = xTaskCreateStatic(taskRing, buf, STACK_SIZE, ringConfigs+i, 2, 
    //            RingStacks[i], RingTasks+i);
  }
  handleRing0 = handlesRings[0];
}
