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


#include "../common/sys/platform.h"

const static uint64 prop_keyValue = 1;		// write this property as an attribute for XML;
const static uint64 prop_array = 2;			// write this property as an array;
const static uint64 prop_vector = 4;		// write this property as vector content for XML rather than as an attribute;
const static uint64 prop_properties = 5;	// write this property as a list of properties
const static uint64 prop_reference = 6;		// write this property as a reference to a list of properties
const static uint64 prop_prop_array = 7;	// write this property as a reference to a fixed size array of properties
const static uint64 prop_ref_list = 8;		// write this property as a list of refs.
const static uint64 prop_node_list = 9;		// write this property as a list of linkRefs.
const static uint64 prop_prop_vector = 10;	// write this property as vector of the same properties.
const static uint64 prop_content = 11;		// write this property as content for XML;


const static uint64 uiHint_Edit = 1;		// make this property  editable
const static uint64 uiHint_NoShow = 2;		// Don't show this property


typedef bool(*readElem)(std::istream &input, void *buf);
typedef void(*writeElem)(std::ostream &output, void *buf);
typedef bool(*readBinElem)(std::istream &input, void *buf);
typedef void(*writeBinElem)(std::ostream &output, void *buf);

/**
* Struct for holding reflection properties of member variable that are not available through RTTI.
*/
struct reflectProp
{
	const char *rpName;			/**< name of the member variable as declared in the class*/
	const char *rpType;			/**< name of the property type */
	size_t rpOffset;			/**< offset in bytes from the class pointer to the location of the member variable*/
	size_t rpSize;				/**< size of the property in bytes */
	size_t rpTypeSize;			/**< size of the type in bytes */
	uint64 rpFlags;				/**< additional hints on how to serialize the member variable*/
	readElem rpReadProc;		/**< proc for reading the element*/
	writeElem rpWriteProc;		/**< proc for writing the element*/
	readBinElem rpReadBinProc;		/**< proc for reading the binary element*/
	writeBinElem rpWriteBinProc;	/**< proc for writing the binary element*/
	reflectProp *rpProps;		/**< reflect props for this entry, if it is not a base type*/
	double rpMinValue;			/**< minimum value, if a numeric*/
	double rpMaxValue;			/**< maximum value, if a numeric*/
	double rpDefaultValue;
	char rpDefaultText[1024];
	uint64 rpUiHint;			/**< hints on how to build and interact with UI*/
	const char** rpItemList;

	static void hSetEditable(reflectProp *prop)
	{
		while (prop != NULL && prop->rpName != NULL)
		{
			prop->rpUiHint |= uiHint_Edit;
			switch (prop->rpFlags)
			{
			case prop_properties:
				hSetEditable(prop->rpProps);
				break;
			case prop_prop_array:
				hSetEditable(prop->rpProps);
				break;
			case prop_prop_vector:
				hSetEditable(prop->rpProps);
				break;
			}
			prop++;
		}
	}
};
