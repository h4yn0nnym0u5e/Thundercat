//                            USER DEFINED SETTINGS
//   Set driver type, fonts to be loaded, pins used and SPI control method etc.
//
//   See the User_Setup_Select.h file if you wish to be able to define multiple
//   setups and then easily select which setup file is used by the compiler.
//
//   If this file is edited correctly then all the library example sketches should
//   run without the need to make any more changes for a particular hardware setup!
//   Note that some sketches are designed for a particular TFT pixel width/height

// User defined information reported by "Read_User_Setup" test & diagnostics example
#define USER_SETUP_INFO "Setup405_Teensy_ST7789_Flex"
#define USER_SETUP_ID   405

// Define to disable all #warnings in library (can be put in User_Setup_Select.h)
//#define DISABLE_ALL_LIBRARY_WARNINGS

// ##################################################################################
//
// Section 1. Call up the right driver file and any options for it
//
// ##################################################################################

// Specify Teensy
#define TFT_ESPI_TEENSY

// Only define one driver, the other ones must be commented out
#define ST7789_DRIVER      // Full configuration option, define additional parameters below for this display
#define ST7789_PHASED      // phased initialisation capability
#define TFT_RESET_WAIT_MSEC 120 // must wait 120ms after reset before TFT_SLPOUT command


// Some displays support SPI reads via the MISO pin, other displays have a single
// bi-directional SDA pin and the library will try to read this via the MOSI line.
// To use the SDA line for reading data from the TFT uncomment the following line:

// #define TFT_SDA_READ      // This option is for ESP32 ONLY, tested with ST7789 and GC9A01 display only

// For ST7735, ST7789 and ILI9341 ONLY, define the colour order IF the blue and red are swapped on your display
// Try ONE option at a time to find the correct colour order for your display

//  #define TFT_RGB_ORDER TFT_RGB  // Colour order Red-Green-Blue
  #define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red


// For ST7789, ST7735, ILI9163 and GC9A01 ONLY, define the 
// pixel width and height in portrait orientation
//
// Values specified in the constructor will override these
//
// #define TFT_WIDTH  76 // ST7789 76 x 284
// #define TFT_WIDTH  80
// #define TFT_WIDTH  128
// #define TFT_WIDTH  172 // ST7789 172 x 320
// #define TFT_WIDTH  170 // ST7789 170 x 320
// #define TFT_WIDTH  240 // ST7789 240 x 240 and 240 x 320
#define TFT_WIDTH  320 // ST7789 240 x 240 and 240 x 320

// #define TFT_HEIGHT 160
// #define TFT_HEIGHT 128
//#define TFT_HEIGHT 240 // ST7789 240 x 240
// #define TFT_HEIGHT 284 // ST7789 76 x 284
// #define TFT_HEIGHT 320 // ST7789 240 x 320
#define TFT_HEIGHT 240 // GC9A01 240 x 240

// For ST7735 ONLY, define the type of display, originally this was based on the
// colour of the tab on the screen protector film but this is not always true, so try
// out the different options below if the screen does not display graphics correctly,
// e.g. colours wrong, mirror images, or stray pixels at the edges.
// Comment out ALL BUT ONE of these options for a ST7735 display driver, save this
// this User_Setup file, then rebuild and upload the sketch to the board again:

// #define ST7735_INITB
// #define ST7735_GREENTAB
// #define ST7735_GREENTAB2
// #define ST7735_GREENTAB3
// #define ST7735_GREENTAB128    // For 128 x 128 display
// #define ST7735_GREENTAB160x80 // For 160 x 80 display (BGR, inverted, 26 offset)
// #define ST7735_ROBOTLCD       // For some RobotLCD Arduino shields (128x160, BGR, https://docs.arduino.cc/retired/getting-started-guides/TFT)
// #define ST7735_REDTAB
// #define ST7735_BLACKTAB
// #define ST7735_REDTAB160x80   // For 160 x 80 display with 24 pixel offset

// If colours are inverted (white shows as black) then uncomment one of the next
// 2 lines try both options, one of the options should correct the inversion.

// #define TFT_INVERSION_ON
// #define TFT_INVERSION_OFF


// ##################################################################################
//
// Section 2. Define the pins that are used to interface with the display here
//
// ##################################################################################

// If a backlight control signal is available then define the TFT_BL pin in Section 2
// below. The backlight will be turned ON when tft.begin() is called, but the library
// needs to know if the LEDs are ON with the pin HIGH or LOW. If the LEDs are to be
// driven with a PWM signal or turned OFF/ON then this must be handled by the user
// sketch. e.g. with digitalWrite(TFT_BL, LOW);

//#define TFT_BL    9            // LED back-light control pin
//#define TFT_BACKLIGHT_ON HIGH  // Level to turn ON back-light (HIGH or LOW)

// ###### EDIT THE PIN NUMBERS IN THE LINES FOLLOWING TO SUIT YOUR TEENSY SETUP ######

#define TFT_DC   10 // this IS used: pin 10 should give optimised SPI transactions...
#define TFT_DC_MANAGED // ... if this symbol is defined - means its setting MUST NOT be changed

// For Teensy the SPI instance used in the constructor will determine the pins,
// but we leave this here for information (numbers are for SPI, not SPI1 or SPI2)
#define TFT_SCK  13
#define TFT_MISO 12
#define TFT_MOSI 11

// Define this macro with the number of FlexIOSPI busses expected 
// FlexIOSPI pin assignments are dealt with in the constructor(s)
// Leave undefined to confine use to LPSPI busses
#define TFT_FLEXIOSPI_COUNT 1
#define TFT_FLEXIOSPI_CS_LIST  8
#define TFT_FLEXIOSPI_DC_LIST 10
#define TFT_SPI_MODE (SPI_MODE3 | SPI_MODE_TRANSMIT_ONLY) // transmit-only allows faster SPI clock

#define TFT_CS   CS_from_constructor
//#define TFT_CS   31  // Chip select control
#define TFT_RST -1     // Set TFT_RST to -1 if the display RESET is connected to NodeMCU RST or 3.3V


#define TOUCH_CS -1     // Chip select pin (T_CS) of touch screen

//#define TFT_WR PIN_D2       // Write strobe for modified Raspberry Pi TFT only


// ##################################################################################
//
// Section 3. Define the fonts that are to be used here
//
// ##################################################################################

// Comment out the #defines below with // to stop that font being loaded
// The ESP8366 and ESP32 have plenty of memory so commenting out fonts is not
// normally necessary. If all fonts are loaded the extra FLASH space required is
// about 17Kbytes. To save FLASH space only enable the fonts you need!

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

// Comment out the #define below to stop the SPIFFS filing system and smooth font code being loaded
// this will save ~20kbytes of FLASH
#define SMOOTH_FONT


// ##################################################################################
//
// Section 4. Other options
//
// ##################################################################################

// #define SPI_FREQUENCY   1'000'000
// #define SPI_FREQUENCY   5'000'000
// #define SPI_FREQUENCY  10'000'000
// #define SPI_FREQUENCY  16'000'000
// #define SPI_FREQUENCY  20'000'000
// #define SPI_FREQUENCY  27'000'000
// #define SPI_FREQUENCY  40'000'000
// #define SPI_FREQUENCY  55'000'000 // STM32 SPI1 only (SPI2 maximum is 27MHz)
#define SPI_FREQUENCY  60'000'000
// #define SPI_FREQUENCY  80'000'000

// Optional reduced SPI frequency for reading TFT
#define SPI_READ_FREQUENCY  20000000

// The XPT2046 requires a lower SPI clock rate of 2.5MHz so we define that here:
#define SPI_TOUCH_FREQUENCY  2500000

// The ESP32 has 2 free SPI ports i.e. VSPI and HSPI, the VSPI is the default.
// If the VSPI port is in use and pins are not accessible (e.g. TTGO T-Beam)
// then uncomment the following line:
//#define USE_HSPI_PORT

// Comment out the following #define if "SPI Transactions" do not need to be
// supported. When commented out the code size will be smaller and sketches will
// run slightly faster, so leave it commented out unless you need it!

// Transaction support is needed to work with SD library but not needed with TFT_SdFat
// Transaction support is required if other SPI devices are connected.

// Transactions are automatically enabled by the library for an ESP32 (to use HAL mutex)
// so changing it here has no effect

#define SUPPORT_TRANSACTIONS
