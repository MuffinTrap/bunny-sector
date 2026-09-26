#include "doom-map-reader.h"
#include "../bunny-sector_main.h"
#include "../map/binaryreader.h"
#include <stdio.h>

static FILE* mapfile = nullptr;

static int thingAmount = 0;
static int vertexAmount = 0;
static int linedefAmount = 0;
static int sidedefAmount = 0;
static int sectorAmount = 0;

static DoomMap* map;


// UNITS
// 16 horizontal Doom units = 10 vertical Doom units = 1 foot = 0.6 meters.
// https://doomwiki.org/wiki/Map_unit

#define ENTITY_START "{"
#define ENTITY_END "}"

// String constants for all the identifiers to avoid typos
#define X "x"
#define Y "y"
#define ID "id"
#define V1 "v1"
#define V2 "v2"
#define SPECIAL "special"
#define ARG0 "arg0"
#define ARG1 "arg1"
#define ARG2 "arg2"
#define ARG3 "arg3"
#define ARG4 "arg4"
#define SIDEFRONT "sidefront"
#define SIDEBACK "sideback"
#define OFFSETX "offsetx"
#define OFFSETY "offsety"
#define TEXTURETOP "texturetop"
#define TEXTUREBOTTOM "texturebottom"
#define TEXTUREMIDDLE "texturemiddle"
#define SECTOR "sector"
#define HEIGHTFLOOR "heightfloor"
#define HEIGHTCEILING "heightceiling"
#define TEXTUREFLOOR "texturefloor"
#define TEXTURECEILING "textureceiling"
#define LIGHTLEVEL "lightlevel"
#define HEIGHT "height"
#define ANGLE "angle"
#define TYPE "type"

// Linedef flags
#define BLOCKING "blocking"
#define BLOCKMONSTERS "blockmonsters"
#define TWOSIDED "twosided"
#define DONTPEGTOP "dontpegtop"
#define DONTPEGBOTTOM "dontpegbottom"
#define SECRET "secret"
#define BLOCKSOUND "blocksound"
#define DONTDRAW "dontdraw"
#define MAPPED "mapped"
#define PLAYERCROSS "playercross"

// Thing flags
#define SKILL1 "skill1"
#define SKILL2 "skill2"
#define SKILL3 "skill3"
#define SKILL4 "skill4"
#define AMBUSH "ambush"
#define SINGLE "single"
#define DM "dm"
#define COOP "coop"
#define FRIEND "friend"

static void RewindToFileStart(FILE* fileptr)
{
	rewind(fileptr);
}


static bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

static char advance()
{
	char c = fgetc(mapfile);
	return c;
}

static char peek()
{
	return fgetc(mapfile);
}

static bool match(char expected)
{
	if (advance() == expected)
	{
		return true;
	}
	else
	{
		fseek(mapfile, ftell(mapfile)-1, SEEK_SET);
		return false;
	}
}

static int read_int()
{

	return 0;

}

static float read_float()
{

	return 0.0f;
}

static zstr read_str()
{

	int start = ftell(mapfile);
	while( peek() != '"' && !feof(mapfile))
	{
		advance();
	}
	advance(); // closing "

	int amount = ftell(mapfile) - start;
	char* str = (char*)mgdl_AllocateGeneralMemory((amount + 1) * sizeof(char));

	int readamount = fread(str, 1, amount, mapfile);
	str[readamount] = '\0';
	zstr s = zstr_from(str);
	mgdl_FreeGeneralMemory(str);

	return s;
}


static const int LINEWIDTH = 128;
static char* lineBuffer;
static const int IDWIDTH = 32;
static char* identifierBuffer;
static const int TEXTURENAMEWIDTH = 32;
static char* textureNameBuffer;

static bool line_startswith(const char* keyword)
{
	char* found_thing = strstr(lineBuffer, keyword);
	// The thing that was found and was the first thing
	return (found_thing != NULL && found_thing == lineBuffer);
}

static bool line_has(const char* keyword)
{

	char* found_thing = strstr(lineBuffer, keyword);
	// The thing that was found
	return (found_thing != NULL);
}
static void read_line()
{
	fgets(lineBuffer, LINEWIDTH, mapfile);
}
static bool read_chars_until(const char* keyword)
{
	printf("Reading chars until %s\n", keyword);
	bool allfound = false;
	while(feof(mapfile) == false)
	{
		char c = advance();
		if (c == keyword[0])
		{
			printf("Found first: %c at %ld\n", c, ftell(mapfile));
			int wlen = strlen(keyword);
			int found = 1;
			for (int i = 1; i < wlen; i++)
			{
				c = advance();
				if (c == keyword[i])
				{
					printf("Found %d/%d : %c\n", i ,wlen, c);
					found++;
					if (found == wlen)
					{
						allfound = true;
						printf("Found the keyword\n");
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
static void read_lines_until(const char* keyword)
{
	while(feof(mapfile) == false)
	{
		read_line();
		if (line_has(keyword))
		{
			break;
		}
	}
}

static Vector2 readVector2()
{
	float x, y;
	fscanf(mapfile, "x = %f;", &x);
	fscanf(mapfile, "y = %f;", &y);
	return Vector2New(x, y);
}
static int readInt()
{
	int i;
	sscanf(lineBuffer, "%s = %d;", identifierBuffer, &i);
	return i;
}
static float readFloat()
{
	float i;
	sscanf(lineBuffer, "%s = %f;", identifierBuffer, &i);
	return i;
}
static bool readBool()
{
	return line_has("true");
}
static s8 readByte()
{
	int i;
	sscanf(lineBuffer, "%s = %d;", identifierBuffer, &i);
	return (s8)i;
}

static MaterialId readTextureId()
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
	printf("Doom map has texture %s mapped to id %d\n", zstr_cstr(&textureName), tid);
	zstr_free(&textureName);
	return tid;
}

static void read_thing() {

	read_line();
	if (line_has("{"))
	{
		DoomThing* t = &map->things[thingAmount];
		DoomThing_Init(t);
		while (true)
		{
			read_line();
			if (line_startswith(X))
			{
				t->x = readFloat();
			}
			else if (line_startswith(Y))
			{
				t->y = readFloat();
			}
			else if (line_startswith(ID))
			{
				t->id = readInt();
			}
			else if (line_startswith(TYPE))
			{
				t->type = readInt();
			}
			else if (line_startswith(HEIGHT))
			{
				t->height = readFloat();
			}
			else if (line_startswith(ANGLE))
			{
				t->angleDeg = readFloat();
			}
			else if (line_startswith(SPECIAL))
			{
				t->special = readInt();
			}
			else if (line_startswith(ARG0))
			{
				t->arg0 = readByte();
			}
			else if (line_startswith(ARG1))
			{
				t->arg1 = readByte();
			}
			else if (line_startswith(ARG2))
			{
				t->arg2 = readByte();
			}
			else if (line_startswith(ARG3))
			{
				t->arg3 = readByte();
			}
			else if (line_startswith(ARG4))
			{
				t->arg4 = readByte();
			}

			// Flags

			else if (line_startswith(SKILL1))
			{
				if (readBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_skill1);
				}
			}
			else if (line_startswith(SKILL2))
			{
				if (readBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_skill2);
				}
			}
			else if (line_startswith(SKILL3))
			{
				if (readBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_skill3);
				}
			}
			else if (line_startswith(SKILL4))
			{
				if (readBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_skill4);
				}
			}
			else if (line_startswith(AMBUSH))
			{
				if (readBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_ambush);
				}
			}
			else if (line_startswith(SINGLE))
			{
				if (readBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_single);
				}
			}
			else if (line_startswith(DM))
			{
				if (readBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_dm);
				}
			}
			else if (line_startswith(COOP))
			{
				if (readBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_coop);
				}
			}
			else if (line_startswith(FRIEND))
			{
				if (readBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_friend);
				}
			}

			else if (line_has("}"))
			{
				break;
			}
		}
		thingAmount += 1;
	}
}
static void read_vertex() {

	read_line();
	if (line_has("{"))
	{
		DoomVertex* t = &map->vertices[vertexAmount];
		while (true)
		{
			read_line();
			if (line_startswith(X))
			{
				t->x = readFloat();
			}
			else if (line_startswith(Y))
			{
				t->y = readFloat();
			}
			else if (line_has("}"))
			{
				break;
			}
		}
		vertexAmount += 1;
	}
}

static void read_linedef() {

	read_line();
	if (line_has("{"))
	{
		DoomLinedef* t = &map->linedefs[linedefAmount];
		DoomLinedef_Init(t);
		while (true)
		{
			read_line();
			if (line_startswith(ID))
			{
				t->id = readInt();
			}
			if (line_startswith(V1))
			{
				t->v1 = readInt();
			}
			else if (line_startswith(V2))
			{
				t->v2 = readInt();
			}
			else if (line_startswith(SPECIAL))
			{
				t->special = readInt();
			}
			else if (line_startswith(ARG0))
			{
				t->arg0 = readByte();
			}
			else if (line_startswith(ARG1))
			{
				t->arg1 = readByte();
			}
			else if (line_startswith(ARG2))
			{
				t->arg2 = readByte();
			}
			else if (line_startswith(ARG3))
			{
				t->arg3 = readByte();
			}
			else if (line_startswith(ARG4))
			{
				t->arg4 = readByte();
			}
			else if (line_startswith(SIDEFRONT))
			{
				t->sidefront = readInt();
			}
			else if (line_startswith(SIDEBACK))
			{
				t->sideback = readInt();
			}
			else if (line_startswith(TWOSIDED))
			{
				if (readBool())
				{
					t->linedef_flags = Flag_SetBit(t->linedef_flags, linedef_twosided);
				}
			}
			else if (line_startswith(PLAYERCROSS))
			{
				if (readBool())
				{
					t->linedef_flags = Flag_SetBit(t->linedef_flags, linedef_activate_player_cross);
				}
			}
			else if (line_has("}"))
			{
				break;
			}
		}
		linedefAmount += 1;
	}
}
static void read_sidedef()
{
	read_line();
	if (line_has("{"))
	{
		DoomSidedef* t = &map->sidedefs[sidedefAmount];
		DoomSidedef_Init(t);
		while (true)
		{
			read_line();
			if (line_startswith(OFFSETX))
			{
				t->offsetx = readInt();
			}
			if (line_startswith(OFFSETY))
			{
				t->offsety = readInt();
			}
			else if (line_startswith(TEXTURETOP))
			{
				t->texturetop = readTextureId();
			}
			else if (line_startswith(TEXTUREBOTTOM))
			{
				t->texturebottom = readTextureId();
			}
			else if (line_startswith(TEXTUREMIDDLE))
			{
				t->texturemiddle = readTextureId();
			}
			else if (line_startswith(SECTOR))
			{
				t->sector = readInt();
			}
			else if (line_has("}"))
			{
				break;
			}
		}
		sidedefAmount += 1;
	}
}
static void read_sector() {
	read_line();
	if (line_has("{"))
	{
		DoomSector* t = &map->sectors[sectorAmount];
		DoomSector_Init(t);
		while (true)
		{
			read_line();
			if (line_startswith(HEIGHTFLOOR))
			{
				t->heightfloor = readInt();
			}
			if (line_startswith(HEIGHTCEILING))
			{
				t->heightceiling = readInt();
			}
			else if (line_startswith(TEXTUREFLOOR))
			{
				t->texturefloor = readTextureId();
			}
			else if (line_startswith(TEXTURECEILING))
			{
				t->textureceiling = readTextureId();
			}
			else if (line_startswith(LIGHTLEVEL))
			{
				t->lightlevel = readByte();
			}
			else if (line_startswith(SPECIAL))
			{
				t->special = readInt();
			}
			else if (line_startswith(ID))
			{
				t->id = readInt();
			}
			else if (line_has("}"))
			{
				break;
			}
		}
		sectorAmount += 1;
	}

}

static int INVALID_TARGET_HEIGHT = 0xffff;

static int FindDoorOpenHeight(DoomMap* map, int sectorIndex)
{
	int targetHeight = INVALID_TARGET_HEIGHT;
	for (int li = 0; li < map->lineAmount; li++)
	{
		DoomSector* neighbor  = nullptr;
		DoomLinedef* line = &map->linedefs[li];
		if (Flag_IsBitSet(line->linedef_flags, linedef_twosided))
		{
			DoomSidedef* front = &map->sidedefs[line->sidefront];
			DoomSidedef* back = &map->sidedefs[line->sideback];
			if (front->sector == sectorIndex)
			{
				neighbor = &map->sectors[back->sector];
			}
			else if (back->sector == sectorIndex)
			{
				neighbor = &map->sectors[front->sector];
			}
		}

		if (neighbor && neighbor->heightceiling < targetHeight )
		{
			targetHeight = neighbor->heightceiling;
		}
	}
	return targetHeight;
}

    BunnySector_Map* Doom_ReadMapFromFile(const char* mapfilename)
	{
		mapfile = fopen(mapfilename, "r");
		if (mapfile == NULL)
		{
			return nullptr;
		}

		BunnySector_Map* BunnyMap = nullptr;

		lineBuffer = (char*)mgdl_AllocateGeneralMemory((LINEWIDTH + 1) * sizeof(char));
		identifierBuffer = (char*)mgdl_AllocateGeneralMemory((IDWIDTH + 1) * sizeof(char));
		textureNameBuffer = (char*)mgdl_AllocateGeneralMemory((TEXTURENAMEWIDTH + 1) * sizeof(char));


		StartReadingFile(mapfile);
		// Parsing
		bool counting = true;

		// Read two times. First for counting and then for reading
		while(true)
		{
			// On first pass need to calculate amounts for allocation
			// Second pass these are indices

			thingAmount = 0;
			vertexAmount = 0;
			linedefAmount = 0;
			sidedefAmount = 0;
			sectorAmount = 0;

			// Start from top
			rewind(mapfile);
			while(feof(mapfile) == false)
			{
				read_line();
				// look for keywords
				if (line_startswith("thing"))
				{
					// Start reading thing
					if (counting)
					{
						thingAmount += 1;
						read_lines_until(ENTITY_END);
					}
					else
					{
						read_thing();
					}
				}
				else if (line_startswith("vertex"))
				{
					if (counting)
					{
						vertexAmount += 1;
						read_lines_until(ENTITY_END);
					}
					else
					{
						read_vertex();
					}
				}
				else if (line_startswith("linedef"))
				{
					if (counting)
					{
						linedefAmount += 1;
						read_lines_until(ENTITY_END);
					}
					else
					{
						read_linedef();
					}
				}
				else if (line_startswith("sidedef"))
				{
					if (counting)
					{
						sidedefAmount += 1;
						read_lines_until(ENTITY_END);
					}
					else
					{
						read_sidedef();
					}
				}
				else if (line_startswith("sector"))
				{
					if (counting)
					{
						sectorAmount += 1;
						read_lines_until(ENTITY_END);
					}
					else
					{
						read_sector();
					}
				}
			}

			// File was read
			if (counting)
			{
				printf("Counting done\n");
				// Read additional vertex amount for nodes
				rewind(mapfile);
				printf("Cursor at %ld\n", ftell(mapfile));
				read_chars_until("XGLN");
				u32 OrgVerts = ReadDWORD();
				u32 NewVertes = ReadDWORD();
				printf("Adding %d new vertices for nodes\n", NewVertes);

				BunnyMap = new DoomMap();// (BunnySector_Map*)mgdl_AllocateGeneralMemory(sizeof(BunnySector_Map));
				map = (DoomMap*)BunnyMap;

				DoomMap_Allocate(map, thingAmount, sectorAmount, sidedefAmount, linedefAmount, vertexAmount + NewVertes);

				counting = false;
			}

			else
			{
				printf("Reading done\n");
				printf("Cursor at %ld\n", ftell(mapfile));
				// All data read
				break;
			}
		} // Counting and reading done


		rewind(mapfile);

		printf("Before nodes Cursor at %ld\n", ftell(mapfile));
		// Read the nodes
		// TODO read bit by bit
		read_chars_until("XGLN");
		u32 OrgVerts = ReadDWORD();
		printf("OrgVerts %d\n", OrgVerts);
		u32 NewVertes = ReadDWORD();

		printf("NewVerts %d \n", NewVertes);
		for (int ni = 0; ni < NewVertes; ni++)
		{
			// TODO Fixed integers
			Fixed16 x = ReadFixed();
			Fixed16 y = ReadFixed();

			printf("Vert %d at (%d.%d, %d.%d)\n", ni, x.whole, x.fract,  y.whole, y.fract);
			map->vertices[vertexAmount].x = x.whole;
			map->vertices[vertexAmount].y = y.whole;
			vertexAmount++;
		}
		u32 NumSubsectors = ReadDWORD();
		DoomMap_AllocateSubsectors(map, NumSubsectors);
		printf("NumSubsectors %d\n", NumSubsectors);
		int segmentsRead = 0;
		for (int ni = 0; ni < NumSubsectors; ni++)
		{
			u32 NumSegs = ReadDWORD();
			map->subsectors[ni].firstSegment = segmentsRead;;
			map->subsectors[ni].segmentAmount = NumSegs;
			printf("Subsector %d Segments; %d : First segment %d\n", ni, NumSegs, segmentsRead);
			segmentsRead += NumSegs;
		}

		u32 NumSegs = ReadDWORD();
		mgdl_assert_test(NumSegs == segmentsRead);
		printf("NumSeg %d\n", NumSegs);
		DoomMap_AllocateSegments(map, NumSegs);
		for (int si = 0; si < map->segmentAmount; si++)
		{
			u32 v1 = ReadDWORD(); // Vertex index
			u32 partner = ReadDWORD(); // Segment index
			u16 line = ReadWORD(); // Linedef index
			u8 side = ReadByte(); // Linedef side
			printf("Seg %d: V1 %d partner %d line %d side %d\n", si, v1, partner, line, side);

			map->segments[si].v1 = v1;
			map->segments[si].partnerSegment = partner;
			map->segments[si].linedef = line;
			map->segments[si].lineSide = side;
		}

		u32 NumNodes = ReadDWORD();
		printf("NumNodes : %d\n", NumNodes);
		DoomMap_AllocateNodes(map, NumNodes);
		for (int ni = 0; ni < NumNodes; ni++)
		{
			printf("Node %d\n", ni);
			// The line that splits the node
			s16 x = ReadSWORD();
			s16 y = ReadSWORD();
			s16 dx = ReadSWORD();
			s16 dy = ReadSWORD();

			// Child 0 bounding box
			s16 Top0 = ReadSWORD();
			s16 Bottom0 = ReadSWORD();
			s16 Left0 = ReadSWORD();
			s16 Right0 = ReadSWORD();
			// Child 1 bounding box
			s16 Top1 = ReadSWORD();
			s16 Bottom1 = ReadSWORD();
			s16 Left1 = ReadSWORD();
			s16 Right1 = ReadSWORD();

			// Bit 31 : 1 subsector
			// Bit 31 : 0 node
			u32 child0 = ReadDWORD();
			u32 child1 = ReadDWORD();

			printf("Child 0 %08.8x ", child0);
			if (child0 & 0x80000000)
			{
				printf("is subsector: %d\n", child0 & 0x7FFFFFFF);
			}
			else
			{
				printf("is node: %d\n", child0);
			}
			printf("Bounding box %d %d %d %d\n", Top0, Bottom0, Left0, Right0);

			printf("Child 1 %08.8x ", child1);
			if (child1 & 0x80000000)
			{
				printf("is subsector: %d\n", child1 & 0x7FFFFFFF);
			}
			else
			{
				printf("is node: %d\n", child1);
			}
			printf("Bounding box %d %d %d %d\n", Top1, Bottom1, Left1, Right1);
			DoomNode* n = &map->nodes[ni];
			n->x = x;
			n->y = y;
			n->dx = dx;
			n->dy = dy;

			n->bbox0[BB_TOP] = Top0;
			n->bbox0[BB_BOT] = Bottom0;
			n->bbox0[BB_LFT] = Left0;
			n->bbox0[BB_RGT] = Right0;
			// Child 1 bounding box
			n->bbox1[BB_TOP] = Top1;
			n->bbox1[BB_BOT] = Bottom1;
			n->bbox1[BB_LFT] = Left1;
			n->bbox1[BB_RGT] = Right1;

			// Bit 31 : 1 subsector
			// Bit 31 : 0 node
			n->children[0] = child0;
			n->children[1] = child1;
		}

		fclose(mapfile);

		mgdl_FreeGeneralMemory(lineBuffer);
		mgdl_FreeGeneralMemory(identifierBuffer);
		mgdl_FreeGeneralMemory(textureNameBuffer);

		// NOTE DANGER
		// SLADE stores maps with Y increasing up.
		// so we need to rotate all coordinates by 180 degrees
		bool FLIP_THE_Y = true;
		if (FLIP_THE_Y)
		{
			float rotation = 180.0f;
			for (int i = 0; i < map->thingAmount; i++)
			{
				map->things[i].angleDeg += rotation;
				map->things[i].y *= -1.0f;
			}
			for (int i = 0; i < map->vertexAmount; i++)
			{
				map->vertices[i].y *= -1.0f;
			}

			for (int ni = 0; ni < NumNodes; ni++)
			{
				// Bounding box top and bottom change places
				int btop0 = map->nodes[ni].bbox0[BB_TOP];
				int bbot0 = map->nodes[ni].bbox0[BB_BOT];
				map->nodes[ni].bbox0[BB_TOP] = bbot0 * -1;
				map->nodes[ni].bbox0[BB_BOT] = btop0 * -1;

				int btop1 = map->nodes[ni].bbox1[BB_TOP];
				int bbot1 = map->nodes[ni].bbox1[BB_BOT];
				map->nodes[ni].bbox1[BB_TOP] = bbot1 * -1;
				map->nodes[ni].bbox1[BB_BOT] = btop1 * -1;

				// Divider is flipped
				map->nodes[ni].dy *= -1.0f;
				map->nodes[ni].y *= -1.0f;
			}
		}


		// Link sectors to subsectors directly
		for (int ssi = 0; ssi < map->subSectorAmount; ssi++)
		{
			DoomSubSector* sub = &map->subsectors[ssi];
			bool sectorSet = false;
			for (int segi = 0; segi < sub->segmentAmount; segi++)
			{
				DoomSegment *seg = &map->segments[sub->firstSegment + segi];
				// NOTE Only looking at the first is incorrect, since portals can point either way
				DoomLinedef *linedef = &map->linedefs[seg->linedef];
				bool isPortal = (linedef->sidefront >= 0 && linedef->sideback >=0);

				// Find first that is not portal
				if (!isPortal)
				{
					if (seg->lineSide == DOOM_SIDE_FRONT)
					{
						DoomSidedef *sidedef = &map->sidedefs[linedef->sidefront];
						sub->sector = sidedef->sector;
						sectorSet = true;
						break;
					}
				}
			}
			if (sectorSet == false)
			{
				// Was only portals. This is a pit or pedestral. All portals should have one side in common
				// Take first two and find the common sector
				DoomSegment *seg_0 = &map->segments[sub->firstSegment + 0];
				DoomSegment *seg_1 = &map->segments[sub->firstSegment + 1];
				DoomLinedef *linedef_0 = &map->linedefs[seg_0->linedef];
				DoomLinedef *linedef_1 = &map->linedefs[seg_1->linedef];
				DoomSidedef *sidedef_0f = &map->sidedefs[linedef_0->sidefront];
				DoomSidedef *sidedef_0b = &map->sidedefs[linedef_0->sideback];
				DoomSidedef *sidedef_1f = &map->sidedefs[linedef_1->sidefront];
				DoomSidedef *sidedef_1b = &map->sidedefs[linedef_1->sideback];
				if (sidedef_0f->sector == sidedef_1f->sector)
				{
					sub->sector = sidedef_0f->sector;
				}
				else if (sidedef_0b->sector == sidedef_1b->sector)
				{
					sub->sector = sidedef_0b->sector;
				}
			}
			printf("Subsector %d linked to sector %d\n", ssi, sub->sector);
		}

		// Find neighbourSector values
		for (int ssi = 0; ssi < map->subSectorAmount; ssi++)
		{
			DoomSubSector* sub = &map->subsectors[ssi];
			for (int segi = 0; segi < sub->segmentAmount; segi++)
			{
				DoomSegment *seg = &map->segments[sub->firstSegment + segi];
				if (seg->linedef != DOOM_INVALID_LINEDEF) // Segments that were built for nodes don't have line
				{
					DoomLinedef *linedef = &map->linedefs[seg->linedef];
					// If this segment is from the front side of a line
					// then back side sector is neighbour
					if (seg->lineSide == DOOM_SIDE_FRONT)
					{
						if (linedef->sideback >= 0)
						{
							DoomSidedef *sidedef_back = &map->sidedefs[linedef->sideback];
							seg->neighbourSector = sidedef_back->sector;
						}
						else
						{
							seg->neighbourSector = -1;
						}
					}
					else if (seg->lineSide == DOOM_SIDE_BACK)
					{
						if (linedef->sidefront >= 0)
						{
							DoomSidedef *sidedef_front = &map->sidedefs[linedef->sidefront];
							seg->neighbourSector = sidedef_front->sector;
						}
						else
						{
							seg->neighbourSector = -1;
						}
					}
				}
				else
				{
					// Must be portal somewhere?
					seg->neighbourSector = -1;//DOOM_NO_LINE_NEIGHBOR;// TODO Figure this out
				}

				printf("Subsector %d segment %d neighbor is %d\n", ssi, sub->firstSegment + segi, seg->neighbourSector);
			}
		}

		// TODO
		// for sectors that are doors, calculate the ceilingheight for when the door is open
		// find all doors
		for (int li = 0; li < map->lineAmount; li++ )
		{
			DoomLinedef* line = &map->linedefs[li];
			if (line->special >= 0)
			{
				if (line->special == special_door_close || line->special == special_door_open || line->special == special_door_raise)
				{
					int doorSectorTag = line->arg0;

					if (doorSectorTag == 0)
					{
						// Backside
						DoomSidedef* back = &map->sidedefs[line->sideback];
						DoomSector* sector = &map->sectors[back->sector];
						int openHeight = FindDoorOpenHeight(map, back->sector);
						if (openHeight != INVALID_TARGET_HEIGHT)
						{
							sector->doorOpenHeight = openHeight - 4;
							sector->usecase = sector_door;
						}
					}
					else
					{
						// Find all sectors with this tag
						for (int si = 0; si < sectorAmount; si++)
						{
							DoomSector* sector = &map->sectors[si];
							if (sector->id == doorSectorTag)
							{
								int openHeight = FindDoorOpenHeight(map, si);
								if (openHeight != INVALID_TARGET_HEIGHT)
								{
									sector->doorOpenHeight = openHeight - 4;
									sector->usecase = sector_door;
								}
							}
						}
					}
				}
			}
		}

		// Calculate extra information needed by tesselation
    map->lowY = 35665;
    map->highY = -36665;

    // Build other information needed
    // Build other data needed by game
    for (int si = 0; si < BunnyMap->GetSectorAmount(); si++)
    {
        Vector2 minp = Vector2New(32000, 32000);
        Vector2 maxp = Vector2New(-32000, -32000);
        int sectorwallnum = BunnyMap->GetWallAmountInSector(si);
        for (s16 wi = 0; wi < sectorwallnum; wi++)
        {
            Vector2 w = BunnyMap->GetWallVertexInSector(si, wi);
            minp.x = minF(w.x, minp.x);
            minp.y = minF(w.y, minp.y);
            maxp.x = maxF(w.x, maxp.x);
            maxp.y = maxF(w.y, maxp.y);
        }
        // Found points : calculate tex coords
        float width = (maxp.x - minp.x);
        float height = (maxp.y - minp.y);
        float aspect = width/height;

		DoomSubSector* subs = &map->subsectors[si];
        subs->minXZPoint = minp;
        subs->sizeXZ = Vector2Subtract(maxp, minp);
        subs->maxTexCoord.x = aspect * height;
        subs->maxTexCoord.y = 1.0 * height;
		DoomSector* sector = &map->sectors[subs->sector];
        if (sector->heightfloor < map->lowY)
        {
            map->lowY = sector->heightfloor;
        }
        if (sector->heightceiling > map->highY)
        {
            map->highY = sector->heightceiling;
        }
    }



		return BunnyMap;
	}

	BunnySector_Map * Doom_ReadMapFromFile(const zstr& mapfilename)
	{
		return Doom_ReadMapFromFile(zstr_cstr(&mapfilename));
	}

