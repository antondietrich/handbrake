#pragma once

#include "avr_common.h"

enum DEBUG_STAGE
{
    DEBUG_STAGE_START           = 0,

    DEBUG_STAGE_SUSPENDED       = 1,
    DEBUG_STAGE_ATTACHED        = 2,
    DEBUG_STAGE_VBUS_DETECTED   = 3,
    DEBUG_STAGE_DEFAULT         = 4,
    DEBUG_STAGE_SET_ADDRESS     = 5,
    DEBUG_STAGE_ADDRESSED       = 6,
    DEBUG_STAGE_SET_CONFIG      = 7,
    DEBUG_STAGE_CONFIGURED      = 8,

    DEBUG_STAGE_USB_TASK        = 30,
    DEBUG_STAGE_SETUP_REQUEST   = 31,

    DEBUG_ERROR_GENERIC            = 50,
    DEBUG_ERROR_EARLY_RX           = 51,
    DEBUG_ERROR_INVALID_ADDR       = 52,
    DEBUG_ERROR_INVALID_DESCR      = 53,
    DEBUG_ERROR_VBUS               = 54,
    DEBUG_ERROR_EP_SETUP           = 55,
    DEBUG_ERROR_PLLLOCK            = 56,
    DEBUG_ERROR_USBE               = 57,
    DEBUG_ERROR_FRZCLK             = 58,
    DEBUG_ERROR_ATTACH             = 59,
    DEBUG_ERROR_UNKNOWN_DESCR      = 60,
    DEBUG_ERROR_INVALID_REQUEST    = 61,
    DEBUG_ERROR_UNKNOWN_REQUEST    = 62,
    DEBUG_ERROR_CALIBRATION        = 63,
    DEBUG_ERROR_BANK_FULL          = 64,

};

enum DEBUG_PRIORITY
{
    DEBUG_PRIORITY_MIN = 0,

    DEBUG_PRIORITY_ERROR = 127,

    DEBUG_PRIORITY_MAX = 255
};

#if DEBUG
u8 gDebugOutLatched = 0;
u8 gDebugMinPriority = 2;

void DEBUG_OUT(u16 n, u8 priority = 0, u8 latch = 0)
{
    if (priority < gDebugMinPriority)
    {
        return;
    }

    if (gDebugOutLatched)
    {
        return;
    }

    if (latch)
    {
        gDebugOutLatched = 1;
    }

    TM1637::DisplayNumber(n);
}

#else
#define DEBUG_OUT(n, f, l)
#endif