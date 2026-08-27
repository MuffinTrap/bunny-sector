#pragma once

#include "doom_types.h"
struct Actor;

struct DoomMap
{
	int thingAmount;
	int sectorAmount;
	int sideAmount;
	int lineAmount;
	int vertexAmount;

	DoomThing* things;
	DoomSector* sectors;
	DoomSidedef* sidedefs;
	DoomLinedef* linedefs;
	DoomVertex* vertices;

	// Node data
	int nodeAmount;
	int segmentAmount;
	int subSectorAmount;
	DoomNode* nodes; // NOTE Last one is the root node
	DoomSubSector* subsectors;
	DoomSegment* segments;

};
typedef struct DoomMap DoomMap;

DoomMap* DoomMap_Create(int thingsAmount, int sectorAmount, int sideAmount, int lineAmount, int vertexAmount);
void DoomMap_AllocateNodes(DoomMap* map, int nodeAmount);
void DoomMap_AllocateSegments(DoomMap* map, int segmentAmount);
void DoomMap_AllocateSubsectors(DoomMap* map, int subSectorAmount);

void DoomMap_PrintInfo(DoomMap* map);

// property accessors for AngelScript
DoomThing* DoomMap_GetThing(DoomMap* map, unsigned int index);
DoomSector* DoomMap_GetSector(DoomMap* map, unsigned int index);
DoomSidedef* DoomMap_GetSidedef(DoomMap* map, unsigned int index);
DoomLinedef* DoomMap_GetLinedef(DoomMap* map, unsigned int index);
DoomVertex* DoomMap_GetVertex(DoomMap* map, unsigned int index);
DoomNode* DoomMap_GetNode(DoomMap* map, unsigned int index);
DoomSubSector* DoomMap_GetSubSector(DoomMap* map, unsigned int index);
DoomSegment* DoomMap_GetSegment(DoomMap* map, unsigned int index);

DoomNode* DoomMap_GetRootNode(DoomMap* map);
DoomNode* DoomMap_GetChildNode(DoomMap* map, ChildId id);
DoomSubSector* DoomMap_GetChildSubSector(DoomMap* map, ChildId id);

// Interface BunnySector_Map

void DoomMap_SetActorToStart(DoomMap* map, Actor* actor);

