void safeCSpins(void)
{
  const int CSpins[] = {28,29,30,31,32};
  for (int i =0;i<5;i++)
  {
    pinMode(CSpins[i],OUTPUT);
    digitalWrite(CSpins[i],1);
  }
}


//---------------------------------------------------------
#define CL tft.color565 // for TFT_eSPI library

void setRGB(float a, uint8_t& r, uint8_t& g, uint8_t& b)
{
  if (a<0) a=1.0f+a;
  float aa=a, p=0.5f;
  
  a=1-a; a=1-a*a; // quadratic
  //a=sin(a*PI/2); // sine

  a=a*p+aa*(1-p);
  r=255; g=a*255; b=0;
}


void baseColour(float a, uint8_t& r, uint8_t& g, uint8_t& b)
{
  uint8_t ia=(int) a;
  a -= ia;
  
  switch (ia)
  {
    case 0: setRGB( a,r,g,b); break; // 0-1
    case 1: setRGB(-a,g,r,b); break; // 1-2
    case 2: setRGB( a,g,b,r); break; // 2-3
    case 3: setRGB(-a,b,g,r); break; // 3-4
    case 4: setRGB( a,b,r,g); break; // 4-5
    case 5: setRGB(-a,r,b,g); break; // 5-6
  }
}


void showGamut(TFT_eSPI& tft)
{
  int w=tft.width(), h=tft.height();
  int hw=w/2, hh=h/2;
  //int hw2=hw*hw, hh2=hh*hh;

  // top edge
  for (int x=0; x<w; x++)
  {
    int xo=x-hw;
    float a=atan2(xo,hh)/PI*3 + 3; // divide into 6 sectors
    uint8_t r,g,b;
    
    baseColour(a,r,g,b);
    
    for (int y=0;y<=hh;y++)
    {
      int xp=hw+(x-hw)*(hh-y)/hh;
      uint8_t pr,pg,pb,wh;
      wh=(255*y)/hh;
      pr = (r*(hh-y))/hh + wh;
      pg = (g*(hh-y))/hh + wh;
      pb = (b*(hh-y))/hh + wh;

      tft.drawPixel(xp,y,CL(pr,pg,pb));
    }
  }

  // right edge
  for (int y=0; y<h; y++)
  {
    int yo=hh-y;
    float a=atan2(hw,yo)/PI*3 + 3; // divide into 6 sectors
    uint8_t r,g,b;
    
    baseColour(a,r,g,b);
    
    for (int x=0;x<=hw;x++)
    {
      int yp=hh+(y-hh)*(hw-x)/hw;
      uint8_t pr,pg,pb,wh;
      wh=(255*x)/hw;
      pr = (r*(hw-x))/hw + wh;
      pg = (g*(hw-x))/hw + wh;
      pb = (b*(hw-x))/hw + wh;

      tft.drawPixel(w-x-1,yp,CL(pr,pg,pb));
    }
  }
  
  // bottom edge
  for (int x=w-1; x>=0; x--)
  {
    int xo=x-hw;
    float a=atan2(xo,-hh)/PI*3 + 3; // divide into 6 sectors
    uint8_t r,g,b;
    
    baseColour(a,r,g,b);
    
    for (int y=0;y<=hh;y++)
    {
      int xp=hw+(x-hw)*(hh-y)/hh;
      uint8_t pr,pg,pb,wh;
      wh=(255*y)/hh;
      pr = (r*(hh-y))/hh + wh;
      pg = (g*(hh-y))/hh + wh;
      pb = (b*(hh-y))/hh + wh;

      tft.drawPixel(xp,h-y-1,CL(pr,pg,pb));
    }
  }

  // left edge
  for (int y=0; y<=h; y++)
  {
    int yo=y-hh;
    float a=atan2(-hw,yo)/PI*3 + 3; // divide into 6 sectors
    uint8_t r,g,b;
    
    baseColour(a,r,g,b);
    
    for (int x=0;x<=hw;x++)
    {
      int yp=hh+(hh-y)*(hw-x)/hw;
      uint8_t pr,pg,pb,wh;
      wh=(255*x)/hw;
      pr = (r*(hw-x))/hw + wh;
      pg = (g*(hw-x))/hw + wh;
      pb = (b*(hw-x))/hw + wh;

      tft.drawPixel(x,yp,CL(pr,pg,pb));
    }
  }
}
