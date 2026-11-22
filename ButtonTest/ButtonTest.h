#if !defined(_BUTTON_TEST_H_)
#define _BUTTON_TEST_H_

#include <WS2812Serial.h>
#include "hardware.h"

#define POWER_LED    USB_X_3

class ButtonLED
{
    WS2812Serial& ledString;
    uint8_t* ledMemory;
    int num;
  public:
    ButtonLED(WS2812Serial& _string, uint8_t* mem, int n)
        : ledString{_string}, ledMemory{mem}, num{n}
        {}
          
    void setColour(uint32_t c)    { ledString.setPixel(num,c); }
    void show(void)               { ledString.show(); }
    void setBrightness(uint8_t n) { ledString.setBrightness(n); }
};


#endif // !defined(_BUTTON_TEST_H_)
