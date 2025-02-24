/*
* LogToc Types
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

#include "stdint.h"


/**
* Define the Log TOC types
*/


/**
* The structure for the lookup.
*/
struct ltType
{
	const char* name;
	const char* pytype;
	uint8_t size;
};

/**
* Ordinal types used as c types
*/
enum typeDex :uint8_t
{
	tdUint8 = 0,
	tdUint16 = 1,
	tdUint32 = 2,
	tdInt8 = 3,
	tdInt16 = 4,
	tdInt32 = 5,
	tdFloat16 = 6,
	tdFloat32 = 7,
	tdNone = 0xff
};

/**
* Lookup for names, python packing, size.
*/

ltType types[] = {
	{"uint8_t", "<B", 1},
	{"uint16_t", "<H",2},
	{"uint32_t","<L",4},
	{"int8_t", "<b", 1},
	{"int16_t", "<h", 2},
	{"int32_t", "<i", 4},
	{"float16_t ", "<e", 2},
	{"float32_t", "<f", 4},
};

const static uint8_t gTypesSize = (uint8_t)(sizeof(types) / sizeof(ltType));
const static uint8_t gMaxType = gTypesSize - 1;
const static uint8_t gTypeNotFound = 0xff;

