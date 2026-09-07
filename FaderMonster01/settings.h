
#if !defined(_SETTINGS_CLASSES_)
#define _SETTINGS_CLASSES_

#include "config.h"

#define TO_OFFSET_LEAF_ARRAY(...)

// types: {'MIDIcontrolSetting', 'StripControls', 'cfgButtonLED', 'StripColours', 'cfgRingLEDs', 'FaderMonsterSettings', 'TFTcolours', 'StripSettings'}
#if 0
{ {
     {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
     {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
     {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
     {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
     {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
     {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
     {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
     {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
    },
 {
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
    },
}, { /* fg, */ /* bg, */ /* txt, */}, 
#endif

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
int stripsConfig.controls.1.fader.controlNum
char stripsConfig.controls.1.fader.name
int stripsConfig.controls.1.fader.minVal
int stripsConfig.controls.1.fader.maxVal
int stripsConfig.controls.1.fader.channel
stripsConfig.controls.1.pot
MIDIcontrolType stripsConfig.controls.1.pot.controlType
int stripsConfig.controls.1.pot.controlNum
char stripsConfig.controls.1.pot.name
int stripsConfig.controls.1.pot.minVal
int stripsConfig.controls.1.pot.maxVal
int stripsConfig.controls.1.pot.channel
stripsConfig.controls.1.button
MIDIcontrolType stripsConfig.controls.1.button.controlType
int stripsConfig.controls.1.button.controlNum
char stripsConfig.controls.1.button.name
int stripsConfig.controls.1.button.minVal
int stripsConfig.controls.1.button.maxVal
int stripsConfig.controls.1.button.channel
mainColours
uint16_t mainColours.fg
uint16_t mainColours.bg
uint16_t mainColours.txt

{'int', 'char', 'uint16_t', 'MIDIcontrolType', 'pattern_t'}

27 leaves:
"stripsConfig.colours.1.ringLEDs.colour", // 0
"stripsConfig.colours.1.ringLEDs.pattern", // 1
"stripsConfig.colours.1.buttonLED.colour", // 2
"stripsConfig.colours.1.scribble.fg", // 3
"stripsConfig.colours.1.scribble.bg", // 4
"stripsConfig.colours.1.scribble.txt", // 5
"stripsConfig.controls.1.fader.controlType", // 6
"stripsConfig.controls.1.fader.controlNum", // 7
"stripsConfig.controls.1.fader.name", // 8
"stripsConfig.controls.1.fader.minVal", // 9
"stripsConfig.controls.1.fader.maxVal", // 10
"stripsConfig.controls.1.fader.channel", // 11
"stripsConfig.controls.1.pot.controlType", // 12
"stripsConfig.controls.1.pot.controlNum", // 13
"stripsConfig.controls.1.pot.name", // 14
"stripsConfig.controls.1.pot.minVal", // 15
"stripsConfig.controls.1.pot.maxVal", // 16
"stripsConfig.controls.1.pot.channel", // 17
"stripsConfig.controls.1.button.controlType", // 18
"stripsConfig.controls.1.button.controlNum", // 19
"stripsConfig.controls.1.button.name", // 20
"stripsConfig.controls.1.button.minVal", // 21
"stripsConfig.controls.1.button.maxVal", // 22
"stripsConfig.controls.1.button.channel", // 23
"mainColours.fg", // 24
"mainColours.bg", // 25
"mainColours.txt", // 26

*/
//========================================
extern bool setint(void* dst, const char* src);
extern bool setchar(void* dst, const char* src);
extern bool setuint16_t(void* dst, const char* src);
extern bool setMIDIcontrolType(void* dst, const char* src);
extern bool setpattern_t(void* dst, const char* src);

extern bool getint(char* dst, void* src);
extern bool getchar(char* dst, void* src);
extern bool getuint16_t(char* dst, void* src);
extern bool getMIDIcontrolType(char* dst, void* src);
extern bool getpattern_t(char* dst, void* src);
//========================================

//! colours for use on a TFT display
class TFTcolours : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"TFTcolours"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    TFTcolours(uint16_t _fg, uint16_t _bg = 0x7BEF, uint16_t _txt = 0xD69A)
    : fg{_fg}, bg{_bg}, txt{_txt} {}
    TFTcolours() {}

    uint16_t fg{0xE3DC}; //!< foreground
    uint16_t bg{0x7BEF}; //!< background
    uint16_t txt{0xD69A}; //!< text

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

//! colour and pattern for use on an LED ring
class cfgRingLEDs : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"cfgRingLEDs"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    cfgRingLEDs(int _colour, pattern_t _pattern = 0x00FFFF)
    : colour{_colour}, pattern{_pattern} {}
    cfgRingLEDs() {}

    int colour{0xFFFF00}; //!< colour (24-bit RGB)
    pattern_t pattern{0x00FFFF}; //!< 20x colours (24-bit RGB)

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

//! colour for use on a button
class cfgButtonLED : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"cfgButtonLED"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    cfgButtonLED(int _colour)
    : colour{_colour} {}
    cfgButtonLED() {}

    int colour{0xFF00FF}; //!< colour (24-bit RGB)

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

//! settings to specify MIDI output generated when a control is changed
class MIDIcontrolSetting : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"MIDIcontrolSetting"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    MIDIcontrolSetting(MIDIcontrolType _controlType, int _controlNum = 2, const char _name[MAX_NAME_LENGTH] = "<unnamed>", int _minVal = 0, int _maxVal = 127, int _channel = 0)
    : controlType{_controlType}, controlNum{_controlNum}, minVal{_minVal}, maxVal{_maxVal}, channel{_channel}    {
      for (int i = 0; i < MAX_NAME_LENGTH; i++) name[i] = _name[i];
    }
    MIDIcontrolSetting() {}

    MIDIcontrolType controlType{MIDIcontrolType::CC}; //!< message type: note / CC / bend etc.
    int controlNum{2}; //!< control / note number
    char name[MAX_NAME_LENGTH]{"<unnamed>"}; //!< name to display on scribble strip
    int minVal{0}; //!< minimum value to send
    int maxVal{127}; //!< maximum value to send
    int channel{0}; //!< MIDI channel to send on

    static constexpr const char* memberNames[]{"controlType", "controlNum", "name", "minVal", "maxVal", "channel"};
    int getMemberCount(void) { return 6; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1, 1, 0, nullptr, nullptr}; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET_LEAF(controlType, MIDIcontrolType);
            TO_OFFSET_LEAF(controlNum, int);
            TO_OFFSET_LEAF(name, char);
            TO_OFFSET_LEAF(minVal, int);
            TO_OFFSET_LEAF(maxVal, int);
            TO_OFFSET_LEAF(channel, int);
        } while (0);
        return result;
    }
};

//! settings for strip MIDI controls
class StripControls : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"StripControls"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    StripControls(MIDIcontrolSetting _fader, MIDIcontrolSetting _pot, MIDIcontrolSetting _button)
    : fader{_fader}, pot{_pot}, button{_button} {}
    StripControls() {}

    MIDIcontrolSetting fader; //!< fader MIDI settings
    MIDIcontrolSetting pot; //!< continuous pot MIDI settings
    MIDIcontrolSetting button; //!< button MIDI settings

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

//! settings for strip colours (display and LEDs)
class StripColours : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"StripColours"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    StripColours(cfgRingLEDs _ringLEDs, cfgButtonLED _buttonLED, TFTcolours _scribble)
    : ringLEDs{_ringLEDs}, buttonLED{_buttonLED}, scribble{_scribble} {}
    StripColours() {}

    cfgRingLEDs ringLEDs; //!< ring LEDs settings
    cfgButtonLED buttonLED; //!< button LED settings
    TFTcolours scribble; //!< scribble display colours

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

//! settings for the strips
class StripSettings : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"StripSettings"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    StripSettings(StripColours _colours[NUM_POTS], StripControls _controls[NUM_POTS])
    {
      for (int i = 0; i < NUM_POTS; i++) colours[i] = _colours[i];
      for (int i = 0; i < NUM_POTS; i++) controls[i] = _controls[i];
    }
    StripSettings() {}

    StripColours colours[NUM_POTS]; //!< array of settings for strip colours
    StripControls controls[NUM_POTS]; //!< array of settings for strip MIDI outputs

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

//! all settings
class FaderMonsterSettings : public CfgBaseOffset
{
  public:
    static constexpr const char* className{"FaderMonsterSettings"};
    const char* getName(int n) { return n<0?className:memberNames[n]; }
    FaderMonsterSettings(StripSettings _stripsConfig, TFTcolours _mainColours)
    : stripsConfig{_stripsConfig}, mainColours{_mainColours} {}
    FaderMonsterSettings() {}

    StripSettings stripsConfig; //!< settings for strips
    TFTcolours mainColours; //!< settings for main LCD

    static constexpr const char* memberNames[]{"stripsConfig", "mainColours"};
    int getMemberCount(void) { return 2; }

    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1, 1, 0, nullptr, nullptr}; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {
            TO_OFFSET(stripsConfig);
            TO_OFFSET(mainColours);
        } while (0);
        return result;
    }
};


#endif // !defined(_SETTINGS_CLASSES_)
