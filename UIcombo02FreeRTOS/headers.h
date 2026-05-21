#if !defined(_HEADERS_H_)
#define _HEADERS_H_

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "arduino_freertos.h"

#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])

extern bool echoOnce;
extern bool supplyValid;
extern uint32_t ADCupdateMicros;
extern int bright;

extern void initADCs(void);
extern void updateADCs(void);
extern void potsToRaw(void);
extern void zeroPots(void);
extern void printADCs(void);

extern void initLEDs(void);
extern void updateLEDs(void);

extern void initTouch(void);
extern void updateTouch(void);
extern void calibrateTouch(void);

#endif // !defined(_HEADERS_H_)
