#if !defined(_TOUCHES_H_)
#define _TOUCHES_H_

/*
 * Class to deal with touches or buttons
 * Keeps track of touch timing to yield double and
 * long press information
 */
class TouchStatus 
{
  public:    
    enum class eStatus {OFF, JUST_OFF, JUST_OFF_LONG,  // off statuses
                        ON=11, LONG, DOUBLE, SHORT_LONG, LONG_SHORT, LONG_LONG, // momentary on statuses
                        JUST_LATCHED=21, LATCHED, JUST_UNLATCHED}; // toggled-on statuses
  private:    
    uint8_t status;
    enum eStatus extendedStatus, latchStatus{eStatus::OFF}, 
                 lastReadStatus; // current statuses
    enum eStatus setLatchStatus{eStatus::OFF}, clearLatchStatus{eStatus::OFF}; // statuses thqat trigger / remove latched status
    uint32_t longTouch,     // longer than this is a long touch
             doubleTouch;   // re-touched within this is a double-touch
    elapsedMillis touchTime;

    bool _isLatched(void)
    {
        /*
        return eStatus::JUST_LATCHED   == latchStatus   // latch just started - we're latched
            || eStatus::LATCHED        == latchStatus   // still latched
            || eStatus::JUST_UNLATCHED == latchStatus;  // stay on until clear latch press ends
            */
        return eStatus::OFF != latchStatus; // simpler!
    }

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
            
            case eStatus::DOUBLE:
                if (touchTime > longTouch)
                    extendedStatus = eStatus::SHORT_LONG;
                break;
            
            case eStatus::LONG_SHORT:
                if (touchTime > longTouch)
                    extendedStatus = eStatus::LONG_LONG;
                break;
        }
    }

    bool updateLatchStatus(void)
    {
        if (eStatus::OFF != setLatchStatus) // if it's possible to latch
        {
            // only unlatch or latch fully when press ends
            if (0 == status)
            {
                switch (latchStatus)
                {
                    default:
                        break;

                    case eStatus::JUST_LATCHED:
                        latchStatus = eStatus::LATCHED;
                        break;

                    case eStatus::JUST_UNLATCHED:
                        latchStatus = eStatus::OFF;
                        break;
                }
            }
            else // during press - may go into transitional state
            {
                switch (latchStatus)
                {
                    default:
                        break;

                    case eStatus::OFF:
                        if (setLatchStatus == extendedStatus)
                            latchStatus = eStatus::JUST_LATCHED;
                        break;

                    case eStatus::LATCHED:
                        if (clearLatchStatus == extendedStatus)
                            latchStatus = eStatus::JUST_UNLATCHED;
                        break;
                }
            }
        }

        return _isLatched();
    }

    // do complete status update - needs to be in a critical section
    eStatus _updateStatus(void)
    {
        updateExtendedStatus();
        return updateLatchStatus()?latchStatus:extendedStatus;
    }


  public:
    TouchStatus() 
        : status{0}, extendedStatus{eStatus::OFF},
          longTouch{500}, doubleTouch{250}
        {}

    static int whichTouch(TouchStatus& tch, TouchStatus* keyStatuses, int count = NUM_POTS) 
    { 
        int result = &tch - keyStatuses;
        if (result <  0
         || result >= count
         || &tch != keyStatuses+result)
            result = -1;
        return result;
    }
    
    operator bool(void) const { return 0 != status; } 
    eStatus getExtendedStatus(void)
    {
        taskENTER_CRITICAL();
        lastReadStatus = _updateStatus();
        taskEXIT_CRITICAL();
        return lastReadStatus;
    }

    eStatus peekExtendedStatus(void)
    {
        taskENTER_CRITICAL();
        eStatus result = _updateStatus();
        taskEXIT_CRITICAL();
        return result;
    }

    bool isLatched(void)
    {
        eStatus oldStatus = lastReadStatus; // not really changing this
        getExtendedStatus();
        lastReadStatus = oldStatus;
        return _isLatched();
    }

    void setLatchTriggers(eStatus set, eStatus clear)
    {
        setLatchStatus = set;
        clearLatchStatus = clear;
    }

    void setLatch(bool value)
    {
        if (eStatus::OFF != setLatchStatus)
            latchStatus = value
                            ?eStatus::LATCHED
                            :eStatus::OFF;
    }

    bool isChangedStatus(void) 
    { 
        taskENTER_CRITICAL();
        bool result = lastReadStatus != _updateStatus();
        taskEXIT_CRITICAL();
        
        return result;
    }

    // called when touch status changes
    TouchStatus& operator =(uint8_t v) 
    { 
        taskENTER_CRITICAL();
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
                    extendedStatus = eStatus::JUST_OFF;
                    break;

                // don't do sequences of double states 
                case eStatus::DOUBLE:
                case eStatus::LONG_SHORT:
                case eStatus::SHORT_LONG:
                case eStatus::LONG_LONG:
                    extendedStatus = eStatus::OFF;
                    break;
                
                case eStatus::LONG:
                    extendedStatus = eStatus::JUST_OFF_LONG;
                    break;
                
                default:
                    break;
            }
        }

        if (v != status)    // changed?
            touchTime = 0;  // yes: note timestamp
        status = v; 
        taskEXIT_CRITICAL();
        
        return *this; 
    }
};

#endif // defined(_TOUCHES_H_)