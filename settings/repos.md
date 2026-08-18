# Changed libraries #

This is a list of the library changes "required" (IMHO) for the 8-fader Monster.

In the main these are extensions to existing behaviour, to allow coexistence of the various libraries in use and optimise their performance when called from FreeRTOS tasks. There are a few bug-fixes, too.


# FreeRTOS #
https://github.com/h4yn0nnym0u5e/freertos-teensy/tree/improved_yield%2Bt4_heap_fix

Ensures
 - heap is allocated from RAM2
 - correct heap usage reporting (RAM1 usage report is non-working)
 - improved `yield()` (?)


# WS2812Serial # 
https://github.com/h4yn0nnym0u5e/WS2812Serial/tree/feature/add-busy

Adds `busy()` method


# TFT_eSPI #
https://github.com/h4yn0nnym0u5e/TFT_eSPI/tree/dev-Teensy-02-add-FlexIOSPI 

Many changes!
 - Teensy 4.x support (not necessarily complete...)
   - DMA
   - SPI and FlexIOSPI busses
   - PSRAM for sprites
 - multi-screen support
   - use function to set /CS
   - phased `init()`
 - `pushImage()` into sprite can now have transparent colour
 - arc drawing now uses `float` angles
 - fix issue with clipped gradient fills
 - fix issue with ESP32-S3, used for SmartKnob
 - add "dirty area" support to TFT_eSprite
 - add async push of sprite to display


# cores #
https://github.com/h4yn0nnym0u5e/cores/tree/A-Dunstan/dmachannel_preempt_detect/teensy4

Enable pre-emptible DMA, needed for co-existence of `WS2812Serial` and `TFT_eSPI`. Without it, screen updates interfere with LED string updates because the Serial FIFO is only 4 bytes and gets used up in 10µs.


# FlexIO_t4 #
https://github.com/h4yn0nnym0u5e/FlexIO_t4/tree/feature/wide-DMA

- tolerate repeated `begin()` calls
- add status and data capability to event callback
- 16- and 32-bit DMA transfers


# ADS8688_ADC_Arduino #
https://github.com/h4yn0nnym0u5e/ADS8688_ADC_ARDUINO/tree/dev/use-FlexIOSPI 

May not be needed long-term.

- enable use of FlexIOSPI


# LittleFS #
https://github.com/h4yn0nnym0u5e/LittleFS/tree/feature/more-FRAM-types 

- add PM004M MRAM as an option
- enable use of SPI or FlexIOSPI for FRAM-type parts
  - could be extended to Flash parts, but I haven't done it


# initGT911 #
https://github.com/h4yn0nnym0u5e/initGT911/tree/dev/teensy-01-async

- fix compile-time issue due to use of ESP32-only macro
- allow use of asynchronous I²C library


# teensy4_i2c # 
https://github.com/h4yn0nnym0u5e/teensy4_i2c/tree/dev/no-restart

- allow option to continue read or write without sending a restart 
- add callbacks to allow non-polled usage with RTOS 
