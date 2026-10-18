#pragma once
#include "doom-lump-info.h"
class WADReader
{
public:
	WADReader(FILE* wadFilePtr);
	~WADReader();
	DoomLumpInfo* GetTableItem(const char* name);

	zstr wadFilename;
	u32 lumpAmount;
	u32 infoTableOffset;

	DoomLumpInfo* infoTable;
};
