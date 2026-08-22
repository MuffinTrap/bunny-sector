#include "doom-map.h"
DoomMap* DoomMap_Create(int thingsAmount, int sectorAmount, int sideAmount, int lineAmount, int vertexAmount)
{
	DoomMap* map = (DoomMap*)mgdl_AllocateGeneralMemory(sizeof(DoomMap));
	map->thingAmount = thingsAmount;
	map->sectorAmount = sectorAmount;
	map->sideAmount = sideAmount;
	map->lineAmount = lineAmount;
	map->vertexAmount = vertexAmount;

	map->things = (thing*)mgdl_AllocateGeneralMemory(thingsAmount * sizeof(thing));
	map->sectors = (sector*)mgdl_AllocateGeneralMemory(sectorAmount * sizeof(sector));
	map->linedefs = (linedef*)mgdl_AllocateGeneralMemory(lineAmount * sizeof(linedef));
	map->sidedefs = (sidedef*)mgdl_AllocateGeneralMemory(sideAmount * sizeof(sidedef));
	map->vertices = (vertex*)mgdl_AllocateGeneralMemory(vertexAmount * sizeof(vertex));
	return map;
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
		printf("Sector %d id: %d\n", i, map->sectors[i].id);
	}
	for (int i = 0; i < map->sideAmount; i++)
	{
		printf("Side def %d sector: %d texture mid: %d\n", i, map->sidedefs[i].sector, map->sidedefs[i].texturemiddle);
	}
	for (int i = 0; i < map->lineAmount; i++)
	{
		printf("Line def %d from: %d to: %d\n", i, map->linedefs[i].v1, map->linedefs[i].v2);
	}
}
