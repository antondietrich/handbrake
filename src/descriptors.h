#pragma once

#include <avr/pgmspace.h>
#include "usb_protocol.h"

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

/** Enum for the device interface descriptor IDs within the device. Each interface descriptor
 *  should have a unique ID index associated with it, which can be used to refer to the
 *  interface from other descriptors.
 */
enum InterfaceDescriptors
{
	INTERFACE_ID_Joystick = 0, /**< Joystick interface descriptor ID */
};

/** Enum for the device string descriptor IDs within the device. Each string descriptor should
 *  have a unique ID index associated with it, which can be used to refer to the string from
 *  other descriptors.
 */
enum StringDescriptors
{
	STRING_ID_Language = 0,		/**< Supported Languages string descriptor ID (must be zero) */
	STRING_ID_Manufacturer = 1, /**< Manufacturer string ID */
	STRING_ID_Product = 2,		/**< Product string ID */
};

struct PACKED Configuration
{
    ConfigurationDescriptor configDesc;
    InterfaceDescriptor interfaceDesc;
};
