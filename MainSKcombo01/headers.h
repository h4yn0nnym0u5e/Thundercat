#if !defined(_HEADERS_H_)
#define _HEADERS_H_

#define SER_TERM Serial

extern void initSmartKnob(HardwareSerialIMXRT& knobSerialPort);
extern void updateSmartKnob(void);
extern int current_position;
extern float sub_position;

#endif // !defined(_HEADERS_H_)
