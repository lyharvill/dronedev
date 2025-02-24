/*
* Header-only implementation of high level commander calls for crazyflie
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

using namespace bitcraze::crazyflieLinkCpp;


const static double gPi = 3.14159265359;
const static double gDegreesToRadians = (gPi / 180.0);
const static double gRadiansToDegrees = (180.0/ gPi);
const static double gRadiansNone = gPi * 1000;

/**
* HighLevelCommander structure
* Provides an communication interface to the CrazyFlie 2.x
* for high level control of the position and attitude of the 
* CrazyFlie.
*/

struct HighLevelCommander
{
    enum hlcCommand : uint8_t
    {
        hlcGroupMask = 0,
        hlcStop = 3,
        hlcGoTo = 4,
        hlcStartTrajectory = 5,
        hlcDefineTrajectory = 6,
        hlcTakeOff_2 = 7,
        hlcLand2 = 8,
    };

    enum TrajectoryType : uint8_t
    {
        tTypePoly4d = 0,
        tTypePoly4dCompressed = 1,
    };

    const static uint8_t gAllGroups = 0;
    const static uint8_t gTrajectoryLocationMem = 1;

    PortConnect* connection;

    /**
    * Commander constructor
    */
    HighLevelCommander()
    {
        connection = NULL;
    }

    /**
    * Commander constructor
    * @param The CrazyRadio connection
    */
    HighLevelCommander(PortConnect* portConnect)
    {
        init(portConnect);
    }

    /**
    * Commander destructor
    */
    ~HighLevelCommander()
    {
        connection = NULL;  // the connection is not owned.
    }

    /**
    * Initializes the Commander instance
    * @param The CrazyRadio connection
    */
    void init(PortConnect* portConnect)
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
    * Vertical takeoff from current x-y position to given height
    * @param absolute_height_m, height in world space in meters.
    * @param duration_s, Time it should take until target height is reached in seconds
    * @param group_mask, Mask for applying this to a swarm of crazyflies.
    * @param yaw, in radians, if equals gRadiansNone, use current yaw.
    * @returns The packet data in packet form
    */
    void takeoff(
        float absolute_height_m,
        float duration_s,
        uint8_t group_mask = gAllGroups )
    {
        bool useCurentYaw = true;
        float targetYaw = 0;

        useCurentYaw = true;

        std::array<uint8_t, gMaxBufferSize> buffer;
        buffer[0] = 0xFF;
        uint8_t index = 1;
        index += PackUtils::pack(buffer.data(), index, (uint8_t)hlcTakeOff_2);
        index += PackUtils::pack(buffer.data(), index, group_mask);
        index += PackUtils::pack(buffer.data(), index, absolute_height_m);
        index += PackUtils::pack(buffer.data(), index, targetYaw);
        index += PackUtils::pack(buffer.data(), index, useCurentYaw);
        index += PackUtils::pack(buffer.data(), index, duration_s);

        bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);

        _send_packet(packet);
    }


    /**
    * Vertical landing from current x-y position to given height
    * @param absolute_height_m, height in world space in meters.
    * @param duration_s, Time it should take until target height is reached in seconds
    * @param group_mask, Mask for applying this to a swarm of crazyflies.
    * @param yaw, in radians, if equals gRadiansNone, use current yaw.
    * @returns The packet data in packet form
    */
    void land(
        float absolute_height_m,
        float duration_s,
        uint8_t group_mask = gAllGroups
        )
    {
        float yaw = 0.0f;
        bool useCurentYaw = true;
  
        std::array<uint8_t, gMaxBufferSize> buffer;
        buffer[0] = 0xFF;
        uint8_t index = 1;
        index += PackUtils::pack(buffer.data(), index, (uint8_t)hlcLand2);
        index += PackUtils::pack(buffer.data(), index, group_mask);
        index += PackUtils::pack(buffer.data(), index, absolute_height_m);
        index += PackUtils::pack(buffer.data(), index, yaw);
        index += PackUtils::pack(buffer.data(), index, useCurentYaw);
        index += PackUtils::pack(buffer.data(), index, duration_s);

        bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);

        _send_packet(packet);
    }
    

    /**
    * stops the current trajectory (turns off the motors)
    * @param group_mask, Mask for applying this to a swarm of crazyflies.
    */
    void stop(
        uint8_t group_mask = gAllGroups)
    {
        std::array<uint8_t, gMaxBufferSize> buffer;
        buffer[0] = 0xFF;
        uint8_t index = 1;
        index += PackUtils::pack(buffer.data(), index, (uint8_t)hlcStop);
        index += PackUtils::pack(buffer.data(), index, group_mask);

        bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);
        _send_packet(packet);
    }

    /**
    * Go to an absolute or relative position
    * @param x, in meters.
    * @param y, in meters.
    * @param z, in meters.
    * @param yaw, in radians.
    * @param duration_s, Time it should take to reach the position in seconds.
    * @param relative, True if x, y, z is relative to the current position.
    * @param group_mask, Mask for applying this to a swarm of crazyflies.
    */

    void go_to(float x, float y, float z, float yaw, float duration_s, bool relative = false,
        uint8_t group_mask = gAllGroups)
    {
        std::array<uint8_t, gMaxBufferSize> buffer;
        buffer[0] = 0xFF;
        uint8_t index = 1;
        index += PackUtils::pack(buffer.data(), index, (uint8_t)hlcGoTo);
        index += PackUtils::pack(buffer.data(), index, group_mask);
        index += PackUtils::pack(buffer.data(), index, relative);
        index += PackUtils::pack(buffer.data(), index, x);
        index += PackUtils::pack(buffer.data(), index, y);
        index += PackUtils::pack(buffer.data(), index, z);
        index += PackUtils::pack(buffer.data(), index, yaw);
        index += PackUtils::pack(buffer.data(), index, duration_s);

        bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);

        _send_packet(packet);
    }

   
    /**
    * starts executing a specified trajectory
    * @param trajectory_id, Id of the trajectory (previously defined by define_trajectory)
    * @param time_scale, 1.0 = original speed; >1.0: slower; <1.0: faster
    * @param relative, Set to True, if trajectory should be shifted to current setpoint
    * @param reversed, Set to True, if trajectory should be executed in reverse
    * @param duration_s, Time it should take to reach the position in seconds.
    * @param relative, True if x, y, z is relative to the current position.
    * @param group_mask, Mask for applying this to a swarm of crazyflies.
    */
    void start_trajectory(
        uint8_t trajectory_id, 
        float time_scale = 1.0f, 
        bool relative = false,
        bool reversed = false, 
        uint8_t group_mask = gAllGroups)
    {
        std::array<uint8_t, gMaxBufferSize> buffer;
        buffer[0] = 0xFF;
        uint8_t index = 1;
        index += PackUtils::pack(buffer.data(), index, (uint8_t)hlcStartTrajectory);
        index += PackUtils::pack(buffer.data(), index, group_mask);
        index += PackUtils::pack(buffer.data(), index, relative);
        index += PackUtils::pack(buffer.data(), index, reversed);
        index += PackUtils::pack(buffer.data(), index, trajectory_id);
        index += PackUtils::pack(buffer.data(), index, time_scale);

        bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);

        _send_packet(packet);
    }


    /**
    * Define a trajectory that has previously been uploaded to memory.
    * @param trajectory_id, Id of the trajectory (previously defined by define_trajectory)
    * @param offset, Offset in uploaded memory
    * @param n_pieces, Number of pieces in the trajectory.
    * @param type, The type of trajectory data; tTypePoly4d or tTypePoly4dCompressed
    */
    void define_trajectory(uint8_t trajectory_id, uint32_t offset, uint8_t n_pieces, uint8_t type = tTypePoly4d)
    {
        std::array<uint8_t, gMaxBufferSize> buffer;
        buffer[0] = 0xFF;
        uint8_t index = 1;

        index += PackUtils::pack(buffer.data(), index, (uint8_t)hlcDefineTrajectory);
        index += PackUtils::pack(buffer.data(), index, (uint8_t)trajectory_id);
        index += PackUtils::pack(buffer.data(), index, (uint8_t)gTrajectoryLocationMem);
        index += PackUtils::pack(buffer.data(), index, (uint8_t)type);
        index += PackUtils::pack(buffer.data(), index, (uint8_t)offset);
        index += PackUtils::pack(buffer.data(), index, (uint8_t)n_pieces);
        bitcraze::crazyflieLinkCpp::Packet packet(buffer.data(), index);

        _send_packet(packet);
    }


    /**
    * Send a packet with the port set to crtpPortCommanderHL
    * @param The packet to send.
    */
    void _send_packet(bitcraze::crazyflieLinkCpp::Packet& packet)
    {
        if (connection != NULL)
        {
            packet.setPort((uint8_t)crtpPortCommanderHL);
            connection->send_packet(packet);
        }
    }
};