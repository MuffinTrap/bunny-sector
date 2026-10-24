#include "binaryreader.h"
#include <mgdl/mgdl-util.h>
#include <mgdl/mgdl-logger.h>
#include <stdio.h>
#include "../bunny-sector_main.h"


BinaryReader::BinaryReader(const char* filename)
{
    activeLump = nullptr;
    readError = false;
    openFile = nullptr;
    lineBuffer = nullptr;
    identifierBuffer = nullptr;
    textureNameBuffer = nullptr;
    if (OpenBinary(filename))
    {
		lineBuffer = (char*)mgdl_AllocateGeneralMemory((LINEWIDTH + 1) * sizeof(char));
		identifierBuffer = (char*)mgdl_AllocateGeneralMemory((IDWIDTH + 1) * sizeof(char));
		textureNameBuffer = (char*)mgdl_AllocateGeneralMemory((TEXTURENAMEWIDTH + 1) * sizeof(char));
    }
    else
    {
        Log_ErrorF("Binary reader could not open file %s\n", filename);
    }
}

BinaryReader::~BinaryReader()
{
    if (openFile != nullptr)
    {
        CloseBinary();
		mgdl_FreeGeneralMemory(lineBuffer);
		mgdl_FreeGeneralMemory(identifierBuffer);
		mgdl_FreeGeneralMemory(textureNameBuffer);
    }
}
bool BinaryReader::FileIsValid()
{
    return openFile != nullptr;
}


void BinaryReader::Seek(sizetype offsetFromStart)
{
    fseek(openFile, offsetFromStart, SEEK_SET);
}



bool BinaryReader::OpenBinary(const char* filename)
{
    openFile = fopen(filename, "rb");
    return (openFile != nullptr);
}
bool BinaryReader::CloseBinary()
{
    if (openFile != nullptr)
    {
        fclose(openFile);
        return true;
    }
    return false;
}

void BinaryReader::SetActiveLumpInfo(DoomLumpInfo* info)
{
    activeLump = info;
    Seek(activeLump->offset);
    readError = false;
}

void BinaryReader::FileRead(void* destination, sizetype dataSize, sizetype itemCount, FILE* filePtr)
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

bool BinaryReader::GetReadError()
{
	return readError;
}

bool BinaryReader::IsEOF()
{
    return feof(openFile) > 0;
}

const char * BinaryReader::GetIdentifier()
{
    return identifierBuffer;
}


void BinaryReader::ReadLine()
{
    fgets(lineBuffer, LINEWIDTH, openFile);
    if (ftell(openFile) > activeLump->offset + activeLump->size)
    {
        readError = true;
    }
}
bool BinaryReader::LineHas(const char* keyword)
{
	char* found_thing = strstr(lineBuffer, keyword);
	// The thing that was found
	return (found_thing != NULL);
}
int BinaryReader::ReadLineInt()
{
	int i;
	sscanf(lineBuffer, "%s = %d;", identifierBuffer, &i);
	return i;
}
float BinaryReader::ReadLineFloat()
{
	float i;
	sscanf(lineBuffer, "%s = %f;", identifierBuffer, &i);
	return i;
}

bool BinaryReader::ReadLineBool()
{
	return LineHas("true");
}
s8 BinaryReader::ReadLineByte()
{
	int i;
	sscanf(lineBuffer, "%s = %d;", identifierBuffer, &i);
	return (s8)i;
}

MaterialId BinaryReader::ReadLineTextureId()
{
	sscanf(lineBuffer, "%s = \"%s\";", identifierBuffer, textureNameBuffer);

	// Find the point where is "
	int index = -1;
	for (int i = 0; i < TEXTURENAMEWIDTH; i++)
	{
		if (textureNameBuffer[i] == '"')
		{
			index = i;
			break;
		}
	}

	zstr textureName = zstr_from_len(textureNameBuffer, index);
	MaterialId tid = BunnySector_GetMaterialId(&textureName);
	Log_InfoF("Doom map has texture %s mapped to id %d\n", zstr_cstr(&textureName), tid);
	zstr_free(&textureName);
	return tid;
}
void BinaryReader::ReadLineTextureSize(int* wout, int* hout)
{
	sscanf(lineBuffer, "Texture %*8s, %3d, %3d;", wout, hout);
}

zstr BinaryReader::ReadLineStringBetween(char beginning, char end)
{
    int startIndex = -1;
    int endIndex = -1;
    for (int i = 0; i < LINEWIDTH; i++)
    {
        if (startIndex < 0 && lineBuffer[i] == beginning)
        {
            startIndex = i;
            continue;
        }
        if (endIndex < 0 && lineBuffer[i] == end)
        {
            endIndex = i;
            break;
        }
    }
    if (startIndex >= 0 && endIndex > startIndex)
    {
        zstr_view lineView = zstr_view_from(lineBuffer);
        zstr_view identiefierView = zstr_sub(lineView, startIndex + 1, endIndex - startIndex - 1);
        return zstr_from_view(identiefierView);
    }
    return zstr_init();
}





bool BinaryReader::LineStartsWith(const char* keyword)
{
	char* found_thing = strstr(lineBuffer, keyword);
	// The thing that was found and was the first thing
	return (found_thing != NULL && found_thing == lineBuffer);
}

void BinaryReader::ReadChars(int amount, char* arrayOut)
{
    FileRead(arrayOut, sizeof(char), amount, openFile);
}

s16 BinaryReader::ReadInt16()
{
    FileRead(buffer, sizeof(s16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(s16));
#   endif
    s16 out = *(s16*)(buffer);
    return out;
}
u16 BinaryReader::ReadUInt16()
{
    FileRead(buffer, sizeof(u16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(u16));
#   endif
    u16 out = *(u16*)(buffer);
    return out;
}
s32 BinaryReader::ReadInt32()
{
    FileRead(buffer, sizeof(s32), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(s32));
#   endif
    s32 out = *(s32*)(buffer);
    return out;
}
u32 BinaryReader::ReadUInt32()
{
    FileRead(buffer, sizeof(u32), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(u32));
#   endif
    u32 out = *(u32*)(buffer);
    return out;
}

s8  BinaryReader::ReadSByte()
{
    FileRead(buffer, sizeof(u8), 1, openFile);
    u8 out = *(u8*)(buffer);
    return out;
}
u8  BinaryReader::ReadByte()
{
    FileRead(buffer, sizeof(u8), 1, openFile);
    u8 out = *(u8*)(buffer);
    return out;
}

// DOOM uses these. Stored in little endian
s16 BinaryReader::ReadSWORD()
{
    FileRead(buffer, sizeof(s16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(s16));
#   endif
    s16 out = *(s16*)(buffer);
    return out;
}

u16 BinaryReader::ReadWORD() {
    FileRead(buffer, sizeof(u16), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(u16));
#   endif
    u16 out = *(u16*)(buffer);
    return out;
}
u32 BinaryReader::ReadDWORD() {
    FileRead(buffer, sizeof(u32), 1, openFile);
#   ifdef GEKKO
    RevBytes(buffer, sizeof(u32));
#   endif
    u32 out = *(u32*)(buffer);
    return out;
}
Fixed16 BinaryReader::ReadFixed()
{
    Fixed16 f;
    f.fract = ReadSWORD();
    f.whole = ReadSWORD();
    return f;
}

char BinaryReader::ReadChar()
{
    FileRead(buffer, sizeof(char), 1, openFile);
    return *(char*)buffer;
}

bool BinaryReader::ReadUntilString(const char* string)
{
	bool allfound = false;
	while(IsEOF() == false)
	{
		char c = ReadChar();
		if (c == string[0])
		{
			int wlen = strlen(string);
			int found = 1;
			for (int i = 1; i < wlen; i++)
			{
				c = ReadChar();
				if (c == string[i])
				{
					Log_InfoF("Found %d/%d : %c\n", i ,wlen, c);
					found++;
					if (found == wlen)
					{
						allfound = true;
						Log_InfoF("Found the keyword\n");
						break;
					}
				}
				else
				{
					break;
				}
			}
		}
		if (allfound)
		{
			break;
		}
	}
	return allfound;
}


