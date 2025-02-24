/*
* Header-only implementation of the log for the crazyflie
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
#include "lttype.h"
#include "logtoc.h"


#include <vector>
#include <string>
#include <atomic>
#include <errno.h>
#include "messageout.h"

/**
* The cfLog class for the Crazyflie.
* Handles fetching the table of contents
* and logging LogConfigs.
*/
class cfLog : public PortClient
{	
public:

	const static uint8_t MAX_BLOCKS = 16;
	const static uint8_t MAX_VARIABLES = 128;

	const static uint8_t CHAN_TOC = 0;
	const static uint8_t CHAN_SETTINGS = 1;
	const static uint8_t CHAN_LOGDATA = 2;

	// commands
	const static uint8_t CMD_CREATE_BLOCK = 0;
	const static uint8_t CMD_APPEND_BLOCK = 1;
	const static uint8_t CMD_DELETE_BLOCK = 2;
	const static uint8_t CMD_START_LOGGING = 3;
	const static uint8_t CMD_STOP_LOGGING = 4;
	const static uint8_t CMD_RESET_LOGGING = 5;
	const static uint8_t CMD_CREATE_BLOCK_V2 = 6;
	const static uint8_t CMD_APPEND_BLOCK_V2 = 7;


	/**
	* Holds a LogVariable
	* and provides an atomic to update the variable
	*/
	struct LogVariable
	{
		const static uint8_t TOC_TYPE = 0;
		const static uint8_t MEM_TYPE = 1;

		std::string name;  
		uint32_t address;
		typeDex fetch_as;
		uint8_t _type;
		uint8_t ctype;
		std::atomic<uint64_t> _value;

		/**
		* Constructor for LogVariable
		*/
		LogVariable()
		{
			address = 0;
			fetch_as = tdNone;
			_type = TOC_TYPE;
			_value = (0LL);
			ctype = 0;
		}

		/**
		* Copy constructor for LogVariable
		* @param The variable to copy
		*/
		LogVariable(const LogVariable& a)
		{
			name = a.name;
			address = a.address;
			fetch_as = a.fetch_as;
			_type = a._type;
			int64_t val = a._value;
			ctype = a.ctype;
			_value = val;
		}

		/**
		* Return true if the variable should be in the TOC, false if raw memory
		* @returns true if this is a table of contents variable
		*/
		bool is_toc_variable()
		{
			return (_type == TOC_TYPE);
		}

		/**
		* Return what the variable is stored as and fetched as
		* @returns the storage and fetch_byte.
		*/
		uint8_t get_storage_and_fetch_byte()
		{
			return (fetch_as | (fetch_as << 4));
		}

		/**
		* Set this variable from packet data and time
		* @param The buffer that holds the packed value
		* @param The time for this value.
		*/
		int32_t set(uint8_t* buffer, uint32_t time)
		{
			union
			{
				int64_t value;
				uint32_t times[2];
				uint8_t vbuffer[8];
			};
			value = 0;
			times[1] = time;
			int32_t count = types[fetch_as].size;
			for (int32_t i = 0; i < count; i++)
			{
				vbuffer[i] = buffer[i];
			}
			_value = value;
			return(count);
		}

		
		/**
		* Fetches the value of this variable as a float
		* @param The time for this value (returned)
		* @returns The value of the variable as a float
		*/
		float fetchFloat(uint32_t& timestamp)
		{
			union
			{
				uint64_t value;
				uint32_t times[2];
				uint8_t buffer[8];
			};
			value = _value;

			timestamp = times[1];
			float floatValue = 0;

			switch (fetch_as)
			{
			case tdUint8:
			{
				uint8_t intValue = buffer[0];
				floatValue = (float)intValue;
			}
			break;
			case tdUint16:
			{
				uint16_t value16 = 0;
				PackUtils::unpack(buffer, 0, value16);
				floatValue = (float)value16;
			}
			break;
			case tdUint32:
			{
				uint32_t value32 = 0;
				PackUtils::unpack(buffer, 0, value32);
				floatValue = (float)value32;
			}
			break;
			case tdInt8:
			{
				int8_t intValue = ((int8_t*)buffer)[0];
				floatValue = (float)intValue;
			}
			break;
			case tdInt16:
			{
				int16_t value16 = 0;
				PackUtils::unpack(buffer, 0, value16);
				floatValue = (float)value16;
			}
			break;

			case tdInt32:
			{
				int32_t value32 = 0;
				PackUtils::unpack(buffer, 0, value32);
				floatValue = (float)value32;
			}
			break;
			case tdFloat16:
			{
				floatValue = PackUtils::unPackFloat16(buffer);
			}
			break;
			case tdFloat32:
			{
				float value32 = 0;
				PackUtils::unpack(buffer, 0, value32);
				floatValue = value32;
			}
			break;
			}
			return(floatValue);
		}

		/**
		* Fetches the value of this variable as a integer
		* @param The time for this value (returned)
		* @returns The value of the variable as a integer
		*/
		int64_t fetchInt(uint32_t& timestamp)
		{
			union
			{
				uint64_t value;
				uint32_t times[2];
				uint8_t buffer[8];
			};
			value = _value;

			timestamp = times[1];
			int64_t intValue = 0;


			switch (_type)
			{
			case tdUint8:
			{
				intValue = buffer[0];
			}
			break;
			case tdUint16:
			{
				uint16_t value16 = 0;
				PackUtils::unpack(buffer, 0, value16);
				intValue = value16;
			}
			break;
			case tdUint32:
			{
				uint32_t value32 = 0;
				PackUtils::unpack(buffer, 0, value32);
				intValue = value32;
			}
			break;
			case tdInt8:
			{
				intValue = ((int8_t*)buffer)[0];
			}
			break;
			case tdInt16:
			{
				int16_t value16 = 0;
				PackUtils::unpack(buffer, 0, value16);
				intValue = value16;
			}
			break;
			case tdInt32:
			{
				int32_t value32 = 0;
				PackUtils::unpack(buffer, 0, value32);
				intValue = value32;
			}
			break;
			case tdFloat16:
			{
				float floatValue = PackUtils::unPackFloat16(buffer);
				intValue = round(floatValue);
			}
			break;
			case tdFloat32:
			{
				float value32 = 0;
				PackUtils::unpack(buffer, 0, value32);
				float floatValue = value32;
				intValue = round(floatValue);
			}
			break;
			}
			return(intValue);
		}
	};

	/**
	* Holds list of LogVariables
	* and provides functions to ask and recieve them
	*/
	struct LogConfig
	{
		const static uint8_t NoID = 0xff;
		const static uint8_t MAX_LEN = 26;

		std::string name;
		std::vector<LogVariable*> variables;
		std::vector<LogVariable*> default_fetch_as;
		cfLog* log = NULL;
		bool useV2 = false;
		bool added = false;
		bool started = false;
		int32_t pending = 0;
		bool valid = false;
		std::atomic<bool> connected = false;

		uint16_t period = 1;
		uint32_t period_in_ms = 10;
		uint8_t err_no = 0;
		uint8_t id = NoID;

		/**
		* Constructor for LogConfig
		*/
		LogConfig()
		{

		}

		/**
		* Copy constructor for LogConfig
		* @param The LogConfig to copy
		*/
		LogConfig(const LogConfig& a)
		{
			name = a.name;
			variables = a.variables;
			default_fetch_as = a.default_fetch_as;

			log = a.log;
			useV2 = a.useV2;
			added = a.added;
			started = a.started;
			pending = a.pending;
			valid = a.valid;

			period = a.period;
			period_in_ms = a.period_in_ms;
			err_no = a.err_no;
			id = a.id;
		}

		/**
		* Init Constructor for LogConfig
		* @param The name of the config
		* @param The period in milliseconds for the log to refresh
		*/
		LogConfig(std::string _name, uint32_t _period_in_ms)
		{
			name = _name;
			period = _period_in_ms / 10;
			period_in_ms = _period_in_ms;
		}

		/**
		* Adds a LogVariable to this LogConfig
		* Does not own the variable.
		* The variable pointer must persist for the life
		* of the config.
		* @param A pointer to the variable to add
		*/
		void add_variable(LogVariable* var)
		{
			if (var->fetch_as != tdNone)
			{
				variables.push_back(var);
			}
			else
			{
				default_fetch_as.push_back(var);
			}
		}

		/**
		* Adds a LogVariable to this LogConfig as a memory var
		* Does not own the variable.
		* The variable pointer must persist for the life
		* of the config.
		* @param A pointer to the variable to add
		*/
		void add_memory(LogVariable* memVar)
		{
			memVar->_type = LogVariable::MEM_TYPE;
			variables.push_back(memVar);
		}

		/**
		* Sets if this LogConfig was added.
		* @param true if it is added
		*/
		void _set_added(bool _added)
		{
			if (_added != added)
			{
				added = _added;
			}
		}

		/**
		* Returns if this LogConfig was added.
		* @returns true if it is added
		*/
		bool _get_added()
		{
			return(added);
		}

		/**
		* Sets if this LogConfig was started.
		* @param true if it is started
		*/
		void _set_started(bool _started)
		{
			if (_started != started)
			{
				started = _started;
			}
		}

		/**
		* Returns if this LogConfig was started.
		* @returns true if it is started
		*/
		bool _get_started()
		{
			return(started);
		}

		/**
		* Supplies the command to start the block.
		* @returns the command to start the block
		*/
		uint8_t _cmd_create_block()
		{
			if (useV2)
			{
				return(CMD_CREATE_BLOCK_V2);
			}
			return(CMD_CREATE_BLOCK);
		}

		/**
		* Supplies the command to append the block.
		* @returns the command to append the block
		*/
		uint8_t _cmd_append_block()
		{
			if (useV2)
			{
				return(CMD_APPEND_BLOCK_V2);
			}
			return(CMD_APPEND_BLOCK);
		}

		/**
		* Packs the LogVariables into the packet
		* @param The packet to setup
		* @param The next LogVariable to add by index.
		* @returns if the setup was successful
		*/
		bool _setup_log_elements(Packet& pk, int32_t& next_to_add)
		{
			bool result = true;
			int32_t i = next_to_add;
			for (; i < variables.size(); i++)
			{
				LogVariable& var = *variables[i];
				if (!var.is_toc_variable())
				{
					uint8_t index = pk.payloadSize();
					if ((index + 5) < MAX_LEN)
					{
						uint8_t storage_fetch = var.get_storage_and_fetch_byte();
						index += PackUtils::pack(pk.payload(), index, storage_fetch);
						index += PackUtils::pack(pk.payload(), index, var.address);
						pk.setPayloadSize(index);
						next_to_add = i;
					}
					else
					{
						result = false;
					}
				}
				else  // item is in the TOC
				{
					uint16_t element_id = log->toc.get_element_id(var.name);
					uint8_t index = pk.payloadSize();

					if (useV2)
					{
						if ((index + 3) < MAX_LEN)
						{
							uint8_t storage_fetch = var.get_storage_and_fetch_byte();
							index += PackUtils::pack(pk.payload(), index, storage_fetch);
							index += PackUtils::pack(pk.payload(), index, element_id);
							pk.setPayloadSize(index);
							next_to_add = i;
						}
						else
						{
							result = false;
						}
					}
					else
					{
						if ((index + 2) < MAX_LEN)
						{
							uint8_t storage_fetch = var.get_storage_and_fetch_byte();
							index += PackUtils::pack(pk.payload(), index, storage_fetch);
							index += PackUtils::pack(pk.payload(), index, (uint8_t)(element_id & 0xff));
							pk.setPayloadSize(index);
							next_to_add = i;
						}
						else
						{
							result = false;
						}
					}
				}
			}
			return(result);
		}

		/**
		* Sends the LogConfig to the Crazyflie.
		*/
		bool create()
		{
			bool result = false;
			if (log != NULL)
			{
				uint8_t command = _cmd_create_block();
				int32_t next_to_add = 0;
				bool is_done = false;
				int32_t num_variables = 0;
				int32_t _pending = 0;
				for (size_t i = 0; i < log->blockListSize; i++)
				{
					LogConfig* config = log->blockList[i];
					if (config != NULL)
					{
						if (config->pending || config->added || config->started)
						{
							_pending++;
							num_variables += config->variables.size();

						}
					}
					
				}
				if ((_pending + 1) < MAX_BLOCKS)
				{
					if (num_variables + variables.size() < MAX_VARIABLES)
					{
						pending++;
						result = true;
						_pending++;

						while (!is_done)
						{
							int32_t index = 0;
							Packet pk;
							pk.setPort(LOGGING);
							pk.setChannel(CHAN_SETTINGS);
							uint8_t* data = pk.payload();
							index += PackUtils::pack(data, index, command);
							index += PackUtils::pack(data, index, id);
							pk.setPayloadSize(index);
							is_done = _setup_log_elements(pk, next_to_add);
							log->portConnect->send_packet(pk, 0);
							command = _cmd_append_block();
						}
					}
					else
					{
						result = false;
						messageOut << "Adding this configuration would exceed max number of variables\n\r";
					}
				}
				else
				{
					result = false;
					messageOut << "Configuration exceeds max number of blocks\n\r";
				}
			}
			return(result);
		}

		/**
		* Starts logging for this LogConfig.
		*/
		bool start()
		{
			bool result = false;
			if (log != NULL)
			{
				if (log->portConnect != NULL)		// is connected
				{
					if (!added)
					{
						create();
						result = true;
					}
					else
					{
						int32_t index = 0;
						Packet pk;
						pk.setPort(LOGGING);
						pk.setChannel(CHAN_SETTINGS);
						uint8_t* data = pk.payload();
						index += PackUtils::pack(data, index, CMD_START_LOGGING);
						index += PackUtils::pack(data, index, id);
						index += PackUtils::pack(data, index, period);
						pk.setPayloadSize(index);
						log->portConnect->send_packet(pk, CMD_START_LOGGING);
						result = true;
					}
				}
			}
			return(result);
		}

		/**
		* Stops logging for this LogConfig.
		*/
		bool stop()
		{
			bool result = false;
			if (log != NULL)
			{
				if (log->portConnect != NULL)		// is connected
				{
					if (id == NoID)
					{
						result = false;
						messageOut << "Stopping block, but no block registered\n\r";
					}
					else
					{
						int32_t index = 0;
						Packet pk;
						pk.setPort(LOGGING);
						pk.setChannel(CHAN_SETTINGS);
						uint8_t* data = pk.payload();
						index += PackUtils::pack(pk.payload(), index, CMD_STOP_LOGGING);
						index += PackUtils::pack(pk.payload(), index, id);
						pk.setPayloadSize(index);
						log->portConnect->send_packet(pk, CMD_STOP_LOGGING);
						result = true;
					}
				}
			}
			return(result);
		}

		/**
		* Deletes the memory block for this LogConfig.
		*/
		bool cfDelete()
		{
			bool result = false;
			if (log != NULL)
			{
				if (log->portConnect != NULL)		// is connected
				{
					if (id == NoID)
					{
						result = false;
						messageOut << "Stopping block, but no block registered\n\r";
					}
					else
					{
						int32_t index = 0;
						Packet pk;
						pk.setPort(LOGGING);
						pk.setChannel(CHAN_SETTINGS);
						uint8_t* data = pk.payload();
						index += PackUtils::pack(pk.payload(), index, CMD_DELETE_BLOCK);
						index += PackUtils::pack(pk.payload(), index, id);
						pk.setPayloadSize(index);
						log->portConnect->send_packet(pk, CMD_DELETE_BLOCK);
						result = true;
					}
				}
			}
			return(result);
		}

		/**
		* Unpacks and sets the data for each LogVariable
		* @param The data to unpack
		* @param The timestamp for the data.
		*/
		void unpack_log_data(uint8_t* logData, uint32_t timestamp)
		{
			int32_t dataIndex = 0;
			for (size_t i = 0; i < variables.size(); i++)
			{
				dataIndex += variables[i]->set(logData + dataIndex, timestamp);
			}
		}
	};

	/**
	* Fetches the entire LogToc.
	*/
	struct TocFetcher
	{
		const static uint8_t IDLE = 0;
		const static uint8_t GET_TOC_INFO = 1;
		const static uint8_t GET_TOC_ELEMENT = 2;

		cfLog* log = NULL;
		LogToc* tocHolder = NULL;
		std::vector<std::vector<uint8_t>> elementData;
		bool _useV2 = false;

		uint32_t _crc = 0;
		uint32_t requested_index = 0;
		uint32_t state = 0;
		uint32_t protocolVersion = 4;
		uint32_t expectedReply = 0;
		uint16_t nbr_of_items = 0;
		uint8_t port = 0;

		/**
		* Initialize Constructor
		* @param The log client for the fetch
		* @param The port to use for the fetch
		* @param the tocHolder for the fetch
		*/
		TocFetcher(
			cfLog* aLog,
			uint8_t _port,
			LogToc* toc_holder)
		{
			log = aLog;
			port = _port;
			tocHolder = toc_holder;
			protocolVersion = log->protocolVersion;
		}

		/**
		* Starts fetching the toc
		*/
		void start()
		{
			messageOut << "Start fetching the Log TOC.\n\r";

			_useV2 = protocolVersion >= 4;
			log->tocfetcherCallbacks.push_back(this);
			state = GET_TOC_INFO;
			{
				std::array<uint8_t, gMaxBufferSize> buffer;
				buffer[0] = 0xFF;
				uint8_t index = 1;
				if (_useV2)
				{
					index += PackUtils::pack(buffer.data(), index, (uint8_t)CMD_TOC_INFO_V2);
					expectedReply = CMD_TOC_INFO_V2;
				}
				else
				{
					index += PackUtils::pack(buffer.data(), index, (uint8_t)CMD_TOC_INFO);
					expectedReply = CMD_TOC_INFO;
				}
				Packet pk(buffer.data(), index);
				pk.setPort(port);
				pk.setChannel(TOC_CHANNEL);
				log->portConnect->send_packet(pk, expectedReply);
			}
		}

		/**
		* Read the cached toc based on the crc
		* @param The crc to use for the read.
		*/
		bool readToc(uint32_t crc)
		{
			bool result = false; 
			if (tocHolder != NULL)
			{
				result = tocHolder->read(crc);
				if (result)
				{
					messageOut << "Log TOC was read.\n\r";
				}
				else
				{
					messageOut << "Couldn't read Log TOC.\n\r";
				}
			}
			return(result);
		}

		/**
		* Handle receiving a new packet for fetching the toc.
		* @param The packet to process.
		*/
		void _new_packet_cb(Packet& pk)
		{
			uint8_t channel = pk.channel();
			if (channel == 0)
			{
				if (state == GET_TOC_INFO)
				{
					if (_useV2)
					{
						uint8* buffer = pk.payload() + 1;
						int32_t index = 0;
						index += PackUtils::unpack(buffer, index, nbr_of_items);
						index += PackUtils::unpack(buffer, index, _crc);
					}
					else
					{
						uint8_t itemCount = 0;
						uint8_t* buffer = pk.payload() + 1;
						int32_t index = 0;
						index += PackUtils::unpack(buffer, index, itemCount);
						index += PackUtils::unpack(buffer, index, _crc);
						nbr_of_items = itemCount;
					}
					bool wasFound = false;
					if (tocHolder != NULL)
					{
						if (tocHolder->crc == _crc)
						{
							wasFound = true;
							log->resetComplete = wasFound;
							if (wasFound)
							{
								log->portConnect->logResetComplete();
							}
						}
						else if (tocHolder->tocExists(_crc))
						{
							wasFound = readToc(_crc);
							log->resetComplete = wasFound;
							if (wasFound)
							{
								log->portConnect->logResetComplete();
							}
						}
					}
					if (!wasFound)
					{
						state = GET_TOC_ELEMENT;
						requested_index = 0;
						if (nbr_of_items > 0)
						{
							messageOut << "Requesting ";
							messageOut << nbr_of_items; 
							messageOut << " items for the Log TOC\n\r ";

							elementData.resize(nbr_of_items);
							request_toc_element(requested_index);
						}
					}
				}
				else if (state == GET_TOC_ELEMENT)
				{
					uint16_t ident = 0;
					uint8* data = NULL;
					uint8_t command = pk.payload()[0];
					if (_useV2)
					{
						uint8* buffer = pk.payload() + 1;
						int32_t index = 0;
						index += PackUtils::unpack(buffer, index, ident);
						data = buffer + 2;
					}
					else
					{
						uint8* buffer = pk.payload() + 1;
						ident = buffer[0];
						data = buffer + 1;
					}
					if (ident == requested_index && data != NULL)
					{
						LogTocElement element(ident, data);
						tocHolder->add_element(element);

						if (ident == nbr_of_items - 1)
						{
							messageOut << " Finished updating the Log TOC\n\r ";
							tocHolder->write(_crc);
							log->resetComplete = true;
							log->portConnect->logResetComplete();
						}

						if (requested_index < ((uint32_t)nbr_of_items - 1))
						{
							requested_index += 1;
							request_toc_element(requested_index);
						}
					}
				}
			}
		}

		/**
		* Request a single toc element.
		* @param The index of the element to fetch.
		*/
		void request_toc_element(uint16_t elemDex)
		{
			std::array<uint8_t, gMaxBufferSize> buffer;
			buffer[0] = 0xFF;
			uint8_t index = 1;
			if (_useV2)
			{
				index += PackUtils::pack(buffer.data(), index, (uint8_t)CMD_TOC_ITEM_V2);
				index += PackUtils::pack(buffer.data(), index, elemDex);
				expectedReply = CMD_TOC_ITEM_V2;
			}
			else
			{
				uint8_t _elemDex = elemDex & 0xff;
				index += PackUtils::pack(buffer.data(), index, (uint8_t)CMD_TOC_ELEMENT);
				index += PackUtils::pack(buffer.data(), index, (uint8_t)elemDex);
				expectedReply = CMD_TOC_INFO;
			}
			Packet pk(buffer.data(), index);
			pk.setPort(port);
			pk.setChannel(TOC_CHANNEL);
			log->portConnect->send_packet(pk, expectedReply);
		}

	};


	/**
	* The cfLog class for the Crazyflie.
	* Handles fetching the table of contents
	* and logging LogConfigs.
	*/
	LogToc toc;
	std::atomic<LogConfig*> blockList[MAX_BLOCKS];
	std::atomic<uint8_t> blockListSize = 0;
	std::vector <TocFetcher*> tocfetcherCallbacks;
	std::string linkSource;
	uint8_t protocolVersion = 8;
	bool useV2 = false;

	/**
	* The cfLog constructor
	*/
	cfLog() {

		protocolVersion = 0xff;
		useV2 = false;
		for (int32_t i = 0; i < MAX_BLOCKS; i++)
		{
			blockList[i] = NULL;
		}
	}
	/**
	* The cfLog destructor
	*/
	~cfLog()
	{
	}

	/**
	* Virtual implementation of PortClient::setConnection
	* Sets the portConnect and gets the protocolVersion.
	* @param The PortConnect to use for communication
	*/
	void setConnection(PortConnect* _portConnect)
	{
		portConnect = _portConnect;

		if (portConnect != NULL && portConnect->platform != NULL)
		{
			if (portConnect != NULL)
			{
				resetComplete = false;
			}
			protocolVersion = portConnect->platform->get_version();
			useV2 = protocolVersion >= 4;
		}
	}

	/**
	* Clears the blockList
	* Sets each LogConfig to a disconnected state.
	*/
	void clearBlockList()
	{
		uint8_t listSize = blockListSize;
		blockListSize = 0;
		for (int32 i = 0; i < listSize; i++)
		{
			if (blockList[i] != NULL)
			{
				LogConfig* config = blockList[i];
				config->log = NULL;
				config->useV2 = false;
				config->added = false;
				config->started = false;
				config->pending = 0;
				config->valid = false;
				config->connected = false;
				blockList[i] = NULL;
			}
		}
	}

	/**
	* Adds a LogConfig for logging
	* Does not own the LogConfig, and will not delete.
	* The LogConfig ptr must be valid for the duration of the connection.
	* @param The LogConfig to add.
	* @returns true if the LogConfig was added.
	*/
	bool add_config(LogConfig *config)
	{
		bool result = false;
		if (portConnect)
		{
			LogTocElement element;

			config->valid = true;
			for (size_t i = 0; i < config->default_fetch_as.size(); i++)
			{
				LogVariable* var = config->default_fetch_as[i];
				if (toc.get_element_by_complete_name(var->name, element))
				{
					var->fetch_as = (typeDex)element.ident.n;
					config->add_variable(var);
				}
				else
				{
					config->valid = false;
				}
			}
			int32_t configSize = 0;
			for (size_t i = 0; i < config->variables.size(); i++)
			{
				LogVariable* var = config->variables[i];
				configSize += LogTocElement::get_size_from_id(var->fetch_as);
				if (var->is_toc_variable())
				{
					LogTocElement elem;
					if (!toc.get_element_by_complete_name(var->name.c_str(), elem))
					{
						config->valid = false;
					}
				}
			}
			if (config->valid &&
				configSize < LogConfig::MAX_LEN &&
				config->period > 0 && config->period < 0xff)
			{
				config->log = this;
				int32_t id = blockListSize;
				config->id = id;
				config->useV2 = protocolVersion >= 4;
				blockList[id] = config;
				blockListSize++;
				config->start();
				result = true;
				config->connected = true;
			}
		}
		return(result);
	}

	/**
	* Resets the Log
	* Disconnects the blockList
	* Clears and resets the toc.
	*/
	void reset()
	{
		toc.groups.clear();
		useV2 = protocolVersion >= 4;
		_send_reset_packet();
		messageOut << "Resetting cfLog.\n\r";
	}

	/**
	* Resets the Toc
	* Disconnects the blockList
	* Clears and resets the toc.
	*/
	void refresh_toc()
	{
		toc.groups.clear();
		useV2 = protocolVersion >= 4;
		_send_reset_packet();
	}

	/**
	* Sends a request to reset logging
	*/
	void _send_reset_packet()
	{
		if (portConnect != NULL)
		{
			Packet pk;
			useV2 = protocolVersion >= 4;
			pk.setPort(LOGGING);
			pk.setChannel(CHAN_SETTINGS);
			pk.payload()[0] = CMD_RESET_LOGGING;
			pk.setPayloadSize(1);
			portConnect->send_packet(pk, CMD_RESET_LOGGING);
		}
	}

	/**
	* Stops all logging.
	*/
	void stop()
	{
		clearBlockList();
		if (portConnect != NULL)
		{
			toc.groups.clear();
		}
		_send_stop();
	}

	/**
	* Sends a request to stop logging
	*/
	void _send_stop()
	{
		if (portConnect != NULL)
		{
			Packet pk;
			useV2 = protocolVersion >= 4;
			pk.setPort(LOGGING);
			pk.setChannel(CHAN_SETTINGS);
			pk.payload()[0] = CMD_STOP_LOGGING;
			pk.setPayloadSize(1);
			portConnect->send_packet(pk, CMD_STOP_LOGGING);
		}
	}

	/**
	* Handle receiving a new packet for the cfLog.
	* @param The packet to process.
	*/
	void _new_packet_cb(Packet& pk)
	{
		if (pk.size() > 0)
		{
			uint8_t port = pk.port();
			uint8_t channel = pk.channel();

			if (port == LOGGING)
			{
				uint8_t command = pk.payload()[0];
				if (channel == CHAN_SETTINGS)
				{
					if (pk.size() > 1)
					{
						bool blockExists = false;
						LogConfig* block = NULL;
						uint8_t id = pk.payload()[1];
						uint8_t errorStatus = pk.payload()[2];
						if (this->blockListSize > id)
						{
							block = this->blockList[id];
							blockExists = true;
							if (command == CMD_CREATE_BLOCK || command == CMD_CREATE_BLOCK_V2)
							{
								if (errorStatus == 0 || errorStatus == EEXIST)
								{
									if (!block->added)
									{
										Packet packet;

										packet.setPort(LOGGING);
										packet.setChannel(CHAN_SETTINGS);
										uint8_t* buffer = packet.payload();
										int32_t index = 0;
										index += PackUtils::pack(buffer, index, (uint8_t)CMD_START_LOGGING);
										index += PackUtils::pack(buffer, index, id);
										index += PackUtils::pack(buffer, index, block->period);
										packet.setPayloadSize(index);
										portConnect->send_packet(packet, CMD_START_LOGGING);
										block->added = true;
										block->pending = false;
									}
									else
									{
										block->err_no = errorStatus;
										block->added = false;
										block->pending = false;
										messageOut << "Create block failed.\n\r";
									}
								}
							}
						}
						else
						{
							messageOut << "Ident id out of range:";
							messageOut << (int32_t)id; 
							messageOut << "\n\r";
						}
						if (command == CMD_START_LOGGING)
						{
							if (errorStatus == 0)
							{
								if (blockExists)
								{
									block->started = true;
								}
							}
							else
							{
								if (blockExists)
								{
									block->err_no = errorStatus;
									block->started = false;
								}
								messageOut << "Couldn't start logging the block.\n\r";
							}
						}
						else if (command == CMD_STOP_LOGGING)
						{
							if (errorStatus == 0)
							{
								this->clearBlockList();
							}
						}
						else if (command == CMD_DELETE_BLOCK)
						{
							if (errorStatus == 0 || errorStatus == ENOENT)
							{
								if (blockExists)
								{
									block->added = false;
									block->started = false;
									block->pending = false;
								}
								messageOut << "Couldn't delete logging the block.\n\r";
							}
						}
						else if (command == CMD_RESET_LOGGING)
						{
							if (this->toc.groups.size() == 0)
							{
								this->clearBlockList();
								TocFetcher* tocFetcher =
									new TocFetcher(this, LOGGING, &this->toc);
								tocFetcher->start();
							}
						}
					}
				}
				else if (channel == CHAN_LOGDATA)
				{
					LogConfig* block;
					uint8_t id = pk.payload()[0];
					if (this->blockListSize > id)
					{
						block = this->blockList[id];
						block->started = true;
						uint32_t timestamp = 0;
						int32_t index = 0;
						uint8_t* buffer = pk.payload() + 1;
						uint8_t timestamps[3];
						for (int32_t i = 0; i < 3; i++)
						{
							index += PackUtils::unpack(buffer, index, timestamps[i]);
						}
						timestamp = timestamps[0] | timestamps[1] << 8 | timestamps[2] << 16;
						buffer += index;
						block->unpack_log_data(buffer, timestamp);
					}
				}
				else if (channel == TOC_CHANNEL)
				{
					for (size_t i = 0; i < this->tocfetcherCallbacks.size(); i++)
					{
						TocFetcher* tfetch = this->tocfetcherCallbacks[i];
						if (tfetch != NULL && tfetch->expectedReply == command)
						{
							tfetch->_new_packet_cb(pk);
						}
					}
				}
			}
		}
	}
};

