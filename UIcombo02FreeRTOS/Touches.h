#if !defined(_TOUCHES_H_)
#define _TOUCHES_H_

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

#endif // defined(_TOUCHES_H_)