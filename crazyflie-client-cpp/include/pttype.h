/*
* LogToc Types
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
* Define the Param TOC types
*/


/**
* The structure for the lookup.
*/
struct ptType
{
	const char* name;
	const char* pytype;
	uint8_t size;
};

/**
* Ordinal types used as c types
*/
enum ptTypeDex :uint8_t
{
	ptInt8 = 0,
	ptInt16 = 1,
	ptInt32 = 2,
	ptInt64 = 3,
	ptFP8 = 4,
	ptFP16 = 5,
	ptFloat32 = 6,
	ptFloat64 = 7,
	ptUint8 = 8,
	ptUint16 = 9,
	ptUint32 = 10,
	ptUint64 = 11,
	ptNone = 0xff
};

/**
* Lookup for names, python packing, size.
*/
ptType ptTypes[] = {
	{"int8_t", "<b", 1},
	{"int16_t", "<h", 2},
	{"int32_t", "<i", 4},
	{"int64_t", "<q", 8},
	{"FP8", "" "", 1},
	{"FP16", "", 2},
	{"float32_t", "<f", 4},
	{"float64_t", "<d", 8},
	{"uint8_t", "<B", 1},
	{"uint16_t", "<H",2},
	{"uint32_t","<L",4},
	{"uint64_t","<Q",8},
};

const static uint8_t gPtTypesSize = 12;
const static uint8_t gPtMaxType = gPtTypesSize - 1;

const static uint8_t RW_ACCESS = 0;
const static uint8_t RO_ACCESS = 1;

const static uint8_t EXTENDED_PERSISTENT = 1;
const static uint8_t ptTypeNotFound = 0xff;

