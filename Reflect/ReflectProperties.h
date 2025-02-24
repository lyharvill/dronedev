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
// 

#pragma once

#include "JuceReflect.h"
#include "reflecthelper.h"
#include <vector>		// std::vector
#include "reflectfactory.h"
#include <sstream>      // std::istringstream
#include <string>       // std::string
#include "reflectio.h"
#include "reflect.h"
#include "propvect.h"
#include "stringr.h"
#include "SubPropertyPanel.h"
#include "LabProperty.h"
#include "RgbProperty.h"

inline Colour getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour uiColour, Colour fallback = Colour(0xff4d4d4d))
{
	if (auto* v4 = dynamic_cast<LookAndFeel_V4*> (&LookAndFeel::getDefaultLookAndFeel()))
		return v4->getCurrentColourScheme().getUIColour(uiColour);

	return fallback;
}


class ReflectProperties : public Component
{
public:

	ReflectProperties()
	{
		indentLevel = 0;
		propertyCount = 0;
		setSize(600, 400);
		setOpaque(true);
		Colour textColor = look.findColour(TextPropertyComponent::backgroundColourId);
		textColor = textColor.brighter(0.1f);
		look.setColour(TextPropertyComponent::backgroundColourId, textColor);
		setLookAndFeel(&look);
	}

	void setRef(iotaRef& ref)
	{
		propertyPanel.clear();
		propRef = ref;
		Array<PropertyComponent*> comps;
		addProperties(comps, propRef->vClassName(), propRef.ptr, propRef->getReflect());
		propertyPanel.addProperties(comps);
		addAndMakeVisible(propertyPanel);
	}

	ReflectProperties(iotaRef& ref)
	{
		indentLevel = 0;
		propertyCount = 0;
		propRef = ref;
		setSize(600, 400);
		setOpaque(true);
		Array<PropertyComponent*> comps;
		addProperties(comps, propRef->vClassName(), propRef.ptr, propRef->getReflect());
		propertyPanel.addProperties(comps);
		addAndMakeVisible(propertyPanel);
		Colour textColor = look.findColour(TextPropertyComponent::backgroundColourId);
		textColor = textColor.brighter(0.1f);
		look.setColour(TextPropertyComponent::backgroundColourId, textColor);
		setLookAndFeel(&look);
	}

	~ReflectProperties()
	{
		setLookAndFeel(NULL);
	}

	void paint(Graphics& g) override
	{
		g.fillAll(getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
			Colour::greyLevel(0.8f)));
	}

	void resized() override
	{
		propertyPanel.setBounds(getLocalBounds().reduced(4));
		propertyPanel.resized();
	}

	static int32 calcComponentListHeight(Array<PropertyComponent*>& propertyList)
	{
		propertyList;
		return(200);
	}

	void paintComponentAndChildren(Graphics& g) override {

		resized();
		Component::paintComponentAndChildren(g);
	}

	void addKeyValue(Array<PropertyComponent*>& propertyList, void *base, reflectProp *prop, const char *name)
	{
		void *elem = base;

		if (prop->rpUiHint & uiHint_NoShow)
		{
			return;
		}

		if (prop->rpItemList != NULL && (strncmp(prop->rpType, uint8r::className(), sizeof(uint64)) == 0))
		{
			ComboBoxUint8*comboComp = new ComboBoxUint8(prop, (uint8*)elem, name, indentLevel);
			RValueUint8* rvalue = new RValueUint8(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			comboComp->getValueObject().referTo(aValue);
			propertyList.add(comboComp);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				comboComp->setReadOnly(true);
			}
			comboComp->setValue((double)*(uint8*)elem);
			comboComp->refresh();
			propertyCount++;
		}
		else if (strncmp(prop->rpType, boolr::className(), sizeof(uint64)) == 0)
		{
			CheckBoxBool* checkComp = new CheckBoxBool(prop, (uint8*)elem, name, indentLevel);
			RValueBool* rvalue = new RValueBool(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			checkComp->getValueObject().referTo(aValue);
			propertyList.add(checkComp);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				checkComp->setReadOnly(true);
			}
			checkComp->setValue((double)*(boolr*)elem);
			checkComp->refresh();
			propertyCount++;
		}
		else if (strncmp(prop->rpType, boolfunc::className(), sizeof(uint64)) == 0)
		{
			if (((boolfunc*)elem)->toggle)
			{
				CheckBoxBool* checkComp = new CheckBoxBool(prop, (uint8*)elem, name, indentLevel);
				checkComp->setClickFunction(((boolfunc*)elem)->func, ((boolfunc*)elem)->data);
				RValueBool* rvalue = new RValueBool(prop, (uint8*)elem, name);
				Value aValue(rvalue);
				checkComp->getValueObject().referTo(aValue);
				propertyList.add(checkComp);
				if ((prop->rpUiHint & uiHint_Edit) == 0)
				{
					checkComp->setReadOnly(true);
				}
				checkComp->setValue((double)*(boolr*)elem);
				checkComp->refresh();
				propertyCount++;
			}
			else
			{
				ButtonFunc* checkComp = new ButtonFunc(prop, (uint8*)elem, name, indentLevel);
				checkComp->setClickFunction(((boolfunc*)elem)->func, ((boolfunc*)elem)->data);
				RValueBool* rvalue = new RValueBool(prop, (uint8*)elem, name);
				Value aValue(rvalue);
				checkComp->getValueObject().referTo(aValue);
				propertyList.add(checkComp);
				if ((prop->rpUiHint & uiHint_Edit) == 0)
				{
					checkComp->setReadOnly(true);
				}
				checkComp->setValue((double)*(boolr*)elem);
				checkComp->refresh();
				propertyCount++;
			}
		}
		else if (strncmp(prop->rpType, uint8r::className(), sizeof(uint64)) == 0)
		{
			SliderUint8 *slider = new SliderUint8(prop, (uint8*)elem, name, indentLevel);
			RValueUint8 *rvalue = new RValueUint8(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			slider->getValueObject().referTo(aValue);
			propertyList.add(slider);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				slider->setReadOnly(true);
			}
			propertyCount++;
		}
		else if (strncmp(prop->rpType, int16r::className(), sizeof(uint64)) == 0)
		{
			SliderInt16 *slider = new SliderInt16(prop, (uint8*)elem, name, indentLevel);
			RValueInt16 *rvalue = new RValueInt16(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			slider->getValueObject().referTo(aValue);
			propertyList.add(slider);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				slider->setReadOnly(true);
			}
			propertyCount++;
		}
		else if (strncmp(prop->rpType, uint16r::className(), sizeof(uint64)) == 0)
		{
			SliderUint16 *slider = new SliderUint16(prop, (uint8*)elem, name, indentLevel);
			RValueUint16 *rvalue = new RValueUint16(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			slider->getValueObject().referTo(aValue);
			propertyList.add(slider);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				slider->setReadOnly(true);
			}
			propertyCount++;
		}
		else if (strncmp(prop->rpType, int32r::className(), sizeof(uint64)) == 0)
		{
			SliderInt32 *slider = new SliderInt32(prop, (uint8*)elem, name, indentLevel);
			RValueInt32 *rvalue = new RValueInt32(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			slider->getValueObject().referTo(aValue);
			propertyList.add(slider);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				slider->setReadOnly(true);
			}
			propertyCount++;
		}
		else if (strncmp(prop->rpType, uint32r::className(), sizeof(uint64)) == 0)
		{
			SliderUint32 *slider = new SliderUint32(prop, (uint8*)elem, name, indentLevel);
			RValueUint32 *rvalue = new RValueUint32(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			slider->getValueObject().referTo(aValue);
			propertyList.add(slider);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				slider->setReadOnly(true);
			}
			propertyCount++;
		}
		else if (strncmp(prop->rpType, int64r::className(), sizeof(uint64)) == 0)
		{
			SliderInt64 *slider = new SliderInt64(prop, (uint8*)elem, name, indentLevel);
			RValueInt64 *rvalue = new RValueInt64(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			slider->getValueObject().referTo(aValue);
			propertyList.add(slider);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				slider->setReadOnly(true);
			}
			propertyCount++;
		}
		else if (strncmp(prop->rpType, uint64r::className(), sizeof(uint64)) == 0)
		{
			SliderUint64 *slider = new SliderUint64(prop, (uint8*)elem, name, indentLevel);
			RValueUint64 *rvalue = new RValueUint64(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			slider->getValueObject().referTo(aValue);
			propertyList.add(slider);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				slider->setReadOnly(true);
			}
			propertyCount++;
		}
		else if (strncmp(prop->rpType, real32r::className(), sizeof(uint64)) == 0)
		{
			SliderReal32 *slider = new SliderReal32(prop, (uint8*)elem, name, indentLevel);
			RValueReal32 *rvalue = new RValueReal32(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			slider->getValueObject().referTo(aValue);
			propertyList.add(slider);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				slider->setReadOnly(true);
			}
			propertyCount++;
		}
		else if (strncmp(prop->rpType, real64r::className(), sizeof(uint64)) == 0)
		{
			SliderReal64 *slider = new SliderReal64(prop, (uint8*)elem, name, indentLevel);
			RValueReal64 *rvalue = new RValueReal64(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			slider->getValueObject().referTo(aValue);
			propertyList.add(slider);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				slider->setReadOnly(true);
			}
			propertyCount++;
		}
		else if (strncmp(prop->rpType, stringr::className(), sizeof(uint64)) == 0)
		{
			TextEditStringR *textEdit = new TextEditStringR(prop, (uint8*)elem, name, indentLevel);
			RValueString *rvalue = new RValueString(prop, (uint8*)elem, name);
			Value aValue(rvalue);
			textEdit->getValue().referTo(aValue);
			propertyList.add(textEdit);
			if ((prop->rpUiHint & uiHint_Edit) == 0)
			{
				textEdit->setReadOnly(true);
			}
			propertyCount++;
		}
	}

	static bool hFindComponent(Component& aParent, std::string compName, Component*& propComp)
	{
		bool result = false;
		propComp = NULL;
		for (auto* c : aParent.getChildren())
		{
			if (auto b = dynamic_cast<Component*> (c))
			{
				const juce::String &childName = b->getName();
				if (childName == compName)
				{
					propComp = b;
					result = true;
					break;
				}
				else
				{
					result = hFindComponent(*b, compName, propComp);
					if (result)
					{
						break;
					}
				}
			}
		}
		return(result);
	}

	bool findComponent(std::string compName, Component*& propComp)
	{
		bool result = hFindComponent(propertyPanel, compName, propComp);
		return(result);
	}

	void addArray(Array<PropertyComponent*>& propertyList, void *base, reflectProp *prop)
	{
		uint8 *elem = (uint8*)base + prop->rpOffset;
		size_t rank = prop->rpSize / prop->rpTypeSize;

		if (rank)
		{
			Array<PropertyComponent*> comps;
			indentLevel++;
			for (int32 i = 0; i < rank; i++)
			{
				std::stringstream  indexName;
				indexName << i;
				std::string key = indexName.str();
				addKeyValue(comps, elem, prop, key.data());
				elem += prop->rpTypeSize;
			}
			SubPropertyPanel *subPanel = new SubPropertyPanel(indentLevel, calcComponentListHeight(comps));
			bool open = comps.size() <= 5;
			subPanel->addSection(prop->rpName, comps, open);
			propertyList.add(subPanel);
			indentLevel--;
		}
	}


	void addPropertyVector(Array<PropertyComponent*>& propertyList, void *base, reflectProp *prop)
	{
		_propVect *vect = (_propVect*)((uint8*)base + prop->rpOffset);
		if (vect->size())
		{
			Array<PropertyComponent*> comps;
			indentLevel++;
			for (int32 i = 0; i < vect->size(); i++)
			{
				void *elemBase = vect->getNth(i);
				if (elemBase)
				{
					std::stringstream  indexName;
					indexName << i;
					std::string key = indexName.str();

					addProperties(comps, key.data(), elemBase, prop->rpProps);
				}
			}
			
			if (comps.size())
			{
				SubPropertyPanel *subPanel = new SubPropertyPanel(indentLevel, calcComponentListHeight(comps));
				bool open = comps.size() <= 2;
				subPanel->addSection(prop->rpName, comps, open);
				propertyList.add(subPanel);
			}
			indentLevel--;
		}
	}

	void addProperties(Array<PropertyComponent*>& propertyList, const char *propName, void *base, reflectProp *prop)
	{
		if (prop)
		{
			Array<PropertyComponent*> comps;
			indentLevel++;

			addContents(comps, base, prop);
			
			if (comps.size())
			{
				SubPropertyPanel *subPanel = new SubPropertyPanel(indentLevel, calcComponentListHeight(comps));
				bool open = comps.size() < 5 || indentLevel <= 2;
				subPanel->addSection(propName, comps, open);
				propertyList.add(subPanel);
			}
			indentLevel--;
		}
	}


	void addPropArray(Array<PropertyComponent*>& propertyList, void *base, reflectProp *prop)
	{
		uint8* elem = (uint8*)base + prop->rpOffset;
		int32 arraySize = (int32)(prop->rpSize / prop->rpTypeSize);
		int32 elemSize = (int32)(prop->rpSize / arraySize);

		if (arraySize)
		{
			Array<PropertyComponent*> comps;
			indentLevel++;
			for (int32 i = 0; i < arraySize; i++)
			{
				std::stringstream  indexName;
				indexName << i;
				std::string key = indexName.str();

				addProperties(comps, key.data(), elem, prop->rpProps);
				elem += elemSize;
			}
			SubPropertyPanel *subPanel = new SubPropertyPanel(indentLevel, calcComponentListHeight(comps));
			bool open = comps.size() <= 2;
			subPanel->addSection(prop->rpName, comps, open);
			propertyList.add(subPanel);
			indentLevel--;
		}
	}


	void addRefList(Array<PropertyComponent*>& propertyList, void *base, reflectProp *prop)
	{
		refList &vect = *(refList*)((uint8*)base + prop->rpOffset);
		if (vect.size())
		{
			Array<PropertyComponent*> comps;
			indentLevel++;
			for (int32 i = 0; i < vect.size(); i++)
			{
				iotaRef &ref = vect[i];
				if (ref.ptr != NULL)
				{
					addProperties(propertyList, ref->vClassName(), ref.ptr, ref->getReflect());
				}
			}
			SubPropertyPanel *subPanel = new SubPropertyPanel(indentLevel, calcComponentListHeight(comps));
			bool open = comps.size() <= 2;
			subPanel->addSection(prop->rpName, comps, open);
			propertyList.add(subPanel);
			indentLevel--;
		}
	}


	void addContents(Array<PropertyComponent*>& propertyList, void *base, reflectProp *prop)
	{
		while (prop && prop->rpName && prop->rpName[0] != '\0')
		{
			if (prop->rpUiHint & uiHint_NoShow)
			{
				// nothing
			}
			else if (strncmp(prop->rpType, labColor::typeName(), sizeof(uint64)) == 0)
			{
				uint8 *elem = (uint8*)base + prop->rpOffset;
				LabProperty *labProperty = new LabProperty(prop, (uint8*)elem, prop->rpName, indentLevel);
				propertyList.add(labProperty);
				labProperty->setReadOnly((prop->rpUiHint & uiHint_Edit) == 0);
			}
			else if (strncmp(prop->rpType, rgbColor::typeName(), sizeof(uint64)) == 0)
			{
				uint8 *elem = (uint8*)base + prop->rpOffset;
				RgbProperty *rgbProperty = new RgbProperty(prop, (uint8*)elem, prop->rpName, indentLevel);
				propertyList.add(rgbProperty);
				rgbProperty->setReadOnly((prop->rpUiHint & uiHint_Edit) == 0);
			}
			else
			{
				switch (prop->rpFlags)
				{
				case prop_keyValue:
					addKeyValue(propertyList, ((uint8*)base) + prop->rpOffset, prop, prop->rpName);
					break;

				case prop_array:
					addArray(propertyList, base, prop);
					break;
				case prop_properties:
					addProperties(propertyList, prop->rpName, (uint8*)base + prop->rpOffset, prop->rpProps);
					break;
				case prop_vector:
					addArray(propertyList, base, prop);
					break;

				case prop_reference:
				{
					iotaRef& ref = *(iotaRef*)((uint8*)base + prop->rpOffset);
					if (ref.ptr != NULL)
					{
						addProperties(propertyList, prop->rpName, ref.ptr, ref->getReflect());
					}
				}
				break;

				case prop_prop_array:
				{
					addPropArray(propertyList, base, prop);
				}
				break;

				case prop_ref_list:
				{
					addRefList(propertyList, base, prop);
				}
				break;
				case prop_node_list:
				{
					//addNodeList(propertyList, base, prop);
				}
				break;

				case prop_prop_vector:
				{
					addPropertyVector(propertyList, base, prop);
				}
				break;
				}
			}
			prop++;
		}
	}
private:
	LookAndFeel_V4 look;
	PropertyPanel propertyPanel;
	iotaRef propRef;
	int32 indentLevel;
	int32 propertyCount;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReflectProperties)
};