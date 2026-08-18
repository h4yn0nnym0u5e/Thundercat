#include "classes.h"

#define COUNT_OF(a) (sizeof a / sizeof a[0])
FaderMonsterSettings faderMonsterSettings;
//StripColours stripColours;

const char* testStrings[] = {
	"stripsConfig.0.colours.ringLEDs.colour",
	"stripsConfig.0.colours.ringLEDs.pattern",
	"stripsConfig.0.colours.buttonLED.colour",
	"stripsConfig.0.colours.scribble.fg",
	"stripsConfig.0.colours.scribble.bg",
	"stripsConfig.0.colours.scribble.txt",
	"stripsConfig.0.controls.fader.controlType",
	"stripsConfig.0.controls.fader.minVal",
	"stripsConfig.0.controls.fader.maxVal",
	"stripsConfig.0.controls.fader.channel",
	"stripsConfig.0.controls.fader.controlNum",
	"stripsConfig.0.controls.fader.name",
	"stripsConfig.0.controls.pot.controlType",
	"stripsConfig.0.controls.pot.minVal",
	"stripsConfig.0.controls.pot.maxVal",
	"stripsConfig.0.controls.pot.channel",
	"stripsConfig.0.controls.pot.controlNum",
	"stripsConfig.0.controls.pot.name",
	"stripsConfig.0.controls.button.controlType",
	"stripsConfig.0.controls.button.minVal",
	"stripsConfig.0.controls.button.maxVal",
	"stripsConfig.0.controls.button.channel",
	"stripsConfig.0.controls.button.controlNum",
	"stripsConfig.0.controls.button.name",

	"stripsConfig.1.colours.ringLEDs.colour",
	"stripsConfig.1.controls.button.name",

	"stripsConfig.7.colours.ringLEDs.colour",
	"stripsConfig.7.controls.button.name"
};

int testToOffset(const char* str)
{
  int dummy, offset = faderMonsterSettings.toOffset(str, dummy);
  Serial.printf("Test '%s'; offset is %d\n", str, offset);
  return offset;
}

void setup() 
{
  int offset;
  char* base = (char*) &faderMonsterSettings;

  while (!Serial)
    ;
  Serial.println("\n=======\nStarted"); Serial.flush();
  Serial.printf("sizeof faderMonsterSettings is %d\n", sizeof faderMonsterSettings);
  Serial.printf("sizeof stripsConfig[0] is %d\n", sizeof faderMonsterSettings.stripsConfig[0]);

  for (int i=0;i<COUNT_OF(testStrings); i++)
  {
    offset = testToOffset(testStrings[i]);
  }
}


void loop() {
  // put your main code here, to run repeatedly:

}
