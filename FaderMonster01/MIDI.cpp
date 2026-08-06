#include "header.h"

InterTaskRequest::Result MIDItask::doSendMIDI(MIDImessage& msg, uint32_t reqTime)
{
    uint32_t took = micros() - reqTime;
    Serial.printf("[%lu]: MIDI: type = %d, cmd = %d, value = %d; took %luus\n",
                  micros(), msg.type,  msg.cmd,  msg.value,      took);

    return InterTaskRequest::Result::done;
}

void MIDItask::run(void)
{
    while (1)
    {
        requestPayload payload;
        uint32_t reqTime;
        if (pdPASS == reqQueue.getPayload(payload, 10, &reqTime)) // got MIDI message
            doSendMIDI(payload.message, reqTime);
    }
}

// sending MIDI is the most important thing we do!
MIDItask midiTask{"MIDI", 512, nullptr, configMAX_PRIORITIES-1, NUM_POTS*4};