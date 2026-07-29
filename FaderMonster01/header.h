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
#include "classes.h"
#include "contPot.h"
#include "Touches.h"

extern TouchTask touchTask;
extern void taskRoot(void*);

#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])
#endif // !defined(_HEADER_H_)
