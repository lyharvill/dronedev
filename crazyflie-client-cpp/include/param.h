/*
* Header-only implementation of the parameters for crazyflie
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
#include "pttype.h"
#include "paramtoc.h"
#include "packutils.h"

#include <vector>
#include <queue>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <errno.h>
#include "messageout.h"


/**
* The Param class for the Crazyflie.
* Handles getting and setting parameters.
*/
class Param : public PortClient
{
public:
	
	//Possible states
	const static uint8_t IDLE = 0;
	const static uint8_t WAIT_TOC = 1;
	const static uint8_t WAIT_READ = 2;
	const static uint8_t WAIT_WRITE = 3;

	const static uint8_t READ_CHANNEL = 1;
	const static uint8_t WRITE_CHANNEL = 2;
	const static uint8_t MISC_CHANNEL = 3;

	const static uint8_t MISC_SETBYNAME = 0;
	const static uint8_t MISC_VALUE_UPDATED = 1;
	const static uint8_t MISC_GET_EXTENDED_TYPE = 2;
	const static uint8_t MISC_PERSISTENT_STORE = 3;
	const static uint8_t MISC_PERSISTENT_GET_STATE = 4;
	const static uint8_t MISC_PERSISTENT_CLEAR = 5;
	const static uint8_t MISC_GET_DEFAULT_VALUE = 6;


	/**
	* Fetches the entire ParamToc.
	*/
	struct TocFetcher
	{
		//const static uint8_t IDLE = 0;
		const static uint8_t GET_TOC_INFO = 1;
		const static uint8_t GET_TOC_ELEMENT = 2;

		Param* param = NULL;
		ParamToc* tocHolder = NULL;
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
		* @param The param client for the fetch
		* @param The port to use for the fetch
		* @param the tocHolder for the fetch
		*/
		TocFetcher(
			Param* aParam,
			uint8_t _port,
			ParamToc* toc_holder)
		{
			param = aParam;
			port = _port;
			tocHolder = toc_holder;
			protocolVersion = param->protocolVersion;
		}

		/**
		* Starts fetching the toc
		*/
		void start()
		{
			messageOut << "Start fetching the Param TOC.\n\r";

			_useV2 = protocolVersion >= 4;
			param->tocfetcherCallbacks.push_back(this);
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
				param->portConnect->send_packet(pk, expectedReply);
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
					messageOut << "Param TOC was read.\n\r";
				}
				else
				{
					messageOut << "Couldn't read Param TOC.\n\r";
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
							param->toc_complete();
						}
						else if (tocHolder->tocExists(_crc))
						{
							wasFound = readToc(_crc);
							param->toc_complete();
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
							messageOut << " items for the Param TOC\n\r ";

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
						ParamTocElement element(ident, data);
						tocHolder->add_element(element);

						if (ident == nbr_of_items - 1)
						{
							messageOut << " Finished updating the Log TOC\n\r ";
							tocHolder->write(_crc);
							param->toc_complete();
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
			param->portConnect->send_packet(pk, expectedReply);
		}
	};

	/**
	* Provides thread safe param value store and set.
	*/
	struct ParamValue
	{
		/**
		* Param state
		*/
		const static uint16_t REQUEST_NONE = 0 << 8;	/**< No request pending for this param value */
		const static uint16_t REQUEST_READ = 1 << 8;	/**< A read is requested for this value */
		const static uint16_t REQUEST_WRITE = 2 << 8;	/**< A write is requested for this value */


		const static uint16_t PENDING = 0;				/**< This param has not been updated */
		const static uint16_t REQUESTED = 1;			/**< An update has been requested */
		const static uint16_t SET = 2;					/**< This param has been set and updated. */

		std::atomic<uint64_t> _value;	/**< The packed bytes for the current value */
		std::atomic<uint16_t> _ident;	/**< The identifier for this Param in the current TOC */
		std::atomic<uint16_t> _ctype;	/**< The c language type index for this param */
		std::atomic<uint16_t> _csize;	/**< The packed size of this param */
		std::atomic<uint16_t> _state;	/**< The request state of this param value */

		/**
		* Constructor
		*/
		ParamValue()
		{
			_value = 0;
			_ident = 0;
			_ctype = 0;
			_csize = 0;
			_state = PENDING | REQUEST_NONE;

		}
		/**
		* Destructor
		*/
		~ParamValue() {}

		/**
		* Init Constructor
		* @param The initial state for the ParamValue.
		*/
		ParamValue(uint16_t state)
		{
			_state = state;
		}

		/**
		* Converts the stored value into an integer value
		* @returns The current integer value of the ParamValue.
		*/
		int64_t getIntValue()
		{
			int64_t value64 = 0;
			union
			{
				uint64_t value;
				int8_t int8vals[8];
				int16_t int16vals[4];
				int32_t int32vals[2];
				int64_t int64val;
			};
			value = _value;
			switch (_ctype)
			{
			case ptInt8:
				value64 = int8vals[0];
				break;
			case ptInt16:
				value64 = int16vals[0];
				break;
			case ptInt32:
				value64 = int32vals[0];
				break;
			case ptInt64:
				value64 = int64val;
				break;
			}
			return(value64);
		}

		/**
		* Converts the stored value into an unsigned integer value
		* @returns The current unsigned integer value of the ParamValue.
		*/
		uint64_t getUintValue()
		{
			uint64_t value64 = 0;
			union
			{
				uint64_t value;
				uint8_t uint8vals[8];
				uint16_t uint16vals[4];
				uint32_t uint32vals[2];
			};
			value = _value;

			switch (_ctype)
			{
			case ptUint8:
				value64 = uint8vals[0];
				break;
			case ptInt16:
				value64 = uint16vals[0];
				break;
			case ptInt32:
				value64 = uint32vals[0];
				break;
			case ptInt64:
				value64 = value;
				break;
			}
			return(value64);
		}

		/**
		* Converts the stored value into an float64 value
		* @returns The current float64 value of the ParamValue.
		*/
		double getValue()
		{
			double valued = 0;
			if (ParamTocElement::is_integer(_ctype))
			{
				if (ParamTocElement::is_signed(_ctype))
				{
					valued = (double)getIntValue();
				}
				else
				{
					valued = (double)getUintValue();
				}
			}
			else
			{
				union
				{
					uint64_t value;
					uint8_t buffer[8];
					uint16  buff16[4];
					float floatVals[2];
					double doubleValue;
				};
				value = _value;
				switch (_ctype)
				{
				case ptFloat32:
					valued = floatVals[0];
					break;
				case ptFloat64:
					valued = doubleValue;
					break;
				}
			}
			return(valued);
		}

		/**
		* Sets the stored value as an int64_t.
		* @param The int64_t value to set.
		*/
		void setValue(int64_t value64)
		{
			union
			{
				uint64_t value;
				int8_t int8vals[8];
				int16_t int16vals[4];
				int32_t int32vals[2];
				int64_t int64val;
			};
			value = 0;
			switch (_ctype)
			{
			case ptInt8:
				int8vals[0] = (int8_t)value64;
				break;
			case ptInt16:
				int16vals[0] = (int16_t)value64;
				break;
			case ptInt32:
				int32vals[0] = (int32_t)value64;
				break;
			case ptInt64:
				int64val = value64;
				break;
			}
			_value = value;
		}

		/**
		* Sets the stored value as an uint64_t.
		* @param The uint64_t value to set.
		*/
		void setValue(uint64_t value64)
		{
			union
			{
				uint64_t value;
				uint8_t uint8vals[8];
				uint16_t uint16vals[4];
				uint32_t uint32vals[2];
			};
			value = 0;
			switch (_ctype)
			{
			case ptUint8:
				uint8vals[0] = (uint8_t)value64;
				break;
			case ptInt16:
				uint16vals[0] = (uint16_t)value64;
				break;
			case ptInt32:
				uint32vals[0] = (uint32_t)value64;
				break;
			case ptInt64:
				value = value64;
				break;
			}
			_value = value;
		}

		/**
		* Sets the stored value as an float64.
		* @param The float64 value to set.
		*/
		void setValue(double valued)
		{
			if (ParamTocElement::is_integer(_ctype))
			{
				if (ParamTocElement::is_signed(_ctype))
				{
					setValue((int64_t)valued);
				}
				else
				{
					setValue((uint64_t)valued);
				}
			}
			else
			{
				union
				{
					uint64_t value;
					uint8_t buffer[8];
					double doubleValue;
					float floatVals[2];
				};
				value = 0;
				switch (_ctype)
				{
				
				case ptFloat32:
					floatVals[0] = (float)valued;
					break; 

				case ptFloat64:
					doubleValue = valued;
					break;
				}
				_value = value;
			}
		}

		/**
		* Sets the stored value with a buffer to packed data.
		* @param The packed data buffer to use for setting.
		*/
		void set(uint8_t *data)
		{
			union 
			{
				uint64_t value;
				uint8_t buffer[8];
			};
			value = 0;
			for (int32_t i = 0; i < _csize; i++)
			{
				buffer[i] = data[i];
			}
			_value = value;
			_state = SET | REQUEST_NONE;
		}
	};

	/**
	* Provides a structure for getting and setting a ParmValue
	* without using a name search for each get and set.
	*/
	struct ParamSetting
	{
		std::string completeName;
		double value;
		uint16_t ident;
		bool is_registered; 
		uint8_t ctype;

		ParamSetting()
		{
			value = 0;
			ident = NO_IDENT;
			is_registered = false;
			ctype = 0;
		}
	};

	/**
	* The extended request state
	*/
	const static uint8_t EXTENDED_PENDING = 0;
	const static uint8_t EXTENDED_REQUEST = 1;
	const static uint8_t EXTENDED_SET = 2;

	/**
	* The all params update state
	*/
	const static uint8_t ALL_PARAMS_PENDING = 0;
	const static uint8_t ALL_PARAMS_REQUESTED = 1;
	const static uint8_t ALL_PARAMS_DONE = 2;

	ParamToc toc;	/**< The current table of contents */
	
	std::vector <TocFetcher*> tocfetcherCallbacks;		/**< The active TocFetchers */
	std::vector <ParamValue*> values;					/**< list of ParamValue pointers ordered by identifier */
	std::queue <uint32_t> updateQueue;					/**< The queue of the identifiers being updated */
	std::queue <uint32_t> extendedTypeQueue;			/**< The queue of extended types being updated */
	std::mutex updateQueueMutex;						/**< the mutex to guard thread locking for the updateQueue */
	std::mutex extendedTypeQueueMutex;					/**< the mutex to guard thread locking for the extendedTypeQueue */
	std::thread queueThread;							/**< The thread for handling the queues */
	
	std::atomic <int32_t> idCount = 0;						/**< The total number of parameters in the TOC */
	std::atomic<bool> running = false;						/**< The Param is connected and handling requests */
	std::atomic<uint16_t> extendedRequestIdent = NO_IDENT;	/**< The identifier for the current extended param request */
	std::atomic<uint8_t> extendedState = EXTENDED_PENDING;	/**< The extended request state of the current extendedRequestIdent*/
	std::atomic<uint8_t> updateState = ALL_PARAMS_PENDING;	/**< The all param update state */
	uint8_t protocolVersion = 0;							/**< The protocol version of the connected crazyflie */
	bool useV2 = false;										/**< The protocol version supports uint16_t identifiers. */

	/**
	* Constructor
	*/
	Param() {

		protocolVersion = 0xff;
		useV2 = false;

	}

	/**
	* Destructor
	*/
	~Param()
	{
		clear();
	}

	/**
	* Virtual PortClient call to stop the Param.
	*/
	void stop()
	{
		clear();
	}

	/**
	* Clears the Param of all current state.
	* Stops processing the extended and update queues.
	* Removes all ParamValues and clears the list.
	* Removes all TocFetchers
	* Clears the TOC.
	* Clears the queues.
	* Sets all state as pending.
	*/
	void clear()
	{
		if (running)
		{
			running = false;
			queueThread.join();
		}
		for (size_t i = 0; i < tocfetcherCallbacks.size(); i++)
		{
			if (tocfetcherCallbacks[i] != NULL)
			{
				delete tocfetcherCallbacks[i];
				tocfetcherCallbacks[i] = NULL;
			}
		}
		for (size_t i = 0; i < values.size(); i++)
		{
			if (values[i] != NULL)
			{
				delete values[i];
				values[i] = NULL;
			}
		}

		values.clear();
		tocfetcherCallbacks.clear();
		toc.clear();
		resetComplete = false;
		protocolVersion = 0xff;
		useV2 = false;
		while(!updateQueue.empty())
			updateQueue.pop();
		while (!extendedTypeQueue.empty())
			extendedTypeQueue.pop();
		updateState = ALL_PARAMS_PENDING;

	}

	/**
	* Virtual PortClient call to set a new connection to a crazyflie
	* @param The PortConnect to use for port communication.
	*/
	void setConnection(PortConnect* _portConnect)
	{
		portConnect = _portConnect;

		if (portConnect != NULL && portConnect->platform != NULL)
		{
			if (portConnect != NULL)
			{
				clear();
			}
			protocolVersion = portConnect->platform->get_version();
			useV2 = protocolVersion >= 4;
			running = true;
			queueThread = std::thread(queueThreadFunc, this);
		}
	}

	/**
	* Handles tasks when the TOC complete building.
	* Requests extended information for element in the TOC.
	* @param The PortConnect to use for port communication.
	*/
	void toc_complete()
	{
		idCount = toc.get_id_count();
		values.resize(idCount, NULL);
		bool done = true;

		for (size_t i = 0; i < toc.groups.size(); i++)
		{
			for (size_t j = 0; j < toc.groups[i].elements.size(); j++)
			{
				if (toc.groups[i].elements[j].is_extended())
				{
					uint16_t ident = toc.groups[i].elements[j].ident;
					if (ident < values.size())
					{
						//messageOut << "ExParamQueued: ";
						//messageOut << ident;
						//messageOut << "\n\r";
						std::lock_guard<std::mutex> guard(extendedTypeQueueMutex);
						extendedTypeQueue.push(ident);
						done = false;
					}
				}
			}
		}
		resetComplete = done;
	}

	/**
	* Virtual PortClient call to reset the TOC. 
	*/
	void reset()
	{
		toc.groups.clear();
		TocFetcher* tocFetcher =
			new TocFetcher(this, PARAM, &this->toc);
		messageOut << "Resetting Param.\n\r";
		tocFetcher->start();
	}

	/**
	* Updates the values of all params
	*/
	void update_all()
	{
		request_update_of_all_params();
	}

	/**
	* Performs an update request for each param element.
	*/
	void request_update_of_all_params()
	{
		if (resetComplete)
		{
			for (size_t i = 0; i < toc.groups.size(); i++)
			{
				for (size_t j = 0; j < toc.groups[i].elements.size(); j++)
				{
					std::string completeName = toc.groups[i].elements[j].group; 
					completeName += ".";
					completeName += toc.groups[i].elements[j].name;
					request_param_update(completeName);
				}
			}
		}
		updateState = ALL_PARAMS_REQUESTED;
		messageOut << "Requesting values for all params.\n\r";
	}

	/**
	* Checks to see if all params are aupdated.
	* @returns true if all params have values.
	*/
	bool _check_if_all_updated()
	{
		bool result = false; 
		if (resetComplete)
		{
			if (values.size() > 0)
			{
				idCount = toc.get_id_count();
				if (values.size() == idCount)
				{
					result = true;
					for (size_t i = 0; i < values.size(); i++)
					{
						if (values[i] == NULL)
						{
							result = false;
							break;
						}
					}
				}
			}
		}
		return(result);
	}

	/**
	* Updates a Param and ParamValue from a requested packet.
	* @param The packet to use for the update
	*/
	void _param_updated(Packet& pk)
	{
		uint8_t channel = pk.channel();
		uint8_t id_index = 0;

		if (channel == MISC_CHANNEL)
		{
			id_index = 1;
		}
		uint16_t var_id = 0;
		uint8_t* data = pk.payload();

		if (useV2)
		{
			id_index += PackUtils::unpack(data, id_index, var_id);
		}
		else
		{
			var_id = data[id_index];
			id_index++;
		}
		if (channel == MISC_CHANNEL && var_id == extendedRequestIdent)
		{
			if (extendedState == EXTENDED_REQUEST)
			{
				uint8_t extendedType = data[id_index];
				if (extendedType == EXTENDED_PERSISTENT)
				{
					ParamTocElement element = toc.get_element_by_id(var_id);
					if (element.ident != NO_IDENT)
					{
						element.mark_persistent();
					}
				}
				//messageOut << "ExParm received: ";
				//messageOut << var_id;
				//messageOut << "\n\r";

				extendedState = EXTENDED_SET;
			}
		}
		else if (var_id < values.size())
		{
			if (values[var_id] != NULL)
			{
				if (channel == MISC_CHANNEL)
				{
					values[var_id]->set(data + id_index);
				}
				else if (channel == READ_CHANNEL)
				{
					values[var_id]->set(data + id_index + 1);
					if (updateState == ALL_PARAMS_REQUESTED)
					{
						if (var_id == idCount - 1)
						{
							updateState = ALL_PARAMS_DONE;
							messageOut << "Read values for all params.\n\r";
							portConnect->paramResetComplete();
						}
					}
				}
				else // release the queue in any case.
				{
					values[var_id]->_state = ParamValue::SET | ParamValue::REQUEST_NONE;
				}
			}
		}
	}

	/**
	* Requests an update for an indivdual param.
	* @param The complete name (group.name) of the param.
	*/
	void request_param_update(std::string& completeName)
	{
		ParamTocElement& element = toc.get_element_by_complete_name(completeName);
		if (element.ident != NO_IDENT)
		{
			uint16_t ident = element.ident;
			if (ident < values.size())
			{
				if (values[ident] == NULL)
				{
					ParamValue* paramValue = new ParamValue();
					uint8_t ctype = ParamTocElement::get_id_from_cstring(element.ctype);
					paramValue->_ctype = ctype;
					paramValue->_csize = ParamTocElement::get_size_from_id(ctype);
					paramValue->_state = ParamValue::PENDING | ParamValue::REQUEST_READ;
					values[ident] = paramValue;
				}
				else
				{
					values[ident]->_state = ParamValue::PENDING | ParamValue::REQUEST_READ;
				}
				{
					std::lock_guard<std::mutex> guard(updateQueueMutex);
					updateQueue.push(ident);
				}
			}
		}
	}

	/**
	* Registers a ParamSetting for read and write using its identiier.
	* @param The ParamSetting to register.
	*/
	bool registerParamSetting(ParamSetting &setting)
	{
		bool result = NULL;
		setting.ident = NO_IDENT;
		setting.is_registered = false;
		ParamTocElement& element = toc.get_element_by_complete_name(setting.completeName);
		if (element.ident != NO_IDENT)
		{
			setting.ident = element.ident;
			setting.is_registered = true;
			setting.ctype = ParamTocElement::get_id_from_cstring(element.ctype);
			result = get_value(setting);
		}
		return(result);
	}

	/**
	* Sets the value of a param using the complete name.
	* @param The complete name (group.name) of the param.
	* @param The value to set as a float64.
	*/
	void set_value(std::string complete_name, double value)
	{
		ParamTocElement& element = toc.get_element_by_complete_name(complete_name);
		
		if (element.ident != NO_IDENT)
		{
			uint8_t ctype = ParamTocElement::get_id_from_cstring(element.ctype);
			set_value(element.ident, ctype, value);
		}
	}

	/**
	* Gets the value of a param using a ParamSetting.
	* @param The ParamSetting
	* @returns true if the value was read.
	*/
	bool get_value(ParamSetting &setting)
	{
		bool result = false;
		if (setting.is_registered && setting.ident != NO_IDENT)
		{
			if (setting.ident < values.size())
			{
				if (values[setting.ident] != NULL)
				{
					setting.value = values[setting.ident]->getValue();
					result = true;
				}
			}
		}
		return(result);
	}

	/**
	* Gets the value of a param using the complete name.
	* @param The complete name (group.name) of the param.
* 	* @param The returned value as a float64.
	* @returns true if the value was read.
	*/
	bool get_value(std::string complete_name, double &value)
	{
		bool result = false;
		ParamTocElement& element = toc.get_element_by_complete_name(complete_name);

		if (element.ident != NO_IDENT)
		{
			if (element.ident < values.size())
			{
				if (values[element.ident] != NULL)
				{
					value = values[element.ident]->getValue();
					result = true;
				}
			}
		}
		return(result);
	}

	/**
	* Sets the value of a param using a ParamSetting.
	* @param The ParamSetting
	* @returns true if the value was set.
	*/
	void set_value(ParamSetting &setting)
	{
		if (setting.is_registered && setting.ident != NO_IDENT)
		{
			set_value(setting.ident, setting.ctype, setting.value);
		}
	}

	/**
	* Sets the value of a param its identifier
	* Does not check that the supplied _cType is correct.
	* Use set_value by complete name if _cType is not known, 
	* Or use a ParamSetting.
	* @param The identifier of the param.
	* @param c-language type index for the param.
	* @param The value to set as a float64.
	* @returns true of the request was sent.
	*/
	bool set_value(uint16_t ident, uint8_t _cType, double value)
	{
		bool result = false;
		if (ident != NO_IDENT)
		{
			if (ident < values.size())
			{
				if (values[ident] == NULL)
				{
					ParamValue* paramValue = new ParamValue();
					paramValue->_ctype = _cType;
					paramValue->_csize = ParamTocElement::get_size_from_id(_cType);
					paramValue->_state = ParamValue::PENDING | ParamValue::REQUEST_WRITE;
					values[ident]->setValue(value);
					values[ident] = paramValue;
				}
				else
				{
					values[ident]->setValue(value);
					values[ident]->_state = ParamValue::PENDING | ParamValue::REQUEST_WRITE;
				}
				{
					std::lock_guard<std::mutex> guard(updateQueueMutex);
					updateQueue.push(ident);
					result = true;
				}
			}
		}
		return(result);
	}

	/**
	* Handles receiving a new packet from the PortConnect.
	* This is a Virtual PortClient call to handle a PARAM port packet.
	* @param the requested packet.
	*/
	void _new_packet_cb(Packet& pk)
	{
		if (pk.size() > 0)
		{
			uint8_t port = pk.port();
			uint8_t channel = pk.channel();

			if (port == PARAM)
			{
				if (channel == READ_CHANNEL || channel == WRITE_CHANNEL)
				{
					_param_updated(pk);
				}
				else if (channel == MISC_CHANNEL)
				{
					_param_updated(pk);
				}
				else if (channel == TOC_CHANNEL)
				{
					uint8_t command = pk.payload()[0];
					for (size_t i = 0; i < this->tocfetcherCallbacks.size(); i++)
					{
						TocFetcher* tfetch = this->tocfetcherCallbacks[i];
						if (tfetch->expectedReply == command)
						{
							tfetch->_new_packet_cb(pk);
						}
					}
				}
			}
		}
	}

	/**
	* Handles both the update and the extended queues.
	* @param The owner Param.
	*/
	static void queueThreadFunc(void* data)
	{
		Param* param = (Param*)data;
		if (param != NULL)
		{
			bool useV2 = param->useV2;

			while (param->running)
			{
				bool hasExtendedQueue = false;
				{
					std::lock_guard<std::mutex> guard(param->extendedTypeQueueMutex);
					size_t queueSize = param->extendedTypeQueue.size();
					if (queueSize > 0)
					{
						hasExtendedQueue = true;
						uint16_t var_id = param->extendedTypeQueue.front();
						if (var_id == param->extendedRequestIdent)
						{
							if (param->extendedState == EXTENDED_SET)
							{
								param->extendedTypeQueue.pop();
								param->extendedState = EXTENDED_PENDING;
								param->extendedRequestIdent = NO_IDENT;
								if (param->extendedTypeQueue.size() == 0)
								{
									param->resetComplete = true;
									messageOut << "ExParam update complete.\n\r";
								}
							}
						}
						else
						{
							if (param->extendedRequestIdent == NO_IDENT)
							{
								Packet pk;
								pk.setPort(PARAM);
								pk.setChannel(MISC_CHANNEL);
								int32 index = 0;
								uint8_t* buffer = pk.payload();
								index += PackUtils::pack(buffer, index, (uint8_t)MISC_GET_EXTENDED_TYPE);
								index += PackUtils::pack(buffer, index, var_id);
								pk.setPayloadSize(index);
								param->portConnect->send_packet(pk, MISC_GET_EXTENDED_TYPE);
								param->extendedRequestIdent = var_id;
								param->extendedState = EXTENDED_REQUEST;
								//messageOut << "ExParamRequest: ";
								//messageOut << (int32_t)var_id;
								//messageOut << "\n\r";
							}
						}
					}
				}
				if (!hasExtendedQueue)
				{
					std::lock_guard<std::mutex> guard(param->updateQueueMutex);
					size_t queueSize = param->updateQueue.size();
					if (queueSize > 0)
					{
						uint16_t var_id = param->updateQueue.front();
						if (var_id < param->values.size())
						{
							if (param->values[var_id] != NULL)
							{
								if (param->values[var_id]->_state == 
									(ParamValue::PENDING | ParamValue::REQUEST_READ))
								{
									Packet pk; 
									pk.setPort(PARAM);
									pk.setChannel(READ_CHANNEL); 
									int32 index = 0;
									uint8_t* buffer = pk.payload();
									if (useV2)
									{
										index += PackUtils::pack(buffer, index, var_id); 
									}
									else
									{
										index += PackUtils::pack(buffer, index, (uint8_t)var_id);
									}
									pk.setPayloadSize(index);
									param->portConnect->send_packet(pk, READ_CHANNEL);
									param->values[var_id]->_state = (ParamValue::REQUESTED | ParamValue::REQUEST_READ);
								}
								else if (param->values[var_id]->_state ==
									(ParamValue::PENDING | ParamValue::REQUEST_WRITE))
								{
									Packet pk;
									pk.setPort(PARAM);
									pk.setChannel(WRITE_CHANNEL);
									int32 index = 0;
									uint8_t* buffer = pk.payload();
									if (useV2)
									{
										index += PackUtils::pack(buffer, index, var_id);
									}
									else
									{
										index += PackUtils::pack(buffer, index, (uint8_t)var_id);
									}
									uint8_t csize = param->values[var_id]->_csize;
									uint64_t dataClump = param->values[var_id]->_value;
									uint8_t* data = (uint8_t*)&dataClump;
									for (uint8_t i = 0; i < csize; i++)
									{
										buffer[index] = data[i];
										index++;
									}
									pk.setPayloadSize(index);
									param->portConnect->send_packet(pk, WRITE_CHANNEL);
									param->values[var_id]->_state = (ParamValue::REQUESTED | ParamValue::REQUEST_WRITE);
								}
								else if (param->values[var_id]->_state == (ParamValue::SET | ParamValue::REQUEST_NONE))
								{
									param->updateQueue.pop();
								}
							}
							else
							{
								param->updateQueue.pop();
							}
						}
						else
						{
							param->updateQueue.pop();
						}
					}
				}
				std::this_thread::sleep_for(std::chrono::microseconds(1000));
			}
		}
	}
};
