#include "header.h"
#include "proto_helpers.h"
#include "pb_encode.h"
#include "pb_decode.h"

#define SER_TERM Serial

//============================================================================
//
//                     d8b                   888            
//                     Y8P                   888            
//                                           888            
//    88888b.  888d888 888 888  888  8888b.  888888 .d88b.  
//    888 "88b 888P"   888 888  888     "88b 888   d8P  Y8b 
//    888  888 888     888 Y88  88P .d888888 888   88888888 
//    888 d88P 888     888  Y8bd8P  888  888 Y88b. Y8b.     
//    88888P"  888     888   Y88P   "Y888888  "Y888 "Y8888  
//    888                                                   
//    888                                                   
//    888                                                   
//

//============================================================================
// PacketSerial requires a static callback for its handler,
// so for now we implement this pass-through. A better
// scheme would be needed if we had more than one SmartKnob
/* static */ SmartKnobTask* SmartKnobTask::pThisTask{nullptr};
void SmartKnobTask::knobPacketHandler(const uint8_t* buffer, size_t size)
{
    if (nullptr != pThisTask)
        pThisTask->packetHandler(buffer, size);
}

void SmartKnobTask::packetHandler(const uint8_t* buffer, size_t size)
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

void SmartKnobTask::knobSendConfig(const PB_SmartKnobConfig& cfg)
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

InterTaskRequest::Result SmartKnobTask::doSetConfig(void* _PB_SmartKnobConfig)
{
    const PB_SmartKnobConfig cfg = *(const PB_SmartKnobConfig*) _PB_SmartKnobConfig;
    knobSendConfig(cfg);
    return InterTaskRequest::Result::done;
}

//========================================================================
//
//                      888      888 d8b          
//                      888      888 Y8P          
//                      888      888              
//    88888b.  888  888 88888b.  888 888  .d8888b 
//    888 "88b 888  888 888 "88b 888 888 d88P"    
//    888  888 888  888 888  888 888 888 888      
//    888 d88P Y88b 888 888 d88P 888 888 Y88b.    
//    88888P"   "Y88888 88888P"  888 888  "Y8888P 
//    888                                         
//    888                                         
//    888                                         
//
// functions called by client task to queue a request
// returns reference to the request, so we can interrogate it
// for success immediately
InterTaskRequest& SmartKnobTask::setConfig(InterTaskRequest& req, const PB_SmartKnobConfig& cfg)
{
    requestPayload payload{&SmartKnobTask::doSetConfig, (void*) &cfg};
    RequestQueue<SmartKnobTask, requestPayload>::queueEntry entry{&req, payload};

    reqQueue.request(entry, 0);

    return req;
}

//============================================================================
void SmartKnobTask::run(void)
{
    vTaskDelay(100);
    // USART port to SmartKnob
    SK_SERIAL.begin(115200);
    
    knobSerial.setStream(&SK_SERIAL);
    knobSerial.setPacketHandler(knobPacketHandler);

    pThisTask = this;
    vTaskDelay(100);
    knobSendConfig(configs[0]);
    vTaskDelay(100);
    knobSendConfig(configs[0]);
    
    while (1)
    {
        reqQueue.executeRequest(*this, 5);
        knobSerial.update();            
    }
}

SmartKnobTask smartKnobTask{"SmartKnob", 512, nullptr, 3, 1};