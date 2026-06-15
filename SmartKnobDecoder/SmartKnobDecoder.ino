//#include <TeensyDebug.h>

#define DECODE_SERIAL

#if !defined(DECODE_SERIAL)
#define SER_TERM SerialUSB1
#else
#define SER_TERM Serial

#include <PacketSerial.h>

#include "smartknob.pb.h"
#include "proto_helpers.h"

#include "pb_encode.h"
#include "pb_decode.h"

#include "crc32.h"

PacketSerial_<COBS, 0, 512> knobSerial;

size_t last_size;
int last_position;
void knobPacketHandler(const uint8_t* buffer, size_t size)
{
  do 
  {
    if (size <= 4)
    {
      SER_TERM.print('.');
      break;
    }
    // SER_TERM.println(millis());
  
    uint32_t computed_crc = 0;
    last_size = size;
    crc32(buffer, size-4, &computed_crc);
    uint32_t received_crc = buffer[size - 4]
                         | (buffer[size - 3] << 8)
                         | (buffer[size - 2] << 16)
                         | (buffer[size - 1] << 24);
    bool crc_ok = computed_crc == received_crc;
    /*
    uint8_t* p = buffer;
    for (size_t i=0;i<size-4;i++)
      SER_TERM.printf("%02X ", *p++);
    SER_TERM.printf(": %s", crc_ok?"OK":"CRC fail");    
    SER_TERM.println();
    */
    if (!crc_ok)
      break;
      
    PB_FromSmartKnob pb_rx_buffer_;
    pb_istream_t stream = pb_istream_from_buffer(buffer, size - 4);
    bool decode_ok = pb_decode(&stream, PB_FromSmartKnob_fields, &pb_rx_buffer_);
    
    if (!decode_ok)
    {
      //halt_cpu();
      SER_TERM.printf("Decode failed: %s\n", stream.errmsg);
      break;
    }

    if (last_position != pb_rx_buffer_.payload.smartknob_state.current_position)
    {
      last_position = pb_rx_buffer_.payload.smartknob_state.current_position;
      SER_TERM.printf("Position: %d", pb_rx_buffer_.payload.smartknob_state.current_position);
      SER_TERM.println();
    }
  } while (0);
}
#endif // defined(DECODE_SERIAL)

void setup() 
{
  // Teensy USB serial ports
  Serial.begin(0);
  SerialUSB1.begin(0);

  // USART port to SmartKnob
  Serial1.begin(115200);
  
#if defined(DECODE_SERIAL)
  knobSerial.setStream(&Serial1);
  knobSerial.setPacketHandler(knobPacketHandler);
#endif // defined(DECODE_SERIAL)

  //halt_cpu();
}

int charCount;
void loop() 
{
  int ch;

  ch = Serial.read();
  if (ch >= 0)
    Serial1.print((char) ch);

#if !defined(DECODE_SERIAL)
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
#else
  knobSerial.update();
#endif // defined(DECODE_SERIAL)
}
