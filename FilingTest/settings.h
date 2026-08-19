
#include "config.h"

#define TO_OFFSET_LEAF_ARRAY(...)

// types: {'StripSettings', 'FaderMonsterSettings', 'StripControls', 'cfgRingLEDs', 'MIDIcontrolSetting', 'TFTcolours', 'StripColours', 'cfgButtonLED'}
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

{'MIDIcontrolType', 'char', 'pattern_t', 'uint16_t', 'int'}

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
//========================================
extern bool setMIDIcontrolType(void* dst, const char* src);
extern bool setchar(void* dst, const char* src);
extern bool setpattern_t(void* dst, const char* src);
extern bool setuint16_t(void* dst, const char* src);
extern bool setint(void* dst, const char* src);

extern bool getMIDIcontrolType(char* dst, void* src);
extern bool getchar(char* dst, void* src);
extern bool getpattern_t(char* dst, void* src);
extern bool getuint16_t(char* dst, void* src);
extern bool getint(char* dst, void* src);
//========================================

class TFTcolours : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"TFTcolours"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    uint16_t fg, bg, txt;

    static constexpr const char* memberNames[]{"fg", "bg", "txt"};
    int getMemberCount(void) { return 3; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1}; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_LEAF(fg, uint16_t);
            TO_OFFSET_LEAF(bg, uint16_t);
            TO_OFFSET_LEAF(txt, uint16_t);
        } while (0);
        return result;
    }
};

class cfgRingLEDs : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"cfgRingLEDs"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    int colour;
    pattern_t pattern;

    static constexpr const char* memberNames[]{"colour", "pattern"};
    int getMemberCount(void) { return 2; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1}; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_LEAF(colour, int);
            TO_OFFSET_LEAF(pattern, pattern_t);
        } while (0);
        return result;
    }
};

class cfgButtonLED : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"cfgButtonLED"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    int colour;

    static constexpr const char* memberNames[]{"colour"};
    int getMemberCount(void) { return 1; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1}; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_LEAF(colour, int);
        } while (0);
        return result;
    }
};

class MIDIcontrolSetting : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"MIDIcontrolSetting"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    MIDIcontrolType controlType;
    int minVal, maxVal, channel, controlNum;
    char name[MAX_NAME_LENGTH];

    static constexpr const char* memberNames[]{"controlType", "minVal", "maxVal", "channel", "controlNum", "name"};
    int getMemberCount(void) { return 6; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1}; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_LEAF(controlType, MIDIcontrolType);
            TO_OFFSET_LEAF(minVal, int);
            TO_OFFSET_LEAF(maxVal, int);
            TO_OFFSET_LEAF(channel, int);
            TO_OFFSET_LEAF(controlNum, int);
            TO_OFFSET_LEAF(name, char);
        } while (0);
        return result;
    }
};

class StripControls : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"StripControls"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    MIDIcontrolSetting fader, pot, button;

    static constexpr const char* memberNames[]{"fader", "pot", "button"};
    int getMemberCount(void) { return 3; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1}; // not found
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
    static constexpr const char* className{"StripColours"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    cfgRingLEDs ringLEDs;
    cfgButtonLED buttonLED;
    TFTcolours scribble;

    static constexpr const char* memberNames[]{"ringLEDs", "buttonLED", "scribble"};
    int getMemberCount(void) { return 3; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1}; // not found
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
    static constexpr const char* className{"StripSettings"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    StripColours colours;
    StripControls controls;

    static constexpr const char* memberNames[]{"colours", "controls"};
    int getMemberCount(void) { return 2; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1}; // not found
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
    static constexpr const char* className{"FaderMonsterSettings"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    StripSettings stripsConfig[NUM_POTS];

    static constexpr const char* memberNames[]{"stripsConfig"};
    int getMemberCount(void) { return 1; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1}; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_ARRAY(stripsConfig);
        } while (0);
        return result;
    }
};


