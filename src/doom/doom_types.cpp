#include "doom_types.h"

bool ChildIsNode(ChildId id)
{
	//              12345678
	return  (id & 0x80000000) == 0;
}

DoomSegment* DoomSubSector_GetSegment(unsigned int index, DoomSubSector* obj)
{
	if (index < obj->segmentAmount)
	{
		return &obj->segments[index];
	}
	else
	{
		return &obj->segments[0];
	}
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
