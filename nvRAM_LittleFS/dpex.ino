#define U5_SPI  SPI1
#define U5_ADDR 4
#define U5_CS   4

SPISettings U5_SPIsettings{10'000'000, MSBFIRST, SPI_MODE0};

void initDPEX(void)
{
  U5_SPI.begin();
  pinMode(U5_CS, OUTPUT);
  digitalWriteFast(U5_CS, HIGH);
}

void writeU5(uint8_t reg, uint8_t val)
{
  uint8_t buf[3]{0b01000000 | (U5_ADDR<<1), reg, val};
  U5_SPI.beginTransaction(U5_SPIsettings);
  digitalWriteFast(U5_CS, LOW);
  U5_SPI.transfer(buf,3);
  digitalWriteFast(U5_CS, HIGH);
  U5_SPI.endTransaction();
}
