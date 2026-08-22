#include "binaryreader.h"
#include <mgdl/mgdl-util.h>
#include <mgdl/mgdl-logger.h>
#include <stdio.h>

static FILE* openFile = nullptr;
static s8 buffer[4];

void StartReadingFile(FILE* fileptr)
{
    openFile = fileptr;
}

bool OpenBinary(const char* filename)
{
    openFile = fopen(filename, "rb");
    return (openFile != nullptr);
}
bool CloseBinary()
{
    if (openFile != nullptr)
    {
        fclose(openFile);
        return true;
    }
    return false;
}

s16 ReadInt16()
{
    fread(buffer, sizeof(s16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(s16));
#   endif
    s16 out = *(s16*)(buffer);
    return out;
}
u16 ReadUInt16()
{
    fread(buffer, sizeof(u16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(u16));
#   endif
    u16 out = *(u16*)(buffer);
    return out;
}
s32 ReadInt32()
{
    fread(buffer, sizeof(s32), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(s32));
#   endif
    s32 out = *(s32*)(buffer);
    return out;
}
u32 ReadUInt32()
{
    fread(buffer, sizeof(u32), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(u32));
#   endif
    u32 out = *(u32*)(buffer);
    return out;
}

s8  ReadSByte()
{
    fread(buffer, sizeof(u8), 1, openFile);
    u8 out = *(u8*)(buffer);
    return out;
}
u8  ReadByte()
{
    fread(buffer, sizeof(u8), 1, openFile);
    u8 out = *(u8*)(buffer);
    return out;
}

// DOOM uses these. Stored in little endian
s16 ReadSWORD()
{
    fread(buffer, sizeof(s16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(s16));
#   endif
    s16 out = *(s16*)(buffer);
    return out;
}

u16 ReadWORD() {
    fread(buffer, sizeof(u16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(u16));
#   endif
    u16 out = *(u16*)(buffer);
    return out;
}
u32 ReadDWORD() {
    fread(buffer, sizeof(u32), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(u32));
#   endif
    u32 out = *(u32*)(buffer);
    return out;
}
Fixed16 ReadFixed()
{
    Fixed16 f;
    f.fract = ReadSWORD();
    f.whole = ReadSWORD();
    return f;
}
