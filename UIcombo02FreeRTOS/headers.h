#if !defined(_HEADERS_H_)
#define _HEADERS_H_

#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#undef SPISettings // undo macro hack

#include "config.h"
#include <arduino_freertos.h>
#include "contPot.h"

#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])

extern bool echoOnce;
extern bool supplyValid;
extern uint32_t ADCupdateMicros;
extern int bright;
extern uint8_t keyStatuses[NUM_POTS];
extern ContinuousPot allPots[NUM_POTS];
extern TFT_TYPE* tfts[NUM_POTS];
extern uint16_t colours[NUM_POTS];
extern uint16_t bkgnds[NUM_POTS];
extern uint16_t textColours[NUM_POTS];

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

extern void initScribble(void);

#endif // !defined(_HEADERS_H_)
