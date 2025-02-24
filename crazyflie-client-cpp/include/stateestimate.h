/*
* Header-only implementation of the stateEstimate for crazyflie
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
#include "cflog.h"

using namespace bitcraze::crazyflieLinkCpp;



struct StateEstimate
{
	cfLog::LogVariable posX;	/**< current x position of the crazyflie. */
	cfLog::LogVariable posY;	/**< current y position of the crazyflie. */
	cfLog::LogVariable posZ;	/**< current z position of the crazyflie. */

	cfLog::LogVariable accX;	/**< current x acceleration of the crazyflie. */
	cfLog::LogVariable accY;	/**< current y acceleration of the crazyflie. */
	cfLog::LogVariable accZ;	/**< current z acceleration of the crazyflie. */

	cfLog::LogVariable yaw;		/**< current yaw orientation (rotation around the z axis) of the crazyflie. */
	cfLog::LogVariable pitch;	/**< current pitch orientation  (rotation around the y axis) of the crazyflie. */
	cfLog::LogVariable roll;	/**< current roll orientation  (rotation around the x axis) of the crazyflie. */

	cfLog::LogConfig stateestimate;		/**< group for the stateEstimate variables. */

	/**
	* Constructor
	*/
	StateEstimate()
	{

		posX.name = "stateEstimate.x";
		posX.fetch_as = tdFloat32;

		posY.name = "stateEstimate.y";
		posY.fetch_as = tdFloat32;

		posZ.name = "stateEstimate.z";
		posZ.fetch_as = tdFloat32;

		accX.name = "stateEstimate.ax";
		accX.fetch_as = tdFloat32;

		accY.name = "stateEstimate.ay";
		accY.fetch_as = tdFloat32;

		accZ.name = "stateEstimate.az";
		accZ.fetch_as = tdFloat32;

		yaw.name = "stateEstimate.yaw";
		yaw.fetch_as = tdFloat32;

		pitch.name = "stateEstimate.pitch";
		pitch.fetch_as = tdFloat32;

		roll.name = "stateEstimate.roll";
		roll.fetch_as = tdFloat32;


		stateestimate.name = "stateEstimate";
		stateestimate.period = 2;
		stateestimate.period_in_ms = 20;

		stateestimate.add_variable(&posX);
		stateestimate.add_variable(&posY);
		stateestimate.add_variable(&posZ);

		stateestimate.add_variable(&yaw);
		stateestimate.add_variable(&pitch);
		stateestimate.add_variable(&roll);
	}

	/**
	* Checks if the StateEstimate is connected
	* @returns returns true if connected.
	*/
	bool is_connected()
	{
		return(stateestimate.connected);
	}

	/**
	* Connects the StateEstimate to the cfLog.
	* @param The cfLog for the connection
	*/
	bool connect(cfLog *log)
	{
		bool result = false;
		if (log != NULL)
		{
			result = log->add_config(&stateestimate);
		}
		return(result);
	}
};

