#if !defined(_HEADERS_H_)
#define _HEADERS_H_

#include <Arduino.h>

#define LEDS_PER_RING  20
#define LED_TOP_OFFSET  8
#define NUM_POTS        8

#define CONCAT(a,b,c) a##b##c

//#include "config.h"

#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])

extern int bright;
extern void initLEDs(void);
extern void updateLEDs(void);

#endif // !defined(_HEADERS_H_)
