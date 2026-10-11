#pragma once

class Player;

class PlayerManager
{
public:
	PlayerManager();
	bool ReadXML(const char* filename);
	bool LoadPlayer(Player* player, int playerNumber);

};
