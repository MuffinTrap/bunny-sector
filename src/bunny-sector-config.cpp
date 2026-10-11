#include "bunny-sector-config.h"
#include "tinyxml2/tinyxml2.h"

bool BunnySector_ReadAllXML()
{
	return false;

}

bool BunnySector_ReadMaterialsXML()
{
	return false;
}


bool BunnySector_ReadGameXML()
{
	return false;
}

bool BunnySector_ReadPlayerXML()
{
	return false;
}

bool BunnySector_ReadItemsXML()
{
	return false;
}

bool BunnySector_ReadMonstersXML()
{
	return false;
}

bool XML_ReadChildInt(tinyxml2::XMLElement* parent, const char* tag, int* valueOut)
{
	tinyxml2::XMLElement* intElement = parent->FirstChildElement(tag);
	if (intElement)
	{
		intElement->QueryIntText(valueOut);
		return true;
	}
	return false;
}


const char * XML_ReadChildString(tinyxml2::XMLElement* parent, const char* tag)
{
	tinyxml2::XMLElement* stringElement = parent->FirstChildElement(tag);
	if (stringElement)
	{
		return stringElement->GetText();
	}
	return nullptr;;
}

