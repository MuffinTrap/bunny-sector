#include "bunny-sector-game.h"
#include "tinyxml2/tinyxml2.h"
#include "bunny-sector-config.h"
#include "bunny-sector-map.h"
#include "doom/doom-map-reader.h"
#include "render/opengl-render.h"
#include <mgdl.h>

Game::Game()
{
	if (mapInfoArray == nullptr)
	{
		mapInfoArray = (MapInfo*)mgdl_AllocateGeneralMemory(sizeof(MapInfo) * MAP_AMOUNT);
		for (int i = 0; i < MAP_AMOUNT; i++)
		{
			mapInfoArray[i].gravity = 8024;
			mapInfoArray[i].name = zstr_init();
			mapInfoArray[i].number = -1;
			mapInfoArray[i].mapPtr = nullptr;
		}
	}
	activeMapIndex = -1;
	status = GameStatus::status_menu;
}

bool Game::ReadXML(const char* filename)
{
	if (mgdl_DoesFileExist(filename) == false)
	{
		return false;
	}

	tinyxml2::XMLDocument gameXml;
	tinyxml2::XMLError loadresult = gameXml.LoadFile(filename);
	if (loadresult != tinyxml2::XML_SUCCESS)
	{
		Log_ErrorF("Failed to load game xml file %s\n", filename);
		return false;
	}


	tinyxml2::XMLElement* gameElement = gameXml.FirstChildElement()->FirstChildElement("game");
	if (gameElement == nullptr)
	{
		Log_ErrorF("No tag game in xml file %s\n", filename);
		return false;
	}
	const char* gamename = XML_ReadChildString(gameElement, "name");
	if (gamename != nullptr)
	{
		name = zstr_from(gamename);
	}
	else
	{
		name = zstr_from("Bunny Sector Game");
	}

	int mapIndex = 0;
	tinyxml2::XMLElement* mapElement = gameElement->FirstChildElement("map");
	if (mapElement == nullptr)
	{
		Log_ErrorF("No maps specified for game in xml\n");
		return false;
	}
	while(mapElement != nullptr && mapIndex < MAP_AMOUNT)
	{
		const char* mapfile = XML_ReadChildString(mapElement, "file");
		if (mapfile != nullptr)
		{
			mapInfoArray[mapIndex].filename = zstr_from(mapfile);
		}
		else {
			Log_ErrorF("No file specified for map %d\n", mapIndex);;
			mapInfoArray[mapIndex].filename = zstr_init();
		}

		bool gotnumber = XML_ReadChildInt(mapElement, "number", &mapInfoArray[mapIndex].number);
		if (gotnumber)
		{
			if (mapInfoArray[mapIndex].number <= 0)
			{
				Log_ErrorF("Invalid number specified for map %d Must be 1+ \n", mapIndex);;
				mapInfoArray[mapIndex].number = 0;
			}
		}
		if (gotnumber == false)
		{
			Log_ErrorF("No number specified for map %d\n", mapIndex);;
		}

		const char* mapname = XML_ReadChildString(mapElement, "name");
		if (mapname != nullptr)
		{
			mapInfoArray[mapIndex].name = zstr_from(mapname);
		}
		else
		{
			mapInfoArray[mapIndex].name = zstr_from(mgdl_BufferPrintf("map %d", mapInfoArray[mapIndex].number));
		}

		XML_ReadChildInt(mapElement, "gravity", &mapInfoArray[mapIndex].gravity);

		mapElement = mapElement->NextSiblingElement("map");
		mapIndex += 1;
	}



	return true;
}

int Game::GetMapAmount()
{
	int amount = 0;
	for (int i = 0; i < MAP_AMOUNT; i++)
	{
		MapInfo* map = &mapInfoArray[i];
		if (map->number >= 0)
		{
			amount += 1;
		}
	}
	return amount;
}

int Game::GetFirstMapNumber()
{
	int first = -1;
	for (int i = 0; i < MAP_AMOUNT; i++)
	{
		MapInfo* map = &mapInfoArray[i];
		if (map->number >= 0 && (first < 0 || map->number < first))
		{
			first = map->number;
		}
	}
	return first;
}

bool Game::LoadMap(int mapNumber)
{
	if (mapNumber < 0)
	{
		return false;
	}
	// Check if loaded already
	for (int i = 0; i < MAP_AMOUNT; i++)
	{
		MapInfo* map = &mapInfoArray[i];
		if (map->number == mapNumber && map->mapPtr != nullptr)
		{
			return true;
		}
	}
	for (int i = 0; i < MAP_AMOUNT; i++)
	{
		MapInfo* map = &mapInfoArray[i];
		if (map->number == mapNumber && map->mapPtr == nullptr)
		{
			BunnyMapType maptype = Map_Invalid;
			if (zstr_contains(&map->filename, ".wad"))
			{
				maptype = Map_Doom;
			}
			else
			{
				Log_ErrorF("Unknown map format in file %s\n", zstr_cstr(&map->filename));
				return false;
			}
			BunnySector_Map* loaded = nullptr;
			if (maptype == Map_Doom)
			{
				loaded = Doom_ReadMapFromFile(map->filename);
				if (loaded == nullptr)
				{
					Log_ErrorF("Could not load map from file %s\n", zstr_cstr(&map->filename));
					return false;
				}
				loaded->m_type = maptype;
				loaded->PrintInfo();
			}
			if (loaded != nullptr)
			{
				// Buffer the floor and ceiling vertices: The uvs need to be calculated first
				OpenGLRender_CreateFloorBuffers(loaded);

				map->mapPtr = loaded;
				activeMapIndex = i;
				return true;
			}
			else
			{
				mgdl_FreeGeneralMemory(loaded);
			}
			return false;
		}
	}
	return false;
}

BunnySector_Map* Game::GetActiveMap()
{
	if (activeMapIndex >= 0)
	{
		return mapInfoArray[activeMapIndex].mapPtr;
	}
	return nullptr;
}

bool Game::StartActiveMap()
{
	if (activeMapIndex >= 0)
	{
		status = GameStatus::status_player_alive;
		return true;
	}
	return false;
}

int Game::GetNextMapNumber(int currentNumber)
{
	for (int i = 0; i < MAP_AMOUNT; i++)
	{
		MapInfo* map = &mapInfoArray[i];
		if (map->number == currentNumber + 1)
		{
			return map->number;
		}
	}
	return -1;
}
void Start2D()
{
	int screenWidth = mgdl_GetScreenWidth();
	int screenHeight = mgdl_GetScreenHeight();

glPushMatrix();

	float screen_half_width = screenWidth / 2.0f;
	float screen_half_height = screenHeight / 2.0f;
	glTranslatef(screen_half_width, screen_half_height, 0);

}
void End2D()
{
	glPopMatrix();

}
bool Game::UpdateMenu()
{
	Start2D();
	mgdl_DrawText("Menu", 10, 16, 32, Debug_LightYellow);
	mgdl_DrawText("Press (A) to start", 10, 16 + 32, 16, Debug_LightBlue);
	End2D();
	return (mgdl_IsButtonPressed(0, ButtonA));
}

bool Game::UpdateMapEndMenu()
{
	Start2D();
		// Map is over
		mgdl_DrawText("Exit map", 10, 16, 32, Debug_LightYellow);
		mgdl_DrawText("Press (A) and (B) together to restart", 10, 16 + 32, 8, Debug_LightBlue);

	End2D();

	return (mgdl_IsButtonDown(0, ButtonA) && mgdl_IsButtonDown(0, ButtonB));
}


