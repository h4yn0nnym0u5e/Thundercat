#include <MIDI.h>
#include <FlexIO_t4.h>
#include <FlexIOSPI.h>

#define MAINLCD_MISO 34 // not used for LCD, but is used for MRAM
#define MAINLCD_MOSI 36
#define MAINLCD_SCK  37
#define MAINLCD_SPI_PINS MAINLCD_MOSI,MAINLCD_MISO,MAINLCD_SCK
FlexIOSPI SPIflex(MAINLCD_SPI_PINS, -1); // Setup on (int mosiPin, int misoPin, int sckPin, int csPin=-1)

MIDI_CREATE_INSTANCE(HardwareSerialIMXRT, Serial8, SerialMIDI);

void setup() {
  SerialMIDI.begin();
  SPIflex.begin();
  pinMode(34,INPUT);
}

uint8_t cc, val;

void loop() {
  SerialMIDI.sendControlChange(cc,val, 1);
  delay(100);
  if (++cc > 127)
  {
    cc = 0;
    if (++val > 127)
      val = 0;
  }
}
