/*
  Adapted from examples/Simplified Examples/Example_4_MTP_LFS_SPI_SD/Example_4_MTP_LFS_SPI_SD.ino

  The example showed the use of a available LittleFS wrapper function for multiple
  SPI chips which can include FRAM, NOR FLASH and NAND FLASH.

  Adapted to a single MRAM in the FaderMonster main PCB

  This example code is in the public domain.
*/
#include <LittleFS.h>
//#include <TeensyDebug.h>

/*
   There are two wrapper classes available for use with LittleFS:
   1. lfs_spi: for SPI memory chips
   2. lfs_qspi: for QSPI memory chips - NOR or NAND Flash

   Using these wrappers makes it a bit simpler as all you need to remember
   are the chip select pins for the memory chips on SPI not whether its a NAND or NOR or FRAM

*/
// So for this example let's put memory on /CS pin 3
//FlexIOSPI flexSPI{11,12,13}; // MOSI, MISO, SCK
FlexIOSPI flexSPI{36,34,37}; // MOSI, MISO, SCK - FaderMonster
LittleFS_SPIFram FRAMfs;

extern bool dumpFile(const char* buf);

void setup() {
  Serial.begin(2'000'000);
  // **********************************************************************
  pinMode(LED_BUILTIN, OUTPUT); // ****** THIS WILL BREAK SPI USE! ********
  // **********************************************************************

  initDPEX();
  writeU5(0x01, ~0x02); // B.1 is output
  writeU5(0x13,  0x02); // set B.1 output high
  
  // Open serial communications and wait for port to open:
  while (!Serial) { // && millis() < 5000) {
    // wait for serial port to connect.
  }

  if (CrashReport) {
    Serial.print(CrashReport);
  }

  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  delay(100);
  //halt_cpu();

  // Now let's try our LittleFS SPI
#define notUsedCS 4
//#define FRAM_CS 3
#define FRAM_CS 33 // FaderMonster
  pinMode(notUsedCS, OUTPUT);
  digitalWrite(notUsedCS, HIGH);

  bool ok;
  /*
  if ((ok = FRAMfs.begin(FRAM_CS))) // use SPI bus
  /*/
  if ((ok = FRAMfs.begin(FRAM_CS, flexSPI, false))) // use FlexIOSPI, configured above
  //*/
  {}//  MTP.addFilesystem(FRAMfs, FRAMfs.name());
  else
    Serial.printf("\nStorage not added for pin %d", FRAM_CS);  

  Serial.printf("\n%u Storage list initialized.\n", millis());
  if (ok)
    dumpFile("log.txt");
}

extern "C"
int fnLFS_ERR_NOENT(void)
{
  //halt_cpu();
  return -2;  
}

bool LEDstate;
void loop() 
{
  static elapsedMillis em = 0;
//  MTP.loop();
  procSerial();
  if (em >= 250)
  {
    em = 0;
    digitalWriteFast(LED_BUILTIN, LEDstate);
    writeU5(0x13,  LEDstate?0x02:0); // set B.1 output
    LEDstate = !LEDstate;
  }
}

char buf[50];
int idx;
#define BUF_MAX (int)(sizeof buf - 1)

bool dumpFile(const char* buf)
{
  File f = FRAMfs.open(buf);
  bool ok=false;
  if (f)
  {
    int fch;
    Serial.println("=======================");
    do
    {
      fch = f.read();
      if (fch >= 0)
        Serial.print((char) fch);
        
    } while (fch >= 0);
    f.close();
    Serial.println("\n=======================");
  
    ok = true;
  }
  return ok;  
}

void procSerial(void)
{
  int ch = Serial.read();
  if (ch >= 0)
  {
    buf[idx] = ch;
    if (idx < BUF_MAX)
      idx++;
    if (ch < 32)
    {
      buf[idx-1] = 0;
      Serial.println(buf);

      bool ok = dumpFile(buf);
      
      File f = FRAMfs.open("log.txt", FILE_WRITE);
      if (f)
      {
        f.printf("%s: %s\n", buf, ok?"OK":"fail");
        f.close();
      }
      else
      {
        Serial.println("log failed");
        if (0 == strcmp("fmt", buf))
          FRAMfs.quickFormat();
      }
      //halt_cpu();
      idx = 0;
    }
  }
}
