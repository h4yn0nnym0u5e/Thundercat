
#include "config.h"

#define TO_OFFSET_LEAF_ARRAY(...)

// types: {'cfgRingLEDs', 'StripControls', 'StripColours', 'FaderMonsterSettings', 'StripSettings', 'cfgButtonLED', 'TFTcolours', 'MIDIcontrolSetting'}
/*
stripsConfig
stripsConfig.colours.1
stripsConfig.colours.1.ringLEDs
int stripsConfig.colours.1.ringLEDs.colour
pattern_t stripsConfig.colours.1.ringLEDs.pattern
stripsConfig.colours.1.buttonLED
int stripsConfig.colours.1.buttonLED.colour
stripsConfig.colours.1.scribble
uint16_t stripsConfig.colours.1.scribble.fg
uint16_t stripsConfig.colours.1.scribble.bg
uint16_t stripsConfig.colours.1.scribble.txt
stripsConfig.controls.1
stripsConfig.controls.1.fader
MIDIcontrolType stripsConfig.controls.1.fader.controlType
int stripsConfig.controls.1.fader.minVal
int stripsConfig.controls.1.fader.maxVal
int stripsConfig.controls.1.fader.channel
int stripsConfig.controls.1.fader.controlNum
char stripsConfig.controls.1.fader.name
stripsConfig.controls.1.pot
MIDIcontrolType stripsConfig.controls.1.pot.controlType
int stripsConfig.controls.1.pot.minVal
int stripsConfig.controls.1.pot.maxVal
int stripsConfig.controls.1.pot.channel
int stripsConfig.controls.1.pot.controlNum
char stripsConfig.controls.1.pot.name
stripsConfig.controls.1.button
MIDIcontrolType stripsConfig.controls.1.button.controlType
int stripsConfig.controls.1.button.minVal
int stripsConfig.controls.1.button.maxVal
int stripsConfig.controls.1.button.channel
int stripsConfig.controls.1.button.controlNum
char stripsConfig.controls.1.button.name

{'uint16_t', 'pattern_t', 'int', 'MIDIcontrolType', 'char'}

24 leaves:
stripsConfig.colours.1.ringLEDs.colour
stripsConfig.colours.1.ringLEDs.pattern
stripsConfig.colours.1.buttonLED.colour
stripsConfig.colours.1.scribble.fg
stripsConfig.colours.1.scribble.bg
stripsConfig.colours.1.scribble.txt
stripsConfig.controls.1.fader.controlType
stripsConfig.controls.1.fader.minVal
stripsConfig.controls.1.fader.maxVal
stripsConfig.controls.1.fader.channel
stripsConfig.controls.1.fader.controlNum
stripsConfig.controls.1.fader.name
stripsConfig.controls.1.pot.controlType
stripsConfig.controls.1.pot.minVal
stripsConfig.controls.1.pot.maxVal
stripsConfig.controls.1.pot.channel
stripsConfig.controls.1.pot.controlNum
stripsConfig.controls.1.pot.name
stripsConfig.controls.1.button.controlType
stripsConfig.controls.1.button.minVal
stripsConfig.controls.1.button.maxVal
stripsConfig.controls.1.button.channel
stripsConfig.controls.1.button.controlNum
stripsConfig.controls.1.button.name

*/
//========================================
extern bool setuint16_t(void* dst, const char* src);
extern bool setpattern_t(void* dst, const char* src);
extern bool setint(void* dst, const char* src);
extern bool setMIDIcontrolType(void* dst, const char* src);
extern bool setchar(void* dst, const char* src);

extern bool getuint16_t(char* dst, void* src);
extern bool getpattern_t(char* dst, void* src);
extern bool getint(char* dst, void* src);
extern bool getMIDIcontrolType(char* dst, void* src);
extern bool getchar(char* dst, void* src);
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
        offsetResult result{-1, 1, 0, nullptr, nullptr}; // not found
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
        offsetResult result{-1, 1, 0, nullptr, nullptr}; // not found
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
        offsetResult result{-1, 1, 0, nullptr, nullptr}; // not found
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
        offsetResult result{-1, 1, 0, nullptr, nullptr}; // not found
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
        offsetResult result{-1, 1, 0, nullptr, nullptr}; // not found
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
        offsetResult result{-1, 1, 0, nullptr, nullptr}; // not found
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
    StripColours colours[NUM_POTS];
    StripControls controls[NUM_POTS];

    static constexpr const char* memberNames[]{"colours", "controls"};
    int getMemberCount(void) { return 2; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1, 1, 0, nullptr, nullptr}; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_ARRAY(colours);
            TO_OFFSET_ARRAY(controls);
        } while (0);
        return result;
    }
};

class FaderMonsterSettings : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"FaderMonsterSettings"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    StripSettings stripsConfig;

    static constexpr const char* memberNames[]{"stripsConfig"};
    int getMemberCount(void) { return 1; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1, 1, 0, nullptr, nullptr}; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET(stripsConfig);
        } while (0);
        return result;
    }
};


