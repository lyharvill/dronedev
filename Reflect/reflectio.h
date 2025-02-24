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


#include <iostream>     // istream, ostream


class reflectIO
{
public:

	reflectIO() {}

	inline static void writeSeparator(std::ostream &ouput)
	{
		ouput << ',';
	}

	inline static void lineIndent(std::ostream &ouput, int32 indent)
	{
		ouput << '\n';
		while (indent > 0)
		{
			ouput << '\t';
			indent--;
		}
	}

	inline static bool readSeparator(std::istream &input)
	{
		char n = input.peek();
		bool result = n != EOF;
		if (isspace(n) || n == ',')
		{
			input.get();
		}
		return(result);
	}

	inline static bool readEndOfLine(std::istream &input)
	{
		char n = input.peek();
		
		while (n != EOF && (isspace(n) || n == '\n' || n == '\r'))
		{
			input.get();
			n = input.peek();
		}
		bool result = n != EOF;
		return(result);
	}

	inline static bool readSpace(std::istream &input)
	{
		char n = input.peek();
		while (n != EOF && isspace(n))
		{
			input.get();
			n = input.peek();
		}
		return(n != EOF);
	}

	inline static bool eof(std::istream &input)
	{
		bool result = input.eof();
		result = result || (input.peek() == EOF);
		return(result);
	}
};
