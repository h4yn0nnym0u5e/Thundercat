#include <Arduino.h>
#include <SPI.h>
#include "hardware.h"
#undef CREATE_BUTTON_LEDS
#include "expanders.h"
#include "dpex.h"


SPISettings DPex::spiSettings{10'000'000, MSBFIRST, SPI_MODE0};

DPex U3{ADDR_U3};
DPex U5{ADDR_U5};

void initDPEX(void)
{
  DPEX_SPI.begin();
  pinMode(DPEX_CS, OUTPUT);
  digitalWriteFast(DPEX_CS, HIGH);

  U3.write16(REG_IODIRA, (MCP23S17_SETTINGS::U3::IODIRA << 8) | MCP23S17_SETTINGS::U3::IODIRB);
  U3.write16(REG_GPPUA,  (MCP23S17_SETTINGS::U3::GPPUA << 8)  | MCP23S17_SETTINGS::U3::GPPUB);
  U5.write16(REG_IODIRA, (MCP23S17_SETTINGS::U5::IODIRA << 8) | MCP23S17_SETTINGS::U5::IODIRB);
  U5.write16(REG_GPPUA,  (MCP23S17_SETTINGS::U5::GPPUA << 8)  | MCP23S17_SETTINGS::U5::GPPUB);
}


void DPex::write16(uint8_t reg, uint16_t val, uint16_t mask)
{
  val = (val & mask) | (gpio & ~mask); // mask in only the bits we want to change
  uint32_t buf{((0b01000000ul | (addr<<1)) << 24) | ( reg<<16 ) | val};
  DPEX_SPI.beginTransaction(spiSettings);
  digitalWriteFast(DPEX_CS, LOW);
  DPEX_SPI.transfer32(buf);
  digitalWriteFast(DPEX_CS, HIGH);
  DPEX_SPI.endTransaction();
}

uint16_t DPex::read16(uint8_t reg)
{
  //uint8_t buf[4]{0b01000001 | (MCP23S17_SETTINGS::U5::ADDR<<1), reg};
  uint32_t buf{((0b01000001ul | (addr<<1)) << 24) | ( reg<<16 )};
  DPEX_SPI.beginTransaction(spiSettings);
  digitalWriteFast(DPEX_CS, LOW);
  uint32_t result32 = DPEX_SPI.transfer32(buf);
  digitalWriteFast(DPEX_CS, HIGH);
  DPEX_SPI.endTransaction();

  return (uint16_t)(result32 & 0xFFFF);
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
