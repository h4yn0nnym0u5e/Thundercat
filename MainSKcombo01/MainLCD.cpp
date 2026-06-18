/*
 * Test of Composer Pro main LCD
 * Gnd  Gnd Black
 * Vcc  3V3 Red
 * SCK  13  Purple
 * MOSI 11  Grey
 * MISO 12  Brown
 * /RST  9  Orange
 * DC   10  White
 * /CS   8  Blue
 * BLK  14  Green; active high
 * SCL  16  Yellow
 * SDA  17  Black (again!)
 * /INT 15  Red (again!)
 */
#include "headers.h"

#define USE_FLEXIOSPI


//---------------------------------------------------------
#if defined(USE_FLEXIOSPI)

#include <FlexIO_t4.h> 
#include <FlexIOSPI.h>

FlexIOSPI SPIFLEX(11, 12, 13, -1); // Setup on (int mosiPin, int misoPin, int sckPin, int csPin=-1) :

#define TFT_SPI_BUS SPIFLEX
#define TFT_SPI_BUS_TEXT "FlexSPI"
#else
#define TFT_SPI_BUS SPI
#define TFT_SPI_BUS_TEXT "SPI"
#endif // defined(USE_FLEXIOSI)
//---------------------------------------------------------

// Uses Setup405_Teensy_ST7789_Flex.h 
#include <TFT_eSPI.h>
#include "logo.h"

//=========================================================
// Change pin numbers to suit your hardware!
//                        CS DC RST
//                       { 8,10, 9}
#define TFT_CS_PIN 8
TFT_eSPI tft = TFT_eSPI(240,320,TFT_SPI_BUS,TFT_CS_PIN);
#define TFT_BL      14
#define TFT_CTP_INT 15
#define TFT_CTP_I2C Wire2

// off-screen version
TFT_eSprite sprite(&tft);
TFT_eSprite spritePos(&tft);
TFT_eSprite spriteButtons(&tft);
int spaceOffset;
int posX = 170, posY = 180;
uint16_t theBuffer[320*240]; // intermediate buffer
uint16_t TRANSPARENT = 0x0020; // transparent colour (very dark green)

//---------------------------------------------------------
static void xrandomRect(void)
{
 /* 
  if (!tft.asyncUpdateActive())
  {
 */
    // choose new rectangle dimensions
    int16_t x=999,y=999,w,h;
    w = random(tft.width() / 2);
    h = random(tft.height() / 2);
    while (x+w >= tft.width())
      x = random(tft.width());
    while (y+h >= tft.height())
      y = random(tft.height());

    // write to frame buffer
  //  tft.clearChangedArea();
    tft.fillRect(x,y,w,h,random(65536));    
 /*    
    tft.changeAsyncClipArea();

    // update changed area only to screen
    tft.updateScreenAsync(false,true);
  }
  */
}


void randomRect(TFT_eSPI& tft, uint16_t& x, uint16_t& y, uint16_t& w, uint16_t& h)
{
  // choose new rectangle dimensions
  x=999; y=999;
  
  w = random(tft.width() / 2);
  h = random(tft.height() / 2);
  while (x+w >= tft.width())
    x = random(tft.width());
  while (y+h >= tft.height())
    y = random(tft.height());

  tft.fillRect(x,y,w,h,random(65536));
}


//---------------------------------------------------------
extern const logo_t gimp_image, gimp_image2;

void drawLogo(TFT_eSprite& dst, const logo_t& logo, int x = -1, int y = -1)
{
  if (-1 == x) x = (dst.width() - logo.width) / 2;
  if (-1 == y) y = (dst.height() - logo.height) / 2;;
  
  bool swapped = dst.getSwapBytes();

#if defined(USE_FLEXIOSPI)
  dst.setSwapBytes(!swapped);
#endif // defined(USE_FLEXIOSI)

  dst.pushImage(x,y,
                logo.width, logo.height,
                logo.pixel_data,
                0, 0x0040);
  dst.setSwapBytes(swapped);    
}


//---------------------------------------------------------
void copyAreaToBuffer(TFT_eSprite& sprt, uint16_t* dst, 
                      int x, int y, int w, int h)
{
  uint16_t* src = (uint16_t*) sprt.getPointer();

  src = src + y*sprt.width() + x;
  for (;h>0;h--)
  {
    for (int w0 = 0; w0 < w; w0++)
      *dst++ = src[w0];
    src += sprt.width();      
  }
}

//=========================================================
static void startMainLCD() 
{
  // standard TFT display setup
  tft.init();
  //tft.setSPISpeed(60'000'000);
  tft.setRotation(1);
  tft.invertDisplay(true);
  //tft.setSwapBytes(true);
  
  pinMode(TFT_BL,arduino::OUTPUT);
  digitalWriteFast(TFT_BL,arduino::HIGH);
  tft.fillScreen(0);
  tft.setTextColor(TFT_YELLOW);
  tft.println("Hello world");
  tft.setTextColor(TFT_RED); tft.print("Red ");
  tft.setTextColor(TFT_GREEN); tft.print("Green ");
  tft.setTextColor(TFT_BLUE); tft.print("Blue");
  
#if defined(USE_FLEXIOSPI)
  // See if we can update the speed...
  //SPIFLEX.flexIOHandler()->setClockSettings(2, 1, 7);	// clksel(0-3PLL4, Pll3 PFD2 PLL5, *PLL3_sw)
  Serial.printf("Flex IO speed: %u\n", SPIFLEX.flexIOHandler()->computeClockRate());
#endif // defined(USE_FLEXIOSI)

/*
  // now prepare to use async area updates
  tft.setAsyncInterruptPriority(224); 
  tft.useIntermediateBuffer(tft.width() * 10 * 2);
  tft.useFrameBuffer(true);
  tft.updateChangedAreasOnly(true);
*/
  sprite.createSprite(320,240);
  sprite.setSpriteSwapBytes(
#if defined(USE_FLEXIOSPI)
    true
#else
    false    
#endif // defined(USE_FLEXIOSI)
    );

  spritePos.createSprite(tft.width() - posX - 5,45);
  spritePos.setSpriteSwapBytes(
#if defined(USE_FLEXIOSPI)
    true
#else
    false    
#endif // defined(USE_FLEXIOSI)
    );
  spritePos.setFreeFont(&FONT_DP); // this is the font we're using
  spaceOffset = spritePos.textWidth("-0") - spritePos.textWidth(" 0");

  spriteButtons.createInPSRAM(true);
  spriteButtons.createSprite(320,45);
  spriteButtons.setSpriteSwapBytes(
#if defined(USE_FLEXIOSPI)
    true
#else
    false    
#endif // defined(USE_FLEXIOSI)
    );
  spriteButtons.fillScreen(TRANSPARENT);

  tft.initDMA(TFT_CS_PIN);
  uint16_t* sprite_data = (uint16_t*) sprite.getPointer();
  
  showGamut(sprite);
  Serial.printf("TFT settings: %s; SPI on %s\n", USER_SETUP_INFO, TFT_SPI_BUS_TEXT);
  Serial.printf("First pixel is %04X\n", sprite_data[0]);
  /*
  sprite.pushSprite(0,0);
  /*/
  tft.startWrite();
  tft.pushImageDMA(0, 0, tft.width(), tft.height(), sprite_data);
  tft.endWrite();
  //*/
/*  
  tft.changeAsyncClipArea();
  // update changed area only to screen
  elapsedMicros eus = 0;
  tft.updateScreenAsync(false,true);
  tft.waitUpdateAsyncComplete();

  uint32_t tupd = eus;
  Serial.printf("Async screen fill took %dus\n", tupd);
 */
  //delay(1500);
}

//=========================================================
static int last_position;
static float last_sub;
static bool updateMainLCD() 
{
  uint16_t x,y,w,h;
   
  randomRect(sprite, x,y,w,h);
  
  drawLogo(sprite, gimp_image);
  drawLogo(sprite, gimp_image2,80,140);
  spritePos.pushToSprite(&sprite, posX, posY);
  spriteButtons.pushToSprite(&sprite, 0,0, 
#if defined(USE_FLEXIOSPI)
    TRANSPARENT
#else
    SWAP(TRANSPARENT)
#endif // defined(USE_FLEXIOSI)
  );
  
  copyAreaToBuffer(sprite, theBuffer, x,y,w,h);
  tft.startWrite();
  tft.pushImageDMA(x,y,w,h, theBuffer);
  //tft.dmaWait();

  return true;
}


char positionText[10];
bool positionUpdated;
static bool updatePosition(void)
{
  bool doUpdate = false;

  if (1 == whichKnob) // pitch bend - analogue
  {
    if (fabs(last_sub - sub_position) >= 0.001f)
    {
      last_sub = sub_position;
      float show = last_sub;
      if (show < 0.000f && show > -0.0005f)
        show = 0.0f;
      sprintf(positionText,"%.3f", show);
      doUpdate = true;
    }
  }
  else // digital
  {
    if (last_position != current_position)
    {
      last_position = current_position;
      sprintf(positionText,"%d", last_position);
      doUpdate = true;
    }
  }

  if (doUpdate)
  {
    uint16_t x = posX, y = posY,
      w = spritePos.width(),h = spritePos.height();

    spritePos.fillScreen(TFT_DARKGREY);
    spritePos.setFreeFont(&FONT_DP);
    spritePos.setTextColor(TFT_LIGHTGREY);
    spritePos.setTextDatum(TR_DATUM);
    //spritePos.drawString(buf, (buf[0] == ' '?spaceOffset:0) + 3, 3);
    spritePos.drawString(positionText, w - 5, 3);

    copyAreaToBuffer(spritePos, theBuffer, 0,0,w,h);
    tft.startWrite();
    tft.pushImageDMA(x,y,w,h, theBuffer);

  }

  positionUpdated |= doUpdate;

  return doUpdate;
}


static uint16_t hues[]{TFT_VIOLET, TFT_YELLOW, TFT_ORANGE, TFT_CYAN, TFT_GREEN};
static const char* names[]{"Unbounded", "Pitch", "Fine", "Coarse", "Coarse"};
static const char* detents[]{"none", nullptr, nullptr, "strong", "weak"};
static void updateButtons(void)
{
  int bw = 64, bh = 45;

  //spriteButtons.setFreeFont(&FreeSans12pt7b);
  //spriteButtons.setTextFont(3);
  spriteButtons.setTextColor(TFT_BLACK);
  //spriteButtons.setTextDatum(CC_DATUM);
  for (int i=0;i<COUNT_OF(hues);i++)
  {
    spriteButtons.fillRoundRect(3+i*bw,3,bw-6,bh-5, 3, hues[i]);
    spriteButtons.drawRoundRect(3+i*bw,3,bw-6,bh-5, 3, TFT_BLACK);
    //spriteButtons.drawString(names[i], 3+i*bw + bw/2, 3 + bh/2);

    int tw = spriteButtons.textWidth(names[i]);
    spriteButtons.setCursor(3+i*bw + (bw-6-tw)/2,bh/2 - 5);
    spriteButtons.print(names[i]);

    if (nullptr != detents[i])
    {
      tw = spriteButtons.textWidth(detents[i]);
      spriteButtons.setCursor(3+i*bw + (bw-6-tw)/2,bh/2 - 5 + 8);
      spriteButtons.print(detents[i]);
    }
  } 
}

TaskHandle_t handleMainLCD;
static void taskMainLCD(void* params)
{
  bool writing;
  startMainLCD();
  updateButtons();

  while (1)
  {
    if (!tft.dmaBusy())
    {
      if (writing)
      {
        tft.endWrite();
        writing = false;
      }

      // update position display if it's changed...
      writing = updatePosition();

      // ...otherwise draw another rectangle
      if (!writing)
        writing = updateMainLCD();
    }
    vTaskDelay(2);
  }
}


void initMainLCD(void)
{
  xTaskCreate(taskMainLCD, "MainLCD", 512, nullptr, 2, &handleMainLCD);
}
