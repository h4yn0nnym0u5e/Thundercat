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
#include "expanders.h"
#include "dpex.h"

#include <usb_names.h>
extern struct usb_string_descriptor_struct usb_string_serial_number;

extern DPex U3, U5;

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

void getSerialNumber(char* sernum)
{
  //char sernum[10];
  for (size_t i = 0; i < 10; i++)
    sernum[i] = usb_string_serial_number.wString[i];
  sernum[10] = 0;    
}

void setup() {
  Serial.begin(2'000'000);
  // **********************************************************************
  //pinMode(LED_BUILTIN, OUTPUT); // ****** THIS WILL BREAK SPI USE! ********
  // **********************************************************************
  pinMode(USB_T_4, OUTPUT); // 6V enable
  digitalWriteFast(USB_T_4, HIGH); 

  initDPEX();
  initButtonLEDs();
  initRings();
  initScribble();

  // Open serial communications and wait for port to open:
  while (!Serial) { // && millis() < 5000) {
    // wait for serial port to connect.
  }
  Serial.println('\n');

  //writeU5(0x01, ~0x02); // B.1 is output
  Serial.printf("IODIRx set to %04X\n", U5.read16(REG_IODIRA));
  //writeU5_16(REG_GPIOA,  0x02, 0x02); // set B.1 output high
  SET_BIT(USB_X_3, 1);
  
  if (CrashReport) {
    Serial.print(CrashReport);
  }

  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);
  {
    char buf[11];
    getSerialNumber(buf);
    Serial.printf("Teensy serial number is: %s\n", buf);
  }
  //delay(100);
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
  {
    const size_t uidsz = 19;
    uint8_t buffer[uidsz];

    FRAMfs.getUniqueID(buffer,uidsz);

    Serial.printf("MRAM mfr ID: %02X %02X; unique ID: ", buffer[3], buffer[4]);
    for (size_t i=5;i<uidsz;i++)
      Serial.printf("%02X ", buffer[i]);
    Serial.println();      

  }//  MTP.addFilesystem(FRAMfs, FRAMfs.name());
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
  static elapsedMillis em = 0;
//  MTP.loop();

  procSerial();
  updateButtonLEDs();
  updateScribble();

  // update to / from port expanders
  U3.poll();
  U5.poll();

  if (em >= 500)
  {
    em = 0;
    SET_BIT(USB_X_3, 1); // set USB_X_3 - power LED
    if (LEDstate & 4)
      digitalWriteFast(USB_T_4, HIGH); // enable 6V supply
    LEDstate++;
  }

  // monitor strip buttons
  static uint16_t lastPins;
  uint16_t newPins = U3.getGPIO() ^ 0xFFFF;
  if (lastPins != newPins)
  {
    lastPins = newPins;
    Serial.printf("U3 pins: %04X\n", newPins);
  }

  // monitor power button
  static elapsedMillis psw = 0;
  if (psw >= 5)
  {
    psw = 0;

    powerButton = GET_BUTTON(USB_X_4); //U5bits2; // update power button status
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
          SET_BIT(USB_X_3,0); // clear power LED B.1 output
          //writeU5(0x13,  0); // clear power LED B.1 output
          delay(1000);
          Serial.println("shutdown!");
          //writeU5(0x12, 0x80); // shutdown!
          SET_BIT(USB_X_1, 1); // shutdown!
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

  // rings uses # for off, 0-9 for brightness
  if (updateRings(ch) && ch >= 0) // not used, parse to string
  {
    buf[idx] = ch;
    if (idx < BUF_MAX)
      idx++;

    do
    {
      if (ch >= 32) // not terminator - done
        break;

      buf[idx-1] = 0; // terminate string
      if (1 == idx)   // zero-length string - ignore
      {
        idx = 0;
        break;
      }

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
    } while (0);
  }
}
