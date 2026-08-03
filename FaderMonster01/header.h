#if !defined(_HEADER_H_)
#define _HEADER_H_

#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#undef SPISettings // undo macro hack

#define CONCAT(a,b,c) a##b##c

#include "config.h"
#include <arduino_freertos.h>
#include <queue.h>
#include "RingLEDs.h"
#include "contPot.h"
#include "Touches.h"
#include "classes.h"

extern TouchTask touchTask;
extern ScribbleTask scribbleTask;
extern PotsTask potsTask;
extern RingLEDsTask ringLEDsTask;
//extern StripTask* StripTask::tasks[NUM_POTS];


extern void taskRoot(void*);
extern uint16_t* allocateDMAbuffer(int w, int h);
extern void cycleLED(elapsedMillis& em, int& colour, int ring, int led = 11);

extern ContinuousPot allPots[NUM_POTS];
//typedef RingLEDs<NUM_POTS> FaderMonsterRingLEDs; 
extern  RingLEDs<NUM_POTS> rings;

extern FaderMonsterSettings faderMonsterSettings;
extern uint8_t bits;
extern char dbgBuffer[200];
extern bool dbgWritten;


#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])
#endif // !defined(_HEADER_H_)
