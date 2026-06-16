#if !defined(_HEADERS_H_)
#define _HEADERS_H_

#define SER_TERM Serial

#define TASK_LIST \
    TASK_LIST_ENTRY(Supervisor) \
    TASK_LIST_ENTRY(SmartKnob) \
    TASK_LIST_ENTRY(MainLCD)

#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])

#include <arduino_freertos.h>
#include <TFT_eSPI.h>

extern void initSmartKnob(HardwareSerialIMXRT& knobSerialPort);
//extern void updateSmartKnob(void);
extern int current_position;
extern float sub_position;
extern int cmdSK;

#define FONT_DP FreeSansBold24pt7b
extern void initMainLCD(void);
extern void showGamut(TFT_eSPI& tft);

#endif // !defined(_HEADERS_H_)
