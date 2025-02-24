#pragma once

#include"reflect.h"
#include"stringr.h"

class DefaultSettings
{
public:

	static void setDefaults(void *base, reflectProp *prop)
	{
		if (base != NULL && prop != NULL && prop->rpName != NULL)
		{
			while (prop && prop->rpName && prop->rpName[0] != '\0')
			{
				switch (prop->rpFlags)
				{
				case prop_keyValue:
				{
					void *elem = (uint8*)base + prop->rpOffset;
					setKeyValue(elem, prop);

				}
				break;

				case prop_array:
				{
					uint8 *elem = (uint8*)base + prop->rpOffset;
					size_t rank = prop->rpSize / prop->rpTypeSize;

					if (rank)
					{
						for (int32 i = 0; i < rank; i++)
						{
							setKeyValue(elem, prop);
							elem += prop->rpTypeSize;
						}
					}
				}
				break;
				}
				prop++;
			}
		}
	}

	static void setKeyValue(void *elem, reflectProp *prop)
	{
		if (strncmp(prop->rpType, int8r::className(), sizeof(uint64)) == 0)
		{
			((int8r*)elem)->n = prop->rpDefaultValue;
		}
		else if (strncmp(prop->rpType, uint8r::className(), sizeof(uint64)) == 0)
		{
			((uint8r*)elem)->n = prop->rpDefaultValue;
		}
		else if (strncmp(prop->rpType, int16r::className(), sizeof(uint64)) == 0)
		{
			((int16r*)elem)->n = prop->rpDefaultValue;
		}
		else if (strncmp(prop->rpType, uint16r::className(), sizeof(uint64)) == 0)
		{
			((uint16r*)elem)->n = prop->rpDefaultValue;
		}
		else if (strncmp(prop->rpType, int32r::className(), sizeof(uint64)) == 0)
		{
			((int32r*)elem)->n = prop->rpDefaultValue;
		}
		else if (strncmp(prop->rpType, uint32r::className(), sizeof(uint64)) == 0)
		{
			((uint32r*)elem)->n = prop->rpDefaultValue;
		}
		else if (strncmp(prop->rpType, int64r::className(), sizeof(uint64)) == 0)
		{
			((int64r*)elem)->n = prop->rpDefaultValue;
		}
		else if (strncmp(prop->rpType, uint64r::className(), sizeof(uint64)) == 0)
		{
			((uint64r*)elem)->n = prop->rpDefaultValue;
		}
		else if (strncmp(prop->rpType, real32r::className(), sizeof(uint64)) == 0)
		{
			((real32r*)elem)->n = prop->rpDefaultValue;
		}
		else if (strncmp(prop->rpType, real64r::className(), sizeof(uint64)) == 0)
		{
			((real64r*)elem)->n = prop->rpDefaultValue;
		}
		else if (strncmp(prop->rpType, stringr::className(), sizeof(uint64)) == 0)
		{
			((stringr*)elem)->assign(prop->rpDefaultText);
		}
	}
};