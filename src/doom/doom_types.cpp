#include "doom_types.h"

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

int BB_TOP = 0;
int BB_BOT = 1;
int BB_LFT = 2;
int BB_RGT = 3;

bool ChildIsNode(ChildId id)
{
	//              12345678
	return  (id & 0x80000000) == 0;
}

int ChildToSubSectorIndex(ChildId id)
{
	return  (id & 0x7fffffff);
}

void DoomLinedef_Init(DoomLinedef* def)
{
	def->id = -1;
	def->linedef_flags = 0;
	def->special = 0;
	def->arg0 = 0;
	def->arg1 = 0;
	def->arg2 = 0;
	def->arg3 = 0;
	def->arg4 = 0;
	def->sideback = -1;
}

void DoomSidedef_Init(DoomSidedef* def)
{
	def->offsetx = 0;
	def->offsety = 0;
	def->texturetop = -1;
	def->texturebottom = -1;
	def->texturemiddle = -1;
}

void DoomSector_Init(DoomSector* def)
{
	def->heightceiling = 0;
	def->heightfloor = 0;
	def->id = 0;
	def->special = 0;
	def->lightlevel = 160;
	def->textureceiling = -1;
	def->texturefloor = -1;
	def->usecase = sector_none;
	def->textureceiling = -1;
	def->texturefloor = -1;
}

void DoomSegment_Init(DoomSegment* def)
{
	def->v1 = -1;
	def->linedef = -1;
	def->partnerSegment = -1;
	def->lineSide = 0;
}

void DoomThing_Init(DoomThing* def)
{
	def->id = 0;
	def->type = 0;
	def->height = 0;
	def->angleDeg = 0;
	def->thing_flags = 0;
	def->special = 0;
	def->arg0 = 0;
	def->arg1 = 0;
	def->arg2 = 0;
	def->arg3 = 0;
	def->arg4 = 0;
}

ChildId DoomNode::GetChild( unsigned int index)
{
	return children[index];
}
s16 DoomNode::GetBBox0( unsigned int index)
{
	return bbox0[index];
}
s16 DoomNode::GetBBox1( unsigned int index)
{
	return bbox1[index];
}
s16 DoomNode::GetBBox( unsigned int index) // Combined 0-7 index
{
	if (index < 4){
		return bbox0[index];
	}
	else
	{
		return bbox1[index-4];
	}
}

// NOTE This was changed because map data was flipped with FLIP_THE_Y
#define CHILD_0 1
#define CHILD_1 0

int DoomNode::GetChildSide( float px, float py)
{
	if (this->dx == 0)
	{
		// Vertical cut
		if (px < this->x)
		{
			if (this->dy > 0) {return CHILD_1;}
			else {return CHILD_0;}
		}
		if (this->dy < 0) {return CHILD_1;}
		else {return CHILD_0;}
	}
	else if (this->dy == 0)
	{
		// Horizontal cut
		if (py < this->y)
		{
			if (dx > 0) {return CHILD_0;}
			else {return CHILD_1;}
		}
		if (this->dx < 0) {return CHILD_0;}
		else {return CHILD_1;}
	}
	float mx = px - this->x;
	float my = py - this->y;
	if( mx * this->dy < my * this->dx)
	{
		return CHILD_1;
	}
	return CHILD_0;
}

bool DoomNode::PointInsideBox( Vector2 point, int childIndex)
{
	s16 left =bbox0[BB_LFT];
	s16 right =bbox0[BB_RGT];
	s16 top =bbox0[BB_TOP];
	s16 bot =bbox0[BB_BOT];
	if (childIndex == 1)
	{
	 left =bbox1[BB_LFT];
	 right =bbox1[BB_RGT];
	 top =bbox1[BB_TOP];
	 bot =bbox1[BB_BOT];
	}
	if (point.x < left || point.x > right)
	{
		return false;
	}
	if (point.y < bot || point.y > top)
	{
		return false;
	}
	return true;
}

float DoomSpeedToUnits(int doomSpeed)
{
	 return ((float)doomSpeed * DOOM_SPEED_TO_UNITS) / DOOM_TICK_DURATION_SECONDS;
}



