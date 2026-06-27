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
    HID                     = 0x21,
    HID_REPORT              = 0x22,
    HID_PHYSICAL            = 0x23,
};

enum class StandardFeature : u8
{
    ENDPOINT_HALT           = 0, // recipient = ep
    DEVICE_REMOTE_WAKEUP    = 1, // recipient = device
    TEST_MODE               = 2 // recipient = device
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

enum class EPATTR_TransferType : u8
{
    Control = 0b00,
    Isochronous = 0b01,
    Bulk = 0b10,
    Interrupt = 0b11
};

enum class EPATTR_SynchronizationType : u8
{
    NoSynchronization = 0b00,
    Asynchronous = 0b01,
    Adaptive = 0b10,
    Synchronous = 0b11
};

enum class EPATTR_UsageType : u8
{
    Data = 0b00,
    Feedback = 0b01,
    ImplicitFeedback = 0b10
};

struct PACKED EndpointDescriptor
{
    u8 size;
    DescriptorType type;
    u8 address; // b7: 0 = OUT, 1 = IN; b6..4: reserved; b3..0: EP number
    u8 attributes; // b7..6: reserved; b5..4: usage type; b3..2: synchronization type; b1..0: transfer type
    u16 maxPacketSize; // b12..11: additional transactions, b10..0: size
    u8 pollingInterval; // in 1ms frames for full-speed
};

struct PACKED HIDDescriptor
{
    u8 size;
    DescriptorType type;
    u16 specVersion;
    u8 countryCode;
    u8 numClassDescriptors; // at least Report
    u8 classDescriptorType;
    u16 classDescriptorSize;
    /* Optional descriptors omitted */
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

enum class HIDRequestCode : u8
{
    GET_REPORT          = 0x01,
    GET_IDLE            = 0x02,
    GET_PROTOCOL        = 0x03,
    SET_REPORT          = 0x09,
    SET_IDLE            = 0x0A,
    SET_PROTOCOL        = 0x0B
};

enum class HIDReportType : u8
{
    INPUT               = 1,
    OUTPUT              = 2,
    FEATURE             = 3
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

#define REQUEST_INDEX_ENDPOINT_DIRECTION_MASK   0b10000000
#define REQUEST_INDEX_ENDPOINT_DIRECTION_OUT    0b00000000
#define REQUEST_INDEX_ENDPOINT_DIRECTION_IN     0b10000000
#define REQUEST_INDEX_ENDPOINT_NUMBER_MASK      0b00001111
#define REQUEST_INDEX_INTERFACE_NUMBER_MASK     0b11111111

enum class DataDirection : u8
{
    OUT = 0,
    IN = 1
};

#define GET_REQUEST_DATA_DIRECTION(attr)    ((DataDirection)((attr & REQUEST_ATTR_DATA_DIRECTION_MASK) >> 7))
#define GET_REQUEST_EP_DIRECTION(index)     ((DataDirection)((index & REQUEST_INDEX_ENDPOINT_DIRECTION_MASK) >> 7))
#define GET_REQUEST_EP_NUMBER(index)        ((u8)(index & REQUEST_INDEX_ENDPOINT_NUMBER_MASK))

struct PACKED USBSetupRequest
{
    u8  attributes; // bitmask of REQUEST_ATTR
    u8  code;
    u16 value; // request-specific parameter
    u16 index; // when ATTRT_RECEPIENT is interface or endpoint, specifies the index of the I. or EP. according to REQUEST_INDEX_
    u16 dataSize; // num bytes to send in the Data stage, 0 means no Data stage after this request
};