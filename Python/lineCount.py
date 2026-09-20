# Quick and dirty line count utility
import os
import fileinput
import re

dp = "../FaderMonster01"
tlc = 0
tfc = 0
for root,dirs,files in os.walk(dp):
    for file in sorted(files, key=str.lower):
        if re.search("[.](c|h|cpp|ino)$", file):
            tfc += 1
            lc = 0
            f = fileinput.input(os.path.join(dp,file))
            for l in f:
                lc += 1
            f.close()
            lc += 1 # doesn't count the last line  
            tlc += lc              
            print(f"{file}: {lc}")
print(f"{tfc} files; total lines: {tlc}")            
