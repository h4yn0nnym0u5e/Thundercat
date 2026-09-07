#if !defined(_CONFIG_H_)
#define _CONFIG_H_


// ============== Pot board ==============
// common reset pin
// #define RST_PIN 6 // on a port expander now

// Pots touch controller
//*
#define TOUCH_WIRE       Wire1
#define TOUCH_WIRE_ASYNC Master1
#define TOUCH_ADDR 0x1C
#define CHANGE_PIN POTS_TOUCH_INT
/*/
// Temporary test for faders touch
#define TOUCH_WIRE FADERS_TOUCH_I2C
#define TOUCH_ADDR 0x1C
#define CHANGE_PIN FADERS_TOUCH_INT
//*/

// ADCs
#define ADC_SPI SPI1
#define ADC_CS  ADCS_COMMON_CS
#define ADC_CLK 15'000'000
#define ADC_CS_HIGH_NS (30+10) // 30ns + 10ns margin
#define CH1_POL -1.0f
#define CH2_POL +1.0f
#define NUM_POTS   8
#define NUM_FADERS 8
#define POT_MAP {4,2,0,6}
#define POT_READ_INTERVAL_US 1'000


// ============== LED rings ==============
#define LEDS_PER_RING 20
#define LED_DRIVE_PIN 20
#define LED_TOP_OFFSET 8

// some basic colours
#define xRED    0xFF0000
#define xORANGE 0xC02000
#define xYELLOW 0xB09000
#define xGREEN  0x00A000 // be a bit conservative here
#define xBLUE   0x0000FF
#define xPURPLE 0x2000C0
#define xPINK   0xC00060
#define xWHITE  0xC0C0C0
#define xBLACK  0x000000

// ============== Scribble board ==============
#define MUX_A     SCRIBBLE_MUX_A
#define MUX_B     SCRIBBLE_MUX_B
#define MUX_C     SCRIBBLE_MUX_C
#define MUX_G     SCRIBBLE_CS

#define TFT_BLK   SCRIBBLE_BL
#define TFT_TYPE  TFT_eSPI

// TFT_RST should be set to -1 in hardware select file, 
// we need a different value
#if defined(TFT_RST)
#if TFT_RST >= 0
#error "TFT_eSPI will try to reset displays!"  
#endif
#undef TFT_RST
#endif // defined(TFT_RST)

// #define TFT_RST    8 // on a port expander now
// #define TFT_DC    10 // in user config file

#define SCRIBBLE_SPI SPI
#define TFT_ROTATION 0

// inherent in using SPI for scribble strip:
// #define TFT_SCK  13
// #define TFT_MISO 12
// #define TFT_MOSI 11

// ============== Main LCD ==============
#define MAIN_TFT_ROTATION 1

#define TFT_ORANGE2      0xFD00      /* 255, 160,   0 */

#define FONT_3DP FreeSansBold24pt7b
#define FONT_4DP FreeSansBold18pt7b
#define FMT_3DP "%6.3f"
#define FMT_4DP "%7.4f"

#define xFONT_DP(n) CONCAT(FONT_,n,DP)
#define xFMT_DP(n) CONCAT(FMT_,n,DP)
#define xCHG_DP(n) CONCAT(50.0e,-,n)

#define SCRIBBLE_DP 4
#define FONT_DP xFONT_DP(SCRIBBLE_DP)
#define FMT_DP xFMT_DP(SCRIBBLE_DP)
#define CHGTHR_DP xCHG_DP(SCRIBBLE_DP)

// ============== Port expanders ==============
// expander pin assignments in expanders.h


// inherent in using SPI1 for expanders:
// #define DPEX_SCK  27
// #define DPEX_MISO  1
// #define DPEX_MOSI 26

// ============== Power board control ==============
// mapped using 0R resistors
// signal names from USB+power schematic:
#define TOGGLE_POWER USB_X_1
#define SK_EN        USB_X_2
#define SOFT_POWER   USB_X_4
#define POWER_LED    USB_X_3
#define EN_6V        USB_T_4 // Teensy pin

// ============== MIDI ==============
#define MAX_NAME_LENGTH 20


#endif // !defined(_CONFIG_H_)
