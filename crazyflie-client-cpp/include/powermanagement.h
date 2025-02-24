/*
* Header-only implementation of power management for crazyflie
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

/**
* Provides a cfLog interface to the Battery PowerManagement
*/
struct PowerManagement
{
	cfLog::LogVariable vbat;			/**< The current battery voltage. */
	cfLog::LogVariable batteryLevel;	/**< The current battery level. */
	cfLog::LogConfig pm;				/**< The cPowerManagement group. */

	/**
	* Constructor
	*/
	PowerManagement()
	{
		vbat.name = "pm.vbat";
		vbat.fetch_as = tdFloat32;

		batteryLevel.name = "pm.batteryLevel";
		batteryLevel.fetch_as = tdFloat32;

		pm.name = "pm";
		pm.period = 2;
		pm.period_in_ms = 20;

		pm.add_variable(&vbat);
		pm.add_variable(&batteryLevel);
	}

	/**
	* Checks if PowerManagement is connected
	* @returns returns true if connected.
	*/
	bool is_connected()
	{
		return(pm.connected);
	}

	/**
	* Connects PowerManagement to the cfLog.
	* @param The cfLog for the connection
	*/
	bool connect(cfLog* log)
	{
		bool result = false;
		if (log != NULL)
		{
			result = log->add_config(&pm);
		}
		return(result);
	}
};