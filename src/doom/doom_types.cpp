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

