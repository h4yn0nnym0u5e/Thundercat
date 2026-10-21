#include "header.h"

//extern digital_pin_bitband_and_config_table_struct* digital_pin_to_info_PGM;
static void setDriveStrength(void)
{
	const struct digital_pin_bitband_and_config_table_struct *p;
    uint8_t pin = MIDI_TX;

	if (pin >= CORE_NUM_DIGITAL) return;
	p = digital_pin_to_info_PGM + pin;

    Serial.printf("MIDI output pad was: %08X\n", *(p->pad));
    *(p->pad) = IOMUXC_PAD_DSE(7); // high drive, low speed and slew rate
    Serial.printf("MIDI output pad is: %08X\n", *(p->pad));
}
//================================================================
MIDI_CREATE_INSTANCE(HardwareSerialIMXRT, MIDI_SERIAL, SerialMIDI);
void MIDIbegin(void)
{
    SerialMIDI.begin();
    pinMode(NOT_MIDI_RX,arduino::INPUT); // restore Rx to normal operation
    setDriveStrength();
    SerialMIDI.sendNoteOff(0x42, 0x12, 1);

}

//================================================================
/* static */ int MIDItask::lastValues[(int) CTL_COUNT];
//================================================================

int& MIDItask::lastSent(MIDImessage& msg)
{
    int* pResult = lastValues;

    switch (msg.control)
    {
        case CTL_POT:    pResult = lastValues + (msg.strip * NUM_STRIP_CONTROLS + 0); break;
        case CTL_FADER:  pResult = lastValues + (msg.strip * NUM_STRIP_CONTROLS + 1); break;
        case CTL_BUTTON: pResult = lastValues + (msg.strip * NUM_STRIP_CONTROLS + 2); break;
        default:
            pResult = lastValues + (int) msg.control; 
            break;
    }

    return *pResult;
}

MIDIcontrolSetting& MIDItask::findSettings(MIDImessage& msg, FaderMonsterSettings& settings /* FaderMonsterSettings */)
{
    MIDIcontrolSetting* ctlSetting{&settings.stripsConfig.controls[0].pot};
    switch (msg.control)
    {
        default: break;

        // miscellaneous controls
        case CTL_PEDAL:     ctlSetting = &settings.miscControls.pedal;     break;
        case CTL_SMARTKNOB: ctlSetting = &settings.miscControls.smartknob; break;
        case CTL_SCREEN_X:  ctlSetting = &settings.miscControls.touchX;    break;
        case CTL_SCREEN_Y:  ctlSetting = &settings.miscControls.touchY;    break;
            
        // strip controls
        case CTL_POT:    ctlSetting = &settings.stripsConfig.controls[msg.strip].pot;    break;
        case CTL_FADER:  ctlSetting = &settings.stripsConfig.controls[msg.strip].fader;  break;
        case CTL_BUTTON: ctlSetting = &settings.stripsConfig.controls[msg.strip].button; break;
    }

    return *ctlSetting;
}


/**
 * This is for MIDI 1.0
 */
int MIDItask::toInt(MIDIcontrolSetting& ctl, float value)
{
    // map full control range to desired output range
    int result = (value + 1.0f)/2.0f
               *(ctl.maxVal - ctl.minVal+0.99f) 
               + ctl.minVal;

    switch (ctl.controlType)
    {
        default:
        // fundamentally 0-127
        case MIDIcontrolType::CC:
        case MIDIcontrolType::PC:
        case MIDIcontrolType::AT:
        case MIDIcontrolType::NOTE:
            if (ctl.minVal < 0) // using -64 to +63 range? Guessing a bit here...
                result += 64; 
            result = constrain(result, 0, 127);
            break;

        case MIDIcontrolType::RPN:
        case MIDIcontrolType::NRPN:
        case MIDIcontrolType::BEND:
            if (ctl.minVal < 0) // using -8192 to +8191 range? Guessing a bit here...
                result += 8192; 
            result = constrain(result, 0, 16383);
            break;
    }

    return result;
}


InterTaskRequest::Result MIDItask::doSendMIDI(MIDImessage& msg, uint32_t reqTime)
{
    MIDIcontrolSetting& ctl = findSettings(msg);
    //*
    uint32_t took = micros() - reqTime;
    char buf[20];
    Textifier t{buf,20};
    int& last = lastSent(msg);
    int toSend = toInt(ctl, msg.value);

    if (last != toSend)
    {
        uint8_t MIDIch = ctl.channel+1; // MIDI library uses 1-based channels
        //* test code
        t.cc(ctl.controlType, ctl.controlNum).toTarget();
        Serial.printf("[%lu]: MIDI: channel %d, cmd = %s, value = %d; took %luus ",
                    micros(),        MIDIch,    buf,      toSend,     took);
        //*/ 
        
        // actual MIDI!
        switch (ctl.controlType)
        {
            default: Serial.print('?'); break;

            case MIDIcontrolType::CC:   SerialMIDI.sendControlChange(ctl.controlNum, toSend, MIDIch); break;
            case MIDIcontrolType::PC:   SerialMIDI.sendProgramChange(toSend, MIDIch); break;
            case MIDIcontrolType::AT:   SerialMIDI.sendAfterTouch(toSend, MIDIch); break;
            case MIDIcontrolType::NOTE: SerialMIDI.sendNoteOn(ctl.controlNum, toSend, MIDIch); break;
//            case MIDIcontrolType::RPN:
//            case MIDIcontrolType::NRPN:
            case MIDIcontrolType::BEND: SerialMIDI.sendPitchBend(toSend, MIDIch); break;
        }
        
        last = toSend; 
        Serial.println();                   
    }                    
    //*/
    return InterTaskRequest::Result::done;
}

void MIDItask::run(void)
{
    //////////////////////////////////////////////////////////////////////
    // DO NOT initialise the MIDI serial here, as it will change the Rx //
    // pin mode, which we almost certainly don't want to occur          //
    //////////////////////////////////////////////////////////////////////

    int count=0, toSend=0;
    while (1)
    {
        requestPayload payload;
        uint32_t reqTime;
        if (pdPASS == reqQueue.getPayload(payload, 10, &reqTime)) // got MIDI message
            doSendMIDI(payload.message, reqTime);
        if (--count < 0)
        {
            count = 9;
            if (++toSend > 127)
                toSend = 0;
            SerialMIDI.sendControlChange(1, toSend, 3);
        }
    }
}

// sending MIDI is the most important thing we do!
MIDItask midiTask{"MIDI", 512, nullptr, configMAX_PRIORITIES-1, NUM_POTS*4};