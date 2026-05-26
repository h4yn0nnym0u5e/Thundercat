#include <TFT_eSPI.h>
#include "headers.h"
#include "contPot.h"

// define this to use DMA to write the rectangles
#define USE_DMA

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


const char* libString = "TFT_eSPI";
TFT_TYPE tft1{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(0, negate); }};
TFT_TYPE tft2{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(1, negate); }};
TFT_TYPE tft3{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(2, negate); }};
TFT_TYPE tft4{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(3, negate); }};
TFT_TYPE tft5{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(4, negate); }};
TFT_TYPE tft6{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(5, negate); }};
TFT_TYPE tft7{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(6, negate); }};
TFT_TYPE tft8{240,240,SCRIBBLE_SPI,-1, [](bool negate) { CSfn(7, negate); }};
const char* busString = "SPI";

#define INIT begin()
// #define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */ actually 248,180,0
#define TFT_ORANGE2      0xFD00      /* 255, 128,   0 */

uint16_t colours[]{TFT_RED, TFT_ORANGE2, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_BLUE, TFT_MAGENTA, TFT_VIOLET};
uint16_t bkgnds[NUM_POTS];
uint16_t textColours[NUM_POTS];


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
  vTaskDelay(1);
  digitalWriteFast(TFT_RST, arduino::LOW);
  vTaskDelay(1);
  digitalWriteFast(TFT_RST, arduino::HIGH);
}

#define ALL_TFTS for (int i=0;i<8;i++) (*tfts[i])
#define FN_TFTS(fn) for (int i=0;i<8;i++) fn(*tfts[i],i)

static void fillUnique(TFT_TYPE& tft, int i)
{
  tft.fillScreen(colours[i]);  
}

[[maybe_unused]] static void initUnique(TFT_TYPE& tft, int i)
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
    vTaskDelay(1);
  }
  Serial.printf("Phased init - took %dms\n", (int) em);
  FN_TFTS(fillUnique);
}

#endif // defined(ST7789_PHASED)

//=========================================================================================
static void setupScribble() 
{
  Serial.println("Started");
  initDisplayPins();
 
  // standard setup
  phasedInit();

  // set backlights to half-power
  for (int i=0;i<129;i++)
  {
    analogWrite(TFT_BLK,i);
    vTaskDelay(5);
  }
  vTaskDelay(100);
 

#if defined(USE_FLEXIO_SPI)
  // This gives us a base clock of 120MHz:
  SPIflex.flexIOHandler()->setClock(120'000'000.0f);

  uint32_t clk = SPIflex.flexIOHandler()->computeClockRate();
  Serial.printf("Updated Flex IO speed: %u; SPI clock will be an integer division of %u\n", clk, clk/2);
#endif // defined(USE_FLEXIO_SPI)
  
  FN_TFTS(fillUnique);

  // this will depend on your hardware!
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

[[maybe_unused]] static void randomRect(TFT_TYPE& tft, int i = -1)
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

//-----------------------------------------------------------------------------------
static float lastPots[NUM_POTS];
static bool  lastTouches[NUM_POTS];
static constexpr float POT_NOT_SET = -999.0f;
static const float sa = 2*18.0f, ea = 360.0f - 2*18.0f; // TFT_eSPI has zero at 6 o'clock

static void drawArc(TFT_TYPE& tft, float s, float e, uint16_t fg, uint16_t bkgnd)
{
  tft.drawArc(120, 120, 110, 80, s+sa, e+sa, fg, bkgnd);
  //Serial.printf("%.1f-%.1f; %04X", s+sa, e+sa, fg);
}

static void drawTouch(TFT_TYPE& tft, uint16_t colour)
{
  tft.fillEllipse(120,210,24,16,colour);
  //Serial.printf("touch: %04X\n", colour);
}

void setArc(TFT_TYPE& tft, 
            float newPot, float& lastPot, char* buf,
            int8_t touch, bool& lastTouch,
            int fg, int bg, int txt)
{
  if (fabs(newPot - lastPot) > 0.5f)
  {
    //Serial.printf("Pot %d: ", i);
    if (POT_NOT_SET == lastPot)
    {
      tft.fillScreen(bg);
      drawArc(tft, 0.0f, ea-sa, TFT_BLACK, bg);
      lastPot = 0.0f;
      lastTouch = touch;
    }

    if (newPot < lastPot)
      drawArc(tft, newPot, lastPot, TFT_BLACK, bg);
    else
      drawArc(tft, lastPot, newPot, fg, bg);

 #if defined(USE_DMA)
    // create sprite to draw the current level, and draw it
    int x = 65, y = 100, w = 120, h = 50;

    TFT_eSprite sprite{&tft};
    sprite.setSpriteSwapBytes(false);
    sprite.createInPSRAM(1); // create in PSRAM

    uint16_t* r = (uint16_t*) sprite.createSprite(w,h);

    sprite.fillRect(0,0,w,h,bg); //fillSprite(bkgnds[i]);

    sprite.setFreeFont(&FreeSansBold24pt7b);
    sprite.setTextColor(txt);
    sprite.setCursor(0,35);
    sprite.print(buf);

    // write to the display using DMA
    tft.startWrite();
    tft.pushImageDMA(x,y,w,h,r);
    //vTaskDelay(250); // delay task until DMA is complete or timeout
    //vTaskSuspend(nullptr); // suspend task until DMA is complete
    uint32_t ulNotifiedValue = ulTaskNotifyTake( pdFALSE, 1000);
    if (0 == ulNotifiedValue) { /* panic! */}
    tft.dmaWait();   // could do something useful here
    tft.endWrite();
  
 #else
    // this works, but flickers
    tft.setCursor(70,140);
    tft.fillRect(70,105,120,50,bg);
    tft.print(buf);
 #endif // defined(USE_DMA)

    lastPot = newPot;
    //Serial.println();
  }

  if (lastTouch != touch)
  {
    drawTouch(tft, touch?fg:bg);
    lastTouch = touch;
  }
}

static void ssetArc(TFT_TYPE& tft, int i = -1)
{
  // get new value in degrees, relative to start angle:
  float potPos = allPots[i].getCurrent(); // -1.0 to +1.0
  float newPot = (potPos + 1.0f) * (ea - sa) / 2.0f;

  char buf[10];
  sprintf(buf,"%5.2f",potPos);

  setArc(tft, newPot, lastPots[i], buf, keyStatuses[i], lastTouches[i],
         colours[i], bkgnds[i], textColours[i]);
}
//=========================================================================================
int rectCount;
TaskHandle_t handleScribble;
static void completionISR(TFT_TYPE& tft)
{
//  if (nullptr != handleScribble)
//    xTaskResumeFromISR(handleScribble);
  BaseType_t xHigherPriorityTaskWoken;
  vTaskNotifyGiveFromISR( handleScribble, &xHigherPriorityTaskWoken );
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static void loopScribble() 
{
  //FN_TFTS(randomRect);
  FN_TFTS(ssetArc);
}

static void taskScribble(void*)
{
  setupScribble();
  for (int i=0; i<NUM_POTS; i++)
  {
    lastPots[i] = POT_NOT_SET;
    bkgnds[i]      = tft1.alphaBlend( 70 /* / 255 */, colours[i], TFT_BLACK);
    textColours[i] = tft1.alphaBlend( 80 /* / 255 */, colours[i], TFT_WHITE);
    tfts[i]->setFreeFont(&FreeSansBold24pt7b);
    tfts[i]->setTextColor(textColours[i], bkgnds[i], true);

 #if defined(USE_DMA)  
    tfts[i]->dmaAttachCompletionISR(completionISR);
 #endif // defined(USE_DMA)
   }

  while (1)
  {
    loopScribble();
    vTaskDelay(5);
  }
}

static constexpr size_t STACK_SIZE{512};
//static DMAMEM StackType_t ScribbleStack[STACK_SIZE];
//static DMAMEM StaticTask_t ScribbleTask;
void initScribble(void)
{
//Serial.printf("Create Scribble task: \n");    
  xTaskCreate(taskScribble, "Scribble", 512, nullptr, 2, &handleScribble);
  //handleScribble = xTaskCreateStatic(taskScribble, "Scribble", STACK_SIZE, nullptr, 2,
  //                                   ScribbleStack, &ScribbleTask);
}
