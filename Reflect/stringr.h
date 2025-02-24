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
#include <string>


class stringr : public std::string
{
public:

	stringr(){};
	~stringr(){};

	stringr(const std::string& str){ assign(str); }
	stringr(const std::string& str, size_t pos, size_t len = npos){ assign(str, pos, len); }
	stringr(const char* s){ assign(s); };
	stringr(const char* s, size_t n){ assign(s, n); }
	stringr(size_t n, char c){ resize(n, c);};

	static const char* className()
	{
		static const char gClassName[] = "stringr\0\0\0\0";
		return(gClassName);
	}

	static uint64 classID() { return(makeClassID(typeName())); }; 
	
	static const char* typeName()
	{
		static const char gTypeName[] = "char\0\0\0\0";
		return(gTypeName);
	}

	static const uint64 typeID()
	{
		return(*((uint64*)typeName()));
	}


	static void write(std::ostream &output, void *buf)
	{
		if (buf)
		{
			output << "\"";
			output << *((stringr*)buf);
			output << "\"";
			
		}
	}

	static bool read(std::istream &input, void *buf)
	{
		bool result = false;
		if (buf)
		{
			char n = input.peek();
			if (n == '"')
			{
				std::string value;
				value = ((std::istringstream&)input).str();
				if (value.size() >= 2)
				{
					*((stringr*)buf) = value.substr(1, value.size() - 2);
				}
			}
			else
			{
				*((stringr*)buf) = ((std::istringstream&)input).str();
				result = !input.eof();
			}
			result = !input.eof();
		}
		return(result);
	}

	static void writeBin(std::ostream &output, void *buf)
	{
		if (buf)
		{
			std::string &vect = *((std::string*)buf);
			size_t n = vect.size();
			output.write((char*)&n, sizeof(size_t));
			if (n > 0)
				output.write((char*)vect.data(), n * sizeof(char));
		}
	}

	static bool readBin(std::istream &input, void *buf)
	{
		bool result = !input.eof();
		if (buf && result)
		{
			std::string &vect = *((std::string*)buf);
			size_t n = 0;
			input.read((char*)&n, sizeof(size_t));
			if (n > 0)
			{
				vect.resize(n);
				input.read((char*)vect.data(), n * sizeof(char));
				result = true;
			}
		}
		return(result);
	}
};


