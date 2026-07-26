/* WS2812Serial BasicTest Example
   Adapted to test button class
 */

//==================================================================================
#include <WS2812Serial.h>

const int numled = 20;
const int pin = 20;

// Usable pins:
//   Teensy 4.1:  1, 8, 14, 17, 20, 24, 29, 35, 47, 53

static byte buttonLEDmemory[numled*3];         //  3 bytes per LED
static DMAMEM byte displayMemory[numled*12]; // 12 bytes per LED

static WS2812Serial buttonLEDstring(numled, displayMemory, buttonLEDmemory, pin, WS2812_GRB);

#define RED    0xE00000
#define GREEN  0x008000
#define BLUE   0x0000FF
#define YELLOW 0xA07000
#define PINK   0xFF1088
#define ORANGE 0xE04000
#define WHITE  0x707070
#define PURPLE 0x4000FF
#define CYAN   0x00C0E0

uint32_t colours[]{RED,GREEN,BLUE,YELLOW,PINK,ORANGE,WHITE,PURPLE};

//==================================================================================
class ButtonLED
{
    WS2812Serial& ledString;
    uint8_t* ledMemory;
    int num;
  public:
    ButtonLED(WS2812Serial& _string, uint8_t* mem, int n)
        : ledString{_string}, ledMemory{mem}, num{n}
        {}
          
    void setColour(uint32_t c)    { ledString.setPixel(num,c); }
    void show(void)               { ledString.show(); }
    void setBrightness(uint8_t n) { ledString.setBrightness(n); }
};

#define CREATE_BUTTON_LEDS // allow definition of button LEDs
#include "expanders.h"
//==================================================================================

void setup() 
{
  buttonLEDstring.begin();
  //buttonLEDstring.setBrightness(9);
  buttonLEDstring.setBrightness(50);
}

void loop() 
{
  //doAllWipes();
  //bleedTest();
  buttonLEDtest();
}
//==================================================================================

void buttonLEDtest(void)
{
  buttonLED_PCB1.setColour(WHITE);
  buttonLED_PCB2.setColour(WHITE);
  buttonLED_PCB3.setColour(WHITE);
  buttonLED_PCB4.setColour(WHITE);

  buttonLED_REAR_FN.setColour(PINK);
  buttonLED_MAIN_FN.setColour(PURPLE);

  buttonLED_NAV_XY.setColour(RED);
  buttonLED_NAV_OP.setColour(ORANGE);
  buttonLED_NAV_DN.setColour(YELLOW);
  buttonLED_NAV_UP.setColour(GREEN);
  
  buttonLED_KEYSW1.setColour(BLUE);
  buttonLED_KEYSW2.setColour(CYAN);
  buttonLED_KEYSW3.setColour(BLUE);
  buttonLED_KEYSW4.setColour(BLUE);
  buttonLED_KEYSW5.setColour(BLUE);
  buttonLED_KEYSW6.setColour(BLUE);
  buttonLED_KEYSW7.setColour(BLUE);
  buttonLED_KEYSW8.setColour(BLUE);

  buttonLEDstring.show();
}

void bleedTest(void)
{
  for (int i=0;i<7;i++)
    buttonLEDstring.setPixel(i*3, colours[i]);
  buttonLEDstring.show();
}

void doAllWipes(void)
{
  // change all the LEDs in 1.5 seconds
  int microsec = 1'500'000 / buttonLEDstring.numPixels();

  colorWipe(RED, microsec);
  colorWipe(GREEN, microsec);
  colorWipe(BLUE, microsec);
  colorWipe(YELLOW, microsec);
  colorWipe(PINK, microsec);
  colorWipe(ORANGE, microsec);
  colorWipe(WHITE, microsec);
}

void colorWipe(int color, int wait) {
  for (int i=0; i < buttonLEDstring.numPixels(); i++) {
    buttonLEDstring.setPixel(i, color);
    buttonLEDstring.show();
    delayMicroseconds(wait);
  }
}


