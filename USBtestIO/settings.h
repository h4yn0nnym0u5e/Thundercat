#if !defined(SETTINGS_H)
#define SETTINGS_H

#define SW_XMAS 42

#define LED_STRING 3 // not really, just one built-in //(300-8)// 200
#define STRIP_SETTINGS NEO_GRB + NEO_KHZ800

#if defined(ARDUINO_WAVESHARE_RP2040_ZERO)
  #define PIN 16 // for WaveShare RP2040 Zero built-in WS2812
  //#define PIN 14 // for WaveShare RP2040 Zero with LED string
#else  
  #define PIN 28 // for Maker Pi Pico
#endif // defined(ARDUINO_WAVESHARE_RP2040_ZERO)

// Brightness shifts: minimum 8, larger is dimmer
#define SHIFT 13

// USB+Power board connections
#define IO_T_4 13
#define IO_3   12
#define IO_2   11
#define IO_1   10
#define IO_X_4  9

#define TOGGLE_POWER IO_1 // rising edge powers down
#define SK_EN        IO_2 // high powers up SmartKnob
#define POWER_LED    IO_3 // high turns on power LED (in soft-on button; 220R in series)
#define EN_6V      IO_T_4 // high enables 6V PSU (220R in series)
#define SOFT_POWER IO_X_4 // soft switch pressed (open drain)

#endif // !defined(SETTINGS_H)
