#if !defined(_CONFIG_H_)
#define _CONFIG_H_

// common reset pin
#define RST_PIN 6

// touch controller
#define TOUCH_WIRE Wire1
#define TOUCH_ADDR 0x1C
#define CHANGE_PIN 15

// ADCs
#define ADC_SPI SPI
#define ADC_CS  7
#define ADC_CLK 15'000'000
#define CH1_POL -1.0f
#define CH2_POL +1.0f
#define NUM_POTS 8
#define POT_MAP {4,2,0,6}

// LED rings
#define LEDS_PER_RING 20
#define LED_DRIVE_PIN 20
#define LED_TOP_OFFSET 8

#endif // !defined(_CONFIG_H_)
