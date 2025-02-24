/*
* Header-only implementation of message output
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
#include <iostream>

class NullStream : public std::ostream {
public:
	NullStream() : std::ostream(nullptr) {}
	NullStream(const NullStream&) : std::ostream(nullptr) {}
};

template <class T>
NullStream& operator<<(NullStream& os, const T& value) {
	return os;
}


//static NullStream messageOut;
static std::ostream &messageOut = std::cout;

