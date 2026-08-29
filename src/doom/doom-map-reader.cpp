#include "doom-map-reader.h"
#include "../bunny-sector_main.h"
#include "../duke/binaryreader.h"
#include <stdio.h>

static FILE* mapfile = nullptr;

static int thingAmount = 0;
static int vertexAmount = 0;
static int linedefAmount = 0;
static int sidedefAmount = 0;
static int sectorAmount = 0;

static DoomMap* map;



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
	s8 i;
	sscanf(lineBuffer, "%s = %c;", identifierBuffer, &i);
	return i;
}

static textureId readTextureId()
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
	textureId tid = BunnySector_GetTextureId(&textureName);
	zstr_free(&textureName);
	return tid;
}

static void read_thing() {

	printf("Read thing\n");
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
				t->arg0 = readByte();
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

	printf("Read vertex\n");
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

	printf("Read linedef\n");
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
				t->v2 = readFloat();
			}
			else if (line_startswith(SPECIAL))
			{
				t->special = readInt();
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
	printf("Read sidedef\n");
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

    DoomMap* Doom_ReadMapFromFile(const char* mapfilename, int dukesPerUnit)
	{
		mapfile = fopen(mapfilename, "r");
		if (mapfile == NULL)
		{
			return nullptr;
		}

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
				map = DoomMap_Create(thingAmount, sectorAmount, sidedefAmount, linedefAmount, vertexAmount + NewVertes);

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
		for (int ni = 0; ni < NumSubsectors; ni++)
		{
			u32 NumSegs = ReadDWORD();
			printf("Segments in subsector %d : %d\n", ni, NumSegs);
			map->subsectors[ni].segmentAmount = NumSegs;
			map->subsectors[ni].segments = (DoomSegment*)mgdl_AllocateGeneralMemory(NumSegs * sizeof(DoomSegment));
		}
		u32 NumSegs = ReadDWORD();
		printf("NumSeg %d\n", NumSegs);
		DoomMap_AllocateSegments(map, NumSegs);
		for (int subi = 0; subi < NumSubsectors; subi++)
		{
			DoomSubSector* s = &map->subsectors[subi];
			for (int si = 0; si < s->segmentAmount; si++)
			{
				u32 v1 = ReadDWORD(); // Vertex index
				u32 partner = ReadDWORD(); // Segment index
				u16 line = ReadWORD(); // Linedef index
				u8 side = ReadByte(); // Linedef side
				printf("Seg %d: V1 %d partner %d line %d side %d\n", si, v1, partner, line, side);

				s->segments[si].v1 = v1;
				s->segments[si].partnerSegment = partner;
				s->segments[si].linedef = line;
				s->segments[si].lineSide = side;
			}
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
		// FLIP THE Y!
		/*
		float rotation = 180.0f;
		float cosres = cosf(DEG2RAD * rotation);
		float sinres = sinf(DEG2RAD * rotation);
		*/
		for (int i = 0; i < map->thingAmount; i++)
		{
			/*
			float x = map->things[i].x;
			float y = map->things[i].y;
			map->things[i].x = x*cosres - y*sinres;
			map->things[i].y = x*sinres + y*cosres;
			map->things[i].angleDeg += rotation;
			*/
			//map->things[i].y *= -1.0f;
		}
		for (int i = 0; i < map->vertexAmount; i++)
		{
			/*
			float x = map->vertices[i].x;
			float y = map->vertices[i].y;
			map->vertices[i].x = x*cosres - y*sinres;
			map->vertices[i].y = x*sinres + y*cosres;
			*/
			//map->vertices[i].y *= -1.0f;
		}

		return map;
	}

	DoomMap * Doom_ReadMapFromFile(const zstr& mapfilename, int dukesPerUnit)
	{
		return Doom_ReadMapFromFile(zstr_cstr(&mapfilename), dukesPerUnit);
	}

