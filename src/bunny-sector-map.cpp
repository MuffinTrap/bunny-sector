#include "bunny-sector-map.h"


void Map_SetActorToStart(BunnySector_Map* map, Actor* actor)
{
	switch(map->m_type)
	{
		case Map_Duke:
			DukeMap_SetActorToStart(map->mapPtr.dukeMap, actor);
		break;
		case Map_Doom:
			DoomMap_SetActorToStart(map->mapPtr.doomMap, actor);
		break;
		case Map_Invalid:
			//nop
			break;
	}
}

void Map_MoveActorInMap(BunnySector_Map* map, float delta, Actor* actor)
{
}


