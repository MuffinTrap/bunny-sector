#include "doom-numbers.h"

// Define a map that links integer (Doom editor number) to const char* (Type name)
// Create one map for default names that are used when loading items.xml
// Create another map for custom names that are loaded from xml


#define NAME EdNumberToNameMap
#define KEY_TY int
#define VAL_TY zstr
#define HASH_FN vt_hash_integer
#define CMPR_FN vt_cmpr_integer
#include "../verstable/verstable.h"

EdNumberToNameMap defaultNames;
EdNumberToNameMap customNames;

static bool initDone = false;
void InitDoomTypeNames()
{
	if (initDone) { return; }
	EdNumberToNameMap_init( &defaultNames );
	EdNumberToNameMap_reserve(&defaultNames, NAMED_DOOM_TYPE_AMOUNT);
	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_player_start_1, zstr_from("Player start #1"));
	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_player_start_2, zstr_from("Player start #2"));
	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_player_start_3, zstr_from("Player start #3"));
	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_player_start_4, zstr_from("Player start #4"));
	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_blue_card, zstr_from("BlueCard"));
	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_yellow_card, zstr_from("YellowCard"));
	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_red_card, zstr_from("RedCard"));

	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_blue_skull_key, zstr_from("BlueSkull"));
	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_yellow_skull_key, zstr_from("YellowSkull"));
	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_red_skull_key, zstr_from("RedSkull"));

	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_helper_dog, zstr_from("HelperDog"));
	EdNumberToNameMap_insert(&defaultNames, (int)editorNumber_cell_pack, zstr_from("CellPack"));

	EdNumberToNameMap_init( &customNames );
}

void SetDoomTypeCustomName(DOOM_EDITOR_NUMBER typenumber, zstr customName)
{
	int ednumInt = (int)typenumber;
	EdNumberToNameMap_itr found = EdNumberToNameMap_get(&customNames, ednumInt);
	if (EdNumberToNameMap_is_end(found))
	{
		EdNumberToNameMap_insert(&customNames, ednumInt, customName);
	}
	else
	{
		Log_ErrorF("Type %d alread has a custom name\n");
	}
}

const char* DoomTypeToStringChar(DOOM_EDITOR_NUMBER typeNumber)
{
	int ednumInt = (int)typeNumber;
	EdNumberToNameMap_itr foundCustom = EdNumberToNameMap_get(&customNames, ednumInt);
	if (EdNumberToNameMap_is_end(foundCustom) == false)
	{
		return zstr_cstr(&foundCustom.data->val);
	}
	EdNumberToNameMap_itr found = EdNumberToNameMap_get(&defaultNames, ednumInt);
	if (EdNumberToNameMap_is_end(found) == false)
	{
		return zstr_cstr(&found.data->val);
	}
	return nullptr;
}

zstr* DoomTypeToStringZstr(DOOM_EDITOR_NUMBER typeNumber)
{
	int ednumInt = (int)typeNumber;
	EdNumberToNameMap_itr foundCustom = EdNumberToNameMap_get(&customNames, ednumInt);
	if (EdNumberToNameMap_is_end(foundCustom) == false)
	{
		return &foundCustom.data->val;
	}
	EdNumberToNameMap_itr found = EdNumberToNameMap_get(&defaultNames, ednumInt);
	if (EdNumberToNameMap_is_end(found) == false)
	{
		return &found.data->val;
	}
	return nullptr;
}

DOOM_EDITOR_NUMBER NameToDoomEditorNumberChar(const char* name)
{
	zstr namez = zstr_from(name);
	DOOM_EDITOR_NUMBER foundType = editorNumber_none;
	for (EdNumberToNameMap_itr it = EdNumberToNameMap_first(&defaultNames); EdNumberToNameMap_is_end(it) == false; it = EdNumberToNameMap_next(it))
	{
		if (zstr_eq(&namez, &it.data->val))
		{
			foundType = (DOOM_EDITOR_NUMBER)it.data->key;
			break;
		}
	}
	zstr_free(&namez);
	return foundType;
}

ActorType GetActorTypeOfDoomType(DOOM_EDITOR_NUMBER typeNumber)
{
    if (typeNumber >= editorNumber_player_start_1 && typeNumber <= editorNumber_player_start_4)
    {
        return actor_player;
    }
    switch(typeNumber)
    {
        // All these items are scattered around the list
        case editorNumber_backpack:
        case editorNumber_blue_card:
        case editorNumber_yellow_card:
        case editorNumber_red_card:
        case editorNumber_cell_pack:
        case editorNumber_red_skull_key:
        case editorNumber_blue_skull_key:
        case editorNumber_yellow_skull_key:
        case editorNumber_rocket_box:
        case editorNumber_cell:
        case editorNumber_clip_box:
        case editorNumber_shell_box:
        case editorNumber_pistol:
            return actor_item;
        break;
        default:
            // NOP
        break;
    };
    if (typeNumber >= editorNumber_shotgun && typeNumber < editorNumber_all_map_reveal)
    {
        return actor_item;
    }
    if (typeNumber >= editorNumber_doom_imp && typeNumber < editorNumber_player_start_5)
    {
        return actor_monster;
    }

    if (typeNumber < editorNumber_all_map_reveal)
    {
        return actor_decoration;
    }
    else
    {
        return actor_none;
    }
}
