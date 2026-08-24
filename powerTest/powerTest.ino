/*
  Adapted from examples/Simplified Examples/Example_4_MTP_LFS_SPI_SD/Example_4_MTP_LFS_SPI_SD.ino

  The example showed the use of a available LittleFS wrapper function for multiple
  SPI chips which can include FRAM, NOR FLASH and NAND FLASH.

  Adapted to a single MRAM in the FaderMonster main PCB

  This example code is in the public domain.
*/
#include <LittleFS.h>
//#include <TeensyDebug.h>
#include "Touches.h"
#include "hardware.h"

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
  pinMode(USB_T_4, OUTPUT); // 6V enable

  initDPEX();

  // Open serial communications and wait for port to open:
  while (!Serial) { // && millis() < 5000) {
    // wait for serial port to connect.
  }
  Serial.println('\n');

  //writeU5(0x01, ~0x02); // B.1 is output
  Serial.printf("IODIRx set to %04X\n", readU5_16(0));
  writeU5(0x13,  0x02); // set B.1 output high
  
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
  if ((ok = FRAMfs.begin(FRAM_CS, flexSPI, true))) // use FlexIOSPI, configured above
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

uint8_t LEDstate;
TouchStatus powerButton;
void loop() 
{
  static uint16_t U5bits;
  static elapsedMillis em = 0;
//  MTP.loop();
  procSerial();
  if (em >= 500)
  {
    em = 0;
    digitalWriteFast(LED_BUILTIN, LEDstate & 1);
    //writeU5(0x13,  LEDstate?0x02:0); // set B.1 output
    writeU5(0x13,  LEDstate&2); // set B.1 output
    digitalWriteFast(USB_T_4, LEDstate & 4); // toggle 6V supply
    LEDstate++;
    //Serial.printf("%04X\n", U5bits);
  }

  static elapsedMillis psw = 0;
  if (psw >= 5)
  {
    psw = 0;
    U5bits = readU5_16(0x12);

    uint16_t U5bits2 = ~U5bits & 0x0020;
    powerButton = U5bits2; // update power button status
    /*
    if (0 != (power_switch ^ U5bits2))
    {
      power_switch = U5bits2;
      Serial.printf("Power switch %s\n", 0 != power_switch?"pressed":"released");
    }
    */
  }

  if (powerButton.isChangedStatus())
  {
    static bool hadLongPress = false;
    TouchStatus::eStatus e = powerButton.getExtendedStatus();
    Serial.printf("Power button status is %d\n", e);

    switch (e)
    {
      default:
        break;

      case TouchStatus::eStatus::LONG:
        hadLongPress = true;
        Serial.println("Release to power off");
        break;

      case TouchStatus::eStatus::OFF:
        if (hadLongPress)
        {
          Serial.print("Off ... ");
          delay(1000);
          Serial.print("6V ... ");
          digitalWriteFast(USB_T_4, LOW); // 6V supply off
          delay(1000);
          Serial.print("power LED ... ");
          writeU5(0x13,  0); // clear power LED B.1 output
          delay(1000);
          Serial.println("shutdown!");
          writeU5(0x12, 0x80); // shutdown!
          for (int i=0;i<100;i++)
          {
            Serial.print('.');
            delay(10);
          }
        }
        break;
    }
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
