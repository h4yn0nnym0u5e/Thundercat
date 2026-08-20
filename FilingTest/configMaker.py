d1 = {
    "TFTcolours":
        [
            "colours for use on a TFT display",
            {"type": "uint16_t", "name": "fg",  "default": "0xE3DC", "doc": "foreground"},
            {"type": "uint16_t", "name": "bg",  "default": "0x7BEF", "doc": "background"},
            {"type": "uint16_t", "name": "txt", "default": "0xD69A", "doc": "text"},
        ],

    "cfgRingLEDs":
        [
            "colour and pattern for use on an LED ring",
            {"type": "int", "name": "colour", "default": "0xFFFF00", "doc": "colour (24-bit RGB)"},
            {"type": "pattern_t", "name": "pattern", "default": "0x00FFFF", "doc": "20x colours (24-bit RGB)"}
        ],

    "cfgButtonLED":
        [
            "colour for use on a button",
            {"type": "int", "name": "colour", "default": "0xFF00FF", "doc": "colour (24-bit RGB)"},
        ],

    "MIDIcontrolSetting":
        [
            "settings to specify MIDI output generated when a control is changed",
            {"type": "MIDIcontrolType", "name": "controlType", "doc": "message type: note / CC / bend etc."},
            {"type": "int", "name": "minVal",  "doc": "minimum value to send"},
            {"type": "int", "name": "maxVal", "default": "127", "doc": "maximum value to send"},
            {"type": "int", "name": "channel", "doc": "MIDI channel to send on"},
            {"type": "int", "name": "controlNum", "default": "2", "doc": "control / note number"},
            {"type": "char", "name": "name",  "default": '"<unnamed>"', "count": "-MAX_NAME_LENGTH", "doc": "name to display on scribble strip"} # char array, but treated as one item
        ],

    "StripControls":
        [
            "settings for strip MIDI controls",
            {"type": "MIDIcontrolSetting", "name": "fader", "doc": "fader MIDI settings"},
            {"type": "MIDIcontrolSetting", "name": "pot", "doc": "continuous pot MIDI settings"},
            {"type": "MIDIcontrolSetting", "name": "button", "doc": "button MIDI settings"},
        ],


    "StripColours":
        [
            "settings for strip colours (display and LEDs)",
            {"type": "cfgRingLEDs", "name": "ringLEDs", "doc": "ring LEDs settings"},
            {"type": "cfgButtonLED", "name": "buttonLED", "doc": "button LED settings"},
            {"type": "TFTcolours", "name": "scribble", "doc": "scribble display colours"}
        ],

    "StripSettings":
        [
            "settings for the strips",
            {"type": "StripColours", "name": "colours", "count": "NUM_POTS", "doc": "array of settings for strip colours"},
            {"type": "StripControls", "name": "controls", "count": "NUM_POTS", "doc": "array of settings for strip MIDI outputs"}, 
        ],

    "FaderMonsterSettings": 
        [
            "all settings",
            {"type": "StripSettings", "name": "stripsConfig", "doc": "settings for strips"}
        ],
}

types = set()

def myPrint(s, addNewline = True):
    global f
    #print(str(s))
    f.write(str(s))
    if addNewline:
        f.write("\n")

def myAdd(s, addNewline = True):
    if addNewline:
        s += '\n'
    return s        

def varToList(name):
    newnames = name.split(',') # in case of multiple members of same type
    return [n.strip() for n in newnames]

defValues = {'MIDIcontrolType': 'MIDIcontrolType::CC', 
             'pattern_t': '0', 
             'int': '0', 
             'uint16_t': '0', 
             'char': '0'}

def listToMembers(l,td):
    global types
    names = []
    memberTypes = []
    s = ""
    for m in l:
        sz = ""
        doc = ""
        defv = ""
        if isinstance(m,str):
            pass
        else:
            type, name = (m["type"], m["name"])
            try:
                if "default" in m:
                    defv = f"{{{m["default"]}}}"
                else:
                    defv = f"{{{defValues[m["type"]]}}}"
            except:
                pass                
            if "doc" in m:
                doc = f" //!< {m['doc']}"
            if "count" in m: # array
                sz = m["count"]
                sz2 = sz.strip('-')
                s += myAdd(f"    {type} {name}[{sz2}]{defv};{doc}")
            else: # simple type or class
                s += myAdd(f"    {type} {name}{defv};{doc}")

            l = varToList(name)
            names += l
            memberTypes += [type]*len(l)
            for n in l:
                td[n] = (type, sz)

    return (s,names,memberTypes) # all the members' names

def dictToStructs(d):
    global types
    for t in d:
        types |= {t}
        myPrint(f"struct {t}\n{{")
        names = listToMembers(d[t])

        myPrint(names)
        myPrint("}\n")

def dictToClasses(d):
    global types
    s = ""
    for t in d:
        types |= {t}
        td = {}
        if isinstance(d[t][0], str):
            s += myAdd(f"//! {d[t][0]}")
        s += myAdd(f"class {t} : public CfgBaseOffset\n{{\n  public:")
        s += myAdd(f"""    static constexpr const char* className{{"{t}"}};""")
        s += myAdd( """    const char* getName(int n) { return n<0?className:memberNames[n]; }""")
        ns, names, memberTypes = listToMembers(d[t], td)

        params = ""
        inits = ""
        sep = ""
        for i in range(0,len(names)):
            params += f"{sep}{memberTypes[i]} _{names[i]}"
            inits += f"{sep}{names[i]}{{_{names[i]}}}"
            sep = ", "

        s += myAdd(f"    {t}({params})")
        s += myAdd(f"    : {inits} {{}}")
        s += myAdd(f"    {t}() {{}}\n")
        s += myAdd(ns)
        #myPrint(names)

        nameStringArrayValue = '", "'.join(names)
        s += myAdd(f"""    static constexpr const char* memberNames[]{{"{nameStringArrayValue}"}};""")
        s += myAdd(f"""    int getMemberCount(void) {{ return {len(names)}; }}""")

        s += myAdd("""
    //-------------------------------------------------
    virtual offsetResult toOffset(const char* str, int& consume)
    {
        offsetResult result{-1, 1, 0, nullptr, nullptr}; // not found
        [[maybe_unused]] int consumed = 0;
        do
        {""")

        for n in names:
            extra = ""
            if td[n][1] != "" and td[n][1][0] != '-':
                extra = "_ARRAY"                
            if td[n][0] in types:
                s += myAdd(f"            TO_OFFSET{extra}({n});")
            else:
                s += myAdd(f"            TO_OFFSET_LEAF{extra}({n}, {td[n][0]});")

        s += myAdd("""        } while (0);
        return result;
    }""")

        s += myAdd("};\n")

    return s        

setTypes = set()
leaves = []
def printAllPaths(d, elem, s):
    global setTypes, leaves
    root = d[elem]
    for e in root:
        if isinstance(e, dict):
            l = varToList(e["name"])
            index = ""
            if "count" in e and e["count"][0] != '-':
                index = ".1"
            for n in l:
                s2 = s+n+index
                type = e["type"]
                if type in d:
                    type = ""
                else:     
                    setTypes |= {type}           
                    type += " "
                    leaves += [s2]
                myPrint(f"{type}{s2}")
                if e["type"] in d:
                    printAllPaths(d, e["type"], s2+'.')

def makeExternSetters(types):
    for type in types:
        myPrint(f"extern bool set{type}(void* dst, const char* src);")

def makeExternGetters(types):
    for type in types:
        myPrint(f"extern bool get{type}(char* dst, void* src);")

##########################################################
includeGuard = "_SETTINGS_CLASSES_"
#dictToStructs(d1)
f = open("settings.h", "w")
f.write(f"""
#if !defined({includeGuard})
#define {includeGuard}

#include "config.h"

#define TO_OFFSET_LEAF_ARRAY(...)

""")

clss = dictToClasses(d1) # creates types

myPrint("// types: " + str(types))
myPrint("/*")
printAllPaths(d1,"FaderMonsterSettings","") # creates leaves and setTypes
myPrint("")
myPrint(setTypes)
myPrint("")
myPrint(f"{len(leaves)} leaves:")
n = 0
for leaf in leaves:
    myPrint(f'"{leaf}", // {n}')
    n += 1
myPrint("")
myPrint("*/")

myPrint("//========================================")
makeExternSetters(setTypes)
myPrint("")
makeExternGetters(setTypes)
myPrint("//========================================\n")

myPrint(clss)
myPrint(f"#endif // !defined({includeGuard})")

f.close()

##########################################################

f = open("settings.csv", "w")
for i in range(1,9):
    for leaf in leaves:
        myPrint(leaf.replace('.1.', f".{i}.") + ",")
f.close()
