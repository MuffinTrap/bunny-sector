
#include "../bunny-sector-types.h"
#include "doom_types.h"
#include "binaryreader.h"
#include "doom-wad-reader.h"
#include <mgdl.h>


WADReader::WADReader(FILE* wadFILEPtr)
{
	// Read the header
	if (wadFILEPtr != nullptr)
	{
		char identification[4];
		StartReadingFile(wadFILEPtr);
		ReadChars(4, identification); // PWAD or IWAD
		zstr idstr = zstr_from_len(identification, 4);
		lumpAmount = ReadUInt32();
		infoTableOffset = ReadUInt32();

		Log_InfoF("Wad file.Id %s Lumps %d Infotable at %d\n", zstr_cstr(&idstr), lumpAmount, infoTableOffset);

		// Read the infotable
		// Each table entry is 16 bytes
		char lumpName[8];
		infoTable = new DoomLumpInfo[lumpAmount];
		for (int i = 0; i < lumpAmount; i++)
		{
			fseek(wadFILEPtr, infoTableOffset + i * 16, SEEK_SET);
			infoTable[i].offset = ReadUInt32();
			infoTable[i].size = ReadUInt32();
			ReadChars(8, lumpName);
			infoTable[i].name = zstr_from_len(lumpName, 8);
		}

		for (int i = 0; i < lumpAmount; i++)
		{
			Log_InfoF("Lump %d: Name: %s offset %d, size %d\n",
						i,
						zstr_cstr(&infoTable[i].name),
						infoTable[i].offset,
						infoTable[i].size);
		}
	}
}
WADReader::~WADReader()
{
	delete[] infoTable;
}


DoomLumpInfo * WADReader::GetTableItem(const char* name)
{
	DoomLumpInfo* found = nullptr;
	zstr targetName = zstr_from_len(name, 8); // NOTE Must be equal length for comparison to work
	for (int i = 0; i < lumpAmount; i++)
	{
		if (zstr_cmp(&infoTable[i].name, &targetName) == 0)
		{
			found = &infoTable[i];
			break;

		}
	}
	return found;
}

