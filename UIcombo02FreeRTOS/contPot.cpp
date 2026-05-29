/*
  Analog Input

  Two channels, continuous pot.
  Fundamentally scaled to a "raw" range of ±1.0
  Raw value processed to "current" range with 
  [optional] user-set limits, scaling and acceleration
*/
#include "headers.h"
#include "contPot.h"

#define DBG(x) if (debug) { Serial.printf(" %.4f ", x); }
//#define DBG(x) if (debug) { Serial.printf(#x ": %.34f; ", x); }
#define NL if (debug) { Serial.println(); }
float ContinuousPot::update(float a1, float a2)
{
  // map ADC range to ±1.0
  a1 = map(a1, 0.0f, adcMax,ch1Pol,-ch1Pol);
  a2 = map(a2, 0.0f, adcMax,ch2Pol,-ch2Pol);
DBG(a1); DBG(a2); 

  // map readings to angle in range ±1.0:
  float t1 = (a2>0.0f?(a1 - 1.0f):(1.0f - a1))*0.5f;
  float t2 = (a1<0.0f?(a2 - 2.0f):(0.0f - a2))*0.5f;
  if (t2 < -1.0) t2 += 2.0f;
DBG(t1); DBG(t2); 
//NL;


  // map out pot dead zones:
  float weight = 1.0f + deadZone - fabs(a1*(1.0f + 2.0f * deadZone));
  if (weight < 0.0f) weight = 0.0f;
  if (weight > 1.0f) weight = 1.0f;
  float t3 = t1 * weight + t2 * (1.0f - weight);
DBG(weight); DBG(t3);

  // decouple absolute position from output value:
  float delta = t3 - raw; // change since last update
  if (delta < -1.8f) delta += 2.0f; // jumped clockwise...
  if (delta >  1.8f) delta -= 2.0f; // ...or anticlockwise
  rate = delta * 500000.0 / updateInterval; // rate of change
DBG(delta);

  // accelerate fast moves:
  float accel = fabs(rate) - accelThreshold;
  if (0 == accelDisable && accelFactor > 0.0f && accel > 0.0f)
    delta *= 1.0f + accel * accel * accelFactor;
DBG(delta);
  
  if (accelDisable > 0)
    accelDisable--;
    
  // apply accelerated and scaled change:
  float newVal = current + delta * scale;
  current = newVal * smooth + current * (1.0f - smooth);
DBG(current);

  // limit output as requested (soft stops):
  if (limitsApplied)
  {
    if (current > maxLimit) current = maxLimit;
    if (current < minLimit) current = minLimit;
  }

  // stash values ready for next update:
  raw = t3;
  updateInterval = 0;

NL;  
  return current;  
}
