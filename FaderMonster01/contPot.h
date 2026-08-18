#if !defined(_CONT_POT_H_)
#define _CONT_POT_H_

//! Continuous pot status class.
// Each pot has two ADC channels in quadrature. Readings are 
// scaled and converted into a convenient range, typically
// ±1.0. 
class ContinuousPot
{
    float adcMax,   //!< maximum expected ADC reading (volts)
          ch1Pol,   //!< channel 1 polarity
          ch2Pol,   //!< channel 2 polarity
          deadZone; //!< width of "dead zone" at the end of a channel's travel
    float minLimit, //!< lowest allowed value for current position
          maxLimit; //!< highest allowed value for current position
    float raw,      //!< position in range -1.0 to 1.0 
          rate,     //!< rate of change 
          current;  //!< current value

          // change "feel" of pot
    float scale,          //!< scale up fundamental ±1.0 position to give greater range
          accelThreshold, //!< threshold at which acceleration starts
          accelFactor,    //!< factor to accelerate by
          smooth;         //!< eliminate jitter
    float minChange;      //!< minimum reported change
    bool limitsApplied;   //!< true to apply limits to current value
    bool changed;         //!< true if value change is greater than minChange
    int accelDisable;     //!< true to disable acceleration
    elapsedMicros updateInterval; //!< interval since last update (microseconds)
  public:
    ContinuousPot(float _adcMax, float _ch1Pol, float _ch2Pol, float _deadZone)
      : adcMax{_adcMax}, ch1Pol{_ch1Pol}, ch2Pol{_ch2Pol}, deadZone{_deadZone},
        minLimit{-1.0f}, maxLimit{1.0f},
        rate{0.0f}, current{0.0f},
        scale{1.0f}, accelThreshold{0.0f}, accelFactor{0.0f}, 
        smooth{0.1f}, minChange{0.000002f},
        limitsApplied{false}, changed{false},
        accelDisable{5}, updateInterval{0}
        ,debug{false}
      {}

    //! update from new ADC readings
    float update(float a1,  //!< channel 1 reading
                 float a2); //!< channel 2 reading

    //! \return true if pot value has changed                 
    bool available(void) { bool result = changed; changed = false; return result; }

    //! \return physical position: ±1.0
    float     getRaw(void) { return raw; }  

    //! \return rate of change: turns/sec
    float    getRate(void) { return rate; }

    //! \return logical position (scaled / accelerated / limited / de-jittered)
    float getCurrent(void) { return current; } 

    //! [re]set logical position
    void  setCurrent(float c) //!< new position
      { current = c; } 

    //! set limits of logical position
    void setLimits(float l, float h) { minLimit = l; maxLimit = h; } 

    //! optionally apply limits to logical position
    void applyLimits(bool b) { limitsApplied = true; }

    //! scale physical to logical position
    void setScale(float s) //!< scale factor
      { scale = s; } 

    //! reading-to-reading smoothing
    void setSmooth(float s) { smooth = s; }
    
    //! accelerate changes if turned quickly
    void setAccel(float thr, //!< threshold: turn rate below this is not accelerated
                  float fac) //!< factor: amount to accelerate by
      { accelThreshold = thr; accelFactor = fac; }

    //! set minimum change that will show a new reading
    float setMinChange(float m) 
    { 
      float result = minChange; 
      if (m > 0.0f)
      {
        minChange = m;
      }
      return result;
    }
    bool debug;   //!< set true to enable serial debug output (development only!)
};

#endif // !defined(_CONT_POT_H_)
