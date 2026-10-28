#include "header.h"

//=================================================
#if defined(LIBRARY_LIST) // we have the hacky build recipe!
PROGMEM LibInfo libinfo[]{
#define LIBRARY_ENTRY(lib,path,version,sha,branch,mod) {lib,path,version,sha,branch,mod},
  LIBRARY_LIST
#undef LIBRARY_ENTRY
};

PROGMEM SketchInfo sketchInfo{SKETCH_INFO};

FLASHMEM 
void printSketchInfo(void)
{
  Serial.printf("branch: %s; SHA: %s%s\n", sketchInfo.branch, sketchInfo.sha, sketchInfo.modified?"; modified":"");
}

FLASHMEM 
void printLibraryInfo(void)
{
    Serial.println("Libraries:");
    for (int i=0;i<COUNT_OF(libinfo);i++)
      Serial.printf("%22s @ %-12s; %30s (%7s%s)\n", 
                    libinfo[i].lib, libinfo[i].version,
                    libinfo[i].branch, libinfo[i].sha,
                  libinfo[i].modified?"*":" ");
}
//=================================================

#else

FLASHMEM 
void printSketchInfo(void){}
FLASHMEM 
void printLibraryInfo(void){}

#endif // defined(LIBRARY_LIST)