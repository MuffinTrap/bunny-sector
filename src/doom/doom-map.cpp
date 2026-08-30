#include "doom-map.h"
#include "../duke/actor.h"


DoomMap* DoomMap_Create(int thingsAmount, int sectorAmount, int sideAmount, int lineAmount, int vertexAmount)
{
	DoomMap* map = (DoomMap*)mgdl_AllocateGeneralMemory(sizeof(DoomMap));
	map->thingAmount = thingsAmount;
	map->sectorAmount = sectorAmount;
	map->sideAmount = sideAmount;
	map->lineAmount = lineAmount;
	map->vertexAmount = vertexAmount;

	map->things = (DoomThing*)mgdl_AllocateGeneralMemory(thingsAmount * sizeof(DoomThing));
	map->sectors = (DoomSector*)mgdl_AllocateGeneralMemory(sectorAmount * sizeof(DoomSector));
	map->linedefs = (DoomLinedef*)mgdl_AllocateGeneralMemory(lineAmount * sizeof(DoomLinedef));
	map->sidedefs = (DoomSidedef*)mgdl_AllocateGeneralMemory(sideAmount * sizeof(DoomSidedef));
	map->vertices = (DoomVertex*)mgdl_AllocateGeneralMemory(vertexAmount * sizeof(DoomVertex));

	return map;
}


void DoomMap_AllocateNodes(DoomMap* map, int nodeAmount)
{
	map->nodeAmount = nodeAmount;
	map->nodes = (DoomNode*)mgdl_AllocateGeneralMemory(nodeAmount * sizeof(DoomNode));
}
void DoomMap_AllocateSegments(DoomMap* map, int segmentAmount)
{
	map->segmentAmount = segmentAmount;
	map->segments = (DoomSegment*)mgdl_AllocateGeneralMemory(segmentAmount * sizeof(DoomSegment));
}
void DoomMap_AllocateSubsectors(DoomMap* map, int subSectorAmount)
{
	map->subSectorAmount = subSectorAmount;
	map->subsectors = (DoomSubSector*)mgdl_AllocateGeneralMemory(subSectorAmount * sizeof(DoomSubSector));
}

void DoomMap_SetActorToStart(DoomMap* map, Actor* actor)
{
	// Find thing 0
	actor->doomPosition.x = map->things[0].x;
	actor->doomPosition.y = map->things[0].y;
	actor->position.x = actor->doomPosition.x;
	actor->position.y = actor->doomPosition.y;
	actor->elevation = 0.0f;
	actor->yawRad = DEG2RAD * map->things[0].angleDeg;

}

void DoomMap_PrintInfo(DoomMap* map)
{
	printf("Doom map: things: %d, sectors %d, sides %d, lines %d, vertices %d\n", map->thingAmount, map->sectorAmount, map->sideAmount, map->lineAmount, map->vertexAmount);

	for (int i = 0; i < map->thingAmount; i++)
	{
		printf("Thing %d (%.2f, %.2f)\n", i, map->things[i].x, map->things[i].y);
	}

	for (int i = 0; i < map->vertexAmount; i++)
	{
		printf("Vertex %d (%.2f, %.2f)\n", i, map->vertices[i].x, map->vertices[i].y);
	}

	for (int i = 0; i < map->sectorAmount; i++)
	{
		printf("Sector %d id: %d floor %d, ceiling %d\n", i,
			   map->sectors[i].id,
			   map->sectors[i].heightfloor,
			   map->sectors[i].heightceiling
			   );
	}
	for (int i = 0; i < map->sideAmount; i++)
	{
		printf("Side def %d sector: %d texture mid: %d\n", i,
			   map->sidedefs[i].sector, map->sidedefs[i].texturemiddle);
	}
	for (int i = 0; i < map->lineAmount; i++)
	{
		printf("Line def %d. id %d from: %d to: %d front side %d back side %d\n", i,
		map->linedefs[i].id,
		map->linedefs[i].v1,
		map->linedefs[i].v2,
		map->linedefs[i].sidefront,
		map->linedefs[i].sideback
		);
	}
	for (int i = 0; i < map->segmentAmount; i++)
	{
		printf("Segment %d. V1: %d, partner: %d, linedef: %d, side %d\n",
			   i,
			   map->segments[i].v1,
			   map->segments[i].partnerSegment,
			   map->segments[i].linedef,
			   map->segments[i].lineSide
		);
	}

}
DoomThing* DoomMap_GetThing(DoomMap* map, unsigned int index) { return &map->things[index];}
DoomSector* DoomMap_GetSector(DoomMap* map, unsigned int index) { return &map->sectors[index];}
DoomSidedef* DoomMap_GetSidedef(DoomMap* map, unsigned int index) { return &map->sidedefs[index];}
DoomLinedef* DoomMap_GetLinedef(DoomMap* map, unsigned int index) { return &map->linedefs[index];}
DoomVertex* DoomMap_GetVertex(DoomMap* map, unsigned int index) { return &map->vertices[index];}
DoomNode* DoomMap_GetNode(DoomMap* map, unsigned int index) { return &map->nodes[index];}
DoomSubSector* DoomMap_GetSubSector(DoomMap* map, unsigned int index) { return &map->subsectors[index];}
DoomSegment* DoomMap_GetSegment(DoomMap* map, unsigned int index) { return &map->segments[index];}

DoomNode * DoomMap_GetRootNode(DoomMap* map) { return &map->nodes[map->nodeAmount-1]; }

DoomNode* DoomMap_GetChildNode(DoomMap* map, ChildId id) {return &map->nodes[id];}
DoomSubSector* DoomMap_GetChildSubSector(DoomMap* map, ChildId id) { return &map->subsectors[(id & 0x7fffffff)];}
