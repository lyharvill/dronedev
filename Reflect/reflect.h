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


#include "reflecthelper.h"
//#include "mExceptions.h"
#include <typeinfo>     // operator typeid
#include <iostream>     // istream, ostream
#include <vector>
#include <typeindex>


static inline uint64 makeClassID(const char *name)
{
    uint64 id = 0;
    size_t length = strlen(name);
    if (length > sizeof(uint64))
        length = sizeof(uint64);
    memcpy(&id, name, length);
    return(id);
}
/**
* Starts building a list of reflectProps for a given structure.
*/
#define beginProperties(_struct)\
		static size_t size() {return(sizeof(_struct));};\
static  const char *typeName() {const static char gTypeName[] = # _struct; return((const char*)gTypeName);};\
		static uint64 classID(){ return(makeClassID(typeName())); };\
		static  bool IsA(uint64 id) {return(classID() == id);};\
	    reflectProp *getReflect(void) {typedef _struct _gClass;  static reflectProp gProps[] = {

/**
* Creates a reflectProp for member variable as a key-value.
*/
#define reflectKeyValue(_type, _name){#_name, #_type, offsetof(_gClass, _name), sizeof(_name), sizeof(_type), prop_keyValue, _type::read, _type::write, _type::readBin, _type::writeBin, NULL, 0, 1, 0, "", 0, NULL},

/**
* Creates a reflectProp for member variable as a key-value.
*/
#define reflectContent(_type, _name){#_name, #_type, offsetof(_gClass, _name), sizeof(_name), sizeof(_type), prop_content, _type::read, _type::write, _type::readBin, _type::writeBin, NULL, 0, 1, 0, "", 0, NULL},

/**
* Creates a reflectProp for member variable as a key-value.
*/
#define reflectTextValue(_type, _name, defaultName){#_name, #_type, offsetof(_gClass, _name), sizeof(_name), sizeof(_type), prop_keyValue, _type::read, _type::write, _type::readBin, _type::writeBin, NULL, 0, 1, 0, defaultName, 0, NULL},


/**
* Creates a reflectProp for member variable as a key-value.
*/
#define reflectNumericRange(_type, _name, minValue, maxValue, defaultValue){#_name, #_type, offsetof(_gClass, _name), sizeof(_name), sizeof(_type), prop_keyValue, _type::read, _type::write, _type::readBin, _type::writeBin, NULL, minValue, maxValue, defaultValue, "", 0, NULL},

/**
* Creates a reflectProp for an array of member variables as an attribute.
*/
#define reflectArray(_type, _name){#_name, #_type, offsetof(_gClass, _name), sizeof(_name),  _type::size(), prop_array, _type::read, _type::write, _type::readBin, _type::writeBin, NULL, 0, 1, 0, "", 0, NULL},

/**
* Creates a reflectProp for an array of member variables as an attribute.
*/
#define reflectNumericArray(_type, _name, minValue, maxValue, defaultValue){#_name, #_type, offsetof(_gClass, _name), sizeof(_name),  _type::size(), prop_array, _type::read, _type::write, _type::readBin, _type::writeBin, NULL, minValue, maxValue, defaultValue, "", 0, NULL},

/**
* Creates an reflectProp for member variable as content.
*/
#define reflectVector(_type, _name){#_name, #_type, offsetof(_gClass, _name),  sizeof(_name), sizeof(_name), prop_vector, _type::read, _type::write, _type::readBin, _type::writeBin, NULL, 0, 1, 0, "", 0, NULL},


/**
* Creates an reflectProp for a struct as a list of properties.
*/
#define reflectProperty(_type, _name){#_name, #_type, offsetof(_gClass, _name),  sizeof(_name),  _type::size(), prop_properties, NULL, NULL, NULL, NULL, _name.getReflect(), 0, 1, 0, "", 0, NULL},

/**
* Creates an reflectProp for an array of a struct as a list of properties.
*/
#define reflectPropArray(_type, _name){#_name, #_type, offsetof(_gClass, _name),  sizeof(_name), sizeof(_type), prop_prop_array, NULL, NULL, NULL, NULL, _name->getReflect(), 0, 1, 0, "", 0, NULL},

/**
* Creates an reflectProp for a struct as a reference of properties.
*/
#define reflectReference(_type, _name){#_name, #_type, offsetof(_gClass, _name),  sizeof(_name),  sizeof(_type), prop_reference, NULL, NULL, NULL, NULL, NULL, 0, 1, 0, "", 0, NULL},,

/**
* Creates an reflectProp for a struct as a list of properties.
*/
#define reflectRefList(_type, _name){#_name, #_type, offsetof(_gClass, _name),  sizeof(_name),  sizeof(_type), prop_ref_list, NULL, NULL, NULL, NULL, NULL, 0, 1, 0, "", 0, NULL},

/**
* Creates an reflectProp for an array of a struct as a vector of properties.
*/
#define reflectPropVector(_type, _name){#_name, #_type, offsetof(_gClass, _name),  sizeof(_name), sizeof(_type), prop_prop_vector, NULL, NULL, NULL, NULL, _name.getReflect(), 0, 1, 0, "", 0, NULL},

/**
* Creates a reflectProp for member variable ordinal list of const strings
*/
#define reflectItemList(_type, _name, count, defaultValue, stringList){#_name, #_type, offsetof(_gClass, _name), sizeof(_name), sizeof(_type), prop_keyValue, _type::read, _type::write, _type::readBin, _type::writeBin, NULL, 0, count, defaultValue, "", 0, stringList},


/**
* Ends building the list of reflectProps
*/
#define endProperties {NULL, NULL, 0, 0, 0, 0, NULL, NULL, NULL, NULL, 0, 0, 0, 0, "", 0, NULL}}; return((reflectProp*)(&gProps)); }


class boolr
{
public:
	bool n = false;

	boolr() { n = false; }
	boolr(const boolr &a) { n = a.n; }
	boolr& operator = (const boolr a) { n = a.n; return *this;}
	boolr& operator = (const bool a) { n = a; return *this;}
	void clear() { n = false; }
	static size_t size() { return(sizeof(boolr)); }
	static const char* typeName() { const static char gTypeName[] = "boolr"; return((const char*)gTypeName); }
	static const char* className() { const static char gClassName[] = "boolr"; return((const char*)gClassName); }
	static uint64 classID() { return(makeClassID(typeName())); }; 
	static void write(std::ostream &output, void *buf)
	{
		if (buf)
		{
			boolr *aBoolr = (boolr*)buf;
			if (aBoolr->n)
			{
				output << "true";
			}
			else
			{
				output << "false";
			}
		}
	}

	static bool read(std::istream &input, void *buf)
	{
		bool result = false;
		if (buf)
		{
			boolr *aBoolr = (boolr*)buf;
			std::string value;
			input >> value;
			if (value == "true")
			{
				aBoolr->n = true;
			}
			else
			{
				aBoolr->n = false;
			}
		}
		return(result);
	}

	static bool readBin(std::istream &input, void *buff) {
			bool result = (buff != NULL) && !input.eof();
			if (result) input.read((char*)buff, size());
				return(result);
	}
	static void writeBin(std::ostream &output, void *buff) { if (buff) output.write((char*)buff, size()); };

	__forceinline operator const bool &(void) const { return (bool &)n; }
	__forceinline operator bool &(void) { return (bool &)n; }
};

class boolfunc
{
public:
	typedef void (*boolFunction)(void* data, bool state);

	bool n;
	boolFunction func;
	void* data;
	bool toggle;

	boolfunc() { n = false; func = NULL; data = NULL;  toggle = false; }
	boolfunc(boolfunc& a) { n = a.n; func = a.func;  data = a.data; toggle = a.toggle;  }
	boolfunc& operator = (const boolfunc a) { n = a.n; func = a.func;  data = a.data;  toggle = a.toggle; return *this; }

	void clear() { n = false; ; func = NULL; data = NULL;}
	static size_t size() { return(sizeof(boolfunc)); }
	static const char* typeName() { const static char gTypeName[] = "boolfunc"; return((const char*)gTypeName); }
	static const char* className() { const static char gClassName[] = "boolfunc"; return((const char*)gClassName); }
	static uint64 classID() { return(makeClassID(typeName())); };
	static void write(std::ostream& output, void* buf)
	{
		if (buf)
		{
			boolfunc* aboolfunc = (boolfunc*)buf;
			if (aboolfunc->n)
			{
				output << "true";
			}
			else
			{
				output << "false";
			}
		}
	}

	static bool read(std::istream& input, void* buf)
	{
		bool result = false;
		if (buf)
		{
			boolfunc* aboolfunc = (boolfunc*)buf;
			std::string value;
			input >> value;
			if (value == "true")
			{
				aboolfunc->n = true;
			}
			else
			{
				aboolfunc->n = false;
			}
		}
		return(result);
	}

	static bool readBin(std::istream& input, void* buff) {
		bool result = (buff != NULL) && !input.eof();
		if (result) input.read((char*)buff, size());
		return(result);
	}
	static void writeBin(std::ostream& output, void* buff) { if (buff) output.write((char*)buff, size()); };

	__forceinline operator const bool& (void) const { return (bool&)n; }
	__forceinline operator bool& (void) { return (bool&)n; }
};



#define makeReflect(_type, _name)\
	class _name {\
	public:\
	_type n; \
	__forceinline _name(void){clear();}\
	__forceinline _name(_type &a){n= a;}\
	__forceinline _name(const _type a){n= a;}\
	__forceinline _type& operator =(const _name other) { n = other.n; return *this; }\
	__forceinline _type& operator =(const _type other) { n = other; return *this; }\
	__forceinline void clear(){n=0;}\
	static size_t size(){ return(sizeof(_type)); }\
        static const char* typeName(){ const static char gTypeName[] = #_type ; return((const char*)gTypeName); }\
    static const char* className(){ const static char gClassName[] = #_name  ; return((const char*)gClassName); }\
	static uint64 classID(){ return(makeClassID(typeName())); };\
	static bool read(std::istream &input, void *buff){\
		bool result = false;char n= input.peek();\
		if (isdigit(n) || n == '-' || n == '+')\
		{result = true; input >> *((_type*)buff);}\
		return(result);\
	}\
	static void write(std::ostream &output, void *buff){ output << *((_type*)buff); };\
	static bool readBin(std::istream &input, void *buff){\
		bool result = (buff != NULL) && !input.eof();\
		if (result) input.read ((char*)buff, size());\
		return(result);\
		}\
	static void writeBin(std::ostream &output, void *buff){if (buff) output.write((char*)buff, size()); };\
	__forceinline operator const _type &(void) const { return (_type &)n; }\
	__forceinline operator _type &(void) { return (_type &)n; }\
      };

#define makeReflectChar(_type, _name)\
	class _name {\
	public:\
	_type n; \
	__forceinline _name(void){clear();}\
	__forceinline _name(_type &a){n= a;}\
	__forceinline _type& operator =(const _name other) { n = other.n; return *this; }\
	__forceinline _type& operator =(const _type other) { n = other; return *this; }\
	__forceinline void clear(){n=0;}\
	static size_t size(){ return(sizeof(_type)); }\
	static const char* typeName(){ const static char gTypeName[] = #_type ; return((const char*)gTypeName); }\
	static const char* className(){ const static char gClassName[] = #_name ; return((const char*)gClassName); }\
	static uint64 classID(){ return(makeClassID(typeName())); };\
	static bool read(std::istream &input, void *buff){\
		bool result = false;char n= input.peek();\
		if (isdigit(n) || n == '-' || n == '+')\
		{int32 n0; result = true; input >> n0; *((_type*)buff) = n0;}\
		return(result);\
		}\
	static void write(std::ostream &output, void *buff){ output << (int32)*((_type*)buff); };\
	static bool readBin(std::istream &input, void *buff){\
		bool result = (buff != NULL) && !input.eof();\
		if (result) input.read ((char*)buff, size());\
		return(result);\
				}\
	static void writeBin(std::ostream &output, void *buff){if (buff) output.write((char*)buff, size()); };\
	__forceinline operator const _type &(void) const { return (_type &)n; }\
	__forceinline operator _type &(void) { return (_type &)n; }\
	        };

typedef float real32;
typedef double real64;
makeReflectChar(char, int8r);
makeReflectChar(uint8, uint8r);
  makeReflect(int16, int16r);
  makeReflect(uint16, uint16r);
  makeReflect(int32, int32r);
  makeReflect(uint32, uint32r);
  makeReflect(int64, int64r);
  makeReflect(uint64, uint64r);
  makeReflect(real32, real32r);
  makeReflect(real64, real64r);


  class numberTypes
  {
  public:

	  static bool isNumberType(const char *typeName)
	  {
		  const static char *numberTypes[]{
			  int8r::className(),
			  uint8r::className(),
			  int16r::className(),
			  uint16r::className(),
			  int32r::className(),
			  uint32r::className(),
			  int64r::className(),
			  uint64r::className(),
			  real32r::className(),
			  real64r::className() };
		  int32 nTypes = sizeof(numberTypes) / sizeof(char *);
		  bool result = false;
		  uint64 targetType = *((uint64*)typeName);
		  for (int32 i = 0; i < nTypes && result == false; i++)
		  {
			  uint64 testType = *((uint64*)numberTypes[i]);
			  result = testType == targetType;
		  }
		  return(result);
	  }
  };

  


 

 

