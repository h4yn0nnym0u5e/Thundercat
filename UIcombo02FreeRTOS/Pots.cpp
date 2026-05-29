/*  
 * Based loosely on ADS8688 library example
 */

#include <Arduino.h>
#include <ADS8688.h>
#include "headers.h"
#include "contPot.h"
#include <TeensyTimerTool.h>
using namespace TeensyTimerTool;

#define USE_TIMER_TOOL

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

const int potMap[] = POT_MAP;
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
    Serial.printf("%sch%d:%+.6f%s ", pad, i+1, allPots[i].getCurrent(), pad);
  }
  Serial.println();          
}

//================================================================================
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


void getAllADCchannelSets(uint16_t* buf, int numADCs)
{
  for (int i=0;i<NUM_POTS;i++)
    getOneADCchannelSet(buf+i*numADCs, numADCs);
}


//================================================================================
static PeriodicTimer  SPItimer;
static EventResponder SPIresponder;
// 2 values per 360° pot, 1 per fader, 1 for overhead
static const int ADC_TX_COUNT{1 + (NUM_POTS*2 + NUM_FADERS)/8};
static uint16_t TxBuffer[ADC_TX_COUNT]{0}; // transmit zeroes
static uint16_t ADCrawBuffer[ADC_TX_COUNT * 8];
static uint16_t ADCbuffer[NUM_POTS*2 + NUM_FADERS];
static uint16_t* pADCraw;
static int ADCtoDo;
static uint32_t now;
uint32_t ADCupdateMicros;

static void transferComplete(EventResponderRef evref)
{
  const int _cs   = ADC_CS;

  switch (ADCtoDo)
  {
    default:
      break;

    case 1 ... NUM_POTS-1: // some left to do - re-start the process
      digitalWrite(_cs, arduino::HIGH);
      ADCtoDo--;
      pADCraw += ADC_TX_COUNT;
      delayNanoseconds(ADC_CS_HIGH_NS); // ensure minimum /CS high time (30ns)
      digitalWrite(_cs, arduino::LOW);
      ADC_SPI.transfer(TxBuffer,pADCraw,sizeof TxBuffer,SPIresponder); // next transfer
      break;

    case 0: // all done
      digitalWrite(_cs, arduino::HIGH);
      ADC_SPI.endTransaction();

      // de-interleave the values from the raw buffer
      {
        int stride = 2 + (NUM_POTS*2 + NUM_FADERS)/4; // stride in bytes
        uint8_t* src = (uint8_t*)(ADCrawBuffer+1); // skip the dummy
        uint16_t* dst = ADCbuffer;

        for (int i=0;i<NUM_POTS;i++) 
        {
          dst[0] = (src[0]<<8) | src[1];
          dst[1] = (src[2]<<8) | src[3];
          dst += 2;
          if (0 != NUM_FADERS)
          {
            dst[0] = (src[4]<<8) | src[5];
            dst++;
          }
          src += stride;
        }
      }
      ADCtoDo = -1; // extra flag to say we're done
      ADCupdateMicros = micros() - now;
      xTaskResumeFromISR(handleADCs);      
      break;
  }
}

uint32_t missedADCcallbackCount;
static void SPItimerCallback(void)
{
  const int _cs   = ADC_CS;
  const int _sclk = ADC_CLK;

  if (NUM_POTS == ADCtoDo) // we're ready for a new set of readings
  {
      now = micros();

      pADCraw = ADCrawBuffer;
      ADCtoDo--; // prevent re-triggering
      ADC_SPI.beginTransaction(SPISettings(_sclk, arduino::MSBFIRST, SPI_MODE0));
      digitalWrite(_cs, arduino::LOW);
      ADC_SPI.transfer(TxBuffer,pADCraw,sizeof TxBuffer,SPIresponder); // first transfer
  }
  else
    missedADCcallbackCount++;
}

void initSPItimer(void)
{
  // Attach the EventResponder function to be triggered when
  // the DMA buffer transfer is done. Note that this is executed 
  // within the SPI async driver's DMA ISR
  SPIresponder.attachImmediate(transferComplete);

  // Trigger SPI transaction sequence at fixed frequency
  SPItimer.begin(SPItimerCallback, 1'000); // fire the SPI sequence every millisecond
}


//================================================================================
float raw2volts(uint16_t raw)
{
  return (float) raw / 65535.0f * 5.0f;
}


void updateADCs() 
{
  // Trigger ADCs to sample analog ports: 
  // (16+16*N)*8 clock cycles, so 384 for 2 ADCs, or 512 for 3 ADCs
  // At 12MHz this will take 5.3µs per channel, so 42.7µs for all 8
  // across 3 ADCs.

  /*
  bank.noOpDaisy();   

  std::vector<float> ADCBuffer1 = bank.ReturnADC_EMG();
  std::vector<float> ADCBuffer2 = bank.ReturnADC_FSR();

  for (byte i=0;i<4;i++)
  {
      allPots[i+0].update(ADCBuffer1[potMap[i]], ADCBuffer1[potMap[i]+1]);
      allPots[i+4].update(ADCBuffer2[potMap[i]], ADCBuffer2[potMap[i]+1]);
  }
  /*/
  {
 #if !defined(USE_TIMER_TOOL)
    const int numADCs = (NUM_POTS*2 + NUM_FADERS)/8;
    uint16_t buffer[NUM_POTS*numADCs]; // each pot has 2 channels

    /*
    for (int i=0;i<NUM_POTS;i++)
      getOneADCchannelSet(buffer+i*numADCs, numADCs);
    /*/
    getAllADCchannelSets(buffer, numADCs);
    //*/
 #else 
    uint16_t* buffer = ADCbuffer;    
 #endif // defined(USE_TIMER_TOOL)

    // Given a potMap[] of {4,2,0,6}, we get a buffer of 16 values thus
    // 2A, 6A,  2B, 6B,   1A, 5A,  1B, 5B,   0A, 4A,  0B, 4B,   3A, 7A,  3B, 7B
    for (int i=0;i<NUM_POTS/2;i++) // each ADC hosts 4 pots
    {
      allPots[i+0].update(raw2volts(buffer[potMap[i]*2  ]), raw2volts(buffer[potMap[i]*2+2]));
      allPots[i+4].update(raw2volts(buffer[potMap[i]*2+1]), raw2volts(buffer[potMap[i]*2+3]));
    }
  }
  //*/
}

static void CPsetup(ContinuousPot& cp)
{
    cp.applyLimits(true);
    cp.setAccel(0.05f, 10.0f);
    cp.setScale(0.2f);

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
    CPsetup(allPots[i]);

 #if defined(USE_TIMER_TOOL)
  initSPItimer();
 #endif // defined(USE_TIMER_TOOL)

  while (1)
  {
    updateADCs();
    ADCtoDo = 8;
 #if defined(USE_TIMER_TOOL)
    vTaskSuspend(nullptr);
 #else    
    vTaskDelay(1);
 #endif // defined(USE_TIMER_TOOL)
  }
}

void initADCs(void)
{
//Serial.printf("Create ADCs task: \n");    
  xTaskCreate(taskADCs, "ADCs", 128, nullptr, 3, &handleADCs);
}



