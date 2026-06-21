#pragma once

#include "avr_common.h"

/********************************************************/
/*                  Descriptors                         */
/********************************************************/

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

#define ENDPOINT_DIR_MASK 0x80
#define ENDPOINT_DIR_OUT 0x00
#define ENDPOINT_DIR_IN 0x80

#define VERSION_BCD(Major, Minor, Revision) ((Major & 0xFF) << 8) | ((Minor & 0x0F) << 4) | (Revision & 0x0F)

enum class DescriptorType : u8
{
    DEVICE                  = 0x01,
    CONFIGURATION           = 0x02,
    STRING                  = 0x03,
    INTERFACE               = 0x04,
    ENDPOINT                = 0x05,
    DEVICE_QUALIFIER        = 0x06,
    OTHER                   = 0x07,
    INTERFACE_POWER         = 0x08,
    //INTERFACE_ASSOCIATION   = 0x0B
};

struct PACKED DeviceDescriptor
{
    u8 size;
    DescriptorType type;
    u16 targetSpecVersion;

    u8 deviceClass;
    u8 deviceSubclass;
    u8 deviceProtocol;

    u8 EP0_MaxPacketSize; // 8, 16, 32 or 64

    u16 vendorID;
    u16 productID;
    u16 deviceVersion;

    u8 strIdxManufacturer;
    u8 strIdxProduct;
    u8 strIdxSerialNumber;

    u8 numConfigurations;
};

struct PACKED ConfigurationDescriptor
{
    u8 size;
    DescriptorType type;
    u16 totalSize;
    u8 numInterfaces;
    u8 configurationId;
    u8 strIndexDescription;
    u8 attributes;
    u8 maxPowerConsumption; // in 2mA units
};

struct PACKED InterfaceDescriptor
{
    u8 size;
    DescriptorType type;
    u8 interfaceIdx;
    u8 alternateSetting;
    u8 numEndpoints;
    u8 interfaceClass;
    u8 interfaceSubclass;
    u8 interfaceProtocol;
    u8 strIndexDescription;
};

#define LANGUAGE_ID_ENG 0x0409

struct PACKED StringDescriptor
{
    u8 size;
    DescriptorType type;
    const wchar_t* string;
};

/*********************************************************/
/*                  Requests                             */
/*********************************************************/

// Control transfer:
// host -> device: SETUP transaction (DATA 0)
// 0 or more IN our OUT DATA transactions
// IN or opposite of DATA: STATUS transaction (DATA1)

enum class SetupRequestCode : u8
{
    GET_STATUS          = 0,
    CLEAR_FEATURE       = 1, // value = feature selectror
    RESERVED1           = 2,
    SET_FEATURE         = 3, // value = feature selector
    RESERVED2           = 4,
    SET_ADDRESS         = 5, // value = device address
    GET_DESCRIPTOR      = 6, // value = descriptor type and index, index = 0 or LANG_ID
    SET_DESCRIPTOR      = 7, // same as above
    GET_CONFIGURATION   = 8, // value and index = 0, configuration sent as data
    SET_CONFIGURATION   = 9, // value = configuration index
    GET_INTERFACE       = 10, // value = 0; index = interface Idx
    SET_INTERFACE       = 11, // value = alternate index; index = interface Idx
    SYNCH_FRAME         = 12, // value = 0; index = endpoint; data = frame number
};

#define REQUEST_ATTR_DATA_DIRECTION_MASK        0b10000000
#define REQUEST_ATTR_DATA_DRECTION_TO_DEVICE    0b00000000
#define REQUEST_ATTR_DATA_DRECTION_TO_HOST      0b10000000

#define REQUEST_ATTR_TYPE_MASK                  0b01100000
#define REQUEST_ATTR_TYPE_STANDARD              0b00000000
#define REQUEST_ATTR_TYPE_CLASS                 0b00100000
#define REQUEST_ATTR_TYPE_VENDOR                0b01000000

#define REQUEST_ATTR_RECEPIENT_MASK             0b00011111
#define REQUEST_ATTR_RECEPIENT_DEVICE           0b00000000
#define REQUEST_ATTR_RECEPIENT_INTERFACE        0b00000001
#define REQUEST_ATTR_RECEPIENT_ENDPOINT         0b00000010
#define REQUEST_ATTR_RECEPIENT_OTHER            0b00000011

struct USBSetupRequest
{
    u8  attributes; // bitmask of REQUEST_ATTR
    SetupRequestCode  code;
    u16 value; // request-specific parameter
    u16 index; // when ATTRT_RECEPIENT is interface or endpoint, specifies the index of the I. or EP.
    u16 dataSize; // num bytes to send in the Data stage, 0 means no Data stage after this request
};