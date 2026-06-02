#if !defined(_HEADERS_H_)
#define _HEADERS_H_

#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#undef SPISettings // undo macro hack

#define CONCAT(a,b,c) a##b##c

#include "config.h"
#include <arduino_freertos.h>
#include "contPot.h"

#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])

extern bool echoOnce;
extern bool supplyValid;
extern uint32_t ADCupdateMicros;
extern int bright;
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

class touchStatus
{
  public:    
    enum class eStatus {OFF, JUST_OFF, JUST_OFF_LONG,  // off statuses
                        ON=11, LONG, DOUBLE, LONG_SHORT}; // on statuses
  private:    
    uint8_t status;
    enum eStatus extendedStatus;
    uint32_t longTouch, doubleTouch;
    elapsedMillis touchTime;

    // update status after time has possibly passed
    void updateExtendedStatus(void)
    {
        switch (extendedStatus)
        {
            default:
                break;
            
            case eStatus::JUST_OFF:
            case eStatus::JUST_OFF_LONG:
                if (touchTime > doubleTouch)
                    extendedStatus = eStatus::OFF;
                break;
            
            case eStatus::ON:
                if (touchTime > longTouch)
                    extendedStatus = eStatus::LONG;
                break;
        }
    }

  public:
    touchStatus() 
        : status{0}, extendedStatus{eStatus::OFF},
          longTouch{500}, doubleTouch{250}
        {}
    operator bool(void) const { return 0 != status; } 
    eStatus getExtendedStatus(void)
    {
        updateExtendedStatus();
        return extendedStatus;
    }

    // called when touch status changes
    touchStatus& operator =(uint8_t v) 
    { 
        updateExtendedStatus();
        if (0 != v) // touched
        {
            switch (extendedStatus)
            {
                case eStatus::OFF:
                    extendedStatus = eStatus::ON;
                    break;
                
                case eStatus::JUST_OFF:
                    extendedStatus = eStatus::DOUBLE;
                    break;

                case eStatus::JUST_OFF_LONG:
                    extendedStatus = eStatus::LONG_SHORT;
                    break;

                default:
                    break;
            }
        }
        else // released
        {
            switch (extendedStatus)
            {
                case eStatus::ON:
                case eStatus::DOUBLE:
                case eStatus::LONG_SHORT:
                    extendedStatus = eStatus::JUST_OFF;
                    break;
                
                case eStatus::LONG:
                    extendedStatus = eStatus::JUST_OFF_LONG;
                    break;
                
                default:
                    break;
            }
        }
        touchTime = 0;
        status = v; 
        return *this; 
    }
};
extern touchStatus keyStatuses[NUM_POTS];
extern void initTouch(void);
extern void updateTouch(void);
extern void calibrateTouch(void);
extern void printTouches(void);

extern void initScribble(void);

#endif // !defined(_HEADERS_H_)
