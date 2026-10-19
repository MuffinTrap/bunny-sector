#pragma once
#include "doom-lump-info.h"
class WADReader
{
public:
	WADReader(BinaryReader* reader);
	~WADReader();
	DoomLumpInfo* FindTableItem(const char* name);

	zstr wadFilename;
	u32 lumpAmount;
	u32 infoTableOffset;

	DoomLumpInfo* infoTable;
};
