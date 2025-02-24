/*
* Header-only implementation of platform calls for crazyflie
*
* Copyright (c) 2024-2025 Young Harvill
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
#
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
* You should have received a copy of the GNU Lesser General Public License
* along with this program. If not, see <https://www.gnu.org/licenses/>.
*
* Implements interfaces found here...
* https://github.com/bitcraze/crazyflie-clients-python
* Using the c++ library found here...
* https://github.com/bitcraze/crazyflie-link-cpp
*
*/

#pragma once


#include"portconnect.h"

using namespace bitcraze::crazyflieLinkCpp;


/**
* Provides PLATFORM port services
* Including plaform protocol version
* arming the crazyflie 
* and crash recovery
*/
class PlatformService : public PortClient
{
public:

    /**
    * Platform channels
    */
    const static uint8_t LINKSERVICE_SOURCE = 1;
    const static uint8_t MISC_CHANNEL = 3;

    /**
    * Platform requests
    */
    const static uint8_t VERSION_GET_PROTOCOL = 0;
    const static uint8_t VERSION_GET_FIRMWARE = 1;

    /**
    * Platform commands
    */
    const static uint8_t VERSION_COMMAND = 1;
    const static uint8_t PLATFORM_SET_CONT_WAVE = 0;
    const static uint8_t PLATFORM_REQUEST_ARMING = 1;
    const static uint8_t PLATFORM_REQUEST_CRASH_RECOVERY = 2;

    const static uint8_t PLATFORM_COMMAND = 0;

 
    std::string linkSource;                     /**< The name of the link source */
    uint8_t protocolVersion = NO_PROTOCOL;      /**< The protocol version  */

    /**
    * Constructor
    */
    PlatformService() 
    { portConnect = NULL; 
    protocolVersion = NO_PROTOCOL; }

    /**
    * Gets the protocol for the current connection
    * If not connected, returns NO_PROTOCOL
    * @returns The protocol version
    */
    virtual uint8_t get_version()
    {
        return(protocolVersion);
    }

    /**
    * Sends a version request to the current connection.
    * This is a virtual function used by the PortConnect.
    */
    virtual void _request_version()
    {
        if (portConnect != NULL)
        {
            std::array<uint8_t, gMaxBufferSize> buffer;
            buffer[0] = 0xFF;
            uint8_t index = 1;

            index += PackUtils::pack(buffer.data(), index, (uint8_t)0);

            bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);

            packet.setPort(LINKCTRL);
            packet.setChannel(PlatformService::LINKSERVICE_SOURCE);
            portConnect->send_packet(packet);
        }
    }


    /**
    * Send a new armingCommand.
    * @param true, if arming
    */
    void send_arming_request(bool doArm)
    {
        if (portConnect)
        {
            std::array<uint8_t, gMaxBufferSize> buffer;
            buffer[0] = 0xFF;
            uint8_t index = 1;
            index += PackUtils::pack(buffer.data(), index, PLATFORM_REQUEST_ARMING);
            index += PackUtils::pack(buffer.data(), index, doArm);

            bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);
            packet.setPort(PLATFORM);
            packet.setChannel(PLATFORM_COMMAND);

            portConnect->send_packet(packet);
        }
    }

  
    /**
   * Send a new armingCommand.
   * @param true, if arming
   */
    void send_crash_recovery_request()
    {
        if (portConnect)
        {
            portConnect->send_packet(recoveryCommand(), PLATFORM_REQUEST_CRASH_RECOVERY);
        }
    }

    /**
    * Constructs an recoveryCommand packet
    * @param true, if arming
    * @returns The packet data in packet form
    */
    static bitcraze::crazyflieLinkCpp::Packet& recoveryCommand() {
        std::array<uint8_t, gMaxBufferSize> buffer;
        buffer[0] = 0xFF;
        uint8_t index = 1;
        index += PackUtils::pack(buffer.data(), index, PLATFORM_REQUEST_CRASH_RECOVERY);

        bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);
        packet.setPort(PLATFORM);
        packet.setChannel(PLATFORM_COMMAND);
        return packet;
    }


    /**
    * Handles receiving a new packet from the PortConnect.
    * This is a Virtual PortClient call to handle a LINKCTRL or PLATFORM port packet.
    * @param the requested packet.
    */
    virtual void _new_packet_cb(Packet& pk)
    {
        if (pk.size() > 0)
        {
            uint8_t port = pk.port();
            uint8_t channel = pk.channel();

            if (port == LINKCTRL)
            {
                channel = pk.channel();
                if (channel == MISC_CHANNEL || channel == LINKSERVICE_SOURCE)
                {
                    const static char platformName[] = "Bitcraze Crazyflie";
                    size_t dataSize = pk.payloadSize();
                    char* dataString = (char*)pk.payload();

                    if (strncmp(dataString, platformName, sizeof(platformName)) == 0)
                    {
                        if (linkSource != platformName)
                        {
                            linkSource = platformName;
                            std::array<uint8_t, gMaxBufferSize> buffer;
                            buffer[0] = 0xFF;
                            uint8_t index = 1;

                            index += PackUtils::pack(buffer.data(), index, (uint8_t)VERSION_GET_PROTOCOL);

                            bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);

                            packet.setPort(PLATFORM);
                            packet.setChannel(VERSION_COMMAND);
                            portConnect->send_packet(packet);
                        }
                        else
                        {

                        }

                    }
                }
            }
            else if (port == PLATFORM)
            {
                if (channel == VERSION_COMMAND)
                {
                    if (pk.payload()[0] == VERSION_GET_PROTOCOL)
                    {
                        protocolVersion = pk.payload()[1];
                    }
                }
            }
        }
    }
};