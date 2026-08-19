d1 = {
    "TFTcolours":
        [
            ("uint16_t", "fg, bg, txt")
        ],

    "cfgRingLEDs":
        [
            ("int", "colour"),
            ("pattern_t", "pattern")
        ],

    "cfgButtonLED":
        [
            ("int", "colour"),
        ],

    "MIDIcontrolSetting":
        [
            ("MIDIcontrolType", "controlType"),
            ("int", "minVal, maxVal, channel, controlNum"),
            ("char", "name", "-MAX_NAME_LENGTH")
        ],

    "StripControls":
        [
            ("MIDIcontrolSetting", "fader, pot, button"),
        ],


    "StripColours":
        [
            ("cfgRingLEDs", "ringLEDs"),
            ("cfgButtonLED", "buttonLED"),
            ("TFTcolours", "scribble")
        ],

    "StripSettings":
        [
            ("StripColours", "colours", "NUM_POTS"),
            ("StripControls", "controls", "NUM_POTS"), 
        ],

    "FaderMonsterSettings": 
        [
            ("StripSettings", "stripsConfig")
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

def listToMembers(l,td):
    global types
    names = []
    s = ""
    for m in l:
        sz = ""
        if 3 == len(m): # array
            type, name, sz = m
            sz2 = sz.strip('-')
            s += myAdd(f"    {type} {name}[{sz2}];")
        else: # simple type or class
            type, name = m
            s += myAdd(f"    {type} {name};")

        l = varToList(name)
        names += l
        for n in l:
            td[n] = (type, sz)

    return (s,names) # all the members' names

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
        s += myAdd(f"class {t} : public CfgBaseOffset\n{{\n  public:")
        s += myAdd(f"""    static constexpr const char* className{{"{t}"}};""")
        s += myAdd( """    const char* getName(int n) { return n<0?className:memberNames[n]; }""")
        ns, names = listToMembers(d[t], td)
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
        l = varToList(e[1])
        index = ""
        if len(e) > 2 and e[2][0] != '-':
            index = ".1"
        for n in l:
            s2 = s+n+index
            type = e[0]
            if type in d:
                type = ""
            else:     
                setTypes |= {type}           
                type += " "
                leaves += [s2]
            myPrint(f"{type}{s2}")
            if e[0] in d:
                printAllPaths(d, e[0], s2+'.')

def makeExternSetters(types):
    for type in types:
        myPrint(f"extern bool set{type}(void* dst, const char* src);")

def makeExternGetters(types):
    for type in types:
        myPrint(f"extern bool get{type}(char* dst, void* src);")

##########################################################
#dictToStructs(d1)
f = open("settings.h", "w")
f.write("""
#include "config.h"

#define TO_OFFSET_LEAF_ARRAY(...)

""")

clss = dictToClasses(d1)

myPrint("// types: " + str(types))
myPrint("/*")
printAllPaths(d1,"FaderMonsterSettings","")
myPrint("")
myPrint(setTypes)
myPrint("")
myPrint(f"{len(leaves)} leaves:")
for leaf in leaves:
    myPrint(leaf)
myPrint("")
myPrint("*/")

myPrint("//========================================")
makeExternSetters(setTypes)
myPrint("")
makeExternGetters(setTypes)
myPrint("//========================================\n")

myPrint(clss)

f.close()

##########################################################

f = open("settings.csv", "w")
for i in range(1,9):
    for leaf in leaves:
        myPrint(leaf.replace('.1.', f".{i}.") + ",")
f.close()
