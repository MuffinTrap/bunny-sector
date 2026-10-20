#pragma once
#include "tinyxml2/tinyxml2.h"

class Player;

bool BunnySector_ReadAllXML();
bool BunnySector_ReadGameXML();
bool BunnySector_ReadPlayerXML();
bool BunnySector_ReadItemsXML();
bool BunnySector_ReadMonstersXML();
bool BunnySector_ReadMaterialsXML();

// TODO Where should this go?
void BunnySector_LoadPlayerDefaults(Player* player, int playerIndex);

// XML reading functions
bool XML_ReadChildInt(tinyxml2::XMLElement*parent, const char* tag, int* valueOut);
bool XML_ReadChildFloat(tinyxml2::XMLElement*parent, const char* tag, float* valueOut);
const char* XML_ReadChildString(tinyxml2::XMLElement*parent, const char* tag);
const char* XML_ReadChildString(tinyxml2::XMLElement*parent, const char* tag);
