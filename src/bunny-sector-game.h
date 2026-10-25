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
	bool useFog;
	int fogDensity;
	color32 fogColor;
	float fogNear;
	float fogFar;

	bool useSkybox;
	color32 skyZenithColor;
	color32 skyHorizonColor;
	color32 skyNadirColor;

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

	Menu* menu;

	// Main menu variables
	color32 titleColor;
	color32 mainBgColor;
	TextureHandle mainBgTexture;


	int activeMapIndex = -1;
	const float MAP_AMOUNT = 4;
};
