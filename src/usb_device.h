#pragma once

#include "avr_common.h"

enum class USBDeviceState : u8
{
	NONE,
    ATTACHED,   // attached to usb, but not powered

    SUSPENDED,  // at least attached and powered, no activity for 3ms. may be addressed and configured. can be entered from any of the further states.

    POWERED,    // attached and powered, not yet reset
    DEFAULT,    // has been reset, responds at the default address, mustn't respond to any transactions before this stage
    ADDRESSED,  // unique address has been assigned
    CONFIGURED, // configuration value received

    ERROR,      // something unexpected happened
};
