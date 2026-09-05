#include "bunny-sector-map.h"


void BunnySector_Map::SetActorToStart(Actor* actor)
{
	switch(m_type)
	{
		case Map_Duke:
			DukeMap_SetActorToStart(mapPtr.dukeMap, actor);
		break;
		case Map_Doom:
			DoomMap_SetActorToStart(mapPtr.doomMap, actor);
		break;
		case Map_Invalid:
			mgdl_assert_test(false);
			break;
	}
}

void BunnySector_Map::MoveActorInMap(float delta, Actor* actor)
{
	switch(m_type)
	{
		case Map_Duke:
			DukeMap_MoveActorInMap(mapPtr.dukeMap, delta, actor);
			break;
		case Map_Doom:
			break;
		case Map_Invalid:
			// NOP
			break;
	}
}

int BunnySector_Map::GetSectorAmount()
{
	switch(m_type)
	{
		case Map_Duke:
			return mapPtr.dukeMap->sectorAmount;
			break;
		case Map_Doom:
			return mapPtr.doomMap->subSectorAmount;
			break; case Map_Invalid:
			return -1;
			break;
	}
}


int BunnySector_Map::GetWallVertexAmount()
{

	switch(m_type)
	{
		case Map_Duke:
			return mapPtr.dukeMap->wallAmount;
			break;
		case Map_Doom:
			return mapPtr.doomMap->vertexAmount;
			break;
		case Map_Invalid:
			return -1;
			break;
	}
}

int BunnySector_Map::GetWallAmountInSector(int sectorIndex)
{
	switch(m_type)
	{
		case Map_Duke:
		{
			Sector* s = &mapPtr.dukeMap->sectors[sectorIndex];
			return s->wallnum;
		}
			break;
		case Map_Doom:
		{
			DoomSubSector* ds = &mapPtr.doomMap->subsectors[sectorIndex];
			return ds->segmentAmount;
		}
			break;
		case Map_Invalid:
			return -1;
			break;
	}

}

int BunnySector_Map::GetSectorFirstWallIndex(int sectorIndex)
{
	switch(m_type)
	{
		case Map_Duke:
		{
			Sector* s = &mapPtr.dukeMap->sectors[sectorIndex];
			return s->wallptr;
		}
			break;
		case Map_Doom:
		{
			DoomSubSector* ds = &mapPtr.doomMap->subsectors[sectorIndex];
			return ds->firstSegment;
		}
			break;
		case Map_Invalid:
			return -1;
			break;
	}
}
Vector2 BunnySector_Map::GetWallVertexInSector(int sectorIndex, int wallIndex)
{
	switch(m_type)
	{
		case Map_Duke:
		{
			Wall* w = Map_GetWallInSector(mapPtr.dukeMap, sectorIndex, wallIndex);
			return Vector2New(w->x, w->z);
		}
			break;
		case Map_Doom:
		{
			DoomMap* dm = mapPtr.doomMap;
			DoomSubSector* ds = &dm->subsectors[sectorIndex];
			DoomSegment* seg = &dm->segments[ds->firstSegment + wallIndex];
			DoomVertex* dv = &mapPtr.doomMap->vertices[seg->v1];
			return Vector2New(dv->x, dv->y);
		}
			break;
		case Map_Invalid:
			return Vector2Zero();
			break;
	}
}

Vector2 BunnySector_Map::GetNextWallVertex(int sectorIndex, int wallIndex)
{
	switch(m_type)
	{
		case Map_Duke:
		{
			Wall* w = Map_GetWall(mapPtr.dukeMap, wallIndex);
			Wall* we = Map_GetWallEnd(mapPtr.dukeMap, w);
			return Vector2New(we->x, we->z);
		}
			break;
		case Map_Doom:
		{
			DoomMap* dm = mapPtr.doomMap;
			DoomSubSector* ds = &dm->subsectors[sectorIndex];
			// TODO Should this check for partner?
			DoomSegment* seg = &dm->segments[ds->firstSegment + ((wallIndex+1) % ds->segmentAmount)];
			DoomVertex* dv = &mapPtr.doomMap->vertices[seg->v1];
			return Vector2New(dv->x, dv->y);
		}
			break;
		case Map_Invalid:
			return Vector2Zero();
			break;
	}
}



