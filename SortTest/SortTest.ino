#include <KickSort.h>
#include <naturalorder.h>

#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])
class PathPtr
{
  public:
    PathPtr(const char* p) : path{p} {}
    const char* path;

    // compare path to r.path

    // use naturalcompare library, modified (sigh)
    // because it didn't have case-independence and put
    // test files in a stupid place...
    bool operator<(const PathPtr& r)  const 
    { 
      bool result;
      char endCh = '.';
      const char *pls =   path, *ple = strrchr(  path, endCh),
                 *prs = r.path, *pre = strrchr(r.path, endCh);

      result = natural_compare(pls,ple,prs,pre,true) < 0;
//Serial.printf("%s is%s less than %s\n", path, result?"":" not", r.path);
      return result;
    }

    bool operator>(const PathPtr& r)  const { return r < *this; }
    bool operator<=(const PathPtr& r) const { return !(*this > r); } // (const PathPtr&)
    bool operator>=(const PathPtr& r) const { return !(r < *this); }
};

//*
PathPtr files[]
{
	"ascene-1.csv",
	"ascene-10.csv",
	"ascene-5-mix.csv",
	"scene-00.csv",
	"scene-12.csv",
	"scene-17.csv",
	"scene-18.csv",
	"scene-19.csv",
	"MixedUp.csv",
	"NordCtl.csv",
	"Silly.csv",
	"ascene-0b.csv",
	"ascene-0.csv",
	"ascene-5.csv",
	"ascene-6.csv",
	"ascene-7.csv",
	"ascene-9.csv",
	"blue.csv",
	"g2b.csv",
	"ascene-5-10b.csv",
	"ascene-5-10a.csv",
	"ascene-5-1b.csv",
	"green.csv",
	"purple.csv",
	"r2g.csv",
	"red.csv",
	"rryyggbb.csv",
	"yellow.csv",
};
/*/
PathPtr files[]
{
	"ascene-10.csv",
	"ascene-1.csv",
	"ascene-5.csv",
	"ascene-5-10b.csv",
	"ascene-5-10a.csv",
	"ascene-5-1b.csv",
	"ascene-5-mix.csv",
};
//*/

PathPtr files2[]
{
	"ascene-1b.csv",
	"ascene-1.csv",
	"ascene-1a.csv",
	"ascene-0a.csv",
	"ascene-0b.csv",
	"ascene-0.csv",
};

extern "C"
void dumpStr(const char* s)
{
  Serial.printf("%s", s);
}

void setup() 
{
  while (!Serial)
    ;
  Serial.println("\n=====================");
  KickSort<PathPtr>::quickSort(files, COUNT_OF(files));
  for (int i=0;i<COUNT_OF(files);i++)
    Serial.println(files[i].path);
}

void loop() 
{
  delay(10);
}
