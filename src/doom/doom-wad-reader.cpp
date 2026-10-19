
#include "../bunny-sector-types.h"
#include "doom_types.h"
#include "binaryreader.h"
#include "doom-wad-reader.h"
#include <mgdl.h>


WADReader::WADReader(BinaryReader* reader)
{
	// Read the header
	if (reader != nullptr)
	{
		char identification[4];
		reader->ReadChars(4, identification); // PWAD or IWAD
		zstr idstr = zstr_from_len(identification, 4);
		lumpAmount = reader->ReadUInt32();
		infoTableOffset = reader->ReadUInt32();

		Log_InfoF("Wad file.Id %s Lumps %d Infotable at %d\n", zstr_cstr(&idstr), lumpAmount, infoTableOffset);

		// Read the infotable
		// Each table entry is 16 bytes
		char lumpName[8];
		infoTable = new DoomLumpInfo[lumpAmount];
		for (int i = 0; i < lumpAmount; i++)
		{
			reader->Seek(infoTableOffset + i * 16);
			infoTable[i].offset = reader->ReadUInt32();
			infoTable[i].size = reader->ReadUInt32();
			reader->ReadChars(8, lumpName);
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


DoomLumpInfo * WADReader::FindTableItem(const char* name)
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

