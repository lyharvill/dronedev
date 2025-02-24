/*
* Header-only implementation of communication for MultiRanger Deck 
* for crazyflie
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
#include "cflog.h"

using namespace bitcraze::crazyflieLinkCpp;


/**
* Provides a cfLog interface to the Multiranger deck
*/
struct MultiRanger
{
	cfLog::LogVariable front;	/**< range in the positive x direction. */
	cfLog::LogVariable back;	/**< range in the negative x direction. */
	cfLog::LogVariable up;		/**< range in the positive z direction. */
	cfLog::LogVariable left;	/**< range in the negative y direction. */
	cfLog::LogVariable right;	/**< range in the positive y direction. */

	cfLog::LogConfig range;		/**< group for the range variables. */

	/**
	* Constructor
	*/
	MultiRanger()
	{

		front.name = "range.front";
		front.fetch_as = tdFloat32;

		back.name = "range.back";
		back.fetch_as = tdFloat32;

		up.name = "range.up";
		up.fetch_as = tdFloat32;

		left.name = "range.left";
		left.fetch_as = tdFloat32;

		right.name = "range.right";
		right.fetch_as = tdFloat32;

		range.name = "range";
		range.period = 2;
		range.period_in_ms = 20;

		range.add_variable(&front);
		range.add_variable(&back);
		range.add_variable(&up);
		range.add_variable(&left);
		range.add_variable(&right);
	}

	/**
	* Checks if the MultiRanger is connected
	* @returns returns true if connected.
	*/
	bool is_connected()
	{
		return(range.connected);
	}

	/**
	* Connects the MultiRanger to the cfLog.
	* @param The cfLog for the connection
	*/
	bool connect(cfLog* log)
	{
		bool result = false;
		if (log != NULL)
		{
			if (!is_connected())
			{
				result = log->add_config(&range);
			}
		}
		return(result);
	}

	/**
	* Gets the current range in positive x in meters
	* @param returns the timestamp for this range.
	* @returns the range in positive x in meters
	*/
	inline float getFront(uint32_t &timestamp)
	{
		return(front.fetchFloat(timestamp) * 1.0f / 1000.0f);
	}

	/**
	* Gets the current range in negative x in meters
	* @param returns the timestamp for this range.
	* @returns the range in negative x in meters
	*/
	inline float getBack(uint32_t& timestamp)
	{
		return(back.fetchFloat(timestamp) * 1.0f / 1000.0f);
	}

	/**
	* Gets the current range in positive z in meters
	* @param returns the timestamp for this range.
	* @returns the range in positive z in meters
	*/
	inline float getUp(uint32_t& timestamp)
	{
		return(up.fetchFloat(timestamp) * 1.0f / 1000.0f);
	}

	/**
	* Gets the current range in negative y in meters
	* @param returns the timestamp for this range.
	* @returns the range in negative y in meters
	*/
	inline float getLeft(uint32_t& timestamp)
	{
		return(left.fetchFloat(timestamp) * 1.0f / 1000.0f);
	}

	/**
	* Gets the current range in positive y in meters
	* @param returns the timestamp for this range.
	* @returns the range in positive y in meters
	*/
	inline float getRight(uint32_t& timestamp)
	{
		return(right.fetchFloat(timestamp) * 1.0f / 1000.0f);
	}
};