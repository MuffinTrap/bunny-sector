#pragma once

class Player;

bool BunnySector_ReadAllXML();
bool BunnySector_ReadGameXML();
bool BunnySector_ReadPlayerXML();
bool BunnySector_ReadItemsXML();
bool BunnySector_ReadMonstersXML();
bool BunnySector_ReadMaterialsXML();

// TODO Where should this go?
void BunnySector_LoadPlayerDefaults(Player* player, int playerIndex);
