#pragma once
#include "dukemap.h"
#include <mgdl/mgdl-types.h>
#include "../doom/doom_types.h"

// Reads binary info from a file
#ifdef __cplusplus
extern "C" {
#endif


bool OpenBinary(const char* filename);
void StartReadingFile(FILE* fileptr);

s16 ReadInt16();
s16 ReadSWORD();

u16 ReadUInt16();
u16 ReadWORD();

s32 ReadInt32();

u32 ReadUInt32();
u32 ReadDWORD();

s8  ReadSByte();
u8  ReadByte();

Fixed16 ReadFixed();

bool CloseBinary();

#ifdef __cplusplus
}
#endif
