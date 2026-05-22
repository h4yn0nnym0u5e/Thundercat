#include <TFT_eSPI.h>
#include "headers.h"

// define this to use DMA to write the rectangles
#define noUSE_DMA

/*
 * Use 74LVC138 decoder to provide /CS signal to one of
 * 8 displays, using only 4 Teensy outputs
 */
[[maybe_unused]] static void CSfn(int which, bool negate)
{
  if (negate)
    digitalWriteFast(MUX_G,1);
  else
  {
    digitalWriteFast(MUX_A,(which&1)!=0);
    digitalWriteFast(MUX_B,(which&2)!=0);
    digitalWriteFast(MUX_C,(which&4)!=0);
    digitalWriteFast(MUX_G,0);
  }
}


#define TFT_TYPE TFT_eSPI

const char* libString = "TFT_eSPI";
TFT_eSPI tft1{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(0, negate); }};
TFT_eSPI tft2{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(1, negate); }};
TFT_eSPI tft3{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(2, negate); }};
TFT_eSPI tft4{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(3, negate); }};
TFT_eSPI tft5{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(4, negate); }};
TFT_eSPI tft6{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(5, negate); }};
TFT_eSPI tft7{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(6, negate); }};
TFT_eSPI tft8{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(7, negate); }};
const char* busString = "SPI";

#define INIT begin()
static uint16_t colours[]{TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_BLUE, TFT_MAGENTA, TFT_VIOLET};


TFT_TYPE* tfts[] = {&tft1, &tft2, &tft3, &tft4, &tft5, &tft6, &tft7, &tft8};

const char* DMAuse =
#if defined(USE_DMA)
  "Using DMA"
#else  
  "Not using DMA"
#endif // defined(USE_DMA)
  ;


static void initDisplayPins(void)
{
  if (TFT_RST < 0)
    Serial.printf("TFT_RST set to %d - probably an error!\n", TFT_RST);
    
  pinMode(TFT_BLK,arduino::OUTPUT);
  pinMode(TFT_RST,arduino::OUTPUT);
  pinMode(MUX_A,arduino::OUTPUT);
  pinMode(MUX_B,arduino::OUTPUT);
  pinMode(MUX_C,arduino::OUTPUT);
  pinMode(MUX_G,arduino::OUTPUT);

  // turn backlight off
  digitalWriteFast(TFT_BLK, arduino::LOW);

  // reset display
  digitalWriteFast(TFT_RST, arduino::HIGH);
  delay(1);
  digitalWriteFast(TFT_RST, arduino::LOW);
  delay(1);
  digitalWriteFast(TFT_RST, arduino::HIGH);
}

#define ALL_TFTS for (int i=0;i<8;i++) (*tfts[i])
#define FN_TFTS(fn) for (int i=0;i<8;i++) fn(*tfts[i],i)

static void fillUnique(TFT_TYPE& tft, int i)
{
  tft.fillScreen(colours[i]);  
}

static void initUnique(TFT_TYPE& tft, int i)
{
  elapsedMillis em = 0;
  tft.INIT;
//  SPI.debugFlag = false;
  Serial.printf("Init %d - took %dms\n", i, (int) em);
  fillUnique(tft, i);
}

#if defined(ST7789_PHASED) // TFT_eSPI set up for phased init
elapsedMicros eu;
static bool doAphase(int i, int& phase)
{
  int newPhase = tfts[i]->phasedInit(0, phase);
  if (newPhase != phase)
  {
//    Serial.printf("%d: tft #%d -> phase %d\n", (int) eu, i, newPhase);
    phase = newPhase;    
  }

  return newPhase < 0;
}

static void phasedInit(void)
{
  elapsedMillis em = 0;
  int phases[8]{0};
  bool finished = false;

  eu = 0;
  while (!finished)
  {
    finished = true;
    for (int i=0;i<8;i++)
    {
      finished &= doAphase(i,phases[i]);
    }
  }
  Serial.printf("Phased init - took %dms\n", (int) em);
  FN_TFTS(fillUnique);
}

#endif // defined(ST7789_PHASED)

//=========================================================================================
static void setupScribble() 
{
//  halt_cpu();
  Serial.println("Started");
  initDisplayPins();

  // turn backlight on
  //digitalWriteFast(TFT_BLK, HIGH);
  
  // standard setup
  //ALL_TFTS.INIT;
  //SPI.debugFlag = true;
#if defined(ST7789_PHASED) // TFT_eSPI set up for phased init
  phasedInit();
#else
  FN_TFTS(initUnique);
#endif // defined(ST7789_PHASED)

  // set backlights to half-power
  for (int i=0;i<129;i++)
  {
    analogWrite(TFT_BLK,i);
    delay(5);
  }
  delay(100);
 
  
#if defined(USE_FLEXIO_SPI)
  // This gives us a base clock of 120MHz:
  SPIflex.flexIOHandler()->setClock(120'000'000.0f);

  uint32_t clk = SPIflex.flexIOHandler()->computeClockRate();
  Serial.printf("Updated Flex IO speed: %u; SPI clock will be an integer division of %u\n", clk, clk/2);
#endif // defined(USE_FLEXIO_SPI)
  
  //tft.fillScreen(0);
  FN_TFTS(fillUnique);

  // this will depend on your hardware!
  //tft.setRotation(1);
  ALL_TFTS.setRotation(TFT_ROTATION);
  //tft.setSPISpeed(60'000'000);
  
#if defined(USE_DMA)  
  ALL_TFTS.initDMA();
#endif // defined(USE_DMA)  
}

//-----------------------------------------------------------
// moved outside function so we can inspect
// in TeensyDebug when it crashes!
int x,y, w, h;
uint16_t* r;

static void randomRect(
#if defined(_TFT_eSPIH_)
  TFT_eSPI&
#else
  ST7789_t3&  
#endif // defined(_TFT_eSPIH_)
  tft, int i = -1
  )
{
  // w = random(140); h = random(80);
  w = random(80); h = random(80);
  uint16_t colour = random(65536);

  do
  {
    x = random(tft.width());
    y = random(tft.height());
  } while (x+w > tft.width() || y+h > tft.height());
  
#if defined(USE_DMA)
  // create sprite to draw the rectangle, and draw it
  TFT_eSprite sprite{&tft};
  sprite.createInPSRAM(1);//random(100) > 49); // maybe create in PSRAM
  r = (uint16_t*) sprite.createSprite(w,h);
// Serial.printf("sprite data at %08X\n", (uint32_t) r);  
  sprite.fillSprite(colour);

  // write to the display using DMA
  tft.startWrite();
  tft.pushImageDMA(x,y,w,h,r);
  tft.dmaWait(); // could do something useful here
  tft.endWrite();
  
#else

  tft.fillRect(x,y,w,h,colour);
  //vTaskDelay(2);
#endif // defined(USE_DMA)  

#define SZ 4  
  //tft.setFreeFont(&FreeSans18pt7b);
  tft.setFreeFont(&FreeSansBold24pt7b);
  //tft.setTextSize(SZ);
  tft.setTextColor(0);
  //tft.setCursor(120-SZ*3,120-SZ*4);
  tft.setCursor(120-SZ*3,120);
  tft.print(i+1);

  //vTaskDelay(1);
}

//=========================================================================================
int rectCount;
static void loopScribble() 
{
//  if (0 == rectCount % 9600)
//    Serial.printf("\n%d: ", millis());
//*    
  FN_TFTS(randomRect);
/*/ 
  for (int i=0;i<8;i++)
    randomRect(*tfts[i]);
//*/  
  rectCount += 8;
  if (0 == rectCount % 96)
  {
//    Serial.print('.');
  }
}

static void taskScribble(void*)
{
  setupScribble();
  while (1)
  {
    loopScribble();
    vTaskDelay(1);
  }
}

TaskHandle_t handleScribble;
void initScribble(void)
{
Serial.printf("Create Scribble task: \n");    
  xTaskCreate(taskScribble, "Scribble", 512, nullptr, 2, &handleScribble);
}
