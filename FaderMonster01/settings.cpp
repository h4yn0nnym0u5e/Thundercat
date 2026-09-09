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

/**
 * Set character array from source buffer.
 * Removes leading and trailing quotation marks if present.
 */
FLASHMEM bool setchar(void* _dst, const char* src) 
{ 
    char* dst = (char*) _dst;
    if ('"' == *src) src++; // strip leading "
    int len = strlen(src);
    strcpy(dst, src);
    if ('"' == dst[len-1]) dst[len-1] = 0; // strip trailing "
    return true; 
}

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

FLASHMEM
offsetResult Settings::testToOffset(char* str)
{
    FaderMonsterSettings* dummyFMS{&faderMonsterSettings}; // get a thing of a type
    // char* base = (char*) dummyFMS;
    int dummy;
    offsetResult offsetS = dummyFMS->toOffset(str, dummy);
    /*
    int offset = offsetS.offset;
    Serial.printf("Test '%s'; offset is %d; address is %08X", str, offset, base+offset);
    if (nullptr != offsetS.getter)
        Serial.printf("; setter at 0x%08x; getter at 0x%08x\n", (uint32_t) offsetS.setter, (uint32_t) offsetS.getter);
    else
        Serial.println();
    */
    return offsetS;
}

int CSVlineCount;
FLASHMEM
void Settings::testToCSV(Stream& s,
               CfgBaseOffset& cfgbo, //!< structure to save
               char* buf,            //!< text buffer: must be big enough!
               const int bufOff)     //!< where to append
{
  int mc = cfgbo.getMemberCount();
  char* base = (char*) &cfgbo;

  if (0 == bufOff)
  {
taskENTER_CRITICAL();
    s.printf("\nCSV for %s settings\n", cfgbo.getName(-1));
taskEXIT_CRITICAL();
  }

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
        testToCSV(s, *(CfgBaseOffset*)(base + ofs.offset + n*ofs.size), buf, newOff);
      }
      else 
      {
        newOff = newOff + sprintf(buf + newOff, ", ");         // add CSV separator
        ofs.getter(buf+newOff,base + ofs.offset + n*ofs.size); // and append the value
taskENTER_CRITICAL();
        s.println(buf+1); // omit spurious leading '.'
taskEXIT_CRITICAL();
        CSVlineCount++;
      }
      if (1 != ofs.count)
      {
taskENTER_CRITICAL();
        s.println();
taskEXIT_CRITICAL();
      }
    }
  }
}

FLASHMEM
void Settings::loadFromCSV(File& s,
                           CfgBaseOffset& cfgbo)
{
    [[maybe_unused]] char* base = (char*) &cfgbo;
    offsetResult offsetS;
    char buf[300];
    int offset = 0, avail = 0;

    do 
    {
taskENTER_CRITICAL();
        avail += s.read(buf+offset, sizeof buf - offset); // ensure buffer is full
taskEXIT_CRITICAL();
        char* comma = (char*) memchr(buf, ',', sizeof buf);
        char* eol   = (char*) memchr(buf, '\n', sizeof buf);
        if (nullptr == eol) // badly-formed file - give up
            break;

        if (nullptr != comma && comma < eol) // no comma - empty or comment
        {
            settingsTypes bitBucket;
            char valueAsString[300]{"<bad>"};
            bool valueOK = false;

            *comma = 0;                     // terminate the element path
            *eol = 0;                       // and the value
            offsetS = testToOffset(buf);    // find the info about it
            if (nullptr != offsetS.setter)
            {
                do
                {
                    comma++;
                } while (*comma == ' ' && comma < eol);
                valueOK = offsetS.setter(&bitBucket, comma); // get the value
                if (valueOK) // actually load to structure!
                    offsetS.setter(base + offsetS.offset, comma);
                offsetS.getter(valueAsString, &bitBucket);
            }
            if (offsetS.offset < 464)
                Serial.printf("Element '%s' with value field '%s'; offset %d; parsed value %s\n",
                            buf, comma, offsetS.offset, valueAsString);

        }
        offset = eol - buf + 1; // absorb this many characters
        memmove(buf, buf+offset, sizeof buf - offset); // shuffle bytes up
        offset = sizeof buf - offset; // where to load more data
        avail -= eol - buf + 1;       // how much we currently have
        if (avail < 5) break; // magic - 'n,1\n' could be a setting!
    } while (1);
}

FLASHMEM 
void Settings::save(Stream& s)
{
    char buf[300];
    CSVlineCount = 0;
    testToCSV(s, faderMonsterSettings, buf);
    s.printf("// %d settings lines\n\n", CSVlineCount);
}

FLASHMEM 
void Settings::load(File& s, FaderMonsterSettings& f)
{
    loadFromCSV(s,f);
}
