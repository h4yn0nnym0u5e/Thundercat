#include "header.h"

FlexIOSPI SPIFLEX(11, 12, 13, -1); // Setup on (int mosiPin, int misoPin, int sckPin, int csPin=-1) :

#define TFT_CS_PIN  8
TFT_eSPI tft = TFT_eSPI(240,320,SPIFLEX,TFT_CS_PIN);
#define TFT_BL      14

uint16_t cmap[16];

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
  tft.fillScreen(TFT_NAVY);
}


void setup() 
{
  initTFT(tft);

}

int x,y,a,img;
void pushGrid(void)
{
  int16_t colour = tft.color565(random(16,256),random(16,256),random(16,256));
  for (int i=0;i<16;i++)
    cmap[i] = tft.alphaBlend(i*16,colour,TFT_BLACK);
  tft.pushImage(x,y,
                scene_info.width,scene_info.height,
                scene_info.data,
                false, cmap);
  x += 60;
  if (x >= 300)
  {
    x = 0;
    y += 60;
    if (y >= 240)
      y = 0;
  }
}

const image_4bit_info* images[] = {
    &scene_info,
    &bulb_info,
    &cross_info,
    &gears_info,
    &note_info,
    &rainbow_info,
    &runner_info,
    &specs_info};

void pushCircle(void)
{
  const image_4bit_info& oneOfTheInfos = *images[img];
  img++;
  if (img > 7)
    img = 0;

  int16_t colour = tft.color565(random(16,256),random(16,256),random(16,256));
  for (int i=0;i<16;i++)
    cmap[i] = tft.alphaBlend(i*16,colour,TFT_BLACK);

  int xa = 90*sinf(a*PI/180.0f) + 90,
      ya = 90*cosf(a*PI/180.0f) + 90;
  tft.pushImage(xa,ya,
                oneOfTheInfos.width,oneOfTheInfos.height,
                oneOfTheInfos.data,
                false, cmap);
  a += 45;
  if (a >= 360)
    a -= 359;
}

void loop() 
{
  // pushGrid();
  pushCircle();
}
