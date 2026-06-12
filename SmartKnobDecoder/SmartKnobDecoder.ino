#include <PacketSerial.h>

#include "smartknob.pb.h"
#include "proto_helpers.h"

#include "pb_encode.h"
#include "pb_decode.h"

#include "crc32.h"

PacketSerial_<COBS, 0, 512> knobSerial;

void knobPacketHandler(const uint8_t* buffer, size_t size)
{
  uint32_t computed_crc = 0;
  crc32(buffer, size-4, &computed_crc);
  uint32_t received_crc = buffer[size - 4]
                       | (buffer[size - 3] << 8)
                       | (buffer[size - 2] << 16)
                       | (buffer[size - 1] << 24);
  bool crc_ok = computed_crc == received_crc;
  for (size_t i=0;i<size-4;i++)
    SerialUSB1.printf("%02X ", *buffer++);
  SerialUSB1.printf(": %s", crc_ok?"OK":"CRC fail");    
  SerialUSB1.println();
  if (crc_ok)
  {
    PB_FromSmartKnob pb_rx_buffer_;
    pb_istream_t stream = pb_istream_from_buffer(buffer, size - 4);
    bool decode_ok = pb_decode(&stream, PB_FromSmartKnob_fields, &pb_rx_buffer_);
    if (!decode_ok)
      SerialUSB1.printf("Decode failed: %s\n", stream.errmsg);
  }
  SerialUSB1.println();
}

void setup() 
{
  // Teensy USB serial ports
  Serial.begin(0);
  SerialUSB1.begin(0);

  // USART port to SmartKnob
  Serial1.begin(115200);
  knobSerial.setStream(&Serial1);
  knobSerial.setPacketHandler(knobPacketHandler);
}

int charCount;
void loop() 
{
  int ch;

  ch = Serial.read();
  if (ch >= 0)
    Serial1.print((char) ch);

/*
  ch = Serial1.read();
  if (ch >= 0)
  {
    Serial.print((char) ch);
    SerialUSB1.printf("%02X ", ch);
    charCount++;
    if (charCount >= 80 || 0 == ch)
    {
      SerialUSB1.println();
      charCount = 0;
    }
  }
/*/
  knobSerial.update();
//*/  
}
