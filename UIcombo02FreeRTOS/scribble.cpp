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
#define TFT_ORANGE2      0xFD00      /* 255, 128,   0 */

uint16_t colours[]{TFT_RED, TFT_ORANGE2, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_BLUE, TFT_MAGENTA, TFT_VIOLET};
uint16_t bkgnds[NUM_POTS];
uint16_t textColours[NUM_POTS];
int spaceOffset; // leading space numbers are narrower by this much vs. leading minus


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
  for (int i=0;i<256;i+=2)
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

  // Work out spaceOffset value
  // We have to use " 0" since " " apparently has zero width! Bug, methinks...
  tft1.setFreeFont(&FONT_DP); // this is the font we're using
  spaceOffset = tft1.textWidth("-0") - tft1.textWidth(" 0");
}


//-----------------------------------------------------------------------------------
static float lastPots[NUM_POTS];
static bool  lastTouches[NUM_POTS];
static constexpr float POT_NOT_SET = -999.0f;
static const float sa = 2*18.0f, ea = 360.0f - 2*18.0f; // TFT_eSPI has zero at 6 o'clock

static void drawArc(TFT_TYPE& tft, float s, float e, uint16_t fg, uint16_t bkgnd)
{
  tft.drawArc(120, 120, 110, 80, s+sa, e+sa, fg, bkgnd);
}

static void drawTouch(TFT_TYPE& tft, uint16_t colour)
{
  tft.fillEllipse(120,210,24,16,colour);
}

void setArc(TFT_TYPE& tft, 
            float newPot, float& lastPot, char* buf,
            int8_t touch, bool& lastTouch,
            int fg, int bg, int txt)
{
  const float CHANGE_THRESHOLD = CHGTHR_DP;
  if (fabs(newPot - lastPot) > CHANGE_THRESHOLD)
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

    int x = 55+10*(SCRIBBLE_DP - 3), y = 100, w = 140-15*(SCRIBBLE_DP - 3), h = 45;
 #if defined(USE_DMA)
    // create sprite to draw the current level, and draw it
    TFT_eSprite sprite{&tft};
    sprite.setSpriteSwapBytes(false);
    sprite.createInPSRAM(1); // create in PSRAM

    uint16_t* r = (uint16_t*) sprite.createSprite(w,h);

    sprite.fillRect(0,0,w,h,bg); //fillSprite(bkgnds[i]);

    sprite.setFreeFont(&FONT_DP);
    sprite.setTextColor(txt);
    sprite.drawString(buf, buf[0] == ' '?spaceOffset:0, 0);

    // write to the display using DMA
    tft.startWrite(); //##############################################
    tft.pushImageDMA(x,y,w,h,r);
    // wait for DMA to complete - we get notified by the completionISR()
    // when that occurs, and this task resumes execution
    uint32_t ulNotifiedValue = ulTaskNotifyTake(pdFALSE, 1000);
    if (0 == ulNotifiedValue) { /* panic! */}
    tft.dmaWait();  // tidy up after DMA - shouldn't actually wait
    tft.endWrite(); //##############################################
  
 #else
    // this works, but flickers
    tft.setCursor(x,y+35);
    tft.fillRect(x,y,w,h,bg);
    tft.print(buf);
 #endif // defined(USE_DMA)

    lastPot = newPot;
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

  char buf[30];
  if (potPos < 0.0f && potPos > -CHGTHR_DP / 100.0f) potPos = 0.0f; // don't show -0.000
  sprintf(buf,FMT_DP,potPos);
  setArc(tft, newPot, lastPots[i], buf, keyStatuses[i], lastTouches[i],
         colours[i], bkgnds[i], textColours[i]);
}


//=========================================================================================
int rectCount;
TaskHandle_t handleScribble;

// This runs after DMA completes, in an ISR context.
// We use it to notify the Scribble task that the 
// SPI bus and DMA are now idle, and the transaction 
// can be ended (or more stuff can be done).
static void completionISR(TFT_TYPE& tft)
{
  BaseType_t xHigherPriorityTaskWoken;
  vTaskNotifyGiveFromISR( handleScribble, &xHigherPriorityTaskWoken );
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static void loopScribble() 
{
  FN_TFTS(ssetArc); // poll, updating all the displays
}

static void taskScribble(void*)
{
  setupScribble();
  for (int i=0; i<NUM_POTS; i++)
  {
    lastPots[i] = POT_NOT_SET;
    bkgnds[i]      = tft1.alphaBlend( 70 /* / 255 */, colours[i], TFT_BLACK);
    textColours[i] = tft1.alphaBlend( 80 /* / 255 */, colours[i], TFT_WHITE);
    tfts[i]->setFreeFont(&FONT_DP);
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
void initScribble(void)
{
  xTaskCreate(taskScribble, "Scribble", STACK_SIZE, nullptr, 2, &handleScribble);
}
