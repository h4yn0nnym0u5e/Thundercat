/*  
 * Based loosely on ADS8688 library example
 */

#include <Arduino.h>
#include <ADS8688.h>
#include "header.h"
#include <TeensyTimerTool.h>
using namespace TeensyTimerTool;


static uint8_t NmbOfADC = 2;            // Number of ADCs in series. This can only be two as of right now (03/09/19)
static ADS8688 bank = ADS8688(ADC_CS, ADC_SPI);  // Instantiate ADS8688 with PIN 7 as CS, default to SPI

ContinuousPot PotsTask::allPots[NUM_POTS]
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

StripTask* PotsTask::stripTasks[NUM_POTS]{nullptr};

static const int potMap[] = POT_MAP;

//================================================================================
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

/*
void printADCs(void)
{
  for (int i=0;i<NUM_POTS;i++)
  {
    const char* pad = keyStatuses[i]?"  ":"";
    Serial.printf("%sch%d:%+.6f%s ", pad, i+1, allPots[i].getCurrent(), pad);
  }
  Serial.println();          
}
*/


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

/*
 * EventResponder function, called in ISR when an 
 * asynchronous SPI transaction completes. We can't 
 * do everything in one, because the ADCs have very
 * specific requirements on CS in order to trigger
 * a sample, so we have to do one transaction per set
 * of channels (a set being 2x continuous pots + 1 fader).
 * 
 * At the end of the sequence, we consolidate the data
 * and resume the task, which processes raw readings 
 * into usable values.
 */
static void transferComplete(EventResponderRef evref)
{
  const int _cs   = ADC_CS;

  switch (ADCtoDo)
  {
    default: // should do something sensible here, really
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

      // De-interleave the values from the raw buffer.
      // raw format in bytes is:
      // d d A0H A0L B0H B0L [C0H C0L] d d A1H A1L ... B7H B7L [C7H C7L]
      // Where A, B, C are the ADCs, 0-7 are the channels, 
      // H and L are the MSB and LSB
      {
        int stride = 2 + (NUM_POTS*2 + NUM_FADERS)/4; // stride in bytes
        uint8_t* src = (uint8_t*)(ADCrawBuffer+1); // skip the dummy
        uint16_t* dst = ADCbuffer;

        for (int i=0;i<NUM_POTS;i++) 
        {
          // swap byte order, it comes in big-endian:
          dst[0] = (src[0]<<8) | src[1];
          dst[1] = (src[2]<<8) | src[3];
          dst += 2;

          // process faders, if we have them:
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

      TaskHandle_t owner = *((TaskHandle_t*) evref.getContext());
      xTaskResumeFromISR(owner);      
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

// PeriodicTimer to trigger ADCs to sample analog ports: 
// (16+16*N)*8 clock cycles, so 384 for 2 ADCs, or 512 for 3 ADCs
// At 12MHz this will take 5.3µs per channel, so 42.7µs for all 8
// across 3 ADCs.
void initSPItimer(TaskHandle_t* pHandle)
{
  // Attach the EventResponder function to be triggered when
  // the DMA buffer transfer is done. Note that this is executed 
  // within the SPI async driver's DMA ISR
  SPIresponder.attachImmediate(transferComplete);
  SPIresponder.setContext(pHandle);

  // Trigger SPI transaction sequence at fixed frequency
  SPItimer.begin(SPItimerCallback, POT_READ_INTERVAL_US); // fire the SPI sequence every millisecond
}


//================================================================================
void PotsTask::updateADCs(void) 
{
  {
    uint16_t* buffer = ADCbuffer;    

    // Given a potMap[] of {4,2,0,6}, we get a buffer of 16 values thus
    // 2A, 6A,  2B, 6B,   1A, 5A,  1B, 5B,   0A, 4A,  0B, 4B,   3A, 7A,  3B, 7B
    for (int i=0;i<NUM_POTS/2;i++) // each ADC hosts 4 pots
    {
      allPots[i+0].update(raw2volts(buffer[potMap[i]*2  ]), raw2volts(buffer[potMap[i]*2+2]));
      if (allPots[i+0].available()) notifyOwner(i+0);
      allPots[i+4].update(raw2volts(buffer[potMap[i]*2+1]), raw2volts(buffer[potMap[i]*2+3]));
      if (allPots[i+4].available()) notifyOwner(i+4);
    }
  }
}


// set up continuous pots in a consistent manner
static void CPsetup(ContinuousPot& cp)
{
    cp.applyLimits(true);
    cp.setAccel(0.05f, 10.0f);
    cp.setScale(0.2f);
}

/*
 * ADC task.
 * Wait for power to be valid (due to user or another task),
 * then initialise them and go into a loop waiting for and
 * processing readings
 */
void PotsTask::run(void)
{
  while (!touchTask.supplyValid)
    vTaskDelay(10);

  bank.setChannelSPD(0b11111111);       // bitwise channel selection 
  bank.setDaisyChainsNmb(NmbOfADC);     // Specify number of ADCs in series
  bank.setGlobalRange(R6);              // set range for all channels (R1 = +- 1.25 * Vref, R6 = 0 ... 1.25*Vref)
  bank.autoRst();                       // reset auto sequence

  for (int i=0;i<NUM_POTS;i++)
    CPsetup(allPots[i]);

  // We use a PeriodicTimer to fire off an
  // interrupt / DMA controlled sequence of reads...
  initSPItimer(&handle); // ... initialise that
  while (1)
  {
    ADCtoDo = 8;  // tell timer we're ready for new data...
    vTaskSuspend(nullptr); // ...suspend until it's available...
    updateADCs(); // ...and process it  
  }
}

PotsTask  potsTask("ADCs", 512, nullptr, 3);
