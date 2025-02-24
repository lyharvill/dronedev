/*
* Header-only implementation for port connection and messaging for crazyflie
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
* The usb libray may be found here...
* https://github.com/libusb/libusb
*
*/


#pragma once
#include "Connection.h"
#include "packutils.h"
#include "ctrp.h"
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <mutex>

using namespace bitcraze::crazyflieLinkCpp;

struct PortConnect; 

/**
* Provides a base class for ownership of a PortConnect.
*/
class PortOwner
{
public:

	PortConnect* portConnect;				/**< The owned PortConnect. */
	std::atomic<bool> connected = false;	/**< true if connected to the PortConnect. */

	/**
	* Constructor
	*/
	PortOwner()
	{
		portConnect = NULL;
		connected = false;
	}

	/**
	* Destructor
	*/
	~PortOwner() {}
	
	/**
	* Called when the log has finished reseting
	*/
	virtual void logResetComplete()
	{

	}

	/**
	* Called when the param has finished reseting
	*/
	virtual void paramResetComplete()
	{

	}


};

/**
* Provides a base class for client of a PortConnect.
*/
class PortClient
{
public:

	PortConnect* portConnect;					/**< The PortConnect service. */
	std::atomic<bool> connected = false;		/**< true if connected to the PortConnect. */
	std::atomic<bool> resetComplete = false;	/**< true if the reset is complete. */

	/**
	* Constructor
	*/
	PortClient()
	{
		portConnect = NULL;
		connected = false;
		resetComplete = false;
	}

	/**
	* Destructor
	*/
	~PortClient() {}

	/**
	* Called when a port packet arrives.
	*/
	virtual void _new_packet_cb(Packet& pk) {}

	/**
	* Called when the conenction stops.
	*/
	virtual void stop() {}

	/**
	* Called to set a new connection.
	*/
	virtual void setConnection(PortConnect* _portConnect)
	{
		portConnect = _portConnect;
		connected = true;
	}

	/**
	* Called to request a version for the connection
	*/
	virtual void _request_version() {}

	/**
	* Called to fetch the requested version
	*/
	virtual uint8_t get_version() { return(0); }

	/**
	* Called to reset this client.
	*/
	virtual void reset() {};

	/**
	* Called to completely update this client.
	*/
	virtual void update_all() {};

};

/**
* Provides a connection to crazyflie ports for
* PortClients.
*/
struct PortConnect
{
	const static int32_t packetTimoutSec = 3;					/**< number of seconds with no packets for timeout. */
	bitcraze::crazyflieLinkCpp::Connection* cfConnection;		/**< The connection to the crazyflie. */
	std::string defaultDirectory;								/**< The defualt directory for caching TOCs */
	std::thread portThread;										/**< Thread for async handling of packets */
	std::atomic<double> packetsPerSecond = 0;					/**< Metric found for packets per second. */
	std::atomic<bool> running = false;							/**< true while thread is running. */
	std::atomic<bool> _isConnected = false;						/**< true while connected. */
	std::atomic<bool> timedOut = false;							/**< Set true during packet timeout */
	
	std::mutex sendMutex;					/**< Mutex for protecting sending a packet from multiple threads. */

	PortOwner* owner;						/**< The owner of the PortConnect. */
	PortClient* log;						/**< TThe client which handles LOG port packets. */
	PortClient* platform;					/**< TThe client which handles PLATFORM and LINKCTRL port packets. */
	PortClient* param;						/**< TThe client which handles PARAM port packets. */

	/**
	* Constructor
	*/
	PortConnect()
	{
		cfConnection = NULL;
		log = NULL;
		platform = NULL;
		packetsPerSecond = 0;
		timedOut = false;
	}

	/**
	* Destructor
	*/
	~PortConnect()
	{
		disconnect();
	}

	/**
	* Calls the owner when the log reset is complete.
	*/
	void logResetComplete()
	{
		owner->logResetComplete();
	}

	/**
	* Calls the owner when the param reset is complete.
	*/
	void paramResetComplete()
	{
		owner->paramResetComplete();
	}

	/**
	* Disconnect this session
	*/
	void disconnect()
	{
		if (cfConnection != NULL)
		{
			if (_isConnected)
			{
				if (log != NULL)
				{
					log->stop();
				}
				if (param != NULL)
				{
					param->stop();
				}
				
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2));

		if (cfConnection != NULL)
		{
			cfConnection->close();
			_isConnected = false;
		}
		if (running)
		{
			running = false;
			portThread.join();
		}
		if (cfConnection != NULL)
		{
			delete cfConnection;
			cfConnection = NULL;
		}
	}

	/**
	* Connect a new session.
	*/
	bool connect(std::string uri, PortOwner* _owner, PortClient*_platform, PortClient*_log, PortClient *_param)
	{
		const static double minBandwidth = 0.001;

		bool result = false;

		{
			platform = _platform;
			log = _log;
			param = _param;
			owner = _owner;

			platform->setConnection(this);

			if (uri.size() > 0)
			{
				cfConnection = new bitcraze::crazyflieLinkCpp::Connection(uri);
				std::this_thread::sleep_for(std::chrono::microseconds(1000));
				running = true;
				portThread = std::thread(portThreadFunc, this);
				{
					int32_t iter = 100;
					{
						running = true;

						platform->_request_version();
						while (platform->get_version() == NO_PROTOCOL && iter)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(1));
							iter--;
						}
					}
					if (iter > 0)
					{
						_isConnected = true;
						log->setConnection(this);
						log->reset();
						param->setConnection(this);
						result = true;
					}
				}
			}
		}
		return(result);
	}

	/**
	* Scan for active crazyflie uris
	*/
	static bool scan(std::vector<std::string> &_uris)
	{
		_uris = bitcraze::crazyflieLinkCpp::Connection::scan();
		return(_uris.size() > 0);
	}

	/**
	* Send a packet to the crazyflie
	*/
	void send_packet(bitcraze::crazyflieLinkCpp::Packet& packet, uint8_t expectedReply = 0)
	{
		if (cfConnection != NULL)
		{
			if (packet.size() > 0)
			{
				std::lock_guard<std::mutex> guard(sendMutex);  // can be called from multiple threads
				cfConnection->send(packet);
			}
			else
			{
				packet.setPayloadSize(0);
			}
		}
	}

	/**
	* Check for packets from cfConnection,
	* send to the proper port client.
	* Measure packets per second and set timeout if needed.
	*/
	static void portThreadFunc(void* data)
	{
		bool needsParamReset = true;
		bool needsParamUpdate = true;
		PortConnect* portConnect = (PortConnect*)data;
		if (portConnect->cfConnection != NULL)
		{
			int32_t packetCount = 0;
			int32_t noPacketCount = 0;
			double elapsedTime = 0;
			bool sendTimedOut = true;

			auto lastTime = std::chrono::steady_clock::now();
			std::chrono::duration<double> diff = lastTime - lastTime;
		

			while (portConnect->running)
			{
				Packet pk;
				pk = portConnect->cfConnection->receive(1);
				if (pk.size() > 0)
				{
					uint8_t port = pk.port();
					uint8_t channel = pk.channel();

					if (port == LOGGING)
					{
						if (portConnect->log != NULL)
						{
							portConnect->log->_new_packet_cb(pk);
						}
					}
					else if (port == PARAM)
					{
						if (portConnect->log != NULL)
						{
							portConnect->param->_new_packet_cb(pk);
						}
					}
					else if (port == LINKCTRL || port == PLATFORM)
					{
						portConnect->platform->_new_packet_cb(pk);
					}

					if (portConnect->log != NULL && portConnect->param != NULL)
					{
						if (needsParamReset)
						{
							if (portConnect->log->resetComplete)
							{
								needsParamReset = false;
								portConnect->param->reset();
							}
						}
						else if (needsParamUpdate)
						{
							if (portConnect->param->resetComplete)
							{
								needsParamUpdate = false;
								portConnect->param->update_all();
							}
						}
					}
					packetCount++;
					
				}
				auto thisTime = std::chrono::steady_clock::now();
				diff = thisTime - lastTime;
				elapsedTime = diff.count();

				if (elapsedTime >= 1.0)
				{
					lastTime = thisTime;

					portConnect->packetsPerSecond = (double)packetCount / elapsedTime; 
					elapsedTime = 0;

					if (packetCount < 2)
					{
						noPacketCount++;
					}
					else
					{
						noPacketCount = 0;
					}
					packetCount = 0;
					if (noPacketCount >= packetTimoutSec)
					{
						portConnect->timedOut = true;
						if (sendTimedOut)
						{
							messageOut << "packets timed out\n\r";
							sendTimedOut = false;
						}
						
					}
					else
					{
						portConnect->timedOut = false;
					}
				}
			}
		}
	}
};