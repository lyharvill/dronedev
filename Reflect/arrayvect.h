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
#include "lockIO.h"
#include <vector>


class arrayVect
{
public:
	std::vector<Type> v;

	propVect() {};
	~propVect() {};

	propVect(size_t n, Type t) { v.resize(n, t); };
	propVect(propVect &a) { v.assign(a.v.begin(), a.v.end()); };
	propVect(std::vector<Type> &a) { assign(a.v.begin(), a.v.end()); };

	inline operator std::vector<Type>&(void) { return v; }

	inline Type& operator [] (size_t n)
	{
		return(v[n]);
	};

	size_t size() { return(v.size()); }
	void pushProp(void *buff)
	{
		v.push_back(*((Type*)(buff)));
	}
	void push_back(Type& elem) { v.push_back(elem); }
	void clear() { v.clear(); }
	void resize(size_t n) {
		v.resize(n);
	}

	void resize(size_t n, Type &a) {
		v.resize(n, a);
	}

	void *getNth(size_t n){return(&v[n]); }

	void *newElem() { return(new Type); };
	reflectProp *getProperty(void *elem) { return(((Type*)elem)->getReflect()); };

	void safeElemDelete(void* elem) { if (elem) delete (Type*)elem; }
	
	reflectProp *getReflect(void) {
		if (v.size()) { return(v[0]->getReflect()); } return(NULL);
	}
};