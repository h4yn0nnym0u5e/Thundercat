#include <SPI.h>

#define PIN_MODE 23
#define PIN_CSN  10

SPISettings mySPIsettings{10'000'000, MSBFIRST, SPI_MODE1};

void setup() 
{
  pinMode(PIN_MODE, OUTPUT);
  pinMode(PIN_CSN, OUTPUT);
  digitalWriteFast(PIN_MODE, HIGH); // put chip into SSI mode
  digitalWriteFast(PIN_CSN,  HIGH);  // deselect

  SPI.begin();
}

elapsedMillis em;
void loop() 
{
  uint32_t out=0,in;
  SPI.beginTransaction(mySPIsettings);
  digitalWriteFast(PIN_CSN,  LOW);  // select
  delayNanoseconds(150);
  in = SPI.transfer32(out);
  digitalWriteFast(PIN_CSN,  HIGH);  // deselect
  SPI.endTransaction();

  uint32_t flags = (in>>14)&0xF;
  uint32_t pos = in>>24;
  if (em >= 500 || 0 != (flags & 4))
  {
    em = 0;
    //Serial.printf("%08X %01X %d\n", in, flags, pos);
    Serial.printf("%d: flags:%d angle:%d\n", millis(), flags, pos);
  }

  delay(1);
}
