#pragma once

#include "doom_types.h"

struct DoomMap
{
	int thingAmount;
	int sectorAmount;
	int sideAmount;
	int lineAmount;
	int vertexAmount;

	thing* things;
	sector* sectors;
	sidedef* sidedefs;
	linedef* linedefs;
	vertex* vertices;
};
typedef struct DoomMap DoomMap;

DoomMap* DoomMap_Create(int thingsAmount, int sectorAmount, int sideAmount, int lineAmount, int vertexAmount);

void DoomMap_PrintInfo(DoomMap* map);

