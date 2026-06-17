#include "smartknob.pb.h"
#define PI 3.141265358979323f

PB_SmartKnobConfig configs[] = 
 {
    // int32_t position;
    // float sub_position_unit;
    // uint8_t position_nonce;
    // int32_t min_position;
    // int32_t max_position;
    // float position_width_radians;
    // float detent_strength_unit;
    // float endstop_strength_unit;
    // float snap_point;
    // char text[51];
    // pb_size_t detent_positions_count;
    // int32_t detent_positions[5];
    // float snap_point_bias;
    // int8_t led_hue;

    {
        0,
        0,
        0,
        0,
        -1, // max position < min position indicates no bounds
        10 * PI / 180,
        0,
        1,
        1.1,
        "Unbounded\nno detents",
        0,
        {},
        0,
        200,
    },
    {
        0,
        0,
        4,
        0,
        0,
        160 * PI / 180,
        0.01,
        0.16,
        1.1,
        "Pitch bend",
        0,
        {},
        0,
        45,
    },
    {
        127,
        0,
        5,
        0,
        255,
        1 * PI / 180,
        1,
        1,
        1.1,
        "Fine values\nwith detents",
        0,
        {},
        0,
        25,
    },
    {
        0,
        0,
        6,
        0,
        31,
        8.225806452 * PI / 180,
        2,
        1,
        1.1,
        "Coarse values\nstrong detents",
        0,
        {},
        0,
        150,
    },
    {
        0,
        0,
        6,
        0,
        31,
        8.225806452 * PI / 180,
        0.35, // slightly stronger detents
        1,
        1.1,
        "Coarse values\nweak detents",
        0,
        {},
        0,
        90,
    }
 };
