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

	// Node data
	int nodeAmount;
	int segmentAmount;
	int subSectorAmount;
	DoomNode* nodes;
	SubSector* subsectors;
	Segment* segments;

};
typedef struct DoomMap DoomMap;

DoomMap* DoomMap_Create(int thingsAmount, int sectorAmount, int sideAmount, int lineAmount, int vertexAmount);
void DoomMap_AllocateNodes(DoomMap* map, int nodeAmount);
void DoomMap_AllocateSegments(DoomMap* map, int segmentAmount);
void DoomMap_AllocateSubsectors(DoomMap* map, int subSectorAmount);

void DoomMap_PrintInfo(DoomMap* map);

