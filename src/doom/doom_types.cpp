#include "doom_types.h"

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

static zstr DoomTypeNames[NAMED_DOOM_TYPE_AMOUNT] =
{
	zstr_from("No type"),
	zstr_from("Player start #1"),
	zstr_from("Player start #2"),
	zstr_from("Player start #3"),
	zstr_from("Player start #4"),
	zstr_from("BlueCard")
};

const char* DoomTypeToStringChar(DOOM_EDITOR_NUMBER typeNumber)
{
	if (int(typeNumber)>= 0 && (int)typeNumber < NAMED_DOOM_TYPE_AMOUNT)
	{
		return zstr_cstr(&DoomTypeNames[int(typeNumber)]);
	}
	return nullptr;
}

zstr * DoomTypeToStringZstr(DOOM_EDITOR_NUMBER typeNumber)
{
	if (int(typeNumber)>= 0 && (int)typeNumber < NAMED_DOOM_TYPE_AMOUNT)
	{
		return &DoomTypeNames[int(typeNumber)];
	}
	return nullptr;
}

