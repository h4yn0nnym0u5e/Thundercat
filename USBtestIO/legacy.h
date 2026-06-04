#if !defined(_LEGACY_H_)
#define _LEGACY_H_

#include <sys/types.h>
#include "settings.h"

// Stolen from Teensy!
// elapsedMillis acts as an integer which autoamtically increments 1000 times
// per second.  Useful for creating delays, timeouts, or measuing how long an
// operation takes.  You can create as many elapsedMillis variables as needed.
// All of them are independent.  Any may be written, modified or read at any time.
class elapsedMillis
{
private:
	unsigned long ms;
public:
	elapsedMillis(void) { ms = millis(); }
	elapsedMillis(unsigned long val) { ms = millis() - val; }
	elapsedMillis(const elapsedMillis &orig) { ms = orig.ms; }
	operator unsigned long () const { return millis() - ms; }
	elapsedMillis & operator = (const elapsedMillis &rhs) { ms = rhs.ms; return *this; }
	elapsedMillis & operator = (unsigned long val) { ms = millis() - val; return *this; }
	elapsedMillis & operator -= (unsigned long val)      { ms += val ; return *this; }
	elapsedMillis & operator += (unsigned long val)      { ms -= val ; return *this; }
	elapsedMillis operator - (int val) const           { elapsedMillis r(*this); r.ms += val; return r; }
	elapsedMillis operator - (unsigned int val) const  { elapsedMillis r(*this); r.ms += val; return r; }
	elapsedMillis operator - (long val) const          { elapsedMillis r(*this); r.ms += val; return r; }
	elapsedMillis operator - (unsigned long val) const { elapsedMillis r(*this); r.ms += val; return r; }
	elapsedMillis operator + (int val) const           { elapsedMillis r(*this); r.ms -= val; return r; }
	elapsedMillis operator + (unsigned int val) const  { elapsedMillis r(*this); r.ms -= val; return r; }
	elapsedMillis operator + (long val) const          { elapsedMillis r(*this); r.ms -= val; return r; }
	elapsedMillis operator + (unsigned long val) const { elapsedMillis r(*this); r.ms -= val; return r; }
};

#define NLEDS (LED_STRING)

typedef struct WS2811_s {
  uint8_t g,r,b;
  bool operator==(WS2811_s& a) { return a.r == r && a.g == g && a.b == b; }
} WS2811_t;

typedef struct combo_s {
  uint8_t   n;      //!< number of colours
  WS2811_t*   colours;  //!< colours array
} combo_t;

extern void swapRG(WS2811_t* arr,int n, int order);
extern WS2811_t colours[];

#define MAX_INDEX(a) ((int)((sizeof a/sizeof a[0])-1))
#define COMBO(x) {MAX_INDEX(x)+1,x}

#define TXLED0
#define TXLED1
#define RXLED0
#define RXLED1

#define msTimer millis()

#define red       {0,255,0}
#define darkred   {0,128,0}
#define green     {255,0,0}
#define blue      {0,0,255}
#define yellow    {128,255,0}
#define cyan      {255,0,255}
#define magenta   {0,255,255}
#define white     {255,255,255}
#define black     {0,0,0}
#define orange    {80,255,0}
#define dkorange  {40,128,0}
#define purple    {0,64,128}
#define turquoise {255,0,128}
#define bluered   {0,255,128}
#define pink      {128,255,128}
#define hotpink   {64,255,128}
#define ored      {20,255,0}

#define pmag      {48,255,255}
#define pcyan     {255,40,160}
#define pgrn      {255,40,40}

#define wwhite {192,255,96}
#define lgold     yellow //{192,255, 64}
#define mgold     orange //{128,255, 48}
#define dgold     dkorange //{ 96,255, 32}

#endif // !defined(_LEGACY_H_)
