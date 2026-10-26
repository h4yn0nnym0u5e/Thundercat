# Put this Python file in the teensy-tools folder, e.g.
#       C:\Users\Admin\AppData\Local\Arduino15\packages\teensy\tools\teensy-tools\1.62.0
# the add the following to your platform.local.txt to get it run at the right time (see comment...)
#
# after libraries discovery, before sketch build
# recipe.hooks.sketch.prebuild.05.pattern="python" "{teensytools.path}test.py" "--fqbn='{build.fqbn}'" "--indiscovery={build.library_discovery_phase}" "--buildpath={build.path}" "--sourcepath={build.source.path}"

import argparse
import os
import json
import configparser
from git import Repo

# There are lots of print() statements, which can be removed if you want
print("================================================================")
print("pre-build")
bp = "C:\\Users\\Admin\\AppData\\Local\\arduino\\sketches\\68448E63AE46D114DA14A79C54600EEC" # for testing
sp = "D:\\Jonathan\\Teensy\\!Thundercat\\Thundercat\\FaderMonster01" # for testing

###################################################################
def getGitInfo1(fp):
    sha = ""
    brnch = ""
    mod = None
    try:
        with Repo(fp) as repo:
            sha = repo.head.commit.hexsha
            brnch = repo.active_branch.path
            mod = repo.is_dirty()
    except:
        pass
    return sha, brnch, mod

def getGitInfo(fp):
    sha, brnch, mod = getGitInfo1(fp)
    if "" == sha:
        rt, leaf = os.path.split(fp)
        sha, brnch, mod = getGitInfo1(rt)
        
    brnch = brnch.replace("refs/heads", "")
    sha = sha[:7]
    mod = 1 if mod else 0
    
    return sha, brnch, mod
    
###################################################################
def getLibraryVersion(fp):
    version = "unknown"
    name = ""
    
    # try properties file
    config = configparser.ConfigParser(allow_unnamed_section=True)
    ffp = os.path.join(fp,"library.properties")
    try:
        #print(ffp)
        config.read(ffp)
        #print(config.sections())
        version = config.get(configparser.UNNAMED_SECTION, 'version')
        name    = config.get(configparser.UNNAMED_SECTION, 'name')
    except:
        rt, src = os.path.split(fp)
        #print(rt)
        ffp = os.path.join(rt,"library.properties")
        try:
            config.read(ffp)
            version = config.get(configparser.UNNAMED_SECTION, 'version')
            name    = config.get(configparser.UNNAMED_SECTION, 'name')
            fp = rt
        except:
            # try JSON file
            try:
                f = open(os.path.join(fp,"library.json"))
                js = json.load(f)
                version = js['version']
                name    = js['name']
            except:
                pass
            
    sha, brnch, mod = getGitInfo(fp)    
    return version, sha, brnch, name, mod
    
###################################################################
libd = {}
skd = {}
def getLibs(bp):    
    f = open(os.path.join(bp, "libraries.cache"))
    l2 = json.load(f)
    for e in l2:
        v, sha, br, name, mod = getLibraryVersion(e)
        shax = sha
        if sha != "":
            shax = f", SHA {sha[:7]}, {br}"
        print(f"{e} @ {v}{shax}")
        libd[name] = {'version': v, 'sha': sha, 'branch': br, 'path': e, 'name': name, 'mod': mod}
    f.close()

###################################################################
def writeInfo():    
    try:
        guard = "_BUILDINFO_H_"
        # fresh copy of placeholder file will be here, ready for hacking!
        f = open(os.path.join(bp, "sketch", "buildinfo.h"), mode='a')
        f.write(f"#if !defined({guard})\n")
        f.write(f"#define {guard}\n\n")

        # list of libraries
        f.write("#define LIBRARY_LIST \\\n")
        try:
            for e in sorted(libd, key=str.lower):
                ed = libd[e]
                efwd = ed['path'].replace("\\","/")
                mod = ed['mod']
                if "" != ed['name']:                    
                    f.write(f"""\tLIBRARY_ENTRY("{ed['name']}", "{efwd}", "{ed['version']}", "{ed['sha']}", "{ed['branch']}", {mod}) \\\n""")
        except:
            print(f"oops @ {e}")
            pass

        f.write(f"""\n\n#define SKETCH_INFO "{skd['path']}", "{skd['sha']}", "{skd['branch']}", {skd['mod']}\n""")
        
        f.write(f"\n\n#endif // !defined({guard})\n")
        f.close()            
    except:
        pass    
    
###################################################################
parser = argparse.ArgumentParser(
    description="Create modified Tools menu items for Teensyduino"
)
parser.add_argument("--fqbn") # FQBN
parser.add_argument("--indiscovery") # in discovery
parser.add_argument("--buildpath")   # build path (the temporary folder)
parser.add_argument("--sourcepath")  # source path (the sketch folder)
args = parser.parse_args()

###################################################################
# Convert description of menu options into JSON or text
# Use command line options to decide where output goes
if args.fqbn:    
    print(f"FQBN is {args.fqbn}")

if False and args.indiscovery: # flag is present
    s = "I" if args.indiscovery else "Not i"
    print(f"{s}n library discovery")

if args.buildpath: # flag is present
    bp = args.buildpath
else:
    bp = None
    pass

if args.sourcepath: # flag is present
    sp = args.sourcepath
else:
    sp = None
    pass

if bp:
    
    getLibs(bp)

if sp:
    l = list(getGitInfo(sp))
    #print(l)
    skd = {'path': sp.replace("\\","/"), 'sha': l[0], 'branch': l[1], 'mod': l[2]}
    
writeInfo()


print("================================================================")
