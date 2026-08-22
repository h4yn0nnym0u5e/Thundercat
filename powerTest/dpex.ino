#include "hardware.h"
#include "expanders.h"

// we assume IOCON.BANK = 0
// Unfortunately IOCON moves its own address when the 
// bank bit is changed! This constitutes Poor Design...
#define REG_IODIRA 0x00
#define REG_IODIRB 0x01
#define REG_GPPUA  0x0C
#define REG_GPPUB  0x0D
#define REG_GPIOA  0x12 // can use for read or write
#define REG_GPIOB  0x13



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

void writeU5_16(uint8_t reg, uint16_t val)
{
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
