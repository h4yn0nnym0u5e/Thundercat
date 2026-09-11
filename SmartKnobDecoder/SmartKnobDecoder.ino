//#include <TeensyDebug.h>

#define DECODE_SERIAL

#if !defined(DECODE_SERIAL)
#define SER_TERM SerialUSB1
#else
#define SER_TERM Serial

// settings for Main PCB
#define SK_SERIAL Serial7
#define EN_6V 25

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

    // ignore acks and such for now
    if (pb_rx_buffer_.which_payload != PB_FromSmartKnob_smartknob_state_tag)
      break;

    if (last_position != pb_rx_buffer_.payload.smartknob_state.current_position)
    {
      last_position = pb_rx_buffer_.payload.smartknob_state.current_position;
      SER_TERM.printf("Position: %d", pb_rx_buffer_.payload.smartknob_state.current_position);
      SER_TERM.println();
    }
  } while (0);
}

uint8_t tx_buffer_[300]; // should be enough...
uint32_t tx_nonce;
void knobSendConfig(const PB_SmartKnobConfig& cfg)
{
    // Encode protobuf message to byte buffer
    PB_ToSmartknob pb_tx_buffer_{};
    pb_tx_buffer_.nonce = ++tx_nonce;
    pb_tx_buffer_.which_payload = PB_ToSmartknob_smartknob_config_tag;
    pb_tx_buffer_.payload.smartknob_config = cfg;

    pb_ostream_t stream = pb_ostream_from_buffer(tx_buffer_, sizeof(tx_buffer_));
    pb_tx_buffer_.protocol_version = PROTOBUF_PROTOCOL_VERSION;

    do
    {
      if (!pb_encode(&stream, PB_ToSmartknob_fields, &pb_tx_buffer_)) {
          SER_TERM.println(stream.errmsg);
          SER_TERM.flush();
          break;
      }

      // Compute and append little-endian CRC32
      uint32_t crc = 0;
      crc32(tx_buffer_, stream.bytes_written, &crc);
      tx_buffer_[stream.bytes_written + 0] = (crc >> 0)  & 0xFF;
      tx_buffer_[stream.bytes_written + 1] = (crc >> 8)  & 0xFF;
      tx_buffer_[stream.bytes_written + 2] = (crc >> 16) & 0xFF;
      tx_buffer_[stream.bytes_written + 3] = (crc >> 24) & 0xFF;

      // Encode and send proto+CRC as a COBS packet
      knobSerial.send(tx_buffer_, stream.bytes_written + 4);  
    } while (0);
}

#endif // defined(DECODE_SERIAL)

void setup() 
{
#if defined(EN_6V)
  // FaderMonster main PCB:
  pinMode(EN_6V, OUTPUT);
  digitalWriteFast(EN_6V, HIGH);
#endif // defined(EN_6V)
  // Teensy USB serial ports
  Serial.begin(0);
  SerialUSB1.begin(0);

  // USART port to SmartKnob
  SK_SERIAL.begin(115200);
  
#if defined(DECODE_SERIAL)
  knobSerial.setStream(&SK_SERIAL);
  knobSerial.setPacketHandler(knobPacketHandler);
#endif // defined(DECODE_SERIAL)

  //halt_cpu();
}

extern PB_SmartKnobConfig configs[];
int charCount;
void loop() 
{
  int ch;

  ch = Serial.read();
  switch (ch)
  {
    case -1: break; // nothing received, do nothing

    case '1' ... '5':
      knobSendConfig(configs[ch-'1']);
      break;

    default:
      break;      
  }

#if !defined(DECODE_SERIAL)
  ch = SK_SERIAL.read();
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
