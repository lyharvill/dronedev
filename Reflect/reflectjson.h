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
#include <iostream>     // istream, ostream
#include <vector>		// std::vector
#include <fstream>      // std::ifstream
#include <sstream>      // std::istringstream
#include <string>       // std::string
#include "reflectio.h"
#include "reflect.h"
#include "propvect.h"

#ifdef NEED_FACTORY
#include "reflectfactory.h"
#endif


typedef std::vector<reflectProp> propList;


class reflectJson
{

public:

	// filter out '_', allows reserved words.
	inline static const char* getPropName(reflectProp *prop)
	{
		if (prop->rpName != NULL)
		{
			if (prop->rpName[0] == '_')
			{
				return(prop->rpName + 1);
			}
		}
		return(prop->rpName);
	}

	inline static void writeKeyValue(std::ostream &output, void *base, reflectProp *prop, int32 indent, bool &noSep)
	{
		if (prop->rpWriteProc)
		{
			if (!noSep)
			{
				reflectIO::writeSeparator(output);
			}
			noSep = false;
			reflectIO::lineIndent(output, indent);
			void *elem = ((uint8*)base) + prop->rpOffset;
			output << "\"";
			output << getPropName(prop);
			output << "\": ";
			prop->rpWriteProc(output, elem);
		}
	}


	inline static void writeArray(std::ostream &output, void *base, reflectProp *prop, int32 indent, bool &noSep)
	{
		uint8 *elem = (uint8*)base + prop->rpOffset;
		size_t rank = prop->rpSize / prop->rpTypeSize;

		if (!noSep)
		{
			reflectIO::writeSeparator(output);
		}
		noSep = false;
		reflectIO::lineIndent(output, indent);
		output << "\"";
		output << getPropName(prop);
		output << "\": [";
		for (int32 i = 0; i < rank; i++)
		{
			prop->rpWriteProc(output, elem);
			if (i < rank - 1)
			{
				reflectIO::writeSeparator(output);
			}
			elem += prop->rpTypeSize;
		}
		output << "]";

	}


	inline static bool readArray(std::istream &input, void *base, reflectProp *prop)
	{
		bool result = true;
		uint8 *elem = (uint8*)base + prop->rpOffset;
		size_t rank = prop->rpSize / prop->rpTypeSize;

		for (int32 i = 0; i < rank; i++)
		{
			reflectIO::readSpace(input);
			result &= prop->rpReadProc(input, elem);
			reflectIO::readSeparator(input);
			elem += prop->rpTypeSize;
		}
		return(result);
	}

	inline static void writeVector(std::ostream &output, void *base, reflectProp *prop, int32 indent, bool &noSep)
	{
		uint8 *elem = (uint8*)base + prop->rpOffset;
		size_t rank = prop->rpSize / prop->rpTypeSize;

		if (!noSep)
		{
			reflectIO::writeSeparator(output);
		}
		noSep = false;
		reflectIO::lineIndent(output, indent);
		output << "\"";
		output << getPropName(prop);
		output << "\": [";
		for (int32 i = 0; i < rank; i++)
		{
			prop->rpWriteProc(output, elem);
			if (i < rank - 1)
			{
				reflectIO::writeSeparator(output);
			}
			elem += prop->rpTypeSize;
		}
		output << "]";

	}

	inline static void writePropertyVector(std::ostream &output, void *base, reflectProp *prop, int32 indent, bool &noSep)
	{
		_propVect *vect = (_propVect*)((uint8*)base + prop->rpOffset);
		if (vect->size())
		{
			if (!noSep)
			{
				reflectIO::writeSeparator(output);
			}
			noSep = false;
			reflectIO::lineIndent(output, indent);
			output << "\"";
			output << getPropName(prop);
			output << "\": [";
			bool aSep = true;

			reflectProp *elemProp = vect->getProperty(vect->getNth(0));
			for (int32 i = 0; i < vect->size(); i++)
			{
				void *elemBase = vect->getNth(i);
				if (elemBase)
				{
					writeProperties(output, NULL, elemBase, elemProp, indent + 1, aSep);
				}
			}
			reflectIO::lineIndent(output, indent);
			output << "]";
		}
	}

	inline static bool readVector(std::istream &input, void *base, reflectProp *prop)
	{
		bool result = true;
		uint8 *elem = (uint8*)base + prop->rpOffset;
		result = prop->rpReadProc(input, elem);
		return(result);
	}

	inline static void writeProperties(std::ostream &output, const char *propName, void *base, reflectProp *prop, int32 indent, bool &noSep)
	{
		if (prop)
		{
			if (!noSep)
			{
				reflectIO::writeSeparator(output);
			}
			
			reflectIO::lineIndent(output, indent);
			if (propName)
			{
				output << "\"";
				output << propName;
				output << "\": ";
			}
			output << "{";
			bool sep = true;
			writeKeyValues(output, base, prop, indent + 1, sep);
			writeContents(output, base, prop, indent + 1, sep);
			reflectIO::lineIndent(output, indent);
			output << "}";
			noSep = false;
		}
	}




	inline static void writePropArray(std::ostream &output, void *base, reflectProp *prop, int32 indent, bool &noSep)
	{
		uint8* elem  = (uint8*)base + prop->rpOffset;
		int32 arraySize = (int32)(prop->rpSize / prop->rpTypeSize);
		int32 elemSize = (int32)(prop->rpSize / arraySize);

		noSep = false;
		if (arraySize)
		{

			if (!noSep)
			{
				reflectIO::writeSeparator(output);
			}
			noSep = false;
			reflectIO::lineIndent(output, indent);
			output << "\"";
			output << getPropName(prop);
			output << "\": [";
			bool aSep = true;
			for (int32 i = 0; i < arraySize; i++)
			{
				writeProperties(output, NULL, elem, prop->rpProps, indent + 1, aSep);
				aSep = false;
				elem += elemSize;
			}
			reflectIO::lineIndent(output, indent);
			output << "]";
		}
	}

#ifdef NEED_FACTORY
	inline static void getNodeName(const char *className, int32 index, std::string &extendedName, void *base, reflectProp *prop)
	{
		if (prop && base)
		{
			node *aNode = (node*)base;
			if (aNode->IsA(node::classID()))
			{
				extendedName += className;
				extendedName += ".";
				extendedName += aNode->name;
			}
		}
		else
		{
			extendedName += className;
			extendedName += ".";
			extendedName += std::to_string(index);
		}
	}



	inline static void writeRefList(std::ostream &output, void *base, reflectProp *prop, int32 indent, bool &noSep)
	{
		refList &vect = *(refList*)((uint8*)base + prop->rpOffset);
		if (vect.size())
		{
			if (!noSep)
			{
				reflectIO::writeSeparator(output);
			}
			noSep = false;
			reflectIO::lineIndent(output, indent);
			output << "\"";
			output << getPropName(prop);
			output << "\": [";
			bool aSep = true;
			for (int32 i = 0; i < vect.size(); i++)
			{
				iotaRef &ref = vect[i];
				if (ref.ptr)
				{
					writeProperties(output, ref->vClassName(), ref.ptr, ref->getReflect(), indent+1, aSep);
				}
			}
			reflectIO::lineIndent(output, indent);
			output << "]";
		}
	}


	inline static void writeNodeList(std::ostream &output, void *base, reflectProp *prop, int32 indent, bool &noSep)
	{
		node *aNode = (node*)((uint8*)base + prop->rpOffset);
		if (aNode->size())
		{
			if (!noSep)
			{
				reflectIO::writeSeparator(output);
			}
			noSep = false;
			reflectIO::lineIndent(output, indent);
			output << "\"";
			output << getPropName(prop);
			output << "\": {";
			bool aSep = true;
			for (int32 i = 0; i < aNode->size(); i++)
			{
				node::nodeRef ref;
				aNode->nthRef(i, ref);
				if (ref.ptr)
				{
					std::string extendedName;
					reflectProp *contentProp = ref->getReflect();
					getNodeName(ref->vClassName(), i, extendedName, ref.ptr, contentProp);
					writeProperties(output, extendedName.c_str(), ref.ptr, contentProp, indent + 1, aSep);
					aSep = false;
				}
			}
			reflectIO::lineIndent(output, indent);
			output << "}";
		}
	}

#endif

	inline static void writeKeyValues(std::ostream &output, void *base, reflectProp *prop, int32 indent, bool &noSep)
	{
		while (prop && prop->rpName && getPropName(prop)[0] != '\0')
		{
			if (prop->rpFlags == prop_keyValue)
			{
				writeKeyValue(output, base, prop, indent, noSep);
			}
			prop++;
		}
	}

	inline static void writeContents(std::ostream &output, void *base, reflectProp *prop, int32 indent, bool &noSep)
	{
		while (prop && prop->rpName && getPropName(prop)[0] != '\0')
		{
			switch (prop->rpFlags)
			{
			case prop_array:
				writeArray(output, base, prop, indent, noSep);
				break;
			case prop_properties:
				writeProperties(output, getPropName(prop), (uint8*)base + prop->rpOffset, prop->rpProps, indent, noSep);
				break;
			case prop_vector:
				writeVector(output, base, prop, indent, noSep);
				break;
			
#ifdef NEED_FACTORY
			case prop_reference:
				{
					reflectRef<iota>& ref = *(reflectRef<iota>*)((uint8*)base + prop->rpOffset);
					if (ref.ptr)
					{
						writeProperties(output, getPropName(prop), ref.ptr, ref->getReflect(), indent, noSep);
					}
				}
				break;

			case prop_ref_list:
			{
				writeRefList(output, base, prop, indent, noSep);
			}
			break;
			case prop_node_list:
			{
				writeNodeList(output, base, prop, indent, noSep);
			}
			break;
#endif

			case prop_prop_array:
			{
				writePropArray(output, base, prop, indent, noSep);
			}
				break;

			

			case prop_prop_vector:
			{
				writePropertyVector(output, base, prop, indent, noSep);
			}
			break;
			}
			prop++;
		}
	}

	

	static bool readStructBody(std::istream &input, std::string &structure)
	{
		int32 nest = 1;
		if (reflectIO::readSpace(input))
		{
			char n = input.peek();
			
			if (n == '{')
			{
				input.get();
				n = input.peek();
				while (!reflectIO::eof(input) && nest > 0)
				{
					n = input.peek();
					switch (n)
					{
					case '{':
					{
						nest++;
					}
					break;
					case '}':
					{
						nest--;

					}
					break;
					}
					n = input.get();
					if (nest != 0)
					{
						structure.push_back(n);
					}
				}
			}
		}
		reflectIO::readSeparator(input);
		reflectIO::readEndOfLine(input);
		return(!reflectIO::eof(input));
	}

	static bool readArrayBody(std::istream &input, std::string &structure)
	{
		if (reflectIO::readSpace(input))
		{
			char n = input.peek();

			if (n == '[')
			{
				input.get();
				int32 nest = 1;

				n = input.peek();
				while (!reflectIO::eof(input) && nest > 0)
				{
					n = input.peek();
					switch (n)
					{
					case '[':
					{
						nest++;
					}
					break;
					case ']':
					{
						nest--;

					}
					break;
					}
					n = input.get();
					if (nest != 0)
					{
						structure.push_back(n);
					}
				}
			}
		}
		reflectIO::readSeparator(input);
		reflectIO::readEndOfLine(input);
		return(!reflectIO::eof(input));
	}

	static bool readValueBody(std::istream &input, std::string &value)
	{
		if (reflectIO::readSpace(input))
		{
			char n = input.peek();
			bool needsQuote = false;

			if (n == '"')
			{
				input.get();
				needsQuote = true;
			}
				
			if (needsQuote)
			{
				std::getline(input, value, '\"');
				reflectIO::readSeparator(input);
			}
			else
			{
				std::getline(input, value, ',');
			}
		}
		return(value.size() != 0);
	}

	static bool readAnyBody(std::istream &input, std::string &value)
	{
		if (reflectIO::readSpace(input))
		{
			char n = input.peek();
			switch (n)
			{
			case '[':
				readArrayBody(input, value);
				break;
			case '{':
				readStructBody(input, value);
				break;
			case '(':
				readListBody(input, value);
				break;
			default:
				readValueBody(input, value);
			}
		}
		return(!reflectIO::eof(input));
	}

	static bool readListBody(std::istream &input, std::string &structure)
	{
		if (reflectIO::readSpace(input))
		{
			char n = input.peek();

			if (n == '(')
			{
				input.get();
				int32 nest = 1;

				n = input.peek();
				while (!reflectIO::eof(input) && nest > 0)
				{
					n = input.peek();
					switch (n)
					{
					case '(':
					{
						nest++;
					}
					break;
					case ')':
					{
						nest--;

					}
					break;
					}
					n = input.get();
					if (nest != 0)
					{
						structure.push_back(n);
					}
				}
			}
		}
		reflectIO::readSeparator(input);
		return(structure.size() > 0);
	}


	static int32 findNextProp(std::string &key, int32 &nextProp, reflectProp *prop)
	{
		int32 propId = -1;
		if (prop != NULL)
		{
			bool found = (key == getPropName(&prop[nextProp]));
			if (!found)
			{
				nextProp = 0;
				while (getPropName(&prop[nextProp]) && !found)
				{
					found = (key == getPropName(&prop[nextProp]));
					if (!found)
					{
						nextProp++;
					}
				}
			}
			if (found)
			{
				propId = nextProp;

			}
			if (getPropName(&prop[nextProp]) == NULL)
			{
				nextProp = 0;
			}
			else
			{
				nextProp++;
			}
		}
		return(propId);

	}

	static bool readTag(std::istream &input, std::string &tagName)
	{
		tagName.clear();

		bool result = false;
		char n = input.peek();
		while (n != EOF && n != '\"')
		{
			input.get();
			n = input.peek();
		}

		if (n == '"')
		{
			input.get();
			std::string nameAttrs;
			std::getline(input, nameAttrs, '\"');
			if (nameAttrs.size())
			{
				if (!reflectIO::eof(input))
				{
					std::istringstream iss(nameAttrs);
					reflectIO::readSpace(iss);
					char tn = iss.peek();
					while (!isspace(tn) && tn != '.' && tn != EOF)
					{
						tn = iss.get();
						if (tn != '\"')
							tagName.push_back(tn);

						tn = iss.peek();
					}
					result = tagName.size() > 0;
				}
			}
			if (result)
			{
				reflectIO::readSpace(input);
				char colon = input.get();
				result = colon = ':';
			}
		}
		return(result);
	}

#ifdef NEED_FACTORY
	static bool readRefList(std::istream &ifs, reflectProp *prop, uint8 *base)
	{
		prop;
		bool result = false;
		refList &vect = *(refList*)base;

		while (!reflectIO::eof(ifs))
		{
			reflectRef<iota> ref;
			std::string tagName;

			if (readTag(ifs, tagName))
			{
				if (reflectFactory::build(tagName, (reflectRef<iota>&)ref))
				{
					std::string content;
					readStructBody(ifs, content);
					std::istringstream iss(content);
					readContentTags(iss, ref->getReflect(), (uint8*)ref.ptr);
				}
				if (ref.ptr)
				{
					vect.push_back(ref);
				}
			}
			
		}
		result = vect.size() > 0;
		return(result);
	}
#endif

	static bool readPropertyVector(std::istream &ifs, reflectProp *prop, uint8 *base)
	{
		_propVect *vect = (_propVect*)((uint8*)base + prop->rpOffset);
		bool result = false;

		while (!reflectIO::eof(ifs))
		{
			void *elem = vect->newElem();
			reflectProp *elemProp = vect->getProperty(elem);
			std::string content;
			readStructBody(ifs, content);
			std::istringstream iss(content);
			readContentTags(iss, elemProp, (uint8*)elem);
			vect->pushProp(elem);
			vect->safeElemDelete(elem);
			result = true;
		}
		return(result);
	}

	static bool readPropArray(std::istream &ifs, reflectProp *prop, uint8 *base)
	{
		bool result = false;
		uint8 *elem = base;
		int32 count = (int32)(prop->rpSize / prop->rpTypeSize);
		int32 elemSize = (int32)prop->rpSize / count;

		// set an elem property using the array property 
		reflectProp elemProp = *prop;
		elemProp.rpFlags = prop_properties;
		elemProp.rpSize = prop->rpTypeSize;

		while (!reflectIO::eof(ifs) && count)
		{
			std::string content;
			readStructBody(ifs, content);
			std::istringstream iss(content);
			readContentTags(iss, prop->rpProps, elem);
			elem += elemSize;
			count--;
		}
		return(result);
	}

#ifdef NEED_FACTORY
	static bool readNodeList(std::istream &ifs, reflectProp *prop, uint8 *base)
	{
		prop;
		bool result = false;
		node *aNode = (node*)base;

		while (!reflectIO::eof(ifs))
		{
			reflectRef<iota> ref;
			std::string tagName;

			if (readTag(ifs, tagName))
			{
				if (reflectFactory::build(tagName, (reflectRef<iota>&)ref))
				{
					std::string content;
					readStructBody(ifs, content);
					std::istringstream iss(content);
					readContentTags(iss, ref->getReflect(), (uint8*)ref.ptr);
				}
				if (ref.ptr)
				{
					if (ref->IsA(node::classID()))
					{
						node::nodeRef noderef = (node*)ref.ptr;
						aNode->push_back(noderef);
					}
				}
			}
		}
		result = aNode->size() > 0;
		return(result);
	}
#endif

	static bool readContentTag(std::istream &ifs, reflectProp *prop, uint8 *base)
	{
		bool result = false;
		try
		{
			std::string tagName;
			std::string attrs;
			int32 nextProp = 0;

			if (readTag(ifs, tagName))
			{
				int32 propId = findNextProp(tagName, nextProp, prop);
				if (propId >= 0)
				{
					reflectProp *contentProp = prop + propId;
					uint8 *elem = base + contentProp->rpOffset;
					
					{
						switch (contentProp->rpFlags)
						{
						case prop_array:
						{
							std::string content;
							readArrayBody(ifs, content);
							std::istringstream iss(content);
							result = readArray(iss, base, contentProp);
						}
						break;

						case prop_vector:
						{
							std::string content;
							readArrayBody(ifs, content);
							std::istringstream iss(content);
							result = readVector(iss, base, contentProp);
						}
						break;
						case prop_properties:
						{
							std::string content;
							readStructBody(ifs, content);
							std::istringstream iss(content);
							readContentTags(iss, contentProp->rpProps, elem);
							result = true;
						}
						break;
						case prop_keyValue:
						{
							std::string value;
							if (readValueBody(ifs, value))
							{
								std::istringstream iss(value);
								contentProp->rpReadProc(iss, elem);
							}
						}
						break;
#ifdef NEED_FACTORY
						case prop_reference:
						{
							std::string content;
							readStructBody(ifs, content);
							std::istringstream iss(content);
							reflectRef<iota>* ref = (reflectRef<iota>*)elem;
							ref->build();
							if (ref->ptr)
							{
								readContentTags(iss, ref->ptr->getReflect(), (uint8*)ref->ptr);
							}
						}
						break;
						case prop_ref_list:
						{
							std::string content;
							readArrayBody(ifs, content);
							std::istringstream iss(content);
							readRefList(iss, contentProp->rpProps, elem);
						}
						break;


						case prop_node_list:
						{
							std::string content;
							readStructBody(ifs, content);
							std::istringstream iss(content);
							readNodeList(iss, contentProp->rpProps, elem);
						}
						break;
#endif

						case prop_prop_array:
						{
							std::string content;
							readArrayBody(ifs, content);
							std::istringstream iss(content);
							readPropArray(iss, contentProp, elem);
							result = true;
						}
						break;

						case prop_prop_vector:
						{
							std::string content;
							readArrayBody(ifs, content);
							std::istringstream iss(content);
							readPropertyVector(iss, contentProp, base);
						}
						break;
						}
					}
				}
				else
				{
					std::string value;
					readAnyBody(ifs, value);
					value = "";
				}
			}
		}
		catch (...)
		{

		}
		return(result);
	}



	static bool readContentTags(std::istream &ifs, reflectProp *prop, uint8 *base)
	{
		bool result = true;
		try
		{
			while (!reflectIO::eof(ifs))
			{
				result = readContentTag(ifs, prop, base);
				char n = ifs.peek();
				n = 0;
			}
		}
		catch (...)
		{

		}
		return(result);
	}

#ifdef NEED_FACTORY
	static void readIota(std::istream &ifs, reflectRef<iota>& ref)
	{
		try
		{
			std::string tagName;
			bool reading = true;
			while (reading)
			{
				readTag(ifs, tagName);
				if (reflectFactory::build(tagName, (reflectRef<iota>&)ref))
				{
					std::string content;
					reading = readStructBody(ifs, content);
					std::istringstream iss(content);
					readContentTags(iss, ref->getReflect(), (uint8*)ref.ptr);
					reading = false;
				}
			}
		}
		catch (...)
		{

		}
	}

	public:

	static void writeIota(std::string path, reflectRef<iota>& ref)
	{
		if (ref.ptr)
		{
			bool noSep = true;
			std::ofstream ofs;
			ofs.open(path);
			try
			{
				ofs << "{";
				reflectJson::writeProperties(ofs, ref->vClassName(), ref.ptr, ref->getReflect(), 1, noSep);
				ofs << "\n}\n";
			}
			catch (...)
			{

			}
			ofs.close();
		}
	}

	static void readIota(std::string &path, reflectRef<iota>& ref)
	{
		std::ifstream ifs;
		ifs.open(path);
		readIota(ifs, ref);
		ifs.close();
		if (ref.ptr)
		{
			ref->rootBind();
		}
	}
#endif

	static void readProperties(std::istream &ifs, reflectProp *prop, uint8 *base)
	{
		std::string content;
		readStructBody(ifs, content);
		std::istringstream iss(content);
		readContentTags(iss, prop, base);
	}

	static void readProperties(std::string &path, reflectProp *prop, uint8 *base)
	{
		std::ifstream ifs;
		ifs.open(path);
		readProperties(ifs, prop, base);
	}

	static void writeProperties(std::string &path, void *base, reflectProp *prop)
	{
		std::ofstream ofs;
		ofs.open(path);
		try
		{
			ofs << "{";
			bool sep = true;
			writeKeyValues(ofs, base, prop, 1, sep);
			writeContents(ofs, base, prop, 1, sep);
			reflectIO::lineIndent(ofs, 0);
			ofs << "}";
		}
		catch (...)
		{

		}
		ofs.close();

		
	}

	static void writeProperties(std::ostream &os, void *base, reflectProp *prop)
	{
		try
		{
			os << "{";
			bool sep = true;
			writeKeyValues(os, base, prop, 1, sep);
			writeContents(os, base, prop, 1, sep);
			reflectIO::lineIndent(os, 0);
			os << "}";
		}
		catch (...)
		{

		}
	}
};
