#if !defined(_HEADERS_H_)
#define _HEADERS_H_

#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#undef SPISettings // undo macro hack

#define CONCAT(a,b,c) a##b##c

#include "config.h"

#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])

extern int bright;
extern void initLEDs(void);
extern void updateLEDs(void);

#endif // !defined(_HEADERS_H_)
