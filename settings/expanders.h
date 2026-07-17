// Port expander pin assignments
// U3 has A0 connected high, and U5 has A2 connected high,
// so they're at addresses 1 and 4 respectively
#define KEYSWITCH_BIT_0 U3,A,7
#define KEYSWITCH_BIT_1 U3,A,6
#define KEYSWITCH_BIT_2 U3,A,5
#define KEYSWITCH_BIT_3 U3,A,4
#define KEYSWITCH_BIT_4 U3,A,3
#define KEYSWITCH_BIT_5 U3,A,2
#define KEYSWITCH_BIT_6 U3,A,1
#define KEYSWITCH_BIT_7 U3,A,0
// U3 port B is unused; available on H2

#define MAIN_FN U5,A,0 // aka FN1
#define REAR_FN U5,A,1 // aka FN2
#define NAV_UP  U5,A,2
#define NAV_DN  U5,A,3
#define NAV_OP  U5,A,4
#define NAV_XY  U5,A,5
//#define nc U5,A,6
#define USB_X_1 U5,A,7 // toggle power: rising edge powers off, IF soft power is released

#define USB_X_2      U5,B,0 // SmartKnob enable: active high
#define USB_X_3      U5,B,1 // power LED: active high
#define PEDAL_RSIN   U5,B,2
#define PEDAL_DET    U5,B,3
#define PEDAL_TRCTRL U5,B,4
#define USB_X_4      U5,B,5 // soft power: open-drain, pulls low when pressed
#define LCD_RESET    U5,B,6 // specific timings needed for GT911 touch chip - BEWARE!
#define ADCS_RESET   U5,B,7

