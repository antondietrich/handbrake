#include "descriptors.h"

#include "report_descriptor.h"

const DeviceDescriptor PROGMEM deviceDescriptor = {
    .size                   = sizeof(DeviceDescriptor),
    .type                   = DescriptorType::DEVICE,
    .targetSpecVersion      = VERSION_BCD(2, 0, 0),

    .deviceClass            = 0x00, // defined in the interface
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
        .totalSize              = sizeof(ConfigurationDescriptor) + sizeof(InterfaceDescriptor) + sizeof(EndpointDescriptor) + sizeof(HIDDescriptor),
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
        .numEndpoints           = 1,
        .interfaceClass         = 0x03, // HID
        .interfaceSubclass      = 0x00,
        .interfaceProtocol      = 0x00,
        .strIndexDescription    = 0x00
    },

    .hidDescriptor = {
        .size                   = sizeof(HIDDescriptor),
        .type                   = DescriptorType::HID,
        .specVersion            = VERSION_BCD(1, 11, 0),
        .countryCode            = 0x00,
        .numClassDescriptors    = 0x01,
        .classDescriptorType    = (u8)DescriptorType::HID_REPORT,
        .classDescriptorSize    = sizeof(hidReportDescriptor)
    },

    .endpointDescriptor = {
        .size                   = sizeof(EndpointDescriptor),
        .type                   = DescriptorType::ENDPOINT,
        .address                = (1 << 7) | 1,
        .attributes             = (u8)EPATTR_TransferType::Interrupt,
        .maxPacketSize          = 8,
        .pollingInterval        = 200
    }
};
