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
//#include <TeensyDebug.h>
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
#endif // defined(USE_FLEXIOPSI)
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
uint16_t theBuffer[320*240]; // intermediate buffer

//---------------------------------------------------------
void xrandomRect(void)
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

  /*
  if (w*h < 2)
    Serial.printf("x: %d; y: %d; w: %d; h: %d\n",x,y,w,h);
  */
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
#endif // defined(USE_FLEXIOPSI)

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


void dumpDMA_TCDx(DMAChannel *dmabc) {
    if (nullptr != dmabc)
    {
      Serial.printf("%x (%d) %x:", (uint32_t)dmabc, dmabc->channel, (uint32_t)dmabc->TCD);

      Serial.printf("SA:%x SO:%d AT:%x NB:%x SL:%d DA:%x DO: %d CI:%x DL:%x CS:%x BI:%x\n", (uint32_t)dmabc->TCD->SADDR,
                    dmabc->TCD->SOFF, dmabc->TCD->ATTR, dmabc->TCD->NBYTES, dmabc->TCD->SLAST, (uint32_t)dmabc->TCD->DADDR,
                    dmabc->TCD->DOFF, dmabc->TCD->CITER, dmabc->TCD->DLASTSGA, dmabc->TCD->CSR, dmabc->TCD->BITER);
    }
}

#define DUMP(r) Serial.printf(#r ": %08x\n", r)
#define NVIC_PENDING0 (* (uint32_t*) 0xE000'E200)
void dumpStuff(void)
{
  DUMP(NVIC_ISER0);
  DUMP(NVIC_PENDING0);
#if defined(USE_FLEXIOSPI_AND_DMA_IS_PUBLIC)
  dumpDMA_TCDx(SPIFLEX._dmaTX);
  dumpDMA_TCDx(SPIFLEX._dmaRX);
#endif // defined(USE_FLEXIOPSI)
}

//=========================================================
void setup() 
{
  while (!Serial)
    ;

  // standard TFT display setup
  tft.begin();
  //tft.setSPISpeed(60'000'000);
  tft.setRotation(1);
  tft.invertDisplay(true);
  //tft.setSwapBytes(true);
  
  pinMode(TFT_BL,OUTPUT);
  digitalWriteFast(TFT_BL,HIGH);
  tft.fillScreen(0);
  tft.setTextColor(TFT_YELLOW);
  tft.println("Hello world");
  tft.setTextColor(TFT_RED); tft.print("Red ");
  tft.setTextColor(TFT_GREEN); tft.print("Green ");
  tft.setTextColor(TFT_BLUE); tft.print("Blue");
  delay(1500);
  
#if defined(USE_FLEXIOSPI)
  // See if we can update the speed...
  //SPIFLEX.flexIOHandler()->setClockSettings(2, 1, 7);	// clksel(0-3PLL4, Pll3 PFD2 PLL5, *PLL3_sw)
  Serial.printf("Flex IO speed: %u\n", SPIFLEX.flexIOHandler()->computeClockRate());
#endif // defined(USE_FLEXIOPSI)

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
#endif // defined(USE_FLEXIOPSI)
    );
  tft.initDMA(TFT_CS_PIN);
  uint16_t* sprite_data = (uint16_t*) sprite.getPointer();

  //halt_cpu();
  
  showGamut(sprite);
  Serial.printf("TFT settings: %s; SPI on %s\n", USER_SETUP_INFO, TFT_SPI_BUS_TEXT);
  Serial.printf("First pixel is %04X\n", sprite_data[0]);
  /*
  sprite.pushSprite(0,0);
  /*/
  tft.startWrite();
  //dumpStuff();
  tft.pushImageDMA(0, 0, tft.width(), tft.height(), sprite_data);
  //dumpStuff();
  tft.dmaWait();
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
  delay(1500);
}

//=========================================================
void loop() 
{
  uint16_t x,y,w,h;
  /*
  randomRect(tft, x,y,w,h);
  /*/
  if (!tft.dmaBusy())
  {
    tft.endWrite();
    
    randomRect(sprite, x,y,w,h);
    
    if (false && w*h < 16)
    {
      Serial.printf("w: %d; h: %d\n", w, h); 
      Serial.flush();
    }
    drawLogo(sprite, gimp_image);
    drawLogo(sprite, gimp_image2,85,140);

    
    copyAreaToBuffer(sprite, theBuffer, x,y,w,h);
    tft.startWrite();
    tft.pushImageDMA(x,y,w,h, theBuffer);
    //tft.dmaWait();
  }
  //*/
}
