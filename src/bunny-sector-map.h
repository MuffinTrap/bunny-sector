#pragma once

#include "duke/dukemap.h"
#include "doom/doom-map.h"

// Abstract map file

enum BunnyMapType
{
	Map_Duke,
	Map_Doom,
	Map_Invalid
};
typedef enum BunnyMapType BunnyMapType;

struct BunnySector_Map
{
	BunnyMapType m_type;
    zstr mapfile;

	union MapPointer
	{
		DukeMap* dukeMap;
		DoomMap* doomMap;
	};
	MapPointer mapPtr;
};
typedef struct BunnySector_Map BunnySector_Map;

void Map_SetActorToStart(BunnySector_Map* map, Actor* actor);
void Map_MoveActorInMap(BunnySector_Map* map, float delta,Actor* actor);
