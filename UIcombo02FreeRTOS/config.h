#if !defined(_CONFIG_H_)
#define _CONFIG_H_


// ============== Pot board ==============
// common reset pin
#define RST_PIN 6

// touch controller
#define TOUCH_WIRE Wire1
#define TOUCH_ADDR 0x1C
#define CHANGE_PIN 15

// ADCs
#define ADC_SPI SPI1
#define ADC_CS  7
#define ADC_CLK 15'000'000
#define ADC_CS_HIGH_NS (30+10) // 30ns + 10ns margin
#define CH1_POL -1.0f
#define CH2_POL +1.0f
#define NUM_POTS   8
#define NUM_FADERS 0 // for now!
#define POT_MAP {4,2,0,6}


// ============== LED rings ==============
#define LEDS_PER_RING 20
#define LED_DRIVE_PIN 20
#define LED_TOP_OFFSET 8


// ============== Scribble board ==============
#define MUX_A     38
#define MUX_B     39
#define MUX_C     40
#define MUX_G     41

#define TFT_BLK    9

// TFT_RST should be set to -1 in hardware select file, 
// we need a different value
#if defined(TFT_RST)
#if TFT_RST >= 0
#error "TFT_eSPI will try to reset displays!"  
#endif
#undef TFT_RST
#endif // defined(TFT_RST)

#define TFT_RST    8
// #define TFT_DC    10 // in user config file

#define SCRIBBLE_SPI SPI
// inherent in using SPI for scribble strip:
// #define TFT_SCK  13
// #define TFT_MISO 12
// #define TFT_MOSI 11


#endif // !defined(_CONFIG_H_)
