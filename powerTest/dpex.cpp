#include <Arduino.h>
#include <SPI.h>
#include "hardware.h"
#include "expanders.h"
#include "dpex.h"


SPISettings U5_SPIsettings{10'000'000, MSBFIRST, SPI_MODE0};

void initDPEX(void)
{
  DPEX_SPI.begin();
  pinMode(DPEX_CS, OUTPUT);
  digitalWriteFast(DPEX_CS, HIGH);

  writeU5_16(REG_IODIRA, (MCP23S17_SETTINGS::U5::IODIRA << 8) | MCP23S17_SETTINGS::U5::IODIRB);
  writeU5_16(REG_GPPUA,  (MCP23S17_SETTINGS::U5::GPPUA << 8) | MCP23S17_SETTINGS::U5::GPPUB);
}

void writeU5(uint8_t reg, uint8_t val)
{
  uint8_t buf[3]{0b01000000 | (MCP23S17_SETTINGS::U5::ADDR<<1), reg, val};
  DPEX_SPI.beginTransaction(U5_SPIsettings);
  digitalWriteFast(DPEX_CS, LOW);
  DPEX_SPI.transfer(buf,3);
  digitalWriteFast(DPEX_CS, HIGH);
  DPEX_SPI.endTransaction();
}

void writeU5_16(uint8_t reg, uint16_t val, uint16_t mask)
{
  if (0xFFFF != mask)
  {
    uint16_t curVal = readU5_16(reg);
    val &= mask;
    val |= curVal & ~mask;
  }
  uint8_t buf[4]{0b01000000 | (MCP23S17_SETTINGS::U5::ADDR<<1), reg, (uint8_t) (val>>8), (uint8_t) (val & 0xFF)};
  DPEX_SPI.beginTransaction(U5_SPIsettings);
  digitalWriteFast(DPEX_CS, LOW);
  DPEX_SPI.transfer(buf,4);
  digitalWriteFast(DPEX_CS, HIGH);
  DPEX_SPI.endTransaction();
}

uint16_t readU5_16(uint8_t reg)
{
  uint8_t buf[4]{0b01000001 | (MCP23S17_SETTINGS::U5::ADDR<<1), reg};
  DPEX_SPI.beginTransaction(U5_SPIsettings);
  digitalWriteFast(DPEX_CS, LOW);
  DPEX_SPI.transfer(buf,4);
  digitalWriteFast(DPEX_CS, HIGH);
  DPEX_SPI.endTransaction();

  return ((uint16_t) buf[2] << 8) | buf[3];
}

#define _GET_DPEX_MASK(u,p,b,d) (1<<b)
#define GET_DPEX_MASK(dp) _GET_DPEX_MASK(dp)
void scribbleReset(void)
{
  uint16_t U5state = readU5_16(REG_GPIOA);
  uint16_t mask = GET_DPEX_MASK(LCD_RESET);
  Serial.printf("LCD_RESET mask is %04hX\n", mask);

  U5state |= mask;
  writeU5_16(REG_GPIOA, U5state, mask);
  delay(20);
  U5state &= ~mask;
  writeU5_16(REG_GPIOA, U5state, mask);
  delay(1);
  U5state |= mask;
  writeU5_16(REG_GPIOA, U5state, mask);
  delay(1);
}
