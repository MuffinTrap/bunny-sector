#include "binaryreader.h"
#include <mgdl/mgdl-util.h>
#include <mgdl/mgdl-logger.h>
#include <stdio.h>

static FILE* openFile = nullptr;
static DoomLumpInfo* activeLump;
static s8 buffer[4];
static bool readError = false;

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

void SetActiveLumpInfo(DoomLumpInfo* info)
{
    activeLump = info;
    fseek(openFile, activeLump->offset, SEEK_SET);
}

void FileRead(void* destination, sizetype dataSize, sizetype itemCount, FILE* filePtr)
{
    if (activeLump != nullptr)
    {
        sizetype cursor = ftell(openFile);
        sizetype end = activeLump->offset + activeLump->size;
        if (cursor + (dataSize*itemCount) <= end)
        {
            fread(destination, dataSize, itemCount, filePtr);
        }
        else
        {
            Log_ErrorF("Reading %d bytes past active lump\n", ((cursor + (dataSize*itemCount)) - end));
			readError = true;
        }
    }
    else
    {
        fread(destination, dataSize, itemCount, filePtr);
    }
}

bool GetReadError()
{
	return readError;
}

void ReadChars(int amount, char* arrayOut)
{
    FileRead(arrayOut, 1, amount, openFile);
}

s16 ReadInt16()
{
    FileRead(buffer, sizeof(s16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(s16));
#   endif
    s16 out = *(s16*)(buffer);
    return out;
}
u16 ReadUInt16()
{
    FileRead(buffer, sizeof(u16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(u16));
#   endif
    u16 out = *(u16*)(buffer);
    return out;
}
s32 ReadInt32()
{
    FileRead(buffer, sizeof(s32), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(s32));
#   endif
    s32 out = *(s32*)(buffer);
    return out;
}
u32 ReadUInt32()
{
    FileRead(buffer, sizeof(u32), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(u32));
#   endif
    u32 out = *(u32*)(buffer);
    return out;
}

s8  ReadSByte()
{
    FileRead(buffer, sizeof(u8), 1, openFile);
    u8 out = *(u8*)(buffer);
    return out;
}
u8  ReadByte()
{
    FileRead(buffer, sizeof(u8), 1, openFile);
    u8 out = *(u8*)(buffer);
    return out;
}

// DOOM uses these. Stored in little endian
s16 ReadSWORD()
{
    FileRead(buffer, sizeof(s16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(s16));
#   endif
    s16 out = *(s16*)(buffer);
    return out;
}

u16 ReadWORD() {
    FileRead(buffer, sizeof(u16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(u16));
#   endif
    u16 out = *(u16*)(buffer);
    return out;
}
u32 ReadDWORD() {
    FileRead(buffer, sizeof(u32), 1, openFile);
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
