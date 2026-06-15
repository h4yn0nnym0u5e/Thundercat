/*
 * Test of nanoPb encode and decode, without actually transmitting
 */
#define noDECODE_SERIAL

#include <TeensyDebug.h>
#include <PacketSerial.h>

#include "smartknob.pb.h"
#include "proto_helpers.h"

#include "pb_encode.h"
#include "pb_decode.h"

#include "crc32.h"

#if defined(DECODE_SERIAL)
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
#endif // defined(DECODE_SERIAL)

PB_FromSmartKnob pb_tx_buffer_;
PB_FromSmartKnob pb_rx_buffer_;
uint8_t tx_buffer_[PB_FromSmartKnob_size + 4]; // Max message size + CRC32
size_t encodeTest(void)
{
    // Encode protobuf message to byte buffer
    // buffer for message:
    pb_ostream_t stream = pb_ostream_from_buffer(tx_buffer_, sizeof(tx_buffer_));

    // source of message:
    pb_tx_buffer_.protocol_version = PROTOBUF_PROTOCOL_VERSION;
    pb_tx_buffer_.which_payload = PB_FromSmartKnob_smartknob_state_tag;

    // encode source to buffer:
    if (!pb_encode(&stream, PB_FromSmartKnob_fields, &pb_tx_buffer_)) {
        Serial.println(stream.errmsg);
        Serial.flush();
    }
    return stream.bytes_written;
}

void decodeTest(size_t sz)
{
    // Decode protobuf message from byte buffer
    // buffer for message:
    pb_istream_t stream = pb_istream_from_buffer(tx_buffer_, sz);

    // encode source to buffer:
    if (!pb_decode(&stream, PB_FromSmartKnob_fields, &pb_rx_buffer_)) {
        Serial.println(stream.errmsg);
        Serial.flush();
    }
}

void setup() 
{
  while (!Serial)
    ;
  // Teensy USB serial ports
  Serial.begin(0);
  //SerialUSB1.begin(0);
  halt_cpu();

  // USART port to SmartKnob
  Serial1.begin(115200);
  
#if defined(DECODE_SERIAL)
  knobSerial.setStream(&Serial1);
  knobSerial.setPacketHandler(knobPacketHandler);
#endif // defined(DECODE_SERIAL)
}

int charCount;
void loop() 
{
  size_t sz; 

  pb_tx_buffer_.payload.smartknob_state.current_position++;
  pb_tx_buffer_.payload.smartknob_state.press_nonce++;
  sz = encodeTest();
  halt_cpu();
  decodeTest(sz);
  halt_cpu();
}
