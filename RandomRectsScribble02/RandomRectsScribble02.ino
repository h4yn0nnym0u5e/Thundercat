/*
 * Random rectangle test - scribble board
 * 
 * Mostly OK with https://github.com/h4yn0nnym0u5e/TFT_eSPI/commit/09b91e2d0c1b15d87a19b1d1492efb7b84170f63
 *            and https://github.com/h4yn0nnym0u5e/FlexIO_t4/commit/406facb39919ece30699909d0c3a6ee15944adf9
 * but there's an issue with SPIClass + DMA ...            
 * ... no longer - use 
 *                https://github.com/h4yn0nnym0u5e/TFT_eSPI/commit/1116178d526b1f66e20ba2931b856fa7b1d080ad
 */
//#include <TeensyDebug.h> 

// define this for TFT_eSPI FlexIOSPI testing:
#define noUSE_FLEXIO_SPI

// define this to use DMA to write the rectangles
#define USE_DMA

/*
#include <ST7789_t3.h>
/*/
#include <TFT_eSPI.h>
//*/

#define TFT_BLK    9
#define MUX_A     14
#define MUX_B     15
#define MUX_C     16
#define MUX_G     17


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


#if defined(_TFT_eSPIH_) //=================================
#define TFT_TYPE TFT_eSPI
#define TFT_ROTATION 0
#define ST7789_CS -1, [](bool negate) { CSfn(2, negate); }

// TFT_RST should be set to -1 in hardware select file, 
// we need a different value
#if defined(TFT_RST)
#if TFT_RST >= 0
#error "TFT_eSPI will try to reset displays!"  
#endif
#undef TFT_RST
#endif // defined(TFT_RST)

#define TFT_RST    8

const char* libString = "TFT_eSPI";
#if defined(USE_FLEXIO_SPI) //------------------------------
#include <FlexIOSPI.h>
FlexIOSPI SPIflex{11,12,13,-1}; // mosi / miso / sck / cs
//TFT_eSPI tft{240,240,SPIflex,ST7789_CS};
TFT_eSPI tft1{240,240,SPIflex,-1, [](bool negate) { CSfn(0, negate); }};
TFT_eSPI tft2{240,240,SPIflex,-1, [](bool negate) { CSfn(1, negate); }};
TFT_eSPI tft3{240,240,SPIflex,-1, [](bool negate) { CSfn(2, negate); }};
TFT_eSPI tft4{240,240,SPIflex,-1, [](bool negate) { CSfn(3, negate); }};
TFT_eSPI tft5{240,240,SPIflex,-1, [](bool negate) { CSfn(4, negate); }};
TFT_eSPI tft6{240,240,SPIflex,-1, [](bool negate) { CSfn(5, negate); }};
TFT_eSPI tft7{240,240,SPIflex,-1, [](bool negate) { CSfn(6, negate); }};
TFT_eSPI tft8{240,240,SPIflex,-1, [](bool negate) { CSfn(7, negate); }};
const char* busString = "FlexIOSPI";
#else //----------------------------------------------------
//TFT_eSPI tft{240,240,SPI,ST7789_CS};
TFT_eSPI tft1{240,240,SPI,-1, [](bool negate) { CSfn(0, negate); }};
TFT_eSPI tft2{240,240,SPI,-1, [](bool negate) { CSfn(1, negate); }};
TFT_eSPI tft3{240,240,SPI,-1, [](bool negate) { CSfn(2, negate); }};
TFT_eSPI tft4{240,240,SPI,-1, [](bool negate) { CSfn(3, negate); }};
TFT_eSPI tft5{240,240,SPI,-1, [](bool negate) { CSfn(4, negate); }};
TFT_eSPI tft6{240,240,SPI,-1, [](bool negate) { CSfn(5, negate); }};
TFT_eSPI tft7{240,240,SPI,-1, [](bool negate) { CSfn(6, negate); }};
TFT_eSPI tft8{240,240,SPI,-1, [](bool negate) { CSfn(7, negate); }};
const char* busString = "SPI";
#endif //defined(USE_FLEXIO_SPI) //-------------------------
#define INIT begin()
uint16_t colours[]{TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_BLUE, TFT_MAGENTA, TFT_VIOLET};

#else //=====================================================
#define TFT_TYPE ST7789_t3
#define TFT_ROTATION 2
#define ST7789_CS [](bool negate) { CSfn(2, negate); }
const char* libString = "ST7789_t3";
const char* busString = "SPI";

#define TFT_DC    10
#define TFT_RST    8
#define INIT init(240,240)

// ST7789_t3 tft = ST7789_t3(ST7789_CS, TFT_DC, TFT_RST);
ST7789_t3 tft1 = ST7789_t3([](bool negate) { CSfn(0, negate); }, TFT_DC, -1);
ST7789_t3 tft2 = ST7789_t3([](bool negate) { CSfn(1, negate); }, TFT_DC, -1);
ST7789_t3 tft3 = ST7789_t3([](bool negate) { CSfn(2, negate); }, TFT_DC, -1);
ST7789_t3 tft4 = ST7789_t3([](bool negate) { CSfn(3, negate); }, TFT_DC, -1);
ST7789_t3 tft5 = ST7789_t3([](bool negate) { CSfn(4, negate); }, TFT_DC, -1);
ST7789_t3 tft6 = ST7789_t3([](bool negate) { CSfn(5, negate); }, TFT_DC, -1);
ST7789_t3 tft7 = ST7789_t3([](bool negate) { CSfn(6, negate); }, TFT_DC, -1);
ST7789_t3 tft8 = ST7789_t3([](bool negate) { CSfn(7, negate); }, TFT_DC, -1);
//ST7789_t3& tft = tft3;
uint16_t colours[]{ST77XX_RED, ST77XX_ORANGE , ST77XX_YELLOW, ST77XX_GREEN, 0x03FF, ST77XX_BLUE, ST77XX_MAGENTA, 0x915C};

#endif // defined(_TFT_eSPIH_) ==============================

TFT_TYPE* tfts[] = {&tft1, &tft2, &tft3, &tft4, &tft5, &tft6, &tft7, &tft8};

const char* DMAuse =
#if defined(USE_DMA)
  "Using DMA"
#else  
  "Not using DMA"
#endif // defined(USE_DMA)
  ;



void initDisplayPins(void)
{
  if (TFT_RST < 0)
    Serial.printf("TFT_RST set to %d - probably an error!\n", TFT_RST);
    
  pinMode(TFT_BLK,OUTPUT);
  pinMode(TFT_RST,OUTPUT);
  pinMode(MUX_A,OUTPUT);
  pinMode(MUX_B,OUTPUT);
  pinMode(MUX_C,OUTPUT);
  pinMode(MUX_G,OUTPUT);

  // turn backlight off
  digitalWriteFast(TFT_BLK, LOW);

  // reset display
  digitalWriteFast(TFT_RST, HIGH);
  delay(1);
  digitalWriteFast(TFT_RST, LOW);
  delay(1);
  digitalWriteFast(TFT_RST, HIGH);
}

#define ALL_TFTS for (int i=0;i<8;i++) (*tfts[i])
#define FN_TFTS(fn) for (int i=0;i<8;i++) fn(*tfts[i],i)

void fillUnique(TFT_TYPE& tft, int i)
{
  tft.fillScreen(colours[i]);  
}

void initUnique(TFT_TYPE& tft, int i)
{
  elapsedMillis em = 0;
  tft.INIT;
//  SPI.debugFlag = false;
  Serial.printf("Init %d - took %dms\n", i, (int) em);
  fillUnique(tft, i);
}

#if defined(ST7789_PHASED) // TFT_eSPI set up for phased init
elapsedMicros eu;
bool doAphase(int i, int& phase)
{
  int newPhase = tfts[i]->phasedInit(0, phase);
  if (newPhase != phase)
  {
//    Serial.printf("%d: tft #%d -> phase %d\n", (int) eu, i, newPhase);
    phase = newPhase;    
  }

  return newPhase < 0;
}

void phasedInit(void)
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

/*
 * Do phased init, but one at a time!
 */
void phasedInitWrong(void)
{
  elapsedMillis em = 0;

  for (int i=0;i<8;i++)
  {
    int phase = 0;
    while (phase >= 0)
      phase = tfts[i]->phasedInit(0,phase);
  }
  Serial.printf(" Sequential phased init - took %dms\n", (int) em);
  FN_TFTS(fillUnique);
}

#endif // defined(ST7789_PHASED)

//=========================================================================================
void setup() 
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

void randomRect(
#if defined(_TFT_eSPIH_)
  TFT_eSPI&
#else
  ST7789_t3&  
#endif // defined(_TFT_eSPIH_)
  tft, int i = -1
  )
{
  w = random(140); h = random(80);
  uint16_t colour = random(65536);

  do
  {
    x = random(tft.width());
    y = random(tft.height());
  } while (x+w > tft.width() || y+h > tft.height());
  
#if defined(USE_DMA)
  // create sprite to draw the rectangle, and draw it
  TFT_eSprite sprite{&tft};
  sprite.createInPSRAM(random(100) > 49); // maybe create in PSRAM
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
  
#endif // defined(USE_DMA)  

#define SZ 4  
  //tft.setFreeFont(&FreeSans18pt7b);
  tft.setFreeFont(&FreeSansBold24pt7b);
  //tft.setTextSize(SZ);
  tft.setTextColor(0);
  //tft.setCursor(120-SZ*3,120-SZ*4);
  tft.setCursor(120-SZ*3,120);
  tft.print(i+1);
}

//=========================================================================================
int rectCount;
void loop() 
{
  if (0 == rectCount % 9600)
    Serial.printf("\n%d: ", millis());
//*    
  FN_TFTS(randomRect);
/*/ 
  for (int i=0;i<8;i++)
    randomRect(*tfts[i]);
//*/  
  rectCount += 8;
  if (0 == rectCount % 96)
  {
    Serial.print('.');
  }
}

extern uint8_t external_psram_size;
extern "C"
void startup_late_hook(void)
{
  while (!Serial)
    ;
#if !defined(USER_SETUP_INFO)
#define USER_SETUP_INFO "ST7789_t3"
#endif // !defined(USER_SETUP_INFO)

  if (CrashReport)
    Serial.println(CrashReport);
    
  Serial.printf("\nLate hook\nLibrary: %s; bus: %s; settings: " USER_SETUP_INFO "; %s",libString, busString, DMAuse);
#if defined(ARDUINO_TEENSY41)
  Serial.printf("\n%dMB PSRAM fitted", external_psram_size);
#endif // defined(ARDUINO_TEENSY41)
  Serial.println();
}
