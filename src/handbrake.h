#pragma once

#include "avr_common.h"

#define E2ADDR_CALIBRATION_DONE 0
#define E2ADDR_HALL_MIN 1
#define E2ADDR_HALL_MAX 2
#define BRAKE_MIN 0
#define BRAKE_MAX 255

struct Report
{
    u8 id;
    u8 brake;
    //u8 x;
    //u8 y;
    //u8 buttons;
};