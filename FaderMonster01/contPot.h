#if !defined(_CONT_POT_H_)
#define _CONT_POT_H_

class ContinuousPot
{
    float adcMax, ch1Pol, ch2Pol, deadZone;
    float minLimit, maxLimit;
    float raw,  // position in range -1.0 to 1.0 
          rate, // rate of change 
          current;
    float scale, accelThreshold, accelFactor, smooth; // change "feel" of pot
    float minChange;
    bool limitsApplied;
    bool changed;
    int accelDisable;
    elapsedMicros updateInterval;
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

    float update(float a1, float a2); // update with new ADC readings
    bool available(void) { bool result = changed; changed = false; return result; }
    float     getRaw(void) { return raw; }  // physical position: ±1.0
    float    getRate(void) { return rate; } // rate of change: turns/sec
    float getCurrent(void) { return current; } // logical position
    void  setCurrent(float c) { current = c; } // [re]set logical position
    void setLimits(float l, float h) { minLimit = l; maxLimit = h; } // limits of logical position
    void applyLimits (bool b) { limitsApplied = true; } // apply limits to logical position
    void setScale(float s) { scale = s; } // scale physical to logical position
    void setSmooth(float s) { smooth = s; } // reading-to-reading smoothing
    void setAccel(float thr, float fac) // accelerate changes if turned quickly
      { accelThreshold = thr; accelFactor = fac; }
    float setMinChange(float m) 
    { 
      float result = minChange; 
      if (m > 0.0f)
      {
        minChange = m;
      }
      return result;
    }
    bool debug;      
};

#endif // !defined(_CONT_POT_H_)
