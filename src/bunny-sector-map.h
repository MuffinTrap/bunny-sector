#pragma once

#include "duke/dukemap.h"
#include "doom/doom-map.h"

// Abstract map file

enum BunnyMapType
{
	Map_Duke,
	Map_Doom
};
typedef enum BunnyMapType BunnyMapType;

struct BunnySector_Map
{
	BunnyMapType m_type;

	union MapPointer
	{
		DukeMap* dukeMap;
		DoomMap* doomMap;
	};
};
typedef struct BunnySector_Map BunnySector_Map;
