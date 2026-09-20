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

ChildId DoomNode_GetChild(DoomNode* node, unsigned int index)
{
	return node->children[index];
}
s16 DoomNode_GetBBox0(DoomNode* node, unsigned int index)
{
	return node->bbox0[index];
}
s16 DoomNode_GetBBox1(DoomNode* node, unsigned int index)
{
	return node->bbox1[index];
}
s16 DoomNode_GetBBox(DoomNode* node, unsigned int index) // Combined 0-7 index
{
	if (index < 4){
		return node->bbox0[index];
	}
	else
	{
		return node->bbox1[index-4];
	}
}

// NOTE This was changed because map data was flipped with FLIP_THE_Y
#define CHILD_0 1
#define CHILD_1 0

int DoomNode_GetChildSide(DoomNode* node, float x, float y)
{
	if (node->dx == 0)
	{
		// Vertical cut
		if (x < node->x)
		{
			if (node->dy > 0) {return CHILD_1;}
			else {return CHILD_0;}
		}
		if (node->dy < 0) {return CHILD_1;}
		else {return CHILD_0;}
	}
	if (node->dy == 0)
	{
		// Horizontal cut
		if (y < node->y)
		{
			if (node->dx > 0) {return CHILD_0;}
			else {return CHILD_1;}
		}
		if (node->dx < 0) {return CHILD_0;}
		else {return CHILD_1;}
	}
	float dx = x - node->x;
	float dy = y - node->y;
	if( dx * node->dy < dy * node->dx)
	{
		return CHILD_1;
	}
	return CHILD_0;
}

bool DoomNode_PointInsideBox(DoomNode* node, Vector2 point, int childIndex)
{
	s16 left =node->bbox0[BB_LFT];
	s16 right =node->bbox0[BB_RGT];
	s16 top =node->bbox0[BB_TOP];
	s16 bot =node->bbox0[BB_BOT];
	if (childIndex == 1)
	{
	 left =node->bbox1[BB_LFT];
	 right =node->bbox1[BB_RGT];
	 top =node->bbox1[BB_TOP];
	 bot =node->bbox1[BB_BOT];
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
