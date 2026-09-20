#include "header.h"
//#define PI 3.141265358979323f

PB_SmartKnobConfig SmartKnobTask::configs[] = 
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
        .position = 0,
        .sub_position_unit = 0.0,
        .position_nonce = 6,
        .min_position = 0,
        .max_position = 31,
        .position_width_radians = 8.225806452 * PI / 180,
        .detent_strength_unit = 2.0f,
        .endstop_strength_unit = 1.0f,
        .snap_point = 1.1f,
        .text = "Coarse values\nStrong detents",
        .detent_positions_count = 0,
        .detent_positions = {},
        .snap_point_bias = 0.0f,
        .led_hue = 0, //  red
    },
    {
        .position = 0,
        .sub_position_unit = 0,
        .position_nonce = 0,
        .min_position = 0,
        .max_position = -1, // max position < min position indicates no bounds
        .position_width_radians = 10 * PI / 180,
        .detent_strength_unit = 0,
        .endstop_strength_unit = 1,
        .snap_point = 1.1,
        .text = "Unbounded\nNo detents",
        .detent_positions_count = 0,
        .detent_positions = {},
        .snap_point_bias = 0,
        .led_hue = 200, // purple
    },
    {
        .position = 0,
        .sub_position_unit = 0.0,
        .position_nonce = 4,
        .min_position = 0,
        .max_position = 0,
        .position_width_radians = 90 * PI / 180,
        .detent_strength_unit = 0.01,
        .endstop_strength_unit = 0.6,
        .snap_point = 1.1,
        .text = "Return-to-center",
        .detent_positions_count = 0,
        .detent_positions = {},
        .snap_point_bias = 0,
        .led_hue = 45, // yellow
    },
    {
        .position = 127,
        .sub_position_unit = 0.0,
        .position_nonce = 5,
        .min_position = 0,
        .max_position = 255,
        .position_width_radians = 1.0 * PI / 180,
        .detent_strength_unit = 1,
        .endstop_strength_unit = 1,
        .snap_point = 1.1,
        .text = "Fine values\nWith detents",
        .detent_positions_count = 0,
        .detent_positions = {},
        .snap_point_bias = 0,
        .led_hue = 25, // orange
    },
    {
        .position = 0,
        .sub_position_unit = 0.0,
        .position_nonce = 6,
        .min_position = 0,
        .max_position = 31,
        .position_width_radians = 8.225806452 * PI / 180,
        .detent_strength_unit = 0.35, // slightly stronger detents
        .endstop_strength_unit = 1,
        .snap_point = 1.1,
        .text = "Coarse values\nWeak detents",
        .detent_positions_count = 0,
        .detent_positions = {},
        .snap_point_bias = 0,
        .led_hue = 90, // green
    }
 };