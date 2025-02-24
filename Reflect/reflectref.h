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

#include "sys/mrefcount.h"
#include "sys/lockio.h"

/**
* Starts building a list of reflectProps for a class, including reference counting and IO locking.
*/
#define beginRefProperties(_class, _super)\
		public: \
		lockIO lock;\
		static  const char *className() {const static char gClassName[] = #_class ## "\0,\0,\0,\0,\0,\0,\0,\0"; return((const char*)gClassName);};\
		virtual const char *vClassName(void){ return(className()); };\
		static uint64 classID(){ return(*((uint16*)className())); };\
		virtual  bool IsA(size_t id) {return(classID() == id || _super::IsA(id));}; \
		virtual reflectProp *getReflect(void) {typedef _class _gClass;  static const reflectProp gProps[] = {\
				{#_super, #_super, 0,  sizeof(_super), sizeof(_super), prop_properties, NULL, NULL, NULL, NULL, _super::getReflect()},


/**
* Starts building a list of reflectProps for a node, including reference counting and IO locking.
*/
#define beginNodeProperties()\
		public: \
		lockIO lock;\
		static  const char *className() {const static char gClassName[] = "node\0\0\0\0\0\0\0\0"; return((const char*)gClassName);};\
		virtual const char *vClassName(void){ return(className()); };\
		static uint64 classID(){ return(*((uint16*)className())); };\
		virtual  bool IsA(size_t id) {return(classID() == id || iota::IsA(id));}; \
		virtual reflectProp *getReflect(void) {typedef node _gClass;  static const reflectProp gProps[] = {\
			{"nodelist", "nodelist", 0,  sizeof(node), sizeof(node), prop_node_list, NULL, NULL, NULL, NULL, NULL},


class iota : public embree::RefCount
{
public:
	iota(void):RefCount()
	{
	}
	virtual ~iota(void){};
	iota(const iota& a) {}

	size_t size() { return(0); };
	static  const char *className() { const static char gClassName[] = "iota\0\0\0\0"; return((const char*)gClassName); };
	virtual const char *vClassName(void){ return(className()); };
	virtual void rootBind(void){};
	static uint64 classID(){ return(*((uint16*)className())); };
	virtual  bool IsA(size_t id) { return(classID() == id); };
	virtual reflectProp *getReflect(void) { return(NULL); };

};

typedef embree::Ref<iota>  iotaRef;
typedef std::vector<iotaRef> refList; /**< list of iotaRefs, add to properties using reflectRefList()**/






