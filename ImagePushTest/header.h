#include <Arduino.h>
#include <TFT_eSPI.h>
#include <FlexIO_t4.h> 
#include <FlexIOSPI.h>


//================================================================
typedef unsigned char image_4bit_data;
typedef struct 
{
    int width, height;
    const image_4bit_data* data;
} image_4bit_info;

extern const image_4bit_info 
    scene_info,
    bulb_info,
    cross_info,
    gears_info,
    note_info,
    rainbow_info,
    runner_info,
    specs_info;