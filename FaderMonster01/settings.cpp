#include "header.h"

//===================================================================
//
//                      888    888                             
//                      888    888                             
//                      888    888                             
//    .d8888b   .d88b.  888888 888888 .d88b.  888d888 .d8888b  
//    88K      d8P  Y8b 888    888   d8P  Y8b 888P"   88K      
//    "Y8888b. 88888888 888    888   88888888 888     "Y8888b. 
//         X88 Y8b.     Y88b.  Y88b. Y8b.     888          X88 
//     88888P'  "Y8888   "Y888  "Y888 "Y8888  888      88888P' 
// 
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
//
//                      888    888                             
//                      888    888                             
//                      888    888                             
//     .d88b.   .d88b.  888888 888888 .d88b.  888d888 .d8888b  
//    d88P"88b d8P  Y8b 888    888   d8P  Y8b 888P"   88K      
//    888  888 88888888 888    888   88888888 888     "Y8888b. 
//    Y88b 888 Y8b.     Y88b.  Y88b. Y8b.     888          X88 
//     "Y88888  "Y8888   "Y888  "Y888 "Y8888  888      88888P' 
//         888                                                 
//    Y8b d88P                                                 
//     "Y88P"                                                  
// 
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
//    888                     888             
//    888                     888             
//    888                     888             
//    888888 .d88b.  .d8888b  888888 .d8888b  
//    888   d8P  Y8b 88K      888    88K      
//    888   88888888 "Y8888b. 888    "Y8888b. 
//    Y88b. Y8b.          X88 Y88b.       X88 
//     "Y888 "Y8888   88888P'  "Y888  88888P' 
//
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


FLASHMEM void dumpSettings(void)
{
    char buf[300];
    CSVlineCount = 0;
    testToCSV(faderMonsterSettings, buf);
    Serial.printf("// %d settings lines\n\n", CSVlineCount);
}