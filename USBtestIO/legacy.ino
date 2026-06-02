#include <EEPROM.h>
#include "legacy.h"

// This array should have as many elements as 
// your LED chain, plus one
WS2811_t colours[NLEDS] = {red,green,blue},
         start[NLEDS] = {red,green,blue},
         target[NLEDS] = {red,green,blue};

WS2811_t Wwarm[]={red,ored,orange,yellow},
//   Wcool[]={blue,cyan,turquoise},
   Wpurp[]={blue,purple,magenta,bluered,darkred},
   Wnature[]={green,cyan,blue},
   Wspectrum[]={red,orange,yellow,green,cyan,blue,purple},
   Wrgb[]={red,green,blue},
   Wrwb[]={red,white,blue},
   Wstyle[]={black,white,purple},
   Wgirly[]={white,pink,hotpink,purple},
   Wpastel[]={pcyan,pmag,pgrn},

   WtestRGB[] = {red,green,blue,black},
   
   Wwhite={32,32,32},
   Wblack={0,0,0};

combo_t combos[]={COMBO(Wwarm),
      COMBO(Wpastel),
      COMBO(Wpurp),
      COMBO(Wnature),
      COMBO(Wspectrum),
      COMBO(Wrgb),
      COMBO(Wrwb),
      COMBO(Wstyle),
      COMBO(Wgirly),
      };

int opmode = SW_XMAS;

//==============================================
// Lots of stuff here used to soft-swap RGB order
//==============================================
void checkSort(bool force = false, WS2811_t* colours = colours)
{
  /*
  if (!digitalRead(SW_SORT) || force) // button pressed, or forced check
  {
    // prior to any swapping, colours are in GRB order
    colours[0] = red;
    colours[1] = green;
    colours[2] = blue;
  } 
  */ 
}

//==============================================
enum RGBorder_e {normal,GRB=normal,RGB,BGR,GBR,RBG,BRG,invalid} RGBorder;
uint32_t lastOrderSet;
void setSort(void)
{
  /*
  static bool lastState = 1;
  static uint32_t debounce;
  bool sortPressed = !digitalRead(SW_SORT);

  if (Serial.available())
  {
    char ch = Serial.read();
    if (10 == ch)
      sortPressed = true;
  }
  
  if (sortPressed) // pressed
  {
    if (lastState)
    {
      lastState = 0;

      int newOrder = RGBorder;
      newOrder++;
      if (newOrder >= invalid)
        newOrder = normal;  
      RGBorder = (RGBorder_e) newOrder;
      Serial.println(newOrder);
      lastOrderSet = msTimer;
      EEPROM.begin(256);
        EEPROM.write(0,newOrder);
        EEPROM.commit();
      EEPROM.end();
    }
    debounce = millis();
  }
  else
  {
    if (millis() - debounce > 20)
      lastState = 1;
  } 
  */
}


//==============================================
void initSort(void)
{
  EEPROM.begin(256);
  uint8_t order = EEPROM.read(0);
  EEPROM.end();
  //order = 1;
  
  if (order >= invalid)
    order = normal;

  RGBorder = (RGBorder_e) order;
  Serial.print(F("Order set to "));
  Serial.println(order);

  lastOrderSet = msTimer;
}

//*
//==============================================
// Normal order is GRB - swap as needed
void swapRG(WS2811_t* arr,int n, int order)
{
  switch (order)
  {
    default:
      break;
      
    case RGB:
      for (int i=0;i<n;i++)
      {
        uint8_t t = arr[i].r;
        arr[i].r = arr[i].g;
        arr[i].g = t;
      }
      break;
     
    case BGR:
      for (int i=0;i<n;i++)
      {
        uint8_t t = arr[i].g;
        arr[i].g = arr[i].b;
        arr[i].b = arr[i].r;
        arr[i].r = t;
      }
      break;
     
    case GBR:
      for (int i=0;i<n;i++)
      {
        uint8_t t = arr[i].r;
        arr[i].r = arr[i].b;
        arr[i].b = t;
      }
      break;
     
    case RBG:
      for (int i=0;i<n;i++)
      {
        uint8_t t = arr[i].g;
        arr[i].g = arr[i].r;
        arr[i].r = arr[i].b;
        arr[i].b = t;
      }
      break;
     
    case BRG:
      for (int i=0;i<n;i++)
      {
        uint8_t t = arr[i].g;
        arr[i].g = arr[i].b;
        arr[i].b = t;
      }
      break;
  }
}


//=================== Christmas lights utilities =================== 
// Combine two colours to an intermediate. 
// Here is where we limit the brightness to something sane...
WS2811_t combine(WS2811_t a,WS2811_t b,uint16_t p,uint8_t shft=SHIFT)
{
  WS2811_t r;
  uint16_t q=255-p;

  r.r = (p*a.r + q*b.r) >> shft;
  r.g = (p*a.g + q*b.g) >> shft;
  r.b = (p*a.b + q*b.b) >> shft;

  return r;
}

#define V0 1
#define X0 -128<<8 
#define VS 6

volatile int16_t v=V0;
volatile int16_t x=X0;
uint8_t sinestep(void)
{
  v = v - (x>>7) 
    - (v>>14);  // damping factor to prevent overflow
  x = x + (( v+ (1<<(VS-2)))>>VS);

  return (x>>8)+128;
}
//==================================================================

int xmas(WS2811_t* lights)
{
//#define PATTERN WtestRGB  
#define PATTERN Wspectrum  
  static uint32_t next = 0;
  static uint8_t idx = 0;
  int upd = 0;

  if (millis() >= next)
  {
    next = millis() + 1000;
    lights[0] = combine(PATTERN[idx],PATTERN[idx],255);
    idx++;
    if (idx >= COUNT_OF(PATTERN))
      idx = 0;
  }
  
  return upd;
}

//==============================================

void setupLights(void)
{
  initSort();
  Serial.printf("Output on pin %d\n%d in string\n", 
                 PIN, NLEDS);
}

void updateLights(void)
{
  int upd = xmas(colours); 
  if (upd)
  {
    // prepare to send colours to LED chain
    setSort();
    checkSort();

    // Swap colours if string LEDs use different order
    // to compiled-in order. Star order is compiled in.
    //swapRG(colours,MAX_INDEX(colours)+1-STAR_COUNT,RGBorder);
    //swapRG(colours+MAX_INDEX(colours)+1-STAR_COUNT,STAR_COUNT,STAR_SWAP);
    // UpdateWS2811(colours,MAX_INDEX(colours));
  }  
}
      
