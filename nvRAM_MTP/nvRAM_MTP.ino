/*
  Adapted from examples/Simplified Examples/Example_4_MTP_LFS_SPI_SD/Example_4_MTP_LFS_SPI_SD.ino

  The example showed the use of a available LittleFS wrapper function for multiple
  SPI chips which can include FRAM, NOR FLASH and NAND FLASH.

  Adapted to a single FRAM

  This example code is in the public domain.
*/
#include <LittleFS.h>
#include <MTP_Teensy.h>

#define MAINLCD_SPI_PINS 36,34,37
#define MRAM_CS 33

/*
   There are two wrapper classes available for use with LittleFS:
   1. lfs_spi: for SPI memory chips
   2. lfs_qspi: for QSPI memory chips - NOR or NAND Flash

   Using these wrappers makes it a bit simpler as all you need to remember
   are the chip select pins for the memory chips on SPI not whether its a NAND or NOR or FRAM

*/
// So for this example lets assume we memory on pins 3, 4, 5 and 6.
// This creates an LittleFS_SPIxxxx instance for each chip
FlexIOSPI SPIflex(MAINLCD_SPI_PINS, -1); // Setup on (int mosiPin, int misoPin, int sckPin, int csPin=-1) :
LittleFS_SPIFram FRAMfs;

void setup() {
  // start up MTPD early which will if asked tell the MTP
  // host that we are busy, until we have finished setting
  // up...
  Serial.begin(2'000'000);
  MTP.begin();

  // Open serial communications and wait for port to open:
  while (!Serial && millis() < 5000) {
    // wait for serial port to connect.
  }

  if (CrashReport) {
    Serial.print(CrashReport);
  }

  Serial.println("\n" __FILE__ " " __DATE__ " " __TIME__);

  Serial.printf("%u Initializing MTP Storage list ...", millis());


  // Now let's try our LittleFS SPI
#define notUsedCS 4
#define FRAM_CS 3
  pinMode(notUsedCS, OUTPUT);
  digitalWrite(notUsedCS, HIGH);

  if (FRAMfs.begin(MRAM_CS, SPIflex, true))
    MTP.addFilesystem(FRAMfs, FRAMfs.name());
  else
    Serial.printf("Storage not added for pin %d\n", FRAM_CS);  

  Serial.printf("%u Storage list initialized.\n", millis());
}


void loop() {
  MTP.loop();
}