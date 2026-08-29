#include <Arduino.h>
#include <SPI.h>
#include "hardware.h"
#include "expanders.h"
#include "dpex.h"


SPISettings DPex::spiSettings{10'000'000, MSBFIRST, SPI_MODE0};

DPex U3{ADDR_U3};
DPex U5{ADDR_U5};

void initDPEX(void)
{
  pinMode(DPEX_CS, OUTPUT);
  digitalWriteFast(DPEX_CS, HIGH);
  delay(2);
  DPEX_SPI.begin();
  delay(2);

  U3.write16(REG_IODIRA, (MCP23S17_SETTINGS::U3::IODIRA << 8) | MCP23S17_SETTINGS::U3::IODIRB);
  U3.write16(REG_GPPUA,  (MCP23S17_SETTINGS::U3::GPPUA << 8)  | MCP23S17_SETTINGS::U3::GPPUB);
  U5.write16(REG_IODIRA, (MCP23S17_SETTINGS::U5::IODIRA << 8) | MCP23S17_SETTINGS::U5::IODIRB);
  U5.write16(REG_GPPUA,  (MCP23S17_SETTINGS::U5::GPPUA << 8)  | MCP23S17_SETTINGS::U5::GPPUB);
  delay(2);
  U3.getBit(0);
  U5.getBit(0);

  Serial.println("Port expanders configured");
  Serial.printf("Pull-ups: %04hX\n", U5.read16(REG_GPPUA));
}


void DPex::write16(uint8_t reg, uint16_t val, uint16_t mask)
{
  val = (val & mask) | (gpio & ~mask); // mask in only the bits we want to change
  // uint32_t buf{((0b01000000ul | (addr<<1)) << 24) | ( reg<<16 ) | val};
  uint32_t buf = makeTransactionWord(reg, true, val);
  DPEX_SPI.beginTransaction(spiSettings);
  digitalWriteFast(DPEX_CS, LOW);
  DPEX_SPI.transfer32(buf);
  digitalWriteFast(DPEX_CS, HIGH);
  DPEX_SPI.endTransaction();
}

void DPex::write16async(uint8_t reg, uint16_t val, EventResponderRef event_responder, uint16_t mask)
{
  val = (val & mask) | (gpio & ~mask); // mask in only the bits we want to change
  asyncTx = makeTransactionWord(reg, true, val);
  asyncTx = htonl(asyncTx);
  assertCS();
  DPEX_SPI.transfer(&asyncTx, nullptr, 4, event_responder);
}

uint16_t DPex::read16(uint8_t reg)
{
  //uint8_t buf[4]{0b01000001 | (MCP23S17_SETTINGS::U5::ADDR<<1), reg};
  // uint32_t buf{((0b01000001ul | (addr<<1)) << 24) | ( reg<<16 )};
  uint32_t buf = makeTransactionWord(reg);
  DPEX_SPI.beginTransaction(spiSettings);
  digitalWriteFast(DPEX_CS, LOW);
  uint32_t result32 = DPEX_SPI.transfer32(buf);
  digitalWriteFast(DPEX_CS, HIGH);
  DPEX_SPI.endTransaction();

  return (uint16_t)(result32 & 0xFFFF);
}


void DPex::read16async(uint8_t reg, EventResponderRef event_responder)
{
  asyncTx = makeTransactionWord(reg);
  asyncTx = htonl(asyncTx);
  assertCS();
  DPEX_SPI.transfer(&asyncTx, &asyncResult, 4, event_responder);
}


#define _GET_DPEX_MASK(u,p,b,d) (1<<b)
#define GET_DPEX_MASK(dp) _GET_DPEX_MASK(dp)
void scribbleReset(void)
{
  SET_BIT_NOW(LCD_RESET, 1);
  delay(20);
  SET_BIT_NOW(LCD_RESET, 0);
  delay(1);
  SET_BIT_NOW(LCD_RESET, 1);
  delay(1);
}

void ADCsReset(void)
{
  SET_BIT_NOW(ADCS_RESET, 1);
  delay(20);
  SET_BIT_NOW(ADCS_RESET, 0);
  delay(1);
  SET_BIT_NOW(ADCS_RESET, 1);
  delay(1);
}
