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
#include <initGT911.h>


//================================================================
FlexIOSPI SPIFLEX(11, 12, 13, -1); // Setup on (int mosiPin, int misoPin, int sckPin, int csPin=-1) :

#define TFT_CS_PIN  8
TFT_eSPI tft = TFT_eSPI(240,320,SPIFLEX,TFT_CS_PIN);
#define TFT_BL      14
#define TFT_CTP_INT 15
#define TFT_CTP_I2C Wire2

void initTFT(TFT_eSPI& tft)
{
  // standard TFT display setup
  tft.init();
  //tft.setSPISpeed(60'000'000);
  tft.setRotation(1);
  tft.invertDisplay(true);
  //tft.setSwapBytes(true);
  
  pinMode(TFT_BL,OUTPUT);
  digitalWriteFast(TFT_BL,HIGH);
  tft.fillScreen(TFT_BLACK);
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
uint16_t markHue(TFT_eSPI& tft, 
             int cx, int cy,  // selection ring centre...
             int cr,          // ...and radius:outer...
             int cri,         // ...and inner
             int mr,          // marker radius
             float a)         // conventional angle, ±pi
{
  int crm = (cr+cri)/2;
  int mx = crm * cosf(oldAngle) + cx, my = cy - crm * sinf(oldAngle);
  //tft.drawCircle(mx,my,mr,TFT_BLACK);
  tft.drawArc(mx,my,mr,mr-2, 0,360, TFT_BLACK,TFT_BLACK);
  for (int i=-10;i<11;i++)
  {
    int tftAngle = rad2TFT(oldAngle)+i;
    int hueAngle = tftAngle + 180;
    tftAngle = (tftAngle + 360) % 360;
    tft.drawArc(cx,cy,cr,cri, tftAngle, tftAngle+1, angleToHue(tft, hueAngle), TFT_BLACK);
  }

  oldAngle = a;
  mx = crm * cosf(oldAngle) + cx, my = cy - crm * sinf(oldAngle);
  //tft.drawCircle(mx,my,mr,TFT_LIGHTGREY);
  int tftAngle = rad2TFT(oldAngle);
  int hueAngle = tftAngle + 180;
  uint16_t hue = angleToHue(tft, hueAngle);
  tft.drawArc(mx,my,mr,mr-2, 0,360, TFT_LIGHTGREY,hue);

  return hue;
}

bool isOldAngle(float a)
{
  return fabs(a - oldAngle) < PI/180;
}

//================================================================
extern void processTouch(int n);
extern uint8_t updateGT911touch();
extern void startGT911touch();
extern GTPoint lastTouch;

int hueX = 120, hueY = 120;

void setup() 
{
  Serial1.begin(115200);

  while (!Serial)
    ;
  Serial.println("\n\nStarting");
  startGT911touch();
  initTFT(tft);

  hueCircle(tft,hueX,hueY, 100,80, TFT_BLACK);
  //hueCircle(tft,160,120,  75,60, TFT_BLACK);
  gradients(tft, 240,270, 20,20,200, TFT_RED);
}


//================================================================
elapsedMillis em;
void loop() 
{
  if (em >= 5)
  {
    if (updateGT911touch() > 0)  
    {
      processTouch(0);
      int dx = lastTouch.x - hueX, dy = lastTouch.y - hueY;
      float touchRadius = sqrtf(dx*dx+dy*dy);
      float touchAngle = atan2(hueY - lastTouch.y, lastTouch.x - hueX);
      if (touchRadius < 105.0f && !isOldAngle(touchAngle))
      {
        uint16_t hue = markHue(tft, hueX,hueY, 100,80, 13, touchAngle);
        gradients(tft, 240,270, 20,20,200, hue);
        Serial.printf("%d: %d,%d; %.3f, %d\n", millis(), lastTouch.x, lastTouch.y, touchAngle, rad2TFT(touchAngle));
      }
    }
  }
}
