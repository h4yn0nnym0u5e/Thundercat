/*
 * Using ST7789 display on FlexIOSPI
 */

 /*
 * Composer Pro main LCD:
 * ======================
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
#include <TFT_eSPI.h>
#include <FlexIO_t4.h> 
#include <FlexIOSPI.h>
#include "header.h"


//================================================================
//         888 d8b                   888                   
//         888 Y8P                   888                   
//         888                       888                   
//     .d88888 888 .d8888b  88888b.  888  8888b.  888  888 
//    d88" 888 888 88K      888 "88b 888     "88b 888  888 
//    888  888 888 "Y8888b. 888  888 888 .d888888 888  888 
//    Y88b 888 888      X88 888 d88P 888 888  888 Y88b 888 
//     "Y88888 888  88888P' 88888P"  888 "Y888888  "Y88888 
//                          888                        888 
//                          888                   Y8b d88P 
//                          888                    "Y88P"  
//
/*
// Prototype settings
FlexIOSPI SPIflex(11, 12, 13, -1); // Setup on (int mosiPin, int misoPin, int sckPin, int csPin=-1) :

#define TFT_CS_PIN  8
TFT_eSPI tft = TFT_eSPI(240,320,SPIflex,TFT_CS_PIN);
TFT_eSprite sprite{&tft}; // sprite for off-screen rendering
uint16_t* imageBuffer;    // extra buffer to serialise area of sprite
#define TFT_BL      14
#define TFT_CTP_INT 15
/*/

// Main PCB settings
FlexIOSPI SPIflex(MAINLCD_SPI_PINS, -1); // Setup on (int mosiPin, int misoPin, int sckPin, int csPin=-1) :

#define TFT_CS_PIN  MAINLCD_CS
TFT_eSPI tft = TFT_eSPI(240,320,SPIflex,TFT_CS_PIN);
TFT_eSprite sprite{&tft}; // sprite for off-screen rendering
uint16_t* imageBuffer;    // extra buffer to serialise area of sprite
#define TFT_BL      MAINLCD_BL
#define TFT_CTP_INT CTP_INT
//*/

// callback executed within ISR when DMA SPI transfer completes
static void TFTdmaDoneCB(FlexIOSPI* pFlex)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE; 

  vTaskNotifyGiveFromISR(handleMainLCD, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );  
}


// block until DMA is complete
static void TFTdmaWait(void)
{
  // wait for notification from async TFT_eSPI library
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  

  // now we can...
  tft.dmaWait();  // ...tidy up...
  tft.endWrite(); // ...and release the SPI bus
}


void initTFT(TFT_eSPI& tft, TFT_eSprite& spr)
{
  // standard TFT display setup
  tft.init();

  // FlexIOSPI-specific stuff --------------------------------------------
  // This gives us a base clock of 120MHz:
  SPIflex.flexIOHandler()->setClock(120'000'000.0f);

  uint32_t clk = SPIflex.flexIOHandler()->computeClockRate();
  Serial.printf("Updated Flex IO speed: %u; SPI clock will be an integer division of %u\n", clk, clk/2);
  SPIflex.setTransferCallback(TFTdmaDoneCB);
  // ---------------------------------------------------------------------


  //tft.setSPISpeed(60'000'000);
  tft.setRotation(1);
  tft.invertDisplay(true);
  //tft.setSwapBytes(true);
  
  pinMode(TFT_BL,arduino::OUTPUT);
  digitalWriteFast(TFT_BL,arduino::HIGH);
  tft.fillScreen(TFT_BLACK);

  // initialise a sprite
  // Needs 153'600 bytes for a 320x240 display
  spr.createSprite(tft.width(), tft.height());
  //spr.invertDisplay(true);
  spr.setSpriteSwapBytes(false);
  imageBuffer = (uint16_t*) malloc(115*60*sizeof(uint16_t)); // hack hack..

  // allow DMA
  tft.initDMA();
}
//================================================================
bool spriteAreaToBuffer(TFT_eSprite& spr, uint16_t* dst, int x, int y, int w, int h)
{
  bool result = false;
  uint16_t* src = (uint16_t*) spr.getPointer();
  int sw = spr.width();

  if (nullptr != src && nullptr != dst)
  {
    uint16_t* img = src+y*sw+x; // first pixel
    while (h)
    {
      /*
      memcpy(dst,img,w*sizeof *img); // copy a line
      /*/
      // we have to do byte swapping here, library seems broken
      for (int i=0;i<w;i++)
        dst[i] = (img[i] << 8) | ((img[i] >> 8) & 0xFF); 
      //*/
      dst += w;   // next free buffer area
      img += sw;  // next line in sprite data
      h--;
    }
    result = true;
  }

  return result;
}

//================================================================
// Return hue based on angle: 0=red, 60=yellow etc
uint16_t angleToHue(TFT_eSPI& tft, int a)
{
  uint16_t result = TFT_BLACK;
  if (a < 0) a += 720; // deal with reasonable negative angles
  a %= 360;
  int s = a/60;
  a = a - s*60;
  a = 255-a*255/60;

  switch (s)
  {
    case 0: result = tft.alphaBlend(a,TFT_RED,    TFT_YELLOW ); break;
    case 1: result = tft.alphaBlend(a,TFT_YELLOW, TFT_GREEN  ); break;
    case 2: result = tft.alphaBlend(a,TFT_GREEN,  TFT_CYAN   ); break;
    case 3: result = tft.alphaBlend(a,TFT_CYAN,   TFT_BLUE   ); break;
    case 4: result = tft.alphaBlend(a,TFT_BLUE,   TFT_MAGENTA); break;
    case 5: result = tft.alphaBlend(a,TFT_MAGENTA,TFT_RED    ); break;
    default:
      break;
  }
  return result;
}

// N.B. TFT_eSPI angles for arcs have 0 at the 6 o'clock position
void hueCircle(TFT_eSPI& tft, int x, int y, int r, int ir, uint16_t bgcolour)
{
  for (int ii=0;ii<60;ii++)
  {
    int i = ii;
    int a = i + 181;

    tft.drawArc(x,y,r,ir, (i    )%360,(i+  1)%360, angleToHue(tft,a    ), bgcolour); // red - yellow
    tft.drawArc(x,y,r,ir, (i+ 60)%360,(i+ 61)%360, angleToHue(tft,a+ 60), bgcolour); // yellow - green
    tft.drawArc(x,y,r,ir, (i+120)%360,(i+121)%360, angleToHue(tft,a+120), bgcolour); // green - cyan
    tft.drawArc(x,y,r,ir, (i+180)%360,(i+181)%360, angleToHue(tft,a+180), bgcolour); // cyan - blue
    tft.drawArc(x,y,r,ir, (i+240)%360,(i+241)%360, angleToHue(tft,a+240), bgcolour); // blue - magenta
    tft.drawArc(x,y,r,ir, (i+300)%360,(i+301)%360, angleToHue(tft,a+300), bgcolour); // magenta - red
  }
}

void gradients(TFT_eSPI& tft, int x, int x2, int y, int w, int h, uint16_t c)
{
  tft.fillRectVGradient( x,y,w,h, TFT_WHITE, c);
  tft.fillRectVGradient(x2,y,w,h, TFT_BLACK, c);
}

//const float PI = 3.1415926535f;
// convert angle in radians to TFT_eSPI angle
int rad2TFT(float rad)
{
  return (int)(-90 + 360 - rad*180.0f/PI) % 360;
}

/*
 * Mark the selected hue.
 * Hue angle zero is at 12 o'clock, whereas TFT_eSPI zero is 6 o'clock,
 * and conventional at 3 o'clock and runs anticlockwise. Sigh.
 */
float oldAngle;
void unMarkHue(TFT_eSPI& tft, 
             int cx, int cy,  // selection ring centre...
             int cr,          // ...and radius:outer...
             int cri,         // ...and inner
             int mr,          // marker radius
             float a,         // conventional angle, ±pi
             int& mx,   // return screen position of marker
             int& my)
{
  int crm = (cr+cri)/2;
  mx = crm * cosf(oldAngle) + cx;
  my = cy - crm * sinf(oldAngle);

  //tft.drawCircle(mx,my,mr,TFT_BLACK);
  tft.drawArc(mx,my,mr,mr-2, 0,360, TFT_BLACK,TFT_BLACK);
  for (int i=-10;i<11;i++)
  {
    int tftAngle = rad2TFT(oldAngle)+i;
    int hueAngle = tftAngle + 180;
    tftAngle = (tftAngle + 360) % 360;
    tft.drawArc(cx,cy,cr,cri, tftAngle, tftAngle+1, angleToHue(tft, hueAngle), TFT_BLACK);
  }
}

uint16_t markHue(TFT_eSPI& tft, 
             int cx, int cy,  // selection ring centre...
             int cr,          // ...and radius:outer...
             int cri,         // ...and inner
             int mr,          // marker radius
             float a,         // conventional angle, ±pi
             int& mx,   // return screen position of marker
             int& my)
{
  int crm = (cr+cri)/2;
  oldAngle = a;
  mx = crm * cosf(oldAngle) + cx, my = cy - crm * sinf(oldAngle);
  //tft.drawCircle(mx,my,mr,TFT_LIGHTGREY);
  int tftAngle = rad2TFT(oldAngle);
  int hueAngle = tftAngle + 180;
  uint16_t hue = angleToHue(tft, hueAngle);
  tft.drawArc(mx,my,mr,mr-2, 0,360, TFT_LIGHTGREY,hue);

  return hue;
}


void asyncDrawHue(int mx, int my, int mr)
{
  spriteAreaToBuffer(sprite, imageBuffer, mx - mr - 1,my - mr - 1,mr*2+2,mr*2+2);
  tft.startWrite();
  tft.pushImageDMA(mx - mr - 1,my - mr - 1,mr*2+2,mr*2+2, imageBuffer);
  TFTdmaWait();
}


bool isOldAngle(float a)
{
  return fabs(a - oldAngle) < PI/180;
}

//================================================================
void drawFatRect(TFT_eSPI& tft, int x, int y, int w, int h, int t, int colour)
{
  if (2*t >= h || 2*t >= w) // huge thickness means...
    tft.fillRect(x,y,w,h,colour);
  else
  {
    tft.fillRect(x,    y,    w,t,    colour);
    tft.fillRect(x,    y+h-t,w,t,    colour);
    tft.fillRect(x,    y+t,  t,h-2*t,colour);
    tft.fillRect(x+w-t,y+t,  t,h-2*t,colour);
  }    
}


uint16_t getBlend(TFT_eSPI& tft, float l, uint16_t top, uint16_t hue)
{
  return tft.alphaBlend(l*255+0.5f, top, hue);
}


uint16_t markGradient(TFT_eSPI& tft, 
                      int x, int y, int w, int h, // gradient rectangle
                      uint16_t hue, uint16_t top, // colours
                      int d,                 // depth of marker
                      float l, float& oldL)   // fractional level (from bottom)
{
  int hh = y+(1-oldL)*h;

  if (l != oldL) // might be just changing hue
    drawFatRect(tft, x-2, hh-d/2, w+4, d+1, 2, TFT_BLACK);

  // draw part of the gradient using a viewport
  // needs a bug fix in TFT_eSPI
  tft.setViewport(x-2, hh-d/2, w+4, d+1, false);
  tft.fillRectVGradient(x,y,w,h,top,hue);
  tft.resetViewport();

  oldL = l;
  hh = y+(1-oldL)*h;
  drawFatRect(tft, x-2, hh-d/2, w+4, d+1, 2, top==TFT_WHITE?TFT_DARKGREY:TFT_LIGHTGREY);

  return getBlend(tft, l, top, hue);
}

bool isSameLevel(float level, float oldLevel, uint16_t hue, uint16_t top)
{
  /*
  Serial.printf("%.3f -> %.3f; %04X -> %04X\n", oldLevel, level,
          tft.alphaBlend(oldLevel*255+0.5f, top, hue),
          tft.alphaBlend(   level*255+0.5f, top, hue) 
  );
  */
  return getBlend(tft,    level, top, hue)
      == getBlend(tft, oldLevel, top, hue);
}


//================================================================
int16_t hue, textColour, bgColour;

// render what settings look like inside hue circle
// Hack hack - it's a 115x60 area
void drawSettingsExample(TFT_eSPI& tft)
{
  int yp = 100;
  // Main background:
  //tft.fillRect(65,yp,115,60, bgColour);
  // ... but save time by not overwriting with hue:
  tft.fillRect(65,yp,      115,60-10-15, bgColour);
  tft.fillRect(65,yp+60-10,115,   10   , bgColour);
  tft.fillRect(65,    yp+60-25, 10,   15   , bgColour);
  tft.fillRect(65+105,yp+60-25, 10,   15   , bgColour);
  //tft.setCursor(70,95);
  tft.setTextColor(textColour);
  tft.drawString("Text", 75, yp+5, 4);
  tft.fillRect(75,yp+60-10-15,95,15, hue);
}


void showColours(TFT_eSPI& tft)
{
  char buffer[20];
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(1);

  sprintf(buffer," Hue: %04X", hue & 0xFFFF);
  tft.fillRect(90,60,70,10,TFT_BLACK);
  tft.drawString(buffer, 90,60);

  sprintf(buffer,"Text: %04X", textColour & 0xFFFF);
  tft.fillRect(90,70,70,10,TFT_BLACK);
  tft.drawString(buffer, 90,70);

  sprintf(buffer,"Bgnd: %04X", bgColour & 0xFFFF);
  tft.fillRect(90,80,70,10,TFT_BLACK);
  tft.drawString(buffer, 90,80);
}

//================================================================
//    888                      888      
//    888                      888      
//    888                      888      
//    888888  8888b.  .d8888b  888  888 
//    888        "88b 88K      888 .88P 
//    888    .d888888 "Y8888b. 888888K  
//    Y88b.  888  888      X88 888 "88b 
//     "Y888 "Y888888  88888P' 888  888 
// 
/*
extern void processTouch(int n);
extern uint8_t updateGT911touch();
extern void startGT911touch();
*/
extern GTPoint lastTouch;

int hueX = 120, hueY = 120;
float textLevel = 0.66f, bgLevel=0.66f;

void startTaskMainLCD() 
{
  //Serial1.begin(115200); // kills SPI1 !!!

  while (!Serial)
    ;
  Serial.println("Starting display task");
  initTFT(tft, sprite);

  hueCircle(tft,hueX,hueY, 100,80, TFT_BLACK);
  hueCircle(sprite,hueX,hueY, 100,80, TFT_BLACK);

  int mx,my;
  hue = markHue(tft, hueX,hueY, 100,80, 13, PI/2, mx, my);
  //hueCircle(tft,160,120,  75,60, TFT_BLACK);
  gradients(tft, 240,280, 20,20,200, hue);

  textColour = markGradient(tft, 240,20,20,200, hue,TFT_WHITE, 9, textLevel, textLevel);
  bgColour = markGradient(tft, 280,20,20,200, hue,TFT_BLACK, 9, bgLevel, bgLevel);

  drawSettingsExample(tft);
  showColours(tft);
}

//=====================================================
#define TASK_LIST_ENTRY(tsk) extern TaskHandle_t handle##tsk;
TASK_LIST
#undef TASK_LIST_ENTRY

#define TASK_LIST_ENTRY(tsk) , &handle##tsk
TaskHandle_t* handles[]
  { nullptr, nullptr
    TASK_LIST
  };
#undef TASK_LIST_ENTRY

uint32_t screen_update_us;
void printTaskStates(void)
{
  TaskHandle_t handleIdle = xTaskGetIdleTaskHandle();
  handles[0] = &handleIdle;
  handles[1] = &freertos::g_yield_task;
  configRUN_TIME_COUNTER_TYPE idlePercent = ulTaskGetIdleRunTimePercent(),
                              idleCount = ulTaskGetIdleRunTimeCounter();
  float pct = idleCount * 100.0f / idlePercent; // 100% of counts to date
  SER_TERM.println();
  for (int i = 0;i < COUNT_OF(handles);i++)
  {
    TaskStatus_t s;
    vTaskGetInfo(*(handles[i]), &s, pdTRUE, eInvalid);
    SER_TERM.printf("Name '%s'; priority: %d; unused stack: %d; runtime %d (%.3f%%)\n",
              s.pcTaskName,
              s.uxCurrentPriority,
              s.usStackHighWaterMark,
              s.ulRunTimeCounter,
              (float) s.ulRunTimeCounter / pct * 100.0f
            );
  }
  SER_TERM.printf("Last screen update: %dus\n", screen_update_us);
  freertos::print_ram_usage();
}

void setIdlePin(bool b)
{
#if defined(IDLE_PIN)
  digitalWriteFast(IDLE_PIN,b);
#endif // defined(IDLE_PIN)
}


//================================================================
//                                                     
//    88888b.   .d88b.  888  888  888  .d88b.  888d888 
//    888 "88b d88""88b 888  888  888 d8P  Y8b 888P"   
//    888  888 888  888 888  888  888 88888888 888     
//    888 d88P Y88..88P Y88b 888 d88P Y8b.     888     
//    88888P"   "Y88P"   "Y8888888P"   "Y8888  888     
//    888                                              
//    888                                              
//    888                                              
//
TouchStatus powerButton;
void pollPowerButton(void)
{
  powerButton = !GET_BIT(SOFT_POWER);
  //powerButton = U5.getBit(5);
  if (powerButton.isChangedStatus())
  {
    static bool hadLongPress = false;
    TouchStatus::eStatus e = powerButton.getExtendedStatus();
    Serial.printf("Power button status is %d\n", e);

    switch (e)
    {
      default:
        break;

      case TouchStatus::eStatus::LONG:
        hadLongPress = true;
        Serial.println("Release to power off");
        break;

      case TouchStatus::eStatus::OFF:
        if (hadLongPress)
        {
          Serial.print("Off ... ");
          vTaskDelay(1000);
          Serial.print("6V ... ");
          digitalWriteFast(EN_6V, arduino::LOW); // 6V supply off
          vTaskDelay(1000);
          Serial.print("power LED ... ");
          SET_BIT(POWER_LED,0); // clear power LED B.1 output
          vTaskDelay(1000);
          Serial.println("shutdown!");
          SET_BIT(TOGGLE_POWER, 1); // shutdown!
          for (int i=0;i<100;i++)
          {
            Serial.print('.');
            vTaskDelay(10);
          }
        }
        break;
    }
  }
}


void initPower(void)
{
  pinMode(EN_6V, arduino::OUTPUT);
  digitalWriteFast(EN_6V, arduino::HIGH);
  delay(100);
  Serial.println("6V enabled");
  SET_BIT(POWER_LED, 1);
}

//================================================================
//         888 d8b                   888                   
//         888 Y8P                   888                   
//         888                       888                   
//     .d88888 888 .d8888b  88888b.  888  8888b.  888  888 
//    d88" 888 888 88K      888 "88b 888     "88b 888  888 
//    888  888 888 "Y8888b. 888  888 888 .d888888 888  888 
//    Y88b 888 888      X88 888 d88P 888 888  888 Y88b 888 
//     "Y88888 888  88888P' 88888P"  888 "Y888888  "Y88888 
//                          888                        888 
//                          888                   Y8b d88P 
//                          888                    "Y88P"  
//  
void taskMainLCD(void* params) 
{
  startTaskMainLCD();
  vTaskDelay(9);
  Serial.println("Main LCD task started");

  while (1)
  {
    // wait 5ms for notification from touch screen task
    uint32_t wasNotified = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5)); 

    if (wasNotified) // screen was touched - deal with it
    {
      // Are we in the colour circle - if so select hue
      bool drawExample = false;
      int dx = lastTouch.x - hueX, dy = lastTouch.y - hueY;
      float touchRadius = sqrtf(dx*dx+dy*dy);
      float touchAngle = atan2(hueY - lastTouch.y, lastTouch.x - hueX);
      if (touchRadius < 105.0f && !isOldAngle(touchAngle))
      {
        setIdlePin(1);
        drawExample = true;

        int mx,my, mr = 13;
        // hue = markHue(tft, hueX,hueY, 100,80, 13, touchAngle); // old - direct draw
        unMarkHue(sprite, hueX,hueY, 100,80, mr, touchAngle, mx, my);
        asyncDrawHue(mx,my,mr);
        hue = markHue(sprite, hueX,hueY, 100,80, mr, touchAngle, mx, my);
        asyncDrawHue(mx,my,mr);

        gradients(tft, 240,280, 20,20,200, hue);
        textColour = markGradient(tft, 240,20,20,200, hue,TFT_WHITE, 9, textLevel, textLevel);
        bgColour = markGradient(tft, 280,20,20,200, hue,TFT_BLACK, 9, bgLevel, bgLevel);
      }
      else
      {
        // see if we're in the text or background sliders
        if (lastTouch.x>=230 && lastTouch.y>=18 && lastTouch.y<=222)
        {
          setIdlePin(1);
          float l = (220 - lastTouch.y)/200.0f;
          l = constrain<float>(l,0.0f,1.0f);

          if (lastTouch.x<270)
          {
            if (!isSameLevel(l,textLevel, hue,TFT_WHITE))
            {
              textColour = markGradient(tft, 240,20,20,200, hue,TFT_WHITE, 9, l, textLevel);
              drawExample = true;
            }
          }
          else            
          {
            if (!isSameLevel(l,bgLevel, hue,TFT_BLACK))
            {
              bgColour = markGradient(tft, 280,20,20,200, hue,TFT_BLACK, 9, l, bgLevel);
              drawExample = true;
            }
          }
        }
      }

      if (drawExample)
      {
        elapsedMicros eu = 0;
        bool alreadyPushing = false;

        // Show hex values of colour settings
        //showColours(tft);
        showColours(sprite);
        if (spriteAreaToBuffer(sprite, imageBuffer, 90,60,70,30))
        {
          tft.startWrite();
          tft.pushImageDMA(90,60,70,30, imageBuffer);
          alreadyPushing = true;
        }
        
        // Show example of colours
        //drawSettingsExample(tft);
        drawSettingsExample(sprite);  // update sprite
        if (alreadyPushing)           
          TFTdmaWait(); // wait for running push to complete

        if (spriteAreaToBuffer(sprite, imageBuffer, 65,100,115,60))
        {
          /*
          tft.pushImage(65,100,115,60, imageBuffer);
          /*/
          tft.startWrite();
          tft.pushImageDMA(65,100,115,60, imageBuffer);
          //*/
        }
        screen_update_us = eu;
        TFTdmaWait();   // blocks this task until DMA completes
        //Serial.printf("%d: %d,%d; %.3f, %d\n", millis(), lastTouch.x, lastTouch.y, touchAngle, rad2TFT(touchAngle));
      }
      setIdlePin(0);
    }

    // get here every timeout (5ms or so); deal with Serial input
    int ch;

    ch = SER_TERM.read();
    switch (ch)
    {
      case -1:
      default: 
        break;

      case 't':
        printTaskStates();
        break;   
        
      case 'h':
        Serial.printf("Hue: %04X\n", imageBuffer[0]);  
        break;

      case '3':
        Serial.printf("U3: %04hX\n", U3.getGPIO());
        break;

      case '5':
        Serial.printf("U5: %04hX\n", U5.getGPIO());
        break;
    }

    // this would normally be done by "ADC" task,
    // as ADCs and port expanders are on SPI1
    U3.poll();
    U5.poll();

    // "Super" task, maybe?
    pollPowerButton();
  }
}

TaskHandle_t handleMainLCD;
void initMainLCD(void)
{
  xTaskCreate(taskMainLCD, "MainLCD", 512, nullptr, 2, &handleMainLCD);
}

//=========================================
//                      888                      
//                      888                      
//                      888                      
//    .d8888b   .d88b.  888888 888  888 88888b.  
//    88K      d8P  Y8b 888    888  888 888 "88b 
//    "Y8888b. 88888888 888    888  888 888  888 
//         X88 Y8b.     Y88b.  Y88b 888 888 d88P 
//     88888P'  "Y8888   "Y888  "Y88888 88888P"  
//                                      888      
//                                      888      
//                                      888      
// 
void setup(void)
{
#if defined(IDLE_PIN)
  pinMode(IDLE_PIN,arduino::OUTPUT);
#endif // defined(IDLE_PIN)

  while (!Serial)
    ;
  Serial.println("\n\nStarted...");
  initDPEX();
  initPower();
  initMainLCD();
  initGT911touch();

  Serial.println("Starting scheduler");
  vTaskStartScheduler();
}

void loop() {} // keep Arduino happy

#if defined(IDLE_PIN)
// Called from idle task: must NOT block!
void vApplicationIdleHook() 
{
  static elapsedMicros eu = 0;

  if (eu >= 50)
  {
    eu = 0;
    digitalToggleFast(IDLE_PIN);
  }
}
#endif // defined(IDLE_PIN)
