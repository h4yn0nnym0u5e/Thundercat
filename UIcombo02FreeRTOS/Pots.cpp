/*  
 * Based loosely on ADS8688 library example
 */

#include <Arduino.h>
#include <ADS8688.h>
#include "headers.h"
#include "contPot.h"

static uint8_t NmbOfADC = 2;            // Number of ADCs in series. This can only be two as of right now (03/09/19)
static ADS8688 bank = ADS8688(ADC_CS, ADC_SPI);  // Instantiate ADS8688 with PIN 7 as CS, default to SPI

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

const int potMap[] = {4,2,0,6};
extern uint8_t keyStatuses[NUM_POTS];
TaskHandle_t handleADCs;

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

/*
 * Get all readings from one channel of a set of ADCs
 */
void getOneADCchannelSet(uint16_t* buf, int numADCs)
{
  SPIClass& _spi  = ADC_SPI;
  const int _cs   = ADC_CS;
  const int _sclk = ADC_CLK;

  _spi.beginTransaction(SPISettings(_sclk, arduino::MSBFIRST, SPI_MODE1));
  digitalWrite(_cs, arduino::LOW);
  _spi.transfer16(0x0000); // NO_OP (p45 table 6)
  _spi.endTransaction();

  _spi.beginTransaction(SPISettings(_sclk, arduino::MSBFIRST, SPI_MODE0)); // Necessary for ESP32

  for (int i=0; i< numADCs; i++)
    *buf++ = _spi.transfer16(0);

  digitalWrite(_cs, arduino::HIGH); // we need 30ns after this - should be OK (p11 section 7.6 tPH_CS)
  _spi.endTransaction();
}


void updateADCs() 
{
  // Trigger ADCs to sample analog ports: 
  // (16+16*N)*8 clock cycles, so 384 for 2 ADCs, or 512 for 3 ADCs
  // At 12MHz this will take 5.3µs per channel, so 42.7µs for all 8
  // across 3 ADCs.
  bank.noOpDaisy();   

  std::vector<float> ADCBuffer1 = bank.ReturnADC_EMG();
  std::vector<float> ADCBuffer2 = bank.ReturnADC_FSR();

  for (byte i=0;i<4;i++)
  {
      allPots[i+0].update(ADCBuffer1[potMap[i]], ADCBuffer1[potMap[i]+1]);
      allPots[i+4].update(ADCBuffer2[potMap[i]], ADCBuffer2[potMap[i]+1]);
  }
}

void taskADCs(void*)
{
  while (!supplyValid)
    vTaskDelay(10);

  bank.setChannelSPD(0b11111111);       // bitwise channel selection 
  bank.setDaisyChainsNmb(NmbOfADC);     // Specify number of ADCs in series
  bank.setGlobalRange(R6);              // set range for all channels (R1 = +- 1.25 * Vref, R6 = 0 ... 1.25*Vref)
  bank.autoRst();                       // reset auto sequence

  for (int i=0;i<NUM_POTS;i++)
  {
    allPots[i].applyLimits(true);
    //allPots[i].setAccel(0.05f, 4.0f);
  }

  while (1)
  {
    updateADCs();
    vTaskDelay(1);
  }
}

void initADCs(void)
{
  xTaskCreate(taskADCs, "ADCs", 128, nullptr, 3, &handleADCs);
}



