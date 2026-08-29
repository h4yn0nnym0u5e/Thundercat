#if !defined(_HEADER_H_)
#define _HEADER_H_

#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#undef SPISettings // undo macro hack
#include <WS2812Serial.h>

#define CONCAT(a,b,c) a##b##c

#include "hardware.h"
#include "config.h"
#include <arduino_freertos.h>
#include <queue.h>
#include "expanders.h"
#include "DPex.h"
#include "RingLEDs.h"
#include "contPot.h"
#include "Touches.h"
#include "classes.h"

extern TouchTask touchTask;
extern ScribbleTask scribbleTask;
extern PotsTask potsTask;
extern RingLEDsTask ringLEDsTask;
extern SuperTask superTask;
extern MIDItask midiTask;
// use StripTask& StripTask::getStriptask(n) for strip tasks


extern void taskRoot(void*);
extern uint16_t* allocateDMAbuffer(int w, int h);
extern void cycleLED(elapsedMillis& em, int& colour, int ring, int led = 11);

extern ContinuousPot allPots[NUM_POTS];
//typedef RingLEDs<NUM_POTS> FaderMonsterRingLEDs; 
extern  RingLEDs<NUM_POTS> rings;

extern FaderMonsterSettings faderMonsterSettings;

// hacky things to be got rid of later. Probably.
extern WS2812Serial leds;
extern uint8_t bits;
extern char dbgBuffer[200];
extern bool dbgWritten;
extern uint32_t ADCupdateMicros;
extern void printADCs(void);
namespace freertos
{
    extern TaskHandle_t g_yield_task;
}
extern int rainbow[LEDS_PER_RING], cold2hot[LEDS_PER_RING];


#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])
#endif // !defined(_HEADER_H_)
