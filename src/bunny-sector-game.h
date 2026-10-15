#pragma once
#include <mgdl.h>
#include "bunny-sector-types.h"

class BunnySector_Map;

struct MapInfo
{
	zstr name;
	zstr filename;
	int number;
	int gravity;
	BunnySector_Map* mapPtr;
};

/**
 * @brief This class is the game with a progression of maps and menus
 */
class Game
{
public:
	Game();
	bool ReadXML(const char* filename);
	int GetMapAmount();
	int GetFirstMapNumber();
	int GetNextMapNumber(int currentNumber);
	bool LoadMap(int mapNumber);
	BunnySector_Map* GetActiveMap();

	bool StartActiveMap();

	bool UpdateMenu();
	bool UpdateMapEndMenu();

	GameStatus status;
	zstr name;

	MapInfo* mapInfoArray = nullptr;

	int activeMapIndex = -1;
	const float MAP_AMOUNT = 4;
};
