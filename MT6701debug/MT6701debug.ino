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
float lastAngle;
int tries;
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
  bool valid = 0 == (flags & 0x8);
  uint32_t pos = in>>24;
  float angle = (in>>18)*360.0f/16384;
  if (valid && (em >= 100 || 0 != (flags & 4)))
  {
    em = 0;
    float noise = (lastAngle - angle)*16384/360;
    if (noise > 10000.0f || noise < -10000.0f) // jump!
      noise = 0.42f;
    //Serial.printf("%08X %01X %d\n", in, flags, pos);
    //Serial.printf("%d: flags:%d angle:%d\n", millis(), flags, pos);
    Serial.printf("%d: tries:%d flags:%d angle:%.2f noise:%.2f\n", millis(), tries, flags, angle, noise);
    lastAngle = angle;
  }
  tries++;
  if (valid)
    tries = 0;

  delay(1);
}
