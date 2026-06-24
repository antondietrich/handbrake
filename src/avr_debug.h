#pragma once


#if DEBUG
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
};

u8 gDebugOutLatched = 0;
//#define DEBUG_OUT(n) if (((n) * 25 + 5) > OCR1A && (n) <= DEBUG_STAGE_ERROR) OCR1A = (n) * 25 + 5
//#define DEBUG_OUT(n, l) if (!gDebugOutLatched)      \
{                                                   \
    OCR1A = (n) * 13;                               \
    if (l) gDebugOutLatched = 1;                    \
}

void DEBUG_OUT(u8 n, bool force = false, bool latch = false)
{
    if (gDebugOutLatched)
    {
        return;
    }

    if (n >= DEBUG_ERROR_GENERIC)
    {
        latch = true;
    }

    if (latch)
    {
        gDebugOutLatched = 1;
    }

    u8 ocr = (u8)((n) * 2.55f);
    if (force || ocr > OCR1A)
    {
        OCR1A = ocr;
    }
}

#else
void DEBUG_OUT(u8 n, bool force = false, bool latch = false) {}
#endif