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
