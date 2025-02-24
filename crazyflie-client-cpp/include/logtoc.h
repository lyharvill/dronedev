/*
* Header-only implementation of the log TOC for crazyflie
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


#include <vector>
#include <string>
#include "lttype.h"

#include "..\Reflect\reflect.h"
#include "..\Reflect\stringr.h"
#include "..\Reflect\propvect.h"
#include "..\Reflect\reflectjson.h"
#include "messageout.h"
#include <filesystem>

/**
* Holds a LogTocElement
* and provides an interface to its properties
* for fetching a LogVariable
*/
struct LogTocElement
{
	stringr name;
	stringr group;
	stringr ctype;
	stringr pytype;
	uint16r ident;		// the id in the toc
	uint8r access;

	/**
	* Constructor
	*/
	LogTocElement()
	{
		ident = gTypeNotFound;
		access = 0;
	}

	/**
	* Copy Constructor
	* @param the LogTocElement to copy.
	*/
	LogTocElement(const LogTocElement& a)
	{
		name = a.name;
		group = a.group;
		ctype = a.ctype;
		pytype = a.pytype;
		ident = a.ident;
		access = a.access;
	}

	/**
	* Construct from data
	* @param the identifier for the element.
	* @param the data segment to unpack.
	*/
	LogTocElement(uint16_t id, uint8_t* data)
	{
		ident = id;
		access = 0;
		if (data != NULL)
		{
			uint8_t typeDex = data[0];
			char* naming = (char*)(data + 1);
			group = naming;
			name = naming + (group.size() + 1);
			access = data[0] & 0x10;
			get_cstring_from_id(typeDex, ctype);
			get_unpack_string_from_id(typeDex, pytype);
		}
	}

	/**
	* Finds the ctype id from the ctype name.
	* @param The name of the c type.
	* @returns The id for the ctype.
	*/
	uint8_t get_id_from_cstring(std::string name)
	{
		uint8_t result = gTypeNotFound;
		for (int32_t i = 0; i < gTypesSize; i++)
		{
			if (name == types[i].name)
			{
				result = i;
				break;
			}
		}
		return(result);
	}

	/**
	* Finds the ctype name from the ctype id.
	* @param The id of the c type.
	* @param The returned name of the ctype.
	*/
	bool get_cstring_from_id(uint8_t id, std::string& name)
	{
		bool result = false;
		if (id < gTypesSize)
		{
			name = types[id].name;
			result = true;
		}
		return(result);
	}	
	
	
	/**
	* Finds the python unpack string from the ctype id.
	* @param The id of the c type.
	* @param The returned python unpack string.
	*/
	bool get_unpack_string_from_id(uint8_t id, std::string& pacName)
	{
		bool result = false;
		if (id < gTypesSize)
		{
			pacName = types[id].pytype;
			result = true;
		}
		return(result);

	}

	/**
	* Finds the size in bytes from the ctype id.
	* @param The id of the c type.
	* @returns The size in bytes of the ctype id.
	*/
	static uint8_t get_size_from_id(uint8_t id)
	{
		uint8_t result = 0;
		if (id < gTypesSize)
		{
			result = types[id].size;
		}
		return(result);
	}

	/**
	* Build reflection properties for json output
	* and for UI binding.
	*/
	beginProperties(LogTocElement)
		reflectKeyValue(stringr, name)
		reflectKeyValue(stringr, group)
		reflectKeyValue(stringr, ctype)
		reflectKeyValue(stringr, pytype)
		reflectKeyValue(uint16r, ident)
		reflectKeyValue(uint8r, access)
		endProperties
};



//Commands used when accessing the Table of Contents
const static uint8_t CMD_TOC_ELEMENT = 0;  // original version : up to 255 entries
const static uint8_t CMD_TOC_INFO = 1;    // original version : up to 255 entries
const static uint8_t CMD_TOC_ITEM_V2 = 2;  // version 2: up to 16k entries
const static uint8_t CMD_TOC_INFO_V2 = 3;  // version 2: up to 16k entries


/**
* Holds a named list of LogTocElements
*/
struct LogTocGroup
{
	stringr name;
	propVect<LogTocElement> elements;

	LogTocGroup() {}
	LogTocGroup(const LogTocGroup& a)
	{
		name = a.name;
		elements = a.elements;
	}

	/**
	* Build reflection properties for json output
	* and for UI binding.
	*/
	beginProperties(LogTocGroup)
		reflectKeyValue(stringr, name)
		reflectPropVector(propVect<LogTocElement>, elements)
		endProperties
};

/**
* Holds a list of LogTocGroups.
* Supports reading and writing based on a crc.
* This allows the LogToc to be cached.
*/
struct LogToc		//Container for LogTocElements
{
	uint32_t crc = 0;					/**< The crc for the entire TOC */
	propVect<LogTocGroup> groups;		/**< The list of LogTocGroups in the TOC*/
	std::string defaultPath;			/**< The location for reading and writing the cache.*/
	bool complete = false;				/**< True when the ParamToc is complete.*/

	/**
	* Constructor
	*/
	LogToc()
	{
		crc = 0;
		complete = false;
	}

	/**
	* Clears the TOC.
	*/
	void clear() {
		groups.clear();
		complete = false;
		crc = 0;
	}

	/**
	* Finds the index of the LogTocGroup with groupName.
	* @param The name of the group to find.
	* @returns The index of the group in the group list, or -1 if not found.
	*/
	int32_t groupIndex(const char* groupName)
	{
		int32_t groupDex = -1;
		for (size_t i = 0; i < groups.size(); i++)
		{
			if (groups[i].name == groupName)
			{
				groupDex = i;
				break;
			}
		}
		return(groupDex);
	}

	/**
	* Finds the index of the LogTocElement within the group at groupDex elementName.
	* @param The index of the group in the group list.
	* @param The name of the element to find.
	* @returns The index of the element in the group, or -1 if not found.
	*/
	int32_t nameIndex(int32_t groupDex, const char* elementName)
	{
		int32_t elemDex = -1;
		if (groupDex >= 0 && groupDex < groups.size())
		{
			LogTocGroup& group = groups[groupDex];
			for (size_t i = 0; i < group.elements.size(); i++)
			{
				if (group.elements[i].name == elementName)
				{
					elemDex = i;
					break;
				}
			}
		}

		return(elemDex);
	}

	/**
	* Add an element to the TOC.
	* @param The element to add.
	*/
	void add_element(LogTocElement& element)
	{
		int32_t groupDex = groupIndex(element.group.c_str());
		if (groupDex >= 0)
		{
			int32_t elemDex = nameIndex(groupDex, element.name.c_str());
			if (elemDex < 0)
			{
				LogTocGroup& group = groups[groupDex];
				group.elements.push_back(element);
			}
		}
		else
		{
			LogTocGroup group;
			group.name = element.group;
			group.elements.push_back(element);
			groups.push_back(group);
		}
	}

	/**
	* Get an element by complete name <groupName>.<elementName>.
	* @param The complete name to find.
	* @param  The found element.
	* @returns True if the element was found.
	*/
	bool  get_element_by_name(std::string completeName, LogTocElement& element)
	{
		bool result = false;
		size_t nameStart = completeName.find_first_of(".", 0);
		if (nameStart != std::string::npos)
		{
			std::string groupName = completeName.substr(0, nameStart);
			std::string elemName = completeName.substr(nameStart + 1, completeName.size());
			int32_t groupDex = groupIndex(groupName.c_str());
			if (groupDex >= 0)
			{
				int32_t elemDex = nameIndex(groupDex, elemName.c_str());
				if (elemDex >= 0)
				{
					element = groups[groupDex].elements[elemDex];
					result = true;
				}
			}
		}
		return(result);
	}

	/**
	* Get the element identifier by complete name <groupName>.<elementName>.
	* @param The complete name to find.
	* @returns element identifier or NO_IDENT.
	*/
	uint16_t get_element_id(std::string completeName)
	{
		uint16_t id = NO_IDENT;
		LogTocElement element;
		if (get_element_by_name(completeName, element))
		{
			id = element.ident;
		}
		return(id);
	}

	/**
	* Get the element identifier by complete name <groupName>.<elementName>.
	* @param The complete name to find.
	* @returns element identifier or NO_IDENT.
	*/
	bool get_element_by_complete_name(std::string completeName, LogTocElement& element)
	{
		return(get_element_by_name(completeName, element));
	}


	/**
	* Get an element by group name and element name
	* @param The group name to find.
	* @param The element name to find.
	* @param  The found element.
	* @returns True if the element was found.
	*/
	bool get_element(std::string groupName, std::string elemName, LogTocElement& element)
	{
		bool result = false;
		int32_t groupDex = groupIndex(groupName.c_str());
		if (groupDex >= 0)
		{
			int32_t elemDex = nameIndex(groupDex, elemName.c_str());
			if (elemDex >= 0)
			{
				element = groups[groupDex].elements[elemDex];
				result = true;
			}
		}
		return(result);
	}

	/**
	* Get an element by its identifier.
	* @param The identifier
	* @param  The found element.
	* @returns True if the element was found.
	*/
	bool get_element_by_id(uint16_t ident, LogTocElement& element)
	{
		bool result = false;
		for (size_t i = 0; i < groups.size(); i++)
		{
			for (size_t j = 0; j < groups[i].elements.size(); j++)
			{
				if (groups[i].elements[j].ident == ident)
				{
					element = groups[i].elements[j];
					result = true;
					break;
				}
			}
		}
		return(result);
	}

	/**
	* Read the TOC from a path
	* @param The full path to the file
	*/
	void read(std::string path)
	{
		reflectJson::readProperties(path, getReflect(), (uint8*)this);
	};

	/**
	* Writes the TOC to a path
	* @param The full path to the file
	*/
	bool write(std::string path)
	{
		reflectJson::writeProperties(path, (uint8*)this, getReflect());
		return(true);
	};

	/**
	* Read the TOC from a crc
	* @param The crc of the file
	*/
	bool read(uint32_t crc)
	{
		bool result = false;
		std::string fullPath;
		if (getfullTocPath(crc, fullPath))
		{
			if (std::filesystem::exists(fullPath.c_str()))
			{
				read(fullPath);
				result = groups.size() > 0;
			}
		}
		return(result);
	}

	/**
	* Write the TOC using a crc
	* @param The crc of the file
	*/
	bool write(uint32_t crc)
	{
		bool result = false;
		std::string fullPath;
		if (getfullTocPath(crc, fullPath))
		{
			result = write(fullPath);
			if (result)
			{
				messageOut << "Wrote the Log TOC to: ";
				messageOut << fullPath;
				messageOut << "\n\r";
			}
			else
			{
				messageOut << "Could not write the Log TOC\n\r ";
			}
		}
		return(result);
	}

	/**
	* Builds the full path to a file from the defaultDirectory and the crc.
	* @param The crc of the file
	* @param The returned full path.
	* @returns True if successful.
	*/
	bool getfullTocPath(uint32_t crc, std::string& fullPath)
	{
		bool result = false;
		fullPath.clear();

		static const char defaultTocFolder[] = "\\TocCache";
		std::string folderPath = defaultPath;
		folderPath += defaultTocFolder;
		result = std::filesystem::is_directory(folderPath);
		if (!result)
		{
			result = std::filesystem::create_directory(folderPath);
		}
		if (result)
		{
			char filename[1024];
			sprintf_s(filename, "%08lX_toc.json", crc);
			std::filesystem::path tocPath = folderPath;
			tocPath /= filename;
			fullPath = tocPath.u8string();
		}
		return(result);
	}

	/**
	* Determines of the TOC exists
	* @param The crc of the file
	* @returns True if the TOC exists.
	*/
	bool tocExists(uint32_t _crc)
	{
		std::string fullPath;
		bool result = getfullTocPath(_crc, fullPath);
		return(std::filesystem::exists(fullPath));
	}

	/**
	* Build reflection properties for json output
	* and for UI binding.
	*/
	beginProperties(LogToc)
		reflectPropVector(propVect<LogTocGroup>, groups)
		endProperties
};
