# Development modifications #
## Main PCB v 1.12 ##
 - I: R56 -> 9k1; reduce spurious +6V_PGOOD LED illumination 
 - II: I + R18 / R51 / R54 / R55 / R17 / R30 -> 33R (or 36R or 39R) - speed up SPI edges; scribble now OK at 60MHz; main display OK on FlexIOSPI at 60MHz; MRAM OK (usually...)

## Power switch ##
First batch incorrectly wired 

## Teensy 4.1 ##
Add PSRAM

## LED Ring v2.18c R1 ##
Check cable - power / DIN need swapping

## Scribble strip v1.06 ##
Check cable - top / bottom rows need swapping

## Main LCD ##
Check cable - wiring reversed
