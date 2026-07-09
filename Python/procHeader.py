import re
import fileinput

fn = "scene.h"

fi = fileinput.input(fn)
s = ""
resd = {"image": []}


#def 

for li in fi:
    # Look for image dimensions
    mtch = re.search("SCENE_([^ ]+) * ([0-9]+)", li)
    if mtch:
        resd[mtch.group(1)] = mtch.group(2)

    # Look for image data
    mtch = re.search("^ *0x",li)
    if mtch:
        li = li.strip()
        li = li.replace("0x","").replace(" ","")
        ls = li.split(",")
        if '' == ls[-1]:
            ls = ls[:-1]
        ls = [(int(x,base=16)+1) // 16 for x in ls]
        ls = [f"{x:X}" for x in ls]
        # s = s + "\n" + ''.join(ls) # nibble stream
        s = ''.join(ls) # nibble stream

        hx = ''
        while s != '':
            hx = hx + f"0x{s[:2]}, "
            s = s[2:]
        resd["image"] += [hx]            
        
        

fi.close()
for hx in resd["image"]:
    print(hx)
#print(resd)
#print(s)
