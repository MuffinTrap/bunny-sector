#pragma once
#include "doom/doom_types.h"


class Actor;

namespace BunnySector
{

/**
 * @brief Loads item configurations
 */
class ItemManager
{
public:
	ItemManager();
	~ItemManager();
	bool LoadXML(const char* filename);
	bool LoadItemToActor(Actor* actor, DOOM_EDITOR_NUMBER editorNumber);

private:
	const int PrefabAmount = 128;
	int nextPrefabIndex;
	Actor* ItemPrefabs;
};

};
