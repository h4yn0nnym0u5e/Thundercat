#if !defined(_HEADERS_H_)
#define _HEADERS_H_

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "arduino_freertos.h"

extern bool echoOnce;

extern void initADCs(void);
extern void updateADCs(void);
extern void potsToRaw(void);
extern void zeroPots(void);
extern void printADCs(void);

extern void initLEDs(void);
extern void updateLEDs(void);

extern uint8_t initTouch(void);
extern void updateTouch(void);
extern void calibrateTouch(void);

#endif // !defined(_HEADERS_H_)
