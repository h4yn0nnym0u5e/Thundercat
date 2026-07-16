# RingLEDsOnly sketch #

## Hardware ##
- Teensy 4.x 
- connect Teensy pin 20 to ring board data input
- can just about power from USB 5V, depending on your USB port

## Libraries ##
- needs WS2812Serial library (built into Teensyduino)
  - updated version is needed for The Monster, as it has a `busy()` method added
  - updated `cores` are needed to ensure proper interaction with display DMA

## Usage ##
- compile and upload as normal
- connect to Serial Monitor
- enter numbers from 1 to 9 for intermediate brightnesses; 0 for max brightness
- see feedback in monitor for actual level set
  - note the steps are logarithmic; each is 45% brighter than the previous