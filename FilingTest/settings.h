
#include "config.h"

#define TO_OFFSET_LEAF_ARRAY(...)

class TFTcolours : public CfgBaseOffset
{
  public:
    uint16_t fg, bg, txt;

    //-------------------------------------------------
    virtual int toOffset(const char* str, int& consume)
    {
        int result = -1; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_LEAF(fg);
            TO_OFFSET_LEAF(bg);
            TO_OFFSET_LEAF(txt);
        } while (0);
        return result;
    }
};

class cfgRingLEDs : public CfgBaseOffset
{
  public:
    int colour;
    pattern_t pattern;

    //-------------------------------------------------
    virtual int toOffset(const char* str, int& consume)
    {
        int result = -1; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_LEAF(colour);
            TO_OFFSET_LEAF(pattern);
        } while (0);
        return result;
    }
};

class cfgButtonLED : public CfgBaseOffset
{
  public:
    int colour;

    //-------------------------------------------------
    virtual int toOffset(const char* str, int& consume)
    {
        int result = -1; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_LEAF(colour);
        } while (0);
        return result;
    }
};

class MIDIcontrolSetting : public CfgBaseOffset
{
  public:
    MIDIcontrolType controlType;
    int minVal, maxVal, channel, controlNum;
    char name[MAX_NAME_LENGTH];

    //-------------------------------------------------
    virtual int toOffset(const char* str, int& consume)
    {
        int result = -1; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_LEAF(controlType);
            TO_OFFSET_LEAF(minVal);
            TO_OFFSET_LEAF(maxVal);
            TO_OFFSET_LEAF(channel);
            TO_OFFSET_LEAF(controlNum);
            TO_OFFSET_LEAF(name);
        } while (0);
        return result;
    }
};

class StripControls : public CfgBaseOffset
{
  public:
    MIDIcontrolSetting fader, pot, button;

    //-------------------------------------------------
    virtual int toOffset(const char* str, int& consume)
    {
        int result = -1; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET(fader);
            TO_OFFSET(pot);
            TO_OFFSET(button);
        } while (0);
        return result;
    }
};

class StripColours : public CfgBaseOffset
{
  public:
    cfgRingLEDs ringLEDs;
    cfgButtonLED buttonLED;
    TFTcolours scribble;

    //-------------------------------------------------
    virtual int toOffset(const char* str, int& consume)
    {
        int result = -1; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET(ringLEDs);
            TO_OFFSET(buttonLED);
            TO_OFFSET(scribble);
        } while (0);
        return result;
    }
};

class StripSettings : public CfgBaseOffset
{
  public:
    StripColours colours;
    StripControls controls;

    //-------------------------------------------------
    virtual int toOffset(const char* str, int& consume)
    {
        int result = -1; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET(colours);
            TO_OFFSET(controls);
        } while (0);
        return result;
    }
};

class FaderMonsterSettings : public CfgBaseOffset
{
  public:
    StripSettings stripsConfig[NUM_POTS];

    //-------------------------------------------------
    virtual int toOffset(const char* str, int& consume)
    {
        int result = -1; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_ARRAY(stripsConfig);
        } while (0);
        return result;
    }
};


// types: {'cfgRingLEDs', 'StripControls', 'FaderMonsterSettings', 'cfgButtonLED', 'MIDIcontrolSetting', 'StripSettings', 'StripColours', 'TFTcolours'}
/*
stripsConfig.1
stripsConfig.1.colours
stripsConfig.1.colours.ringLEDs
int stripsConfig.1.colours.ringLEDs.colour
pattern_t stripsConfig.1.colours.ringLEDs.pattern
stripsConfig.1.colours.buttonLED
int stripsConfig.1.colours.buttonLED.colour
stripsConfig.1.colours.scribble
uint16_t stripsConfig.1.colours.scribble.fg
uint16_t stripsConfig.1.colours.scribble.bg
uint16_t stripsConfig.1.colours.scribble.txt
stripsConfig.1.controls
stripsConfig.1.controls.fader
MIDIcontrolType stripsConfig.1.controls.fader.controlType
int stripsConfig.1.controls.fader.minVal
int stripsConfig.1.controls.fader.maxVal
int stripsConfig.1.controls.fader.channel
int stripsConfig.1.controls.fader.controlNum
char stripsConfig.1.controls.fader.name
stripsConfig.1.controls.pot
MIDIcontrolType stripsConfig.1.controls.pot.controlType
int stripsConfig.1.controls.pot.minVal
int stripsConfig.1.controls.pot.maxVal
int stripsConfig.1.controls.pot.channel
int stripsConfig.1.controls.pot.controlNum
char stripsConfig.1.controls.pot.name
stripsConfig.1.controls.button
MIDIcontrolType stripsConfig.1.controls.button.controlType
int stripsConfig.1.controls.button.minVal
int stripsConfig.1.controls.button.maxVal
int stripsConfig.1.controls.button.channel
int stripsConfig.1.controls.button.controlNum
char stripsConfig.1.controls.button.name

{'uint16_t', 'MIDIcontrolType', 'int', 'pattern_t', 'char'}

24 leaves:
stripsConfig.1.colours.ringLEDs.colour
stripsConfig.1.colours.ringLEDs.pattern
stripsConfig.1.colours.buttonLED.colour
stripsConfig.1.colours.scribble.fg
stripsConfig.1.colours.scribble.bg
stripsConfig.1.colours.scribble.txt
stripsConfig.1.controls.fader.controlType
stripsConfig.1.controls.fader.minVal
stripsConfig.1.controls.fader.maxVal
stripsConfig.1.controls.fader.channel
stripsConfig.1.controls.fader.controlNum
stripsConfig.1.controls.fader.name
stripsConfig.1.controls.pot.controlType
stripsConfig.1.controls.pot.minVal
stripsConfig.1.controls.pot.maxVal
stripsConfig.1.controls.pot.channel
stripsConfig.1.controls.pot.controlNum
stripsConfig.1.controls.pot.name
stripsConfig.1.controls.button.controlType
stripsConfig.1.controls.button.minVal
stripsConfig.1.controls.button.maxVal
stripsConfig.1.controls.button.channel
stripsConfig.1.controls.button.controlNum
stripsConfig.1.controls.button.name

*/

extern bool setuint16_t(void* dst, const char* src);
extern bool setMIDIcontrolType(void* dst, const char* src);
extern bool setint(void* dst, const char* src);
extern bool setpattern_t(void* dst, const char* src);
extern bool setchar(void* dst, const char* src);
