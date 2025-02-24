//
// The MIT License (MIT)
//
// Copyright (C) 2000-2025 Young Harvill
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include "reflect.h"
#include "reflectio.h"
#include "stringr.h"
#include <vector>


template<typename Type>
struct vectr : public std::vector<Type>
{
public:
	vectr(){};
	~vectr(){};

	vectr(size_t n, Type t){ resize(n, t); };
	vectr(vectr &a){assign(a.begin(), a.end());};
	vectr(std::vector<Type> &a){ assign(a.begin(), a.end()); };


	static const char* typeName()
	{
		return(Type::typeName());
	}

	static const uint64 typeID()
	{
		return(Type::typeID());
	}

	static void write(std::ostream &output, void *buf)
	{
		if (buf)
		{
			std::vector<Type> &vect = *((std::vector<Type>*)buf);
			for (size_t i = 0; i < vect.size(); i++)
			{
				Type::write(output, &vect[i]);
				if (i < vect.size() - 1)
					reflectIO::writeSeparator(output);
			}
		}
	}

	static bool read(std::istream &input, void *buf)
	{
		bool result = false;
		if (buf)
		{
			std::vector<Type> &vect = *((std::vector<Type>*)buf);
			bool reading = reflectIO::readSpace(input);
			while (reading)
			{
				Type n;
				reading = Type::read(input, &n);
				if (reading)
				{
					vect.push_back(n);
					reading = reflectIO::readSeparator(input);
					result = true;
				}
			}
		}
		return(result);
	}

	static void writeBin(std::ostream &output, void *buf)
	{
		if (buf)
		{
			std::vector<Type> &vect = *((std::vector<Type>*)buf);
			size_t n = vect.size();
			output.write((char*)&n, sizeof(size_t));
			if (buf)
			{
				for (size_t i = 0; i < vect.size(); i++)
				{
					Type::writeBin(output, &vect[i]);
				}
			}
		}
	}

	static bool readBin(std::istream &input, void *buf)
	{
		bool result = !input.eof();
		if (buf && result)
		{
			std::vector<Type> &vect = *((std::vector<Type>*)buf);
			size_t n = 0;
			input.read((char*)&n, sizeof(size_t));
			if (n > 0)
			{
				vect.resize(n);
				for (size_t i = 0; i < vect.size(); i++)
				{
					Type::readBin(input, &vect[i]);
				}
				result = true;
			}
		}
		return(result);
	}
	reflectProp* getReflect(void) {
		static reflectProp gProps[] = {
		{NULL, NULL, 0, 0, 0, 0, NULL, NULL, NULL, NULL, 0, 0, 0, 0, "", 0, NULL} };
		return((reflectProp*)(&gProps));
	}

};

typedef vectr<int32r> int32Vect;
typedef vectr<int64r> int64Vect;
typedef vectr<real32r> real32Vect;
typedef vectr<real64r> real64Vect;
typedef vectr<stringr> stringVect;

