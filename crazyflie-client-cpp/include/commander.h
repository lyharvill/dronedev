/*
* Header-only implementation of the commander calls for crazyflie
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



#include "portconnect.h"
#include "PackUtils.h"

using namespace bitcraze::crazyflieLinkCpp;


/**
* Commander structure
* Provides an communication interface to the CrazyFlie 2.x
* That controls the position, orientation and trhust of the 
* CrazyFlie.
*/
struct Commander
{
    const static uint8_t SET_SETPOINT_CHANNEL = 0; 
    const static uint8_t META_COMMAND_CHANNEL = 1;

    enum CrtpType : uint8_t
    {
        crtpTypeStop = 0,
        crtpTypeVelocityWorld = 1,
        crtpTypeZDistance = 2,
        crtpTypeHover = 5,
        crtpTypeFullState = 6,
        crtpTypePosition = 7,
        crtpTypeLand = 8,
    };

    const static int32_t gMaxBufferSize = 32;
	PortConnect* connection; 

    /**
    * Commander constructor
    */
    Commander()
    {
        connection = NULL;
    }

    /**
    * Commander constructor
    * @param The CrazyRadio connection
    */
	Commander(PortConnect *portConnect)
	{
        init(portConnect);
	}

    /**
    * Commander destructor
    */
	~Commander()
	{
		connection = NULL;  // the connection is not owned.
	}

    /**
    * Initializes the Commander instance
    * @param The CrazyRadio connection
    */
    void init(PortConnect * portConnect)
    {
        connection = portConnect;
    }

    /**
    * Stops the connection
    * @param The CrazyRadio connection
    */
    void _stop()
    {
        connection = NULL;
    }

    /**
    * Send a new control setpoint for roll/pitch/yaw_Rate/thrust to the copter.
    * The meaning of these values is depended on the mode of the RPYT commander in the firmware.
    * The roll, pitch and yaw can be set in a rate or absolute mode with parameter group
    * `flightmode` with variables `stabModeRoll`, `.stabModeRoll` and `.stabModeRoll`.
    * Default settings are roll, pitch, yawrate and thrust
    * Primary use is for manual control.
    * @param roll, in degrees.
    * @param pitch, in degrees.
    * @param yawrate, in degrees per second.
    * @param thrust, an integer value ranging from 10001 (next to no power) to 60000 (full power)
    */
    void send_setpoint(float roll, float pitch, float yawrate, uint16_t thrust)
    {
        std::array<uint8_t, gMaxBufferSize> buffer;
        buffer[0] = 0xFF;
        uint8_t index = 1;
        index += PackUtils::pack(buffer.data(), index, roll);
        index += PackUtils::pack(buffer.data(), index, pitch);
        index += PackUtils::pack(buffer.data(), index, yawrate);
        index += PackUtils::pack(buffer.data(), index, thrust);

        bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);
        packet.setChannel((uint8_t)SET_SETPOINT_CHANNEL);
        packet.setPort((uint8_t)crtpPortCommander);

        connection->send_packet(packet);
    }


    /**
    * Sends a packet so that the priority of the current setpoint to the lowest non-disabled value,
    * so any new setpoint regardless of source will overwrite it.
    * @param remain_valid_milliseconds, number of milliseconds before disabled.
    */
    void send_notify_setpoint_stop(uint32_t remain_valid_milliseconds)
    {
        if (connection)
        {
            std::array<uint8_t, gMaxBufferSize> buffer;
            buffer[0] = 0xFF;
            uint8_t index = 1;
            index += PackUtils::pack(buffer.data(), index, (uint8_t)crtpTypeStop);
            index += PackUtils::pack(buffer.data(), index, remain_valid_milliseconds);

            bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);
            packet.setPort((uint8_t)crtpPortCommanderGeneric);
            packet.setChannel(crtpChannelMetaCommand);
            connection->send_packet(packet);
        }
    }


    /**
    * Send STOP setpoint, stopping the motors and (potentially) falling.
    */
    void send_stop_setpoint()
    {
        if (connection)
        {
            std::array<uint8_t, gMaxBufferSize> buffer;
            buffer[0] = 0xFF;
            uint8_t index = 1;
            index += PackUtils::pack(buffer.data(), index, (uint8_t)crtpTypeStop);

            bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);
            packet.setPort((uint8_t)crtpPortCommanderGeneric);
            packet.setChannel((uint8_t)SET_SETPOINT_CHANNEL);
            connection->send_packet(packet);
        }
    }

    /**
    * Send Velocity in the world frame of reference setpoint with yawrate commands
    * @param vx, The velocity in X, meters per second
    * @param vy, The velocity in Y, meters per second
    * @param vz, The velocity in Z, meters per second
    * @param yawrate, The yaw rotation rate, in degrees per second.
    */
    void send_velocity_world_setpoint(float vx, float vy, float vz, float yawrate)
    {
        if (connection)
        {
            std::array<uint8_t, gMaxBufferSize> buffer;
            buffer[0] = 0xFF;
            uint8_t index = 1;
            index += PackUtils::pack(buffer.data(), index, (uint8_t)crtpTypeVelocityWorld);
            index += PackUtils::pack(buffer.data(), index, vx);             // the x pos to set
            index += PackUtils::pack(buffer.data(), index, vy);             // the y pos to set
            index += PackUtils::pack(buffer.data(), index, vz);             // the z pos to set
            index += PackUtils::pack(buffer.data(), index, yawrate);        // the yaw to set

            bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);
            packet.setPort((uint8_t)crtpPortCommanderGeneric);
            packet.setChannel((uint8_t)SET_SETPOINT_CHANNEL);

            connection->send_packet(packet);
        }
    }

   

    /**
    * Control mode where the height is send_packet as an absolute setpoint (intended
    * to be the distance to the surface under the Crazflie), while giving roll,
    * pitch and yaw rate commands
    * Using zDistance mode.
    * @param roll, in degrees.
    * @param pitch, in degrees.
    * @param yawrate, in degrees per second.
    * @param zdistance, in meters.
    */
    void send_zdistance_setpoint(float roll, float pitch, float yawrate, float zdistance)
    {
        if (connection)
        {
            std::array<uint8_t, gMaxBufferSize> buffer;
            buffer[0] = 0xFF;
            uint8_t index = 1;
            index += PackUtils::pack(buffer.data(), index, (uint8_t)crtpTypeZDistance);
            index += PackUtils::pack(buffer.data(), index, roll);
            index += PackUtils::pack(buffer.data(), index, pitch);
            index += PackUtils::pack(buffer.data(), index, yawrate);
            index += PackUtils::pack(buffer.data(), index, zdistance);

            bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);
            packet.setPort((uint8_t)crtpPortCommanderGeneric);
            packet.setChannel((uint8_t)SET_SETPOINT_CHANNEL);

            connection->send_packet(packet);
        }
    }


   /**
    * Control mode where the height is sent as an absolute setpoint (intended
    * to be the distance to the surface under the Crazflie), while giving roll,
    * pitch and yaw rate commands
    * Using hover mode.
    * @param roll, in degrees.
    * @param pitch, in degrees.
    * @param yawrate, in degrees per second.
    * @param zdistance, in meters.
    */
    void send_hover_setpoint(float vx, float vy, float yawrate, float zdistance)
    {
        if (connection)
        {
            std::array<uint8_t, gMaxBufferSize> buffer;
            buffer[0] = 0xFF;
            uint8_t index = 1;
            index += PackUtils::pack(buffer.data(), index, (uint8_t)crtpTypeHover);
            index += PackUtils::pack(buffer.data(), index, vx);
            index += PackUtils::pack(buffer.data(), index, vy);
            index += PackUtils::pack(buffer.data(), index, yawrate);
            index += PackUtils::pack(buffer.data(), index, zdistance);

            bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);
            packet.setPort((uint8_t)crtpPortCommanderGeneric);
            packet.setChannel((uint8_t)SET_SETPOINT_CHANNEL);

            connection->send_packet(packet);
        }
    }


    /**
    * Control mode where the position is sent as absolute (world) x,y,z coordinate in
    * meter and the yaw is the absolute orientation.
    * @param x, in meters.
    * @param y, in meters.
    * @param z, in meters.
    * @param yaw, in degrees.
    */
	void send_position_setpoint(float x, float y, float z, float yaw)
	{
        if (connection)
        {
            std::array<uint8_t, gMaxBufferSize> buffer;
            buffer[0] = 0xFF;
            uint8_t index = 1;
            index += PackUtils::pack(buffer.data(), index, (uint8_t)crtpTypePosition);
            index += PackUtils::pack(buffer.data(), index, x);             // the x pos to set
            index += PackUtils::pack(buffer.data(), index, y);             // the y pos to set
            index += PackUtils::pack(buffer.data(), index, z);             // the z pos to set
            index += PackUtils::pack(buffer.data(), index, yaw);           // the yaw to set

            bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);
            packet.setChannel((uint8_t)SET_SETPOINT_CHANNEL);
            packet.setPort((uint8_t)crtpPortCommanderGeneric);

            connection->send_packet(packet);
        }
	}

   

    /**
    * Control mode where the position, velocity, acceleration, orientation and angular
    * velocity are sent as absolute (world) values.
    * @param position [x, y, z] are in meters
    * @param velocity [vx, vy, vz] are in meters per second
    * @param acceleration [ax, ay, az] are imeters per second squared
    * @param orientation [qx, qy, qz, qw] are the quaternion components of the orientation
    * @param rollrate is in degrees per second.
    * @param pitchrate is in degrees per second.
    * @param yawrate is in degrees per second.
    */
    void send_full_state_setpoint(
        float pos[3], 
        float vel[3], 
        float acc[3],
        float orientation[4], 
        float rollrate, 
        float pitchrate,
        float yawrate)
    {
        if (connection)
        {
            std::array<uint8_t, gMaxBufferSize> buffer;
            buffer[0] = 0xFF;
            uint8_t index = 1;

            int16_t x = (int16_t)(pos[0] * 1000.0f);
            int16_t y = (int16_t)(pos[1] * 1000.0f);
            int16_t z = (int16_t)(pos[2] * 1000.0f);

            int16_t vx = (int16_t)(vel[0] * 1000.0f);
            int16_t vy = (int16_t)(vel[1] * 1000.0f);
            int16_t vz = (int16_t)(vel[2] * 1000.0f);

            int16_t ax = (int16_t)(acc[0] * 1000.0f);
            int16_t ay = (int16_t)(acc[1] * 1000.0f);
            int16_t az = (int16_t)(acc[2] * 1000.0f);

            int16_t rr = (int16_t)(rollrate * 1000.0f);
            int16_t pr = (int16_t)(pitchrate * 1000.0f);
            int16_t yr = (int16_t)(yawrate * 1000.0f);

            uint32_t quat = PackUtils::quatcompress(orientation);

            index += PackUtils::pack(buffer.data(), index, (uint8_t)crtpTypeFullState);
            index += PackUtils::pack(buffer.data(), index, x);
            index += PackUtils::pack(buffer.data(), index, y);
            index += PackUtils::pack(buffer.data(), index, z);
            index += PackUtils::pack(buffer.data(), index, vx);
            index += PackUtils::pack(buffer.data(), index, vy);
            index += PackUtils::pack(buffer.data(), index, vz);
            index += PackUtils::pack(buffer.data(), index, ax);
            index += PackUtils::pack(buffer.data(), index, ay);
            index += PackUtils::pack(buffer.data(), index, az);
            index += PackUtils::pack(buffer.data(), index, quat);
            index += PackUtils::pack(buffer.data(), index, rr);
            index += PackUtils::pack(buffer.data(), index, pr);
            index += PackUtils::pack(buffer.data(), index, yr);

            bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);
            packet.setChannel(0);
            packet.setPort((uint8_t)crtpPortCommanderGeneric);
            packet.setChannel((uint8_t)SET_SETPOINT_CHANNEL);

            connection->send_packet(packet);
        }
    }
};
