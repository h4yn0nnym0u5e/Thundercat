/*  
 * Based loosely on ADS8688 library example
 */

#include <Arduino.h>
#include <ADS8688.h>
#include "config.h"
#include "contPot.h"

static uint8_t NmbOfADC = 2;            // Number of ADCs in series. This can only be two as of right now (03/09/19)
static ADS8688 bank = ADS8688(ADC_CS);  // Instantiate ADS8688 with PIN 7 as CS, default to SPI

ContinuousPot allPots[NUM_POTS]
  {
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f},
    {4.096f, CH1_POL, CH2_POL, 0.1f}
  };

void initADCs(void)
{
  bank.setChannelSPD(0b11111111);       // bitwise channel selection 
  bank.setDaisyChainsNmb(NmbOfADC);     // Specify number of ADCs in series
  bank.setGlobalRange(R6);              // set range for all channels (R1 = +- 1.25 * Vref, R6 = 0 ... 1.25*Vref)
  bank.autoRst();                       // reset auto sequence

  for (int i=0;i<NUM_POTS;i++)
  {
    allPots[i].applyLimits(true);
    //allPots[i].setAccel(0.05f, 4.0f);
  }
}

void potsToRaw(void)
{
  for (int i=0;i<NUM_POTS;i++)
  {
    allPots[i].setCurrent(allPots[i].getRaw());
  }
}

void zeroPots(void)
{
  for (int i=0;i<NUM_POTS;i++)
  {
    allPots[i].setCurrent(0.0f);
  }
}


const int potMap[] = {4,2,0,6};
extern uint8_t keyStatuses[NUM_POTS];

void updateADCs() {
  static elapsedMillis em = 0;

  if (em >= 10)
  {
    bank.noOpDaisy();   // Trigger ADCs to sample analog ports

    std::vector<float> ADCBuffer1 = bank.ReturnADC_EMG();
    std::vector<float> ADCBuffer2 = bank.ReturnADC_FSR();

    for (byte i=0;i<4;i++)
    {
        allPots[i+0].update(ADCBuffer1[potMap[i]], ADCBuffer1[potMap[i]+1]);
        allPots[i+4].update(ADCBuffer2[potMap[i]], ADCBuffer2[potMap[i]+1]);
    }
  }
}


void printADCs(void)
{
  for (int i=0;i<NUM_POTS;i++)
  {
    const char* pad = keyStatuses[i]?"  ":"";
    //Serial.printf("%+.3f ", allPots[i].getCurrent());
    Serial.printf("%sch%d:%+.3f%s ", pad, i+1, allPots[i].getCurrent(), pad);
  }
  Serial.println();          
}
