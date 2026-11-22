/* WS2812Serial BasicTest Example
   Adapted to test button class
 */

//==================================================================================
#include <WS2812Serial.h>
#include <SPI.h>

const int numled = 20;
const int pin = 14; // 20;

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
#include "ButtonTest.h"
#define CREATE_BUTTON_LEDS // allow definition of button LEDs
#include "expanders.h"     // define them
#include "dpex.h"
//==================================================================================

void setup() 
{
  initDPEX();
  SET_BIT_NOW(POWER_LED, 1);


#define EN_6V 25  
  // enable 6V    
  delay(1000);
  pinMode(EN_6V, OUTPUT);
  digitalWriteFast(EN_6V, HIGH);
  delay(1000);

  buttonLEDstring.begin();
  //buttonLEDstring.setBrightness(9);
  buttonLEDstring.setBrightness(10);
}

void loop() 
{
  //doAllWipes();
  //bleedTest();
  buttonLEDtest();
}
//==================================================================================
int offset;
void buttonLEDtest(void)
{
  static elapsedMillis em = 0;

  if (em >= 500)
  {
    em = 0;
    buttonLED_PCB1.setColour(colours[(offset + 0) % 8]);
    buttonLED_PCB2.setColour(colours[(offset + 1) % 8]);
    buttonLED_PCB3.setColour(colours[(offset + 2) % 8]);
    buttonLED_PCB4.setColour(colours[(offset + 3) % 8]);
    offset++;

    U3.poll(); U5.poll();
    Serial.printf("U3: %02X; U5: %02X\n", (U3.getGPIO()>>8)^0xFF, ((U5.getGPIO()>>8) | 0xC0)^0xFF);
  }

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


