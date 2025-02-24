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

#include "reflectref.h"
#include "stringr.h"
#include "lockio.h"
#include <vector>


/**
* Base class for named elements in an n-tree data structure with reflection.
*/
class linkr : public iota
{
public:

	typedef reflectRef<linkr> linkRef;	/**< reference to a linkr. Provides locking and reference counting. >*/
	typedef std::vector<linkRef> linkRefVect; /**< list of linkRefs*/

	stringr name;	/**< name for this linkr node. >*/
	linkRefVect children;	/**< list of children this linkr owns >*/

	/**
	* Base Constructor for an n-tree data structure instance of linkr
	*/
	linkr(){};

	/**
	* Shallow copy constructor for an n-tree data structure instance of linkr
	* @param input instance of a linkr for constructing this instance.
	*/
	linkr(const linkr& input){ name = input.name;  children = input.children;}

	/**
	* Virtual destructor for linkr.
	*/
	virtual ~linkr(){};

	/**
	* Fetchs the size for a linkr.
	* @return the number of children owned by this linkr.
	*/
	__forceinline size_t size(void)
	{
		return(children.size());
	}
	
	/**
	* Fetches a reference to the nth child.
	* @return the linkRef for the child at index n.
	*/
	__forceinline linkRef& nthChild(const size_t n)
	{
		return(children[n]);
	}

	/**
	* Fetches a reference to the nth child.
	* @return the linkRef for the child at index n.
	*/
	__forceinline linkRef& operator [](const size_t index)
	{
		return(nthChild(index));
	}

	/**
	* Finds a child reference by name
	* @param key, name to use as a key for the search.
	* @param ref, The found linkRef returned by reference.
	* @return true if a reference was found.
	*/
	bool findByName(std::string &key, linkRef &ref)
	{
		bool result = key == name;
		if (result)
		{
			ref = this;
		}
		else
		{
			for (int32 i = 0; i < children.size() && !result; i++)
			{
				if (children[i].ptr)
				{
					result = children[i]->findByName(key, ref);
				}
			}
		}
		return(result);
	}

	/**
	* Finds a child reference by name and class tag.
	* @param key, name to use as a key for the search.
	* @param tag, The class tag used to filter the search.
	* @param ref, The found linkRef returned by reference.
	* @return true if a reference was found.
	*/
	bool findByNameAndTag(std::string &key, linkRef &ref, uint64 tag)
	{
		bool result = key == name && IsA(tag);
		if (result)
		{
			ref = this;
		}
		else
		{
			for (int32 i = 0; i < children.size() && !result; i++)
			{
				if (children[i].ptr)
				{
					result = children[i]->findByNameAndTag(key, ref, tag);
				}
			}
		}
		return(result);
	}

	/**
	* Returns a list of children that match a class tag.
	* @param tag, The class tag used to filter the search.
	* @param list, The found list of linkRefs.
	*/
	void getTagList(uint64 tag, linkRefVect &list)
	{
		list.clear();
		for (int32 i = 0; i < children.size(); i++)
		{
			if (children[i].ptr)
			{
				if (children[i]->IsA(tag))
				{
					list.push_back(children[i]);
				}
			}
		}
	}

	beginRefProperties(linkr, iota)
		reflectAttribute(stringr, name)
		reflectLinkRefList(linkRefVect, children)
	endProperties

};