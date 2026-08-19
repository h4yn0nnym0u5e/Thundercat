#include "classes.h"
#include <string.h>

#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])
FaderMonsterSettings faderMonsterSettings;
//StripColours stripColours;

const char* testStrings[] = {
	"stripsConfig.0.colours.ringLEDs.colour", 
	"stripsConfig.0.colours.ringLEDs.pattern", // 1
	"stripsConfig.0.colours.buttonLED.colour",
	"stripsConfig.0.colours.scribble.fg", //3
	"stripsConfig.0.colours.scribble.bg",
	"stripsConfig.0.colours.scribble.txt",
	"stripsConfig.0.controls.fader.controlType", // 6
	"stripsConfig.0.controls.fader.minVal", // 7
	"stripsConfig.0.controls.fader.maxVal",
	"stripsConfig.0.controls.fader.channel",
	"stripsConfig.0.controls.fader.controlNum",
	"stripsConfig.0.controls.fader.name",
	"stripsConfig.0.controls.pot.controlType",
	"stripsConfig.0.controls.pot.minVal",
	"stripsConfig.0.controls.pot.maxVal",
	"stripsConfig.0.controls.pot.channel",
	"stripsConfig.0.controls.pot.controlNum",
	"stripsConfig.0.controls.pot.name",
	"stripsConfig.0.controls.button.controlType",
	"stripsConfig.0.controls.button.minVal",
	"stripsConfig.0.controls.button.maxVal",
	"stripsConfig.0.controls.button.channel",
	"stripsConfig.0.controls.button.controlNum",
	"stripsConfig.0.controls.button.name", //23

	"stripsConfig.1.colours.ringLEDs.colour",
	"stripsConfig.1.controls.button.name",

	"stripsConfig.7.colours.ringLEDs.colour",
	"stripsConfig.7.controls.button.name",

	"stripsConfig.0.controls"

};

const char* pattern = "1,2,3,0xcafe,0xbabe,6,7,8,9,0xA, \n 11,12,13,14,15,0x10,17,18,0xDEAD,0xBEEF";
const char* pattern2 = "0xFF0000, \n"
"    0xF00800, 0xE02000, 0xD03000,\n"
"    0xC04000, 0xB05000, 0xA06000, 0x908000,\n"
"    0x80A000, 0x60C000, 0x20D000, 0x00FF00, \n"
"    0x00F020, 0x008040, 0x004080, 0x0000FF,\n"
"    0x2000F0, "
"    0x111111, 0x222222, 0x333333";
//"    0x4000E0, 0x6000A0, 0x8000A0";


//===================================================================
//! MIDI control type
bool setMIDIcontrolType(void* dst, const char* src) 
{
  return setint(dst, src);
}

//! Set LED ring pattern.
//! Source string must have a one-character separator
//! between colour values, which can be hex or decimal;
//! use 0x prefix to force hex and avoid ambiguity
bool setpattern_t(void* dst, const char* src) 
{
  bool ok = true;

  for (int i=0;i<LEDS_PER_RING && ok;i++)
  {
    int offset;

    // remap "max to min" order to LED positions
    int n = 8-i;
    if (n < 0) n += LEDS_PER_RING;
    int& value = (*((pattern_t*) dst))[n];

    // absorb non-numeric characters
    while (*src !=0 && (*src < '0' || *src > '9'))
      src++;
    //Serial.println(src);

    if (strlen(src) > 2 && 'x' == src[1])
      ok = sscanf(src, "%lx%n", (unsigned long*) &value, &offset) == 1;
    else
      ok = sscanf(src, "%ld%n", (long*) &value, &offset) == 1;
    src += offset + 1; // skip number and separator
  }

  return ok;
}

//! hex or decimal integer - hex is preferred, use 0xXXXX
bool setint(void* dst, const char* src) 
{ 
  bool ok = false;
  if (strlen(src) > 2 && 'x' == src[1])
    ok = sscanf(src, "%lx", (unsigned long*) dst) == 1; 
  else
    ok = sscanf(src, "%ld", (long*) dst) == 1;

  return ok;
}

bool setchar(void* dst, const char* src) { strcpy((char*) dst, src);  return true; }

bool setuint16_t(void* dst, const char* src)
{ 
  bool ok = false;
  if (strlen(src) > 2 && 'x' == src[1])
    ok = sscanf(src, "%hx", (uint16_t*) dst) == 1;
  else
    ok = sscanf(src, "%hu", (uint16_t*) dst) == 1;

  return ok;
}

//===================================================================
bool getint(char* dst, void* src) { sprintf(dst, "0x%X", *(int*) src); return false; }
bool getchar(char* dst, void* src) { sprintf(dst, "%s", (char*) src); return false; }
bool getpattern_t(char* dst, void* src) 
{ 
  pattern_t& patt = *(pattern_t*) src;
  const char* sep = "";
  for (int i=0;i<LEDS_PER_RING;i++)
  {
    dst += sprintf(dst, "%s0x%06X", sep, patt[i] & 0xFFFFFF);
    sep = ", ";
  }
  return false; 
}
bool getMIDIcontrolType(char* dst, void* src) { return getint(dst,src); }
bool getuint16_t(char* dst, void* src) { sprintf(dst, "0x%04X", *(uint16_t*) src); return false; }

//===================================================================

offsetResult testToOffset(const char* str)
{
  char* base = (char*) &faderMonsterSettings;
  int dummy;
  offsetResult offsetS = faderMonsterSettings.toOffset(str, dummy);
  int offset = offsetS.offset;
  Serial.printf("Test '%s'; offset is %d; address is %08X", str, offset, base+offset);
  if (nullptr != offsetS.getter)
    Serial.printf("; setter at 0x%08x; getter at 0x%08x\n", (uint32_t) offsetS.setter, (uint32_t) offsetS.getter);
  else
    Serial.println();    
  return offsetS;
}

void testCfgBase(CfgBaseOffset& cfgbo, int indent = 0)
{
  char indt[indent+1];
  memset(indt, ' ', indent);
  indt[indent] = 0;

  int mc = cfgbo.getMemberCount();
  char* base = (char*) &cfgbo;

  Serial.printf("%s%s has %d members\n", indt, cfgbo.getName(-1), mc);
  for (int i=0;i<mc;i++)
  {
    int dummy;
    const char* nm = cfgbo.getName(i);
    offsetResult ofs = cfgbo.toOffset(nm, dummy);
    if (nullptr == ofs.getter) // not a leaf
    {
      Serial.printf("%s  %s\n", indt, nm);
      testCfgBase(*(CfgBaseOffset*)(base+ofs.offset), indent+4);
    }
    else 
    {
      char buf[200]; // patterns are big!
      ofs.getter(buf,base+ofs.offset);
      Serial.printf("%s  %s = %s\n", indt, nm, buf);
    }

  }
}

void setup() 
{
  offsetResult offsetS;
  char* base = (char*) &faderMonsterSettings;
  char buf[50];

  while (!Serial)
    ;
  Serial.println("\n=======\nStarted"); Serial.flush();
  Serial.printf("sizeof faderMonsterSettings is %d\n", sizeof faderMonsterSettings);
  Serial.printf("sizeof stripsConfig[0] is %d\n", sizeof faderMonsterSettings.stripsConfig[0]);
  Serial.printf("address of stripsConfig.0.colours.scribble.fg is %08X\n", (uint32_t) &faderMonsterSettings.stripsConfig[0].colours.scribble.fg);

  for (int i=0;i<COUNT_OF(testStrings); i++)
  {
    offsetS = testToOffset(testStrings[i]);
    int offset = offsetS.offset;
    switch (i)
    {
      default:
        break;

      case 0:
      case 2: 
      case 22:
        Serial.println(); break;

      case 1:
        // setpattern_t(base+offset, pattern2);
        offsetS.setter(base+offset, pattern2);
        for (int j=0;j<LEDS_PER_RING;j++)
          Serial.printf("%06X ", faderMonsterSettings.stripsConfig[0].colours.ringLEDs.pattern[j]);
        Serial.println('\n');
        break;  
        

      case 3 ... 5:
        sprintf(buf,"0x%04x", i*0x1111);
        // setuint16_t(base+offset, buf);
        offsetS.setter(base+offset, buf);
        if (5 == i)
          Serial.printf("%04hX,%04hX,%04hX\n\n",
            faderMonsterSettings.stripsConfig[0].colours.scribble.fg,
            faderMonsterSettings.stripsConfig[0].colours.scribble.bg,
            faderMonsterSettings.stripsConfig[0].colours.scribble.txt
                        );
        break;

      case 6:
        sprintf(buf,"%d", 42);
        // setMIDIcontrolType(base+offset, buf);
        offsetS.setter(base+offset, buf);
        Serial.printf("%d\n\n", faderMonsterSettings.stripsConfig[0].controls.fader.controlType);
        break;  
        
      // stripsConfig.0.controls.button.name
      case 23:
        sprintf(buf,"Button name");
        // setchar(base+offset, buf);
        offsetS.setter(base+offset, buf);
        Serial.printf("%s\n\n", faderMonsterSettings.stripsConfig[0].controls.button.name);
        break;  
        
    }
  }

  {
    MIDIcontrolSetting mcs;
    Serial.printf("%s has %d members\n", mcs.getName(-1), mcs.getMemberCount());
    
    StripColours sc;
    testCfgBase(sc);
  }
}


void loop() {
  // put your main code here, to run repeatedly:

}
