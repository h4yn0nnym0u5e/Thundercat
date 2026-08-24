#include "classes.h"
#include <string.h>

#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])
FaderMonsterSettings faderMonsterSettings;
//StripColours stripColours;

const char* testStrings[] = {
	"stripsConfig.colours.0.ringLEDs.colour", 
	"stripsConfig.colours.0.ringLEDs.pattern", // 1
	"stripsConfig.colours.0.buttonLED.colour",
	"stripsConfig.colours.0.scribble.fg", //3
	"stripsConfig.colours.0.scribble.bg",
	"stripsConfig.colours.0.scribble.txt",
	"stripsConfig.controls.0.fader.controlType", // 6
	"stripsConfig.controls.0.fader.minVal", // 7
	"stripsConfig.controls.0.fader.maxVal",
	"stripsConfig.controls.0.fader.channel",
	"stripsConfig.controls.0.fader.controlNum",
	"stripsConfig.controls.0.fader.name",
	"stripsConfig.controls.0.pot.controlType",
	"stripsConfig.controls.0.pot.minVal",
	"stripsConfig.controls.0.pot.maxVal",
	"stripsConfig.controls.0.pot.channel",
	"stripsConfig.controls.0.pot.controlNum",
	"stripsConfig.controls.0.pot.name",
	"stripsConfig.controls.0.button.controlType",
	"stripsConfig.controls.0.button.minVal",
	"stripsConfig.controls.0.button.maxVal",
	"stripsConfig.controls.0.button.channel",
	"stripsConfig.controls.0.button.controlNum",
	"stripsConfig.controls.0.button.name", //23

	"stripsConfig.colours.1.ringLEDs.colour",
	"stripsConfig.controls.1.button.name",

	"stripsConfig.colours.7.ringLEDs.colour",
	"stripsConfig.controls.7.button.name",

	"stripsConfig.controls"

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

#if 0 // test class initialisation syntax
#if 1
FaderMonsterSettings f1 
 {
  { 
  {
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
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
     }
    }
 };
#endif

StripColours sc1
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }
    ;

StripColours sca[8]
  {
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, } 
  };

StripColours scb[2]
  {
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
  };

StripColours scc[2]  
{
  {{},{},{}}, // three parameters
  {} // no parameters
};


MIDIcontrolSetting mcs 
{
  MIDIcontrolType::CC,2,"hi",3,4,5
};

StripControls sx 
{
  {MIDIcontrolType::CC,2,"hi", 3,4,5},
  {MIDIcontrolType::CC,2,"hi", 3,4,5},
  {MIDIcontrolType::CC,2,"hi", 3,4,5},
};

StripControls sxa[8]
{
     {{ MIDIcontrolType::CC /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
     {{ /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, { /* controlType, */ /* controlNum, */ /* name, */ /* minVal, */ /* maxVal, */ /* channel, */}, }, 
};

StripSettings ss 
{
  sca, sxa
};

StripSettings ss2
{
  {
    
  },
  sxa
};

#if 0
StripSettings ss3
{
  {
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, }, 
      {{ /* colour, */ /* pattern, */}, { /* colour, */}, { /* fg, */ /* bg, */ /* txt, */}, } 
  },
  {
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }, 
      {{ /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, { /* controlType, */ /* minVal, */ /* maxVal, */ /* channel, */ /* controlNum, */ /* name, */}, }
     }

};
#endif

FaderMonsterSettings f2 
{
  {sca,sxa}
};

FaderMonsterSettings f3
{
  ss2
};
#endif // test class initialisation

//===================================================================
//! MIDI control type
FLASHMEM bool setMIDIcontrolType(void* dst, const char* src) 
{
  return setint(dst, src);
}

//! Set LED ring pattern.
//! Source string must have a one-character separator
//! between colour values, which can be hex or decimal;
//! use 0x prefix to force hex and avoid ambiguity
FLASHMEM bool setpattern_t(void* dst, const char* src) 
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
FLASHMEM bool setint(void* dst, const char* src) 
{ 
  bool ok = false;
  if (strlen(src) > 2 && 'x' == src[1])
    ok = sscanf(src, "%lx", (unsigned long*) dst) == 1; 
  else
    ok = sscanf(src, "%ld", (long*) dst) == 1;

  return ok;
}

FLASHMEM bool setchar(void* dst, const char* src) { strcpy((char*) dst, src);  return true; }

FLASHMEM bool setuint16_t(void* dst, const char* src)
{ 
  bool ok = false;
  if (strlen(src) > 2 && 'x' == src[1])
    ok = sscanf(src, "%hx", (uint16_t*) dst) == 1;
  else
    ok = sscanf(src, "%hu", (uint16_t*) dst) == 1;

  return ok;
}

//===================================================================
FLASHMEM bool getint(char* dst, void* src) { sprintf(dst, "0x%X", *(int*) src); return false; }
FLASHMEM bool getchar(char* dst, void* src) { sprintf(dst, "\"%s\"", (char*) src); return false; }
FLASHMEM bool getpattern_t(char* dst, void* src) 
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
FLASHMEM bool getMIDIcontrolType(char* dst, void* src) { return getint(dst,src); }
FLASHMEM bool getuint16_t(char* dst, void* src) { sprintf(dst, "0x%04X", *(uint16_t*) src); return false; }

//===================================================================

FLASHMEM
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

FLASHMEM
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

int CSVlineCount;
FLASHMEM
void testToCSV(CfgBaseOffset& cfgbo, //!< structure to save
               char* buf,            //!< text buffer: must be big enough!
               const int bufOff=0)   //!< where to append
{
  int mc = cfgbo.getMemberCount();
  char* base = (char*) &cfgbo;

  if (0 == bufOff)
    Serial.printf("\nCSV for %s settings\n", cfgbo.getName(-1));

  for (int i=0;i<mc;i++)
  {
    int dummy;
    const char* nm = cfgbo.getName(i);
    offsetResult ofs = cfgbo.toOffset(nm, dummy); // information on this member
    //Serial.printf("%s has %d elements of size %d\n", nm, ofs.count, ofs.size);
    for (int n=0;n<ofs.count;n++) // deal with array members
    {
      int newOff;
      if (1 == ofs.count)
        newOff = bufOff + sprintf(buf + bufOff, ".%s", nm);
      else
        newOff = bufOff + sprintf(buf + bufOff, ".%s.%d", nm, n);

      if (nullptr == ofs.getter) // not a leaf
      {
        testToCSV(*(CfgBaseOffset*)(base + ofs.offset + n*ofs.size), buf, newOff);
      }
      else 
      {
        newOff = newOff + sprintf(buf + newOff, ", "); // add CSV separator
        ofs.getter(buf+newOff,base + ofs.offset + n*ofs.size);
        Serial.println(buf+1); // omit spurious leading '.'
        CSVlineCount++;
      }
      if (1 != ofs.count)
        Serial.println();
    }
  }
}

FLASHMEM
void setup() 
{
  pinMode(TFT_BLK, OUTPUT);
  digitalWriteFast(TFT_BLK, 0);

  offsetResult offsetS;
  char* base = (char*) &faderMonsterSettings;
  char buf[50];

  while (!Serial)
    ;
  Serial.println("\n=======\nStarted"); Serial.flush();
  if (CrashReport)
  {
    Serial.println(CrashReport);
    while (1)
      delay(100);
  }

  Serial.printf("sizeof faderMonsterSettings is %d\n", sizeof faderMonsterSettings);
  Serial.printf("sizeof stripsConfig.colours[0] is %d\n", sizeof faderMonsterSettings.stripsConfig.colours[0]);
  Serial.printf("address of stripsConfig.colours.0.scribble.fg is %08X\n", (uint32_t) &faderMonsterSettings.stripsConfig.colours[0].scribble.fg);

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
          Serial.printf("%06X ", faderMonsterSettings.stripsConfig.colours[0].ringLEDs.pattern[j]);
        Serial.println('\n');
        break;  
        

      case 3 ... 5:
        sprintf(buf,"0x%04x", i*0x1111);
        // setuint16_t(base+offset, buf);
        offsetS.setter(base+offset, buf);
        if (5 == i)
          Serial.printf("%04hX,%04hX,%04hX\n\n",
            faderMonsterSettings.stripsConfig.colours[0].scribble.fg,
            faderMonsterSettings.stripsConfig.colours[0].scribble.bg,
            faderMonsterSettings.stripsConfig.colours[0].scribble.txt
                        );
        break;

      case 6:
        sprintf(buf,"%d", 42);
        // setMIDIcontrolType(base+offset, buf);
        offsetS.setter(base+offset, buf);
        Serial.printf("%d\n\n", faderMonsterSettings.stripsConfig.controls[0].fader.controlType);
        break;  
        
      // stripsConfig.0.controls.button.name
      case 23:
        sprintf(buf,"Button name");
        // setchar(base+offset, buf);
        offsetS.setter(base+offset, buf);
        Serial.printf("%s\n\n", faderMonsterSettings.stripsConfig.controls[0].button.name);
        break;  
        
    }
  }

  {
    MIDIcontrolSetting mcs;
    Serial.printf("%s has %d members\n", mcs.getName(-1), mcs.getMemberCount());
    
    {
      char buf[300];
      StripColours sc;
      testCfgBase(sc);

      CSVlineCount = 0;
      testToCSV(sc, buf);
      Serial.printf("// %d settings lines\n\n", CSVlineCount);
    }

    {
      //==============================================================================
      char buf[300];
      FaderMonsterSettings sc;//{{{},{  }}};

      //*
      TFTcolours tftc{4,5,6};
      cfgRingLEDs rlc{1, {2,3}};
      StripColours si({1,{2}}, {3}, {4,5,6});
      sc.stripsConfig.colours[0] = si;
      //*/

      for (int i=0;i<NUM_POTS;i++)
      {
        sc.stripsConfig.controls[i].fader.controlNum  = 0x10+i;
        sc.stripsConfig.controls[i].pot.controlNum    = 0x20+i;
        sc.stripsConfig.controls[i].button.controlNum = 0x30+i;
        sprintf(sc.stripsConfig.controls[i].fader.name, "Fader%d", i+1);
        sprintf(sc.stripsConfig.controls[i].pot.name, "Pot%d", i+1);
        sprintf(sc.stripsConfig.controls[i].button.name, "Button%d", i+1);
      }

      CSVlineCount = 0;
      testToCSV(sc, buf);
      Serial.printf("// %d settings lines\n\n", CSVlineCount);

      //==============================================================================
      // save the strip colour scheme
      // there's probably a better way of doing this...
      CSVlineCount = 0;

      offsetResult offsetS = testToOffset("stripsConfig.colours.6.ringLEDs.pattern");
      int offset = offsetS.offset;
      base = (char*) &sc;
      if (offset >= 0)
        offsetS.setter(base+offset, pattern2);
      else
        Serial.println("Failed to find entry");        

      for (int i=0;i<NUM_POTS;i++)
      {
        CfgBaseOffset* psc2 = sc.stripsConfig.colours+i;
        int offset = sprintf(buf, ".stripsConfig.colours.%d", i);
        testToCSV(*psc2, buf, offset);
        Serial.println();
      }
      Serial.printf("// %d settings lines\n\n", CSVlineCount);

    }
  }
}


void loop() {
  // put your main code here, to run repeatedly:

}
