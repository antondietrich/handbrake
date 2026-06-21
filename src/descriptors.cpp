#include "descriptors.h"

const DeviceDescriptor PROGMEM deviceDescriptor = {
    .size                   = sizeof(DeviceDescriptor),
    .type                   = DescriptorType::DEVICE,
    .targetSpecVersion      = VERSION_BCD(2, 0, 0),

    .deviceClass            = 0xff, // vendor-specific, 0 for HID
    .deviceSubclass         = 0x00,
    .deviceProtocol         = 0x00,
    .EP0_MaxPacketSize      = 8,

	.vendorID               = 0x03EB,
	.productID              = 0x2043,
	.deviceVersion          = VERSION_BCD(0,0,1),

    .strIdxManufacturer     = 0,
    .strIdxProduct          = 0,
    .strIdxSerialNumber     = 0,

    .numConfigurations      = 1
};


const Configuration PROGMEM configuration = {
    .configDesc = {
        .size                   = sizeof(ConfigurationDescriptor),
        .type                   = DescriptorType::CONFIGURATION,
        .totalSize              = sizeof(ConfigurationDescriptor) + sizeof(InterfaceDescriptor),
        .numInterfaces          = 1,
        .configurationId        = 1,
        .strIndexDescription    = 0,
        .attributes             = 0b10000000,
        .maxPowerConsumption    = 50
    },

    .interfaceDesc = {
        .size                   = sizeof(InterfaceDescriptor),
        .type                   = DescriptorType::INTERFACE,
        .interfaceIdx           = 0,
        .alternateSetting       = 0,
        .numEndpoints           = 0,
        .interfaceClass         = 0xff,
        .interfaceSubclass      = 0x00,
        .interfaceProtocol      = 0x00,
        .strIndexDescription    = 0x00
    }
};
