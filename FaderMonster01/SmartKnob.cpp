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
      //SER_TERM.print('.');
      break;
    }
  
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

    packetCount++; // got a valid packet - count it

    // ignore acks and such for now
    if (pb_rx_buffer_.which_payload != PB_FromSmartKnob_smartknob_state_tag)
      break;

    SKstate = pb_rx_buffer_.payload.smartknob_state;
    PB_SmartKnobConfig& config = SKstate.config;

    if (lastConfigChange > smoothAfter)
    {
        smooth_sub_position = SKstate.sub_position_unit * smoothFactor
                            + smooth_sub_position * (1.0f - smoothFactor);
    }
    else
        smooth_sub_position = SKstate.sub_position_unit;

    report = {millis(), SKstate.current_position, smooth_sub_position, true};

    if (0 == config.min_position && 0 == config.max_position) // return-to-centre
    {
        if (fabs(last_sub_position - smooth_sub_position) >= 0.001f)
        {
            last_sub_position = smooth_sub_position;
            report.isInteger = false;
            if (lastConfigChange >= suspendFor)
                superTask.sendSmartKnobReport(toSuper, report, 0);
        }
    }
    else // integer result
    {
        if (last_position != SKstate.current_position)
        {
            last_position = SKstate.current_position;
            superTask.sendSmartKnobReport(toSuper, report, 0);
        }
    }
  } while (0);
}

bool SmartKnobTask::knobSendConfig(const PB_SmartKnobConfig& cfg)
{
    bool result = false;
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
      result = true;
    } while (0);
    pCurrentConfig = &cfg;
    lastConfigChange = 0;
    configChangePending = true; // sent, but may get old-style ones for a while

    return result;
}

InterTaskRequest::Result SmartKnobTask::doSetConfig(void* _PB_SmartKnobConfig)
{
    InterTaskRequest::Result result = InterTaskRequest::Result::done;
    const PB_SmartKnobConfig cfg = *(const PB_SmartKnobConfig*) _PB_SmartKnobConfig;
    if (!knobSendConfig(cfg))
        result = InterTaskRequest::Result::failed;
    return result;
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

/**
 * Weird startup stuff.
 * SmartKnob boots in text mode, and spews stuf at us for a bit. We
 * have to send it a bunch of '\0' bytes to switch to the protobuf
 * mode, which eventually results in a "Small packet" message, 
 * signalling we finally succeeded!
 */
void SmartKnobTask::weirdStartup(void)
{
    elapsedMillis em = 0;
    char buf[10];
    int idx = 0;
    do
    {
        char ch;
        ch = SK_SERIAL.read();
        if (ch > 0)
        {
            //if (0xFF != ch)
            //    Serial.print(ch);
            SK_SERIAL.write('\0');
        }
        else
            vTaskDelay(2);
        
        if (0 != idx)
        {
            buf[idx++] = ch;
            if (idx >= 5)
            {
                buf[idx] = 0;
                //Serial.printf("\nGot '%s'\n", buf);
                if (0 == strncmp(buf, "Small", 5))
                    break;
                else 
                    idx = 0;                        
            }
        }

        if ('S' == ch)
            buf[idx++] = ch;

    } while (em < 500);
}

//============================================================================
void SmartKnobTask::run(void)
{
    // USART port to SmartKnob
    SK_SERIAL.begin(115200);
    
    pThisTask = this;
    weirdStartup();
    vTaskDelay(20); // tests suggest SmartKnob takes 250ms to wake up

    knobSerial.setStream(&SK_SERIAL);
    knobSerial.setPacketHandler(knobPacketHandler);

    knobSendConfig(configs[0]); // send initial config
    Serial.printf("[%ul] SmartKnob ready\n", micros());

    while (1)
    {
        reqQueue.executeRequest(*this, 5);
        knobSerial.update(); // process all available packets
        /*
        if (0 == packetCount) // not getting packets
        {            
            if (lastConfigChange > 50)
                knobSendConfig(configs[0]);
        }
        else 
        {
            if (configChangePending)
            {
                if (0 == strcmp(SKstate.config.text, pCurrentConfig->text))
                    configChangePending = false;
                else
                {
                    if (lastConfigChange > 50)
                        knobSendConfig(*pCurrentConfig);
                }                    
            }
        }
            */
    }
}

SmartKnobTask smartKnobTask{"SmartKnob", 640, nullptr, 3, 1};