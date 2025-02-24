#pragma once

#include "stdint.h"


const static uint16_t NO_IDENT = 0xffff;    /**< The value for a null identifier for log or port variables */

/**
* Port constants for communication with the crazyflie
*/
const static uint8_t CONSOLE = 0x00;
const static uint8_t PARAM = 0x02;
const static uint8_t COMMANDER = 0x03;
const static uint8_t MEM = 0x04;
const static uint8_t LOGGING = 0x05;
const static uint8_t LOCALIZATION = 0x06;
const static uint8_t COMMANDER_GENERIC = 0x07;
const static uint8_t SETPOINT_HL = 0x08;
const static uint8_t PLATFORM = 0x0D;
const static uint8_t LINKCTRL = 0x0F;
const static uint8_t PORT_COUNT = 0x10;
const static uint8_t PORT_NONE = 0xF0;
const static uint8_t PORT_ALL = 0xFF;

/**
* Channels for communication with the crazyflie
*/
const static uint8_t TOC_CHANNEL = 0;
const static uint8_t APP_CHANNEL = 2;
const static uint8_t MISC_CHANNEL = 3;

const static int32_t gMaxBufferSize = 32;

const static uint8_t NO_PROTOCOL = 0xff;

enum CrtpChannel : uint8_t
{
    crtpChannelSetpoint = 0,
    crtpChannelMetaCommand = 1,
};

/**
* Port enum for communication with the crazyflie
*/
enum CrtpPorts : uint8_t
{
    crtpPortConsole = 0,
    crtpPortParam = 2,
    crtpPortCommander = 3,
    crtpPortMem = 4,
    crtpPortLogging = 5,
    crtpPortLocalization = 6,
    crtpPortCommanderGeneric = 7,
    crtpPortCommanderHL = 8,
    crtpPortPlatform = 0x0D,
    crtpPortLinkCtrl = 0x0F,
    crtpPortAll = 0xFF
};

