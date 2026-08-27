import fileinput
import re 

fp = "../settings/expanders.h"

# Improve formatting for binary value
def fmtB(rv):
    return rv[0:4] + "'" + rv[4:]

# Output bits are set to 0 in IODIRx
def toIODIR(s):
    rv = re.sub('O', '0', s)
    rv = re.sub('[^0]', '1', rv)
    return fmtB(rv)

# Pull-up bits set to 1 in GPPUx
def toGPPU(s):
    #rv = re.sub('U', '1', s)
    #rv = re.sub('[^1]', '0', rv)
    rv = re.sub('[IO]', '0', s)
    rv = re.sub('[^0]', '1', rv)
    return fmtB(rv)

# Create class with static values for pin settings
def makeClass(pd):
    print("""
class MCP23S17_SETTINGS
{
  public:""", end='')
    
    for u in pd:
        print(f"""
    struct {u}
    {{
        static const uint8_t
            ADDR   = {pd[u]["addr"]},""", end='')
        comma=''
        for p in ['A', 'B']:
            dl = ['x']*8
            for n in pd[u][p]:
                dl[7-int(n)] = pd[u][p][n]["dir"]
            dd="".join(dl)
            #print(f"{u}_GPIO{p} = {dd}, {toIODIR(dd)}, {toGPPU(dd)}")
            print(f"""{comma}
            IODIR{p} = 0b{toIODIR(dd)},""", end='')
            print(f"""
            GPPU{p}  = 0b{toGPPU(dd)}""", end='')
            comma=','
        print(""";
    };""")                  
    print("};")            

# Nothing in this is dependent on the layout
# so probably shouldn't "auto-generate" it
def printLEDsClass():
    print("""class ButtonLED
{
    WS2812Serial& ledString;
    uint8_t* ledMemory;
    int num;
  public:
    ButtonLED(WS2812Serial& _string, uint8_t* mem, int n)
        : ledString{_string}, ledMemory{mem}, num{n}
        {}
          
    void setColour(uint32_t c)    { ledString.setPixel(num,c); }
    void show(void)               { ledString.show(); }
    void setBrightness(uint8_t n) { ledString.setBrightness(n); }
};
""")


def makeLEDsClass(ld):
    # declare all button LEDs
    for led in ld:
        print(f"extern ButtonLED buttonLED_{led};")

    # if enabled, define the button LEDs
    print("""
#if defined(CREATE_BUTTON_LEDS)""")
    
    for led in ld:
        print(f"ButtonLED buttonLED_{led}{{buttonLEDstring, buttonLEDmemory, {ld[led]}}};")

    print("""#endif // defined(CREATE_BUTTON_LEDS)
#undef CREATE_BUTTON_LEDS    
""")
    

def scanHeader(fp):          
    pd = {}
    ld = {}
    fi = fileinput.input(fp)
    for li in fi:
        li=li.strip()

        # pin definitions
        mtch = re.search("^#define +([A-Z0-9_]+) +(U[^ ]+)", li)
        if mtch:
            name = mtch.group(1)
            u,p,n,d = mtch.group(2).split(',')
            # print(name,u,p,n,d)
            if u not in pd:
                pd[u] = {"A": {}, "B": {}, "addr": -1}
            if p not in pd[u]:
                pd[u][p]={}
            pd[u][p][n]={"name": name, "dir":d}  

        # chip address
        mtch = re.search("^#define +ADDR_(U[0-9]+) +([0-9]+)", li)
        if mtch:
            #print(mtch.group(0))
            u = mtch.group(1)
            addr = mtch.group(2)
            if u not in pd:
                pd[u] = {"A": {}, "B": {}, "addr": -1}
            pd[u]["addr"] = addr

        # Button LEDs
        mtch = re.search("^#define +([A-Z_0-9]+)_LED +([0-9]+)", li)
        if mtch:
            name = mtch.group(1)
            pos = mtch.group(2)
            ld[name] = pos
                     
    fi.close() 

    return pd, ld

#print()
#print(pd)
#print()

pd, ld = scanHeader(fp)
makeClass(pd)
print()
makeLEDsClass(ld)

