from PIL import Image
import re

fn = "scene.png"

# lots of images
# SmartKnob
fnl = [
    "scene.png",
    "bulb.png",
    "cross.png",
    "gears.png",
    "note.png",
    "rainbow.png",
    "runner.png",
    "specs.png",
]

# just the buttons
fnl = [
    "tl_button.png",
    "tr_button.png",
    "bl_button.png",
    "br_button.png",
    "keycap28.png",
    "keycap54.png",
    "keycap90.png",
    "keycap54_1a.png",
    "keycap54_A1.png",
    "keycap54_Aa.png",
    "keycap54_cross.png",
    "keycap54_del.png",
    "keycap54_larr.png",
    "keycap54_tick.png",
]
ofn = "images.cpp"

def dumpHeader(hdr):
    print(hdr["width"], hdr["height"])
    for l in hdr["data"]:
        print(l)


def mkInfo(hdr):
    result = "PROGMEM const image_4bit_info\n"
    result += f"{hdr['name']}_info = {{{hdr['width']}, {hdr['height']}, {hdr['name']}_data }};\n"
    return result

def mkData(hdr):
    result = "PROGMEM static const image_4bit_data\n"
    result += f"{hdr['name']}_data[] = {{\n"
    for l in hdr["data"]:
        result += f"\t{l}\n"
    result += "};\n"
    return result

def mkImageData(hdr):
    result = mkData(hdr)
    result += '\n' + mkInfo(hdr)
    return result

def mkHeader(fn):
    img = Image.open(fn)
    w,h = img.size
    fname = re.sub("[.].*","",fn)
    result = {"name": fname, "width": w, "height": h, "data": []}
    for hh in range(0,h):
        pxs=''
        dtl = ''
        for ww in range(0,w):
            pxv = img.getpixel((ww,hh)) + 1
            pxv = pxv // 16
            pxs += f"{pxv:X}"
            if len(pxs) > 1:
                #print(f"0x{pxs},",end='')
                dtl += f"0x{pxs},"
                pxs = ''
        #print()
        result["data"] += [dtl]
    return result        

f = open(ofn, mode = "w")
f.write("""#include "header.h"

#if !defined(IMAGES_AS_HEADER)

""")
for fn in fnl:
    hdr = mkHeader(fn)
    #dumpHeader(hdr)
    f.write(mkImageData(hdr))
    f.write("\n\n")

f.write("""#else

extern const image_4bit_info
    """)
s = ",\n    ".join(fnl)
s = s.replace(".png","_info")
f.write(s+";\n")
f.write("""
#endif // defined(IMAGES_AS_HEADER)
""")
f.close()    
