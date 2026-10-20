#pragma once
#include <mgdl/mgdl-types.h>
#include "doom-lump-info.h"
#include "doom_types.h"

// Reads binary info from a file
class BinaryReader
{
public:
	BinaryReader(const char* filename);
	~BinaryReader();
	void SetActiveLumpInfo(DoomLumpInfo* activeLump);

	s16 ReadInt16();
	s16 ReadSWORD();

	u16 ReadUInt16();
	u16 ReadWORD();

	s32 ReadInt32();

	u32 ReadUInt32();
	u32 ReadDWORD();

	s8  ReadSByte();
	u8  ReadByte();

	char ReadChar();

	bool ReadUntilString(const char* string);
	void ReadLine();
	bool LineStartsWith(const char* keyword);
	bool LineHas(const char* keyword);
	int ReadLineInt();
	float ReadLineFloat();
	bool ReadLineBool();
	s8 ReadLineByte();
	MaterialId ReadLineTextureId();
	void ReadLineTextureSize(int* wout, int* hout);
	zstr ReadLineStringBetween(char beginning, char end);

	void ReadChars(int amount, char* arrayOut);

	Fixed16 ReadFixed();
	void Seek(sizetype offsetFromStart);

	bool GetReadError();
	bool FileIsValid();
	bool IsEOF();
	const char* GetIdentifier();
private:
	bool OpenBinary(const char* filename);
	void StartReadingFile(FILE* fileptr);
	void FileRead(void* destination, sizetype dataSize, sizetype itemCount, FILE* filePtr);
	bool CloseBinary();

	FILE* openFile = nullptr;
	DoomLumpInfo* activeLump;
	s8 buffer[4];
	bool readError = false;

	const int LINEWIDTH = 128;
	char* lineBuffer;
	const int IDWIDTH = 32;
	char* identifierBuffer;
	const int TEXTURENAMEWIDTH = 32;
	char* textureNameBuffer;
};
