#include "config.h" // gives us some sizes we need
#include <string.h>
/*
 * Configuration for a strip
 * This is outside the strip, as we want a convenient method to
 * load it from the filesystem
 */
 
//! pattern for one LED ring 
class pattern_t
{
    int values[LEDS_PER_RING];
  public:
    int& operator[](int n) { return values[n]; }
    int* getPointer(void) { return &values[0]; }
};

//! Find offset of class member given its name as a string
class CfgBaseOffset
{
  public:
    virtual int toOffset(const char* str, int& consume) = 0;
};

//! structure to 

//! Possible MIDI control types
enum class MIDIcontrolType : int 
{
    undefined = 0,
    CC = 1,     //!< control change
    RPN = 2,    //!< registered parameter number
    NRPN = 3,   //!< non-registered parameter number
    BEND = 4,   //!< pitch bend
    PC = 5,     //!< program change
    AT = 6,     //!< aftertouch
    NOTE = 7    //!< note on / off (button)
};

#define TO_OFFSET(mbr) \
        { int mbrlen = strlen(#mbr); \
        if (0 == strncmp(str, #mbr, mbrlen) && ('.' == str[mbrlen] || 0 == str[mbrlen])) \
            { result = (char*) &mbr - (char*) this; consumed += mbrlen+1; str += mbrlen;\
                if (0 != *str) { int extra = mbr.toOffset(str+1, consume); result = extra<0?extra:(result+extra); }\
                break; }}

#define TO_OFFSET_LEAF(mbr) \
        { int mbrlen = strlen(#mbr); \
        if (0 == strncmp(str, #mbr, mbrlen) && ('.' == str[mbrlen]  || 0 == str[mbrlen])) \
            { result = (char*) &mbr - (char*) this; consumed += mbrlen+1; str += mbrlen;\
                break; }}

#define TO_OFFSET_ARRAY(mbr) \
        { int mbrlen = strlen(#mbr); \
        if (0 == strncmp(str, #mbr, mbrlen) && ('.' == str[mbrlen] || 0 == str[mbrlen])) \
        { \
            result = (char*) &mbr - (char*) this; consumed += mbrlen+1; str += mbrlen; \
            if (0 == *str) { Serial.println(str); result = -1; break; } /* isn't .n. */ \
            int index, n; n = sscanf(str+1,"%d%n",&index,&mbrlen); \
            if (n<1) {Serial.println(str); result = -1; break; } else { result += index*(sizeof mbr[0]); str += mbrlen+1; } \
            if (0 != *str) \
                { int extra = mbr[0].toOffset(str+1, consume); result = extra<0?extra:(result+extra); }\
            break; }}



//                      888    888    d8b                            
//                      888    888    Y8P                            
//                      888    888                                   
//    .d8888b   .d88b.  888888 888888 888 88888b.   .d88b.  .d8888b  
//    88K      d8P  Y8b 888    888    888 888 "88b d88P"88b 88K      
//    "Y8888b. 88888888 888    888    888 888  888 888  888 "Y8888b. 
//         X88 Y8b.     Y88b.  Y88b.  888 888  888 Y88b 888      X88 
//     88888P'  "Y8888   "Y888  "Y888 888 888  888  "Y88888  88888P' 
//                                                      888          
//                                                 Y8b d88P          
//                                                  "Y88P"           
//
// from configMaker.py:
#include "settings.h"

/*
 Examples:
 Complete scene (FaderMonsterSettings)
 settings.stripsConfig[n].colours.scribble.fg = 0x1234
 settings.stripsConfig[n].colours.ringLEDs.colour = 0xFF00FF
 settings.stripsConfig[n].controls.fader.controlType = 2

 Colour scheme (StripColours)
 settings.scribble.bg = 0x1234
 settings.buttonLED.colour = 0x4080E0
 
 External file reference
 settings.stripsConfig[n].colours =  "shiny.csv"
 settings.stripsConfig[n].controls.fader = "CC1.csv"
 settings.stripsConfig[n].controls = "CC1+bend+pedal.csv"

 Regexp
 stripsConfig[1-8].(colours|controls)
*/ 
