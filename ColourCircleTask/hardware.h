#if !defined(_HARDWARE_H_)
#define _HARDWARE_H_

// ADCs - pots and faders
#define ADCS_COMMON_CS 7
#define ADCS_FADERS_CS 9
#define ADCS_MISO 1
#define ADCS_MOSI 26
#define ADCS_POTS2_CS 8
#define ADCS_SCK 27

#define ADCS_SPI SPI1 // shared with port expanders

// Button LEDs
#define BUTTONS_DIN 14

#define BUTTONS_SERIAL Serial3

// Main LCD touch screen
#define CTP_INT 32

#define CTP_I2C Wire1

// Port expanders
#define DPEX_CS 4
#define DPEX1_INT 30
#define DPEX2_INT 31

#define DPEX_SPI SPI1 // shared with ADCs

// Expression pedal
#define EXPR_PED_ADC 40

#define EXPR_PED_I2C Wire

// Faders touch 
#define FADERS_TOUCH_INT 39
#define FADERS_TOUCH_SCL 19
#define FADERS_TOUCH_SDA 18

#define FADERS_TOUCH_I2C Wire

// Ambient light sensor
#define LT_SENS_ADC 41

// Main LCD
#define MAINLCD_BL USB_T_2
#define MAINLCD_CS 2
#define MAINLCD_DC 0
#define MAINLCD_MISO 34
#define MAINLCD_MOSI 36
#define MAINLCD_SCK 37

#define MAINLCD_SPI FlexIOSPI
#define MAINLCD_SPI_PINS MAINLCD_MOSI,MAINLCD_MISO,MAINLCD_SCK

// Serial MIDI
#define MIDI_TX 35

#define MIDI_SERIAL Serial8

// MRAM file storage
#define MRAM_CS 33

#define MRAM_SPI MAINLCD_SPI

// Pots touch
#define POTS_TOUCH_INT 38
#define POTS_TOUCH_SCL 16
#define POTS_TOUCH_SDA 17

#define POTS_TOUCH_I2C Wire1

// Ring LEDs
#define RINGS_DIN 20

#define RINGS_SERIAL Serial5

// Scribble LCDs
#define SCRIBBLE_BL 6
#define SCRIBBLE_CS 15
#define SCRIBBLE_DC 10
#define SCRIBBLE_MISO 12
#define SCRIBBLE_MOSI 11
#define SCRIBBLE_MUX_A 23
#define SCRIBBLE_MUX_B 22
#define SCRIBBLE_MUX_C 21
#define SCRIBBLE_SCK 13

#define SCRIBBLE_SPI SPI

// SmartKnob serial
#define SK_RX 29
#define SK_TX 28

#define SK_SERIAL Serial7
#define SK_I2C Wire // unused

// USB+Power I/O (optional, currently use port expanders)
#define USB_T_1  3
#define USB_T_2  5 // main LCD backlight
#define USB_T_3 24
#define USB_T_4 25 // 6V enable: active high

#endif // !defined(_HARDWARE_H_)
