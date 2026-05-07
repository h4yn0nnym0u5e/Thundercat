/*
  Analog Input

  Two channels, continuous pot.
  Fundamentally scaled to a "raw" range of ±1.0
  Raw value processed to "current" range with 
  [optional] user-set limits, scaling and acceleration
*/
#include "contPot.h"

float ContinuousPot::update(float a1, float a2)
{
  // map ADC range to ±1.0
  a1 = map(a1, 0.0f, adcMax,ch1Pol,-ch1Pol);
  a2 = map(a2, 0.0f, adcMax,ch2Pol,-ch2Pol);

  // map readings to angle in range ±1.0:
  float t1 = (a2>0.0f?(a1 - 1.0f):(1.0f - a1))*0.5f;
  float t2 = (a1<0.0f?(a2 - 2.0f):(0.0f - a2))*0.5f;
  if (t2 < -1.0) t2 += 2.0f;

  // map out pot dead zones:
  float weight = 1.0f + deadZone - fabs(a1*(1.0f + 2.0f * deadZone));
  if (weight < 0.0f) weight = 0.0f;
  if (weight > 1.0f) weight = 1.0f;
  float t3 = t1 * weight + t2 * (1.0f - weight);

  // decouple absolute position from output value:
  float delta = t3 - raw; // change since last update
  if (delta < -1.8f) delta += 2.0f; // jumped clockwise...
  if (delta >  1.8f) delta -= 2.0f; // ...or anticlockwise
  rate = delta * 500000.0 / updateInterval; // rate of change

  // accelerate fast moves:
  float accel = fabs(rate) - accelThreshold;
  if (accelFactor > 0.0f && accel > 0.0f)
    delta *= 1.0f + accel * accel * accelFactor;
    
  // apply accelerated and scaled change:
  current += delta * scale;

  // limit output as requested (soft stops):
  if (limitsApplied)
  {
    if (current > maxLimit) current = maxLimit;
    if (current < minLimit) current = minLimit;
  }

  // stash values ready for next update:
  raw = t3;
  updateInterval = 0;

  return current;  
}
