
/*
* Header-only implementation of the Root class for the CrazyFlie drone.
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
#include "platformservice.h"
#include "cflog.h"
#include "param.h"
#include "messageout.h"
#include "stateestimate.h"
#include "multiranger.h"
#include "powermanagement.h"
#include "commander.h"
#include "highlevelcommander.h"


/**
* Root class for the CrazyFlie drone.
* Provides logging, getting and setting params,
* low level and high level commands for controlling the drone.
*/
class CrazyFlie : public PortOwner
{
public: 

	PortConnect *portConnect;		/**< Provides port-based communication */
	cfLog *log;						/**< For registering LogConfigs for log variable output */
	PlatformService* platform;		/**< For getting protocol, arming, crash recovery */
	Param* param;					/**< For getting and setting parameters on the crazyflie */

	StateEstimate state_estimate;		/**< LogConfig for position and orientation */
	MultiRanger multi_ranger;			/**< LogConfig distance ranging */
	PowerManagement pm;					/**< LogConfig power managment */
	Param::ParamSetting servo_param;	/**< for getting and setting servo angle */
	Commander commander;						/**< low-level commands for flying */
	HighLevelCommander high_level_commander;	/**< high-level commands for flying */

	std::vector<std::string> uris;					/**< List of active uris to connect to a drone */
	std::string defaultDirectory;					/**< The default directory for saving state and cached TOCs */
	std::atomic<bool> foundConnections = false;		/**< True if there are active uris. */

	bool setupComplete = false;
	bool flowDeckPresent = false; 
	bool lighthouseDeckPresent = false;
	bool multirangerDeckPresent = false;


	/**
	* Constructor
	*/
	CrazyFlie()
	{
		portConnect = NULL;
		log = NULL;
		platform = NULL;
		param = NULL;
		servo_param.completeName = "servo.servoAngle";
		setupComplete = false;
		flowDeckPresent = false;
		lighthouseDeckPresent = false;
	}

	/**
	* Destructor
	*/
	~CrazyFlie()
	{
		disconnect();
	}

	/**
	* Scans for active crazyflie drones
	* Sets the uri for each drone in the uris property.
	* @returns true if at least one was found
	*/
	bool scan()
	{
		bool result = false;
		PortConnect::scan(uris);
		foundConnections = uris.size() > 0;
		result = uris.size() > 0;
		if (!result)
		{
			messageOut << "scan failed\n\r";
		}
		return(result);
	}

	/**
	* Disconnects from the current crazyflie drone
	* Stops logging and params
	* disconnect the PortConnect
	* deletes owned classes.
	* @returns true if disconnected
	*/
	bool disconnect()
	{
		bool result = false;
		if (isConnected())
		{
			result = true;
			messageOut << "disconnecting...\n\r";

			if (portConnect)
			{
				portConnect->disconnect();
			}
			if (platform != NULL)
			{
				delete platform;
				platform = NULL;
			}
			if (log != NULL)
			{
				delete log;
				log = NULL;
			}
			if (param != NULL)
			{
				delete param;
				param = NULL;
			}
			if (portConnect != NULL)
			{
				delete portConnect;
				portConnect = NULL;
			}
			commander._stop();
			high_level_commander._stop();
		}
		return(result);
	}

	/**
	* Connects to the supplied index in uris 
	* @param The index to an entry in the uris list.
	* @returns true if connected
	*/
	bool connect(int32_t urlDex = 0)
	{
		bool result = false;
		scan();
		if (uris.size() > urlDex)
		{
			if (isConnected())
			{
				portConnect->disconnect();
			}
			if (portConnect == NULL)
			{
				portConnect = new PortConnect();
				portConnect->defaultDirectory = defaultDirectory;
			}
			if (platform == NULL)
			{
				platform = new PlatformService();
			}
			if (log == NULL)
			{
				log = new cfLog();
				log->toc.defaultPath = defaultDirectory;
			}
			if (param == NULL)
			{
				param = new Param();
				param->toc.defaultPath = defaultDirectory;
			}
			messageOut << "connecting...\n\r";
			result = portConnect->connect(uris[urlDex], this, platform, log, param);
		}
		else
		{
			messageOut << "CrazyFlie not found\n\r";
		}
		return(result);
	}

	/**
	* Checks for the existance of the Flow2 deck
	* @returns true if there is a Flow2 deck.
	*/
	bool hasFlowDeck(void)
	{
		double value = 0;
		param->get_value("deck.bcFlow2", value);
		return(value > 0);
	}

	/**
	* Checks for the existance of the Multiranger deck
	* @returns true if there is a Multiranger deck.
	*/
	bool hasMultiRangerDeck(void)
	{
		double value = 0;
		param->get_value("deck.bcMultiranger", value);
		return(value > 0);
	}

	/**
	* Checks for the existance of the Lighthouse deck
	* @returns true if there is a Lighthouse deck.
	*/
	bool hasLighthouseDeck(void)
	{
		double value = 0;
		param->get_value("deck.bcLighthouse4", value);
		return(value > 0);
	}

	/**
	* Checks for the existance of the Servo deck
	* @returns true if there is a Servo deck.
	*/
	bool hasServoDeck(void)
	{
		double value = 0;
		param->get_value("deck.bcServo", value);
		return(value > 0);
	}

	/**
	* Called when the cfLog's reset has finished
	* This is the place to start classes or functions that 
	* need the cfLog's LogToc to be complete.
	*/
	virtual void logResetComplete()
	{
		
	}

	/**
	* Called when the Param's reset has finished
	* This is the place to start classes or functions that
	* need the Param's ParamToc, the ParamExtensions loaded, 
	* and the initial Param values to be pulled.
	*/
	virtual void paramResetComplete()
	{
		state_estimate.connect(log);

		if (hasFlowDeck())
		{
			messageOut << "has flow deck.\n\r";
			flowDeckPresent = true;
		}
		else
		{
			messageOut << "flow deck not found.\n\r";
			flowDeckPresent = false;
		}

		if (hasMultiRangerDeck())
		{
			messageOut << "connecting multi_ranger.\n\r";
			multi_ranger.connect(log);
			multirangerDeckPresent = true;
		}
		else
		{
			messageOut << "multi_ranger not found.\n\r";
			multirangerDeckPresent = false;
		}
		messageOut << "connecting powerManagement.\n\r";
		pm.connect(log);

		if (hasServoDeck())
		{
			messageOut << "registering servo.\n\r";
			param->registerParamSetting(servo_param);
		}
		else
		{
			messageOut << "Servo not found.\n\r";
		}
		if (hasLighthouseDeck())
		{
			messageOut << "Found Lighthouse.\n\r";
			lighthouseDeckPresent = true;
		}
		else
		{
			messageOut << "Lighthouse not found.\n\r";
			lighthouseDeckPresent = false;
		}
		commander.init(portConnect);
		high_level_commander.init(portConnect);
		setupComplete = true;
	}

	/**
	* Checks to see if the crazyflie is connected
	* @returns true if the portConnect has finished connecting.
	*/
	bool isConnected()
	{
		bool result = false;
		if (portConnect)
		{
			result = portConnect->_isConnected;
		}
		return(result);
	}
};