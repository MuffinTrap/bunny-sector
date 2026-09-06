#include "doom-map.h"
#include "../duke/actor.h"



 void DoomMap_Allocate(DoomMap* map, int thingsAmount, int sectorAmount, int sideAmount, int lineAmount, int vertexAmount)
{
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

}

int DoomMap::GetNeighbourOfWall(int sectorIndex, int wallIndex)
{
	DoomSubSector* sub = &subsectors[sectorIndex];
	DoomSegment* seg = &segments[sub->firstSegment + wallIndex];
	DoomLinedef* linde = &linedefs[seg->linedef];
	if (linde->sideback >= 0)
	{
		DoomSidedef* side = &sidedefs[linde->sideback];
		return side->sector;
	}
	return -1;
}

u8 DoomMap::GetSectorShade(int sectorIndex, bool floor)
{
	return sectors[subsectors[sectorIndex].sector].lightlevel;
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

void SetActorToStart(DoomMap* map, Actor* actor)
{

}
void DoomMap::SetActorToStart(Actor* actor)
{
	// Find thing 0
	actor->doomPosition.x = things[0].x;
	actor->doomPosition.y = things[0].y;
	actor->position.x = actor->doomPosition.x;
	actor->position.y = actor->doomPosition.y;
	actor->elevation = 0.0f;
	actor->yawRad = DEG2RAD * things[0].angleDeg;
}
void DoomMap::MoveActorInMap(float delta, Actor* actor)
{
	// TODO
}


void DoomMap::PrintInfo()
{
	printf("Doom map: things: %d, sectors %d, sides %d, lines %d, vertices %d\n", thingAmount, sectorAmount, sideAmount, lineAmount, vertexAmount);

	for (int i = 0; i < thingAmount; i++)
	{
		printf("Thing %d (%.2f, %.2f)\n", i, things[i].x, things[i].y);
	}

	for (int i = 0; i < vertexAmount; i++)
	{
		printf("Vertex %d (%.2f, %.2f)\n", i, vertices[i].x, vertices[i].y);
	}

	for (int i = 0; i < sectorAmount; i++)
	{
		printf("Sector %d id: %d floor %d, ceiling %d\n", i,
			   sectors[i].id,
			   sectors[i].heightfloor,
			   sectors[i].heightceiling
			   );
	}
	for (int i = 0; i < sideAmount; i++)
	{
		printf("Side def %d sector: %d texture mid: %d\n", i,
			   sidedefs[i].sector, sidedefs[i].texturemiddle);
	}
	for (int i = 0; i < lineAmount; i++)
	{
		printf("Line def %d. id %d from: %d to: %d front side %d back side %d\n", i,
		linedefs[i].id,
		linedefs[i].v1,
		linedefs[i].v2,
		linedefs[i].sidefront,
		linedefs[i].sideback
		);
	}
	for (int i = 0; i < segmentAmount; i++)
	{
		printf("Segment %d. V1: %d, partner: %d, linedef: %d, side %d\n",
			   i,
			   segments[i].v1,
			   segments[i].partnerSegment,
			   segments[i].linedef,
			   segments[i].lineSide
		);
	}

}

float DoomMap::GetCeilingy(int sectorIndex)
{
	return sectors[subsectors[sectorIndex].sector].heightceiling;
}
float DoomMap::GetFloory(int sectorIndex)
{
	return sectors[subsectors[sectorIndex].sector].heightfloor;
}

Vector2 DoomMap::GetWallVertexInSector(int sectorIndex, int wallIndex)
{
	DoomSubSector* ds = &subsectors[sectorIndex];
	// TODO Should this check for partner?
	DoomSegment* seg = &segments[ds->firstSegment + wallIndex];
	DoomVertex* dv = &vertices[seg->v1];
	return Vector2New(dv->x, dv->y);
}
Vector2 DoomMap::GetNextWallVertexInSector(int sectorIndex, int wallIndex)
{
	DoomSubSector* ds = &subsectors[sectorIndex];
	// TODO Should this check for partner?
	DoomSegment* seg = &segments[ds->firstSegment + ((wallIndex+1) % ds->segmentAmount)];
	DoomVertex* dv = &vertices[seg->v1];
	return Vector2New(dv->x, dv->y);
}


int DoomMap::GetNextWallVertexIndexInSector(int sectorIndex, int wallIndex)
{
	DoomSubSector* ds = &subsectors[sectorIndex];
	return ds->firstSegment + ((wallIndex+1) % ds->segmentAmount);
}

int DoomMap::GetSectorAmount()
{
	return subSectorAmount;
}
int DoomMap::GetSectorFirstWallIndex(int sectorIndex)
{
	DoomSubSector* ds = &subsectors[sectorIndex];
	return ds->firstSegment;
}
MaterialId DoomMap::GetSectorMaterial(int sectorIndex, bool floor)
{
	if (floor)
	{
	return sectors[subsectors[sectorIndex].sector].texturefloor;
	}
	else
	{
	return sectors[subsectors[sectorIndex].sector].textureceiling;
	}
}


int DoomMap::GetWallAmountInSector(int sectorIndex)
{
	return subsectors[sectorIndex].segmentAmount;
}
int DoomMap::GetWallVertexAmount()
{
	return segmentAmount;
}

int DoomMap::FindSectorV2(int currentSector, Vector2 currentPosition)
{
	// TODO BSD traversal
	return 0;
}
 Vector2 DoomMap::GetSectorMaxTexCoord(int sectorIndex)
{
	return subsectors[sectorIndex].maxTexCoord;
}
 Vector2 DoomMap::GetSectorMinPoint(int sectorIndex)
{
	return subsectors[sectorIndex].minXZPoint;
}
Vector2 DoomMap::GetSectorSize(int sectorIndex)
{
	return subsectors[sectorIndex].sizeXZ;
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
