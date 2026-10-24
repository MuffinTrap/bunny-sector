#include "doom-map-reader.h"
#include "../bunny-sector_main.h"
#include "../bunny-sector-materials.h"
#include "binaryreader.h"
#include "doom-wad-reader.h"
#include <stdio.h>

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
#define PLAYERPUSH "playerpush"
#define PLAYERUSE "playeruse"

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

// Sector properties
#define XSCALECEILING "xscaleceiling"
#define YSCALECEILING "yscaleceiling"

struct SectorCounter
{
	int number;
	int score;
};

static const int COUNTER_AMOUNT = 16;
static SectorCounter sectorCounterArray[COUNTER_AMOUNT];
static void ResetSectorCounterArray()
{
	for (int i = 0; i < COUNTER_AMOUNT; i++)
	{
		sectorCounterArray[i].number =-1;
		sectorCounterArray[i].score =0;
	}
}
static void AddPointForSector(int sectorNumber)
{
	for (int i = 0; i < COUNTER_AMOUNT; i++)
	{
		if (sectorCounterArray[i].number ==-1)
		{
			sectorCounterArray[i].number = sectorNumber;
			sectorCounterArray[i].score = 1;
			break;
		}
		else if (sectorCounterArray[i].number == sectorNumber)
		{
			sectorCounterArray[i].score += 1;
		}
	}
}
static int GetSectorWinner()
{
	int hiscore = -1;
	int winner = -1;
	for (int i = 0; i < COUNTER_AMOUNT; i++)
	{
		if (sectorCounterArray[i].number != -1)
		{
			if (sectorCounterArray[i].score > hiscore)
			{
				hiscore = sectorCounterArray[i].score;
				winner = sectorCounterArray[i].number;
			}
		}
	}
	return winner;
}

static void read_lines_until(BinaryReader& reader, const char* keyword)
{
	while(reader.IsEOF() == false)
	{
		reader.ReadLine();
		if (reader.LineHas(keyword))
		{
			break;
		}
	}
}

static void read_thing(BinaryReader& reader) {

	reader.ReadLine();
	if (reader.LineHas("{"))
	{
		DoomThing* t = &map->things[thingAmount];
		DoomThing_Init(t);
		while (true)
		{
			reader.ReadLine();
			if (reader.LineStartsWith(X))
			{
				t->x = reader.ReadLineFloat();
			}
			else if (reader.LineStartsWith(Y))
			{
				t->y = reader.ReadLineFloat();
			}
			else if (reader.LineStartsWith(ID))
			{
				t->id = reader.ReadLineInt();
			}
			else if (reader.LineStartsWith(TYPE))
			{
				t->type = reader.ReadLineInt();
			}
			else if (reader.LineStartsWith(HEIGHT))
			{
				t->height = reader.ReadLineFloat();
			}
			else if (reader.LineStartsWith(ANGLE))
			{
				t->angleDeg = reader.ReadLineFloat();
			}
			else if (reader.LineStartsWith(SPECIAL))
			{
				t->special = reader.ReadLineInt();
			}
			else if (reader.LineStartsWith(ARG0))
			{
				t->arg0 = reader.ReadLineByte();
			}
			else if (reader.LineStartsWith(ARG1))
			{
				t->arg1 = reader.ReadLineByte();
			}
			else if (reader.LineStartsWith(ARG2))
			{
				t->arg2 = reader.ReadLineByte();
			}
			else if (reader.LineStartsWith(ARG3))
			{
				t->arg3 = reader.ReadLineByte();
			}
			else if (reader.LineStartsWith(ARG4))
			{
				t->arg4 = reader.ReadLineByte();
			}

			// Flags

			else if (reader.LineStartsWith(SKILL1))
			{
				if (reader.ReadLineBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_skill1);
				}
			}
			else if (reader.LineStartsWith(SKILL2))
			{
				if (reader.ReadLineBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_skill2);
				}
			}
			else if (reader.LineStartsWith(SKILL3))
			{
				if (reader.ReadLineBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_skill3);
				}
			}
			else if (reader.LineStartsWith(SKILL4))
			{
				if (reader.ReadLineBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_skill4);
				}
			}
			else if (reader.LineStartsWith(AMBUSH))
			{
				if (reader.ReadLineBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_ambush);
				}
			}
			else if (reader.LineStartsWith(SINGLE))
			{
				if (reader.ReadLineBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_single);
				}
			}
			else if (reader.LineStartsWith(DM))
			{
				if (reader.ReadLineBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_dm);
				}
			}
			else if (reader.LineStartsWith(COOP))
			{
				if (reader.ReadLineBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_coop);
				}
			}
			else if (reader.LineStartsWith(FRIEND))
			{
				if (reader.ReadLineBool())
				{
					t->thing_flags = Flag_SetBit(t->thing_flags, thing_friend);
				}
			}

			else if (reader.LineHas("}"))
			{
				break;
			}
		}
		thingAmount += 1;
	}
}
static void read_vertex(BinaryReader& reader) {

	reader.ReadLine();
	if (reader.LineHas("{"))
	{
		DoomVertex* t = &map->vertices[vertexAmount];
		while (true)
		{
			reader.ReadLine();
			if (reader.LineStartsWith(X))
			{
				t->x = reader.ReadLineFloat();
			}
			else if (reader.LineStartsWith(Y))
			{
				t->y = reader.ReadLineFloat();
			}
			else if (reader.LineHas("}"))
			{
				break;
			}
		}
		vertexAmount += 1;
	}
}

static void read_linedef(BinaryReader& reader) {

	reader.ReadLine();
	if (reader.LineHas("{"))
	{
		DoomLinedef* t = &map->linedefs[linedefAmount];
		DoomLinedef_Init(t);
		while (true)
		{
			reader.ReadLine();
			if (reader.LineStartsWith(ID))
			{
				t->id = reader.ReadLineInt();
			}
			if (reader.LineStartsWith(V1))
			{
				t->v1 = reader.ReadLineInt();
			}
			else if (reader.LineStartsWith(V2))
			{
				t->v2 = reader.ReadLineInt();
			}
			else if (reader.LineStartsWith(SPECIAL))
			{
				t->special = reader.ReadLineInt();
			}
			else if (reader.LineStartsWith(ARG0))
			{
				t->arg0 = reader.ReadLineByte();
			}
			else if (reader.LineStartsWith(ARG1))
			{
				t->arg1 = reader.ReadLineByte();
			}
			else if (reader.LineStartsWith(ARG2))
			{
				t->arg2 = reader.ReadLineByte();
			}
			else if (reader.LineStartsWith(ARG3))
			{
				t->arg3 = reader.ReadLineByte();
			}
			else if (reader.LineStartsWith(ARG4))
			{
				t->arg4 = reader.ReadLineByte();
			}
			else if (reader.LineStartsWith(SIDEFRONT))
			{
				t->sidefront = reader.ReadLineInt();
			}
			else if (reader.LineStartsWith(SIDEBACK))
			{
				t->sideback = reader.ReadLineInt();
			}
			else if (reader.LineStartsWith(TWOSIDED))
			{
				if (reader.ReadLineBool())
				{
					t->linedef_flags = Flag_SetBit(t->linedef_flags, linedef_twosided);
				}
			}
			else if (reader.LineStartsWith(DONTPEGBOTTOM))
			{
				if (reader.ReadLineBool())
				{
					t->linedef_flags = Flag_SetBit(t->linedef_flags, linedef_dontpegbottom);
				}
			}
			else if (reader.LineStartsWith(PLAYERCROSS))
			{
				if (reader.ReadLineBool())
				{
					t->linedef_flags = Flag_SetBit(t->linedef_flags, linedef_activate_player_cross);
				}
			}
			else if (reader.LineStartsWith(PLAYERPUSH))
			{
				if (reader.ReadLineBool())
				{
					t->linedef_flags = Flag_SetBit(t->linedef_flags, linedef_activate_player_push);
				}
			}
			else if (reader.LineStartsWith(PLAYERUSE))
			{
				if (reader.ReadLineBool())
				{
					t->linedef_flags = Flag_SetBit(t->linedef_flags, linedef_activate_player_use);
				}
			}
			else if (reader.LineHas("}"))
			{
				break;
			}
		}
		linedefAmount += 1;
	}
}
static void read_sidedef(BinaryReader& reader)
{
	reader.ReadLine();
	if (reader.LineHas("{"))
	{
		DoomSidedef* t = &map->sidedefs[sidedefAmount];
		DoomSidedef_Init(t);
		while (true)
		{
			reader.ReadLine();
			if (reader.LineStartsWith(OFFSETX))
			{
				t->offsetx = reader.ReadLineInt();
			}
			if (reader.LineStartsWith(OFFSETY))
			{
				t->offsety = reader.ReadLineInt();
			}
			else if (reader.LineStartsWith(TEXTURETOP))
			{
				t->texturetop = reader.ReadLineTextureId();
			}
			else if (reader.LineStartsWith(TEXTUREBOTTOM))
			{
				t->texturebottom = reader.ReadLineTextureId();
			}
			else if (reader.LineStartsWith(TEXTUREMIDDLE))
			{
				t->texturemiddle = reader.ReadLineTextureId();
			}
			else if (reader.LineStartsWith(SECTOR))
			{
				t->sector = reader.ReadLineInt();
			}
			else if (reader.LineHas(ENTITY_END))
			{
				break;
			}
		}
		sidedefAmount += 1;
	}
}
static void read_sector(BinaryReader& reader) {
	reader.ReadLine();
	if (reader.LineHas("{"))
	{
		DoomSector* t = &map->sectors[sectorAmount];
		DoomSector_Init(t);
		while (true)
		{
			reader.ReadLine();
			if (reader.LineStartsWith(HEIGHTFLOOR))
			{
				t->heightfloor = reader.ReadLineInt();
			}
			if (reader.LineStartsWith(HEIGHTCEILING))
			{
				t->heightceiling = reader.ReadLineInt();
			}
			else if (reader.LineStartsWith(TEXTUREFLOOR))
			{
				t->texturefloor = reader.ReadLineTextureId();
			}
			else if (reader.LineStartsWith(TEXTURECEILING))
			{
				t->textureceiling = reader.ReadLineTextureId();
			}
			else if (reader.LineStartsWith(LIGHTLEVEL))
			{
				t->lightlevel = reader.ReadLineUByte();
			}
			else if (reader.LineStartsWith(XSCALECEILING))
			{
				t->xscaleceiling = reader.ReadLineFloat();
			}
			else if (reader.LineStartsWith(YSCALECEILING))
			{
				t->yscaleceiling = reader.ReadLineFloat();
			}
			else if (reader.LineStartsWith(SPECIAL))
			{
				t->special = reader.ReadLineInt();
			}
			else if (reader.LineStartsWith(ID))
			{
				t->id = reader.ReadLineInt();
			}
			else if (reader.LineHas("}"))
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


static void ReadAndLoadTextures(WADReader& wad, BinaryReader& reader, const char* wadFileName)
{
	DoomLumpInfo* textures = wad.FindTableItem("TEXTURES");
	if (textures == nullptr)
	{
		return;
	}
	BunnySector::MaterialManager* matman = BunnySector_GetMaterialManager();
	reader.SetActiveLumpInfo(textures);
	while(reader.GetReadError() == false)
	{
		reader.ReadLine();
		if (reader.LineStartsWith("Texture"))
		{
			// Read size
			int w = 0;
			int h = 0;
			if (reader.ReadLineTextureSize(&w, &h) == false)
			{
				// NOTE How do we know if it is text or binary?
			}
			zstr textureIdentiefier = reader.ReadLineStringBetween('"', '"');

			int textureName = matman->RecordMaterialName(zstr_cstr(&textureIdentiefier));
			Log_InfoF("Found texture %s, size (%d, %d) -> id %d\n", zstr_cstr(&textureIdentiefier), w, h, textureName);

			// Jump to texture data lump
			DoomLumpInfo* textureData = wad.FindTableItem(zstr_cstr(&textureIdentiefier));
			if (textureData)
			{
				// Load as PNG and record
				if (matman->ConnectTextureDataToMaterialIndex(textureName, zstr_cstr(&textureIdentiefier), wadFileName ,textureData->offset))
				{
					// OK!
				}
			}
			read_lines_until(reader, ENTITY_END);
		}
	}
}



static void CalculateGameplayInfo(DoomMap* map){

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

			for (int ni = 0; ni < map->nodeAmount; ni++)
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
			ResetSectorCounterArray();
			for (int segi = 0; segi < sub->segmentAmount; segi++)
			{
				DoomSegment *seg = &map->segments[sub->firstSegment + segi];
				// NOTE Only looking at the first is incorrect, since portals can point either way
				if (seg->linedef == DOOM_INVALID_LINEDEF)
				{
					continue;
				}

				DoomLinedef *linedef = &map->linedefs[seg->linedef];

				bool isPortal = (linedef->sidefront >= 0 && linedef->sideback >=0);
				if (isPortal)
				{
					// Was only portals. This is a pit or pedestral. All portals should have one side in common
					// Hold a popularity contest
					DoomSidedef *sidedef_f = &map->sidedefs[linedef->sidefront];
					DoomSidedef *sidedef_b = &map->sidedefs[linedef->sideback];

					AddPointForSector(sidedef_f->sector);
					AddPointForSector(sidedef_b->sector);
				}
				else
				{
					if (seg->lineSide == DOOM_SIDE_FRONT)
					{
						DoomSidedef *sidedef = &map->sidedefs[linedef->sidefront];
						sub->sector = sidedef->sector;
						sectorSet = true;
						break;
					}
					else if (seg->lineSide == DOOM_SIDE_BACK)
					{
						DoomSidedef *sidedef = &map->sidedefs[linedef->sideback];
						sub->sector = sidedef->sector;
						sectorSet = true;
						break;
					}
				}
			} // loop segments
			if (!sectorSet)
			{
			 	sub->sector = GetSectorWinner();
			}
			mgdl_assert_print(sub->sector >= 0, "Did not find connected sector for subsector!\n");
			// Log_InfoF("Subsector %d linked to sector %d\n", ssi, sub->sector);
		} // loop subsectors

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

				// Log_InfoF("Subsector %d segment %d neighbor is %d\n", ssi, sub->firstSegment + segi, seg->neighbourSector);
			}
		}

		// DANGER
		// Adjust the height and elevation values of all sectors and things
		// In doom the height is not in same units as width and depth
		//
		/*
		float heightConversion = (float)DOOM_UNITS_TO_METER_H / (float)DOOM_UNITS_TO_METER_V;

		for (int si = 0; si < sectorAmount; si++)
		{
			DoomSector* sector = &map->sectors[si];
			sector->heightfloor *= heightConversion;
			sector->heightceiling *= heightConversion;
		}
		for (int ti= 0; ti < thingAmount; ti++)
		{
			DoomThing* thing = &map->things[ti];
			thing->height *= heightConversion;
		}
		*/

		// TODO
		// for sectors that are doors, calculate the ceilingheight for when the door is open
		// find all doors
		for (int li = 0; li < map->lineAmount; li++ )
		{
			DoomLinedef* line = &map->linedefs[li];
			if (line->special > 0)
			{
				if (line->special == special_door_close
					|| line->special == special_door_open
					|| line->special == special_door_raise
					|| line->special == special_door_locked_raise)
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
    for (int si = 0; si < map->GetSectorAmount(); si++)
    {
        Vector2 minp = Vector2New(32000, 32000);
        Vector2 maxp = Vector2New(-32000, -32000);
		DoomSubSector* sub = &map->subsectors[si];
        int sectorwallnum = map->GetWallAmountInSector(si);
        for (s16 wi = 0; wi < sectorwallnum; wi++)
        {
            Vector2 w = map->GetWallVertexInSector(si, wi);
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
}

    BunnySector_Map* Doom_ReadMapFromFile(const char* mapfilename)
	{

	// DANGER
	// NOTE Windows needs b otherwise it thinks it is UTF-16 or something
		BinaryReader reader(mapfilename);
		if (reader.FileIsValid() == false)
		{
			return nullptr;
		}
		WADReader wadRead(&reader);

		// Loads textures so that they know their position in the wad file for loading them in later
		ReadAndLoadTextures(wadRead, reader, mapfilename);

		BunnySector_Map* BunnyMap = nullptr;

		// Read the textmap lump
		DoomLumpInfo* TEXTMAP = wadRead.FindTableItem("TEXTMAP");
		mgdl_assert_print(TEXTMAP != nullptr, "No TEXTMAP lump found\n");
		DoomLumpInfo* ZNODES = wadRead.FindTableItem("ZNODES");
		mgdl_assert_print(ZNODES != nullptr, "No ZNODES lump found\n");

		// Parsing
		bool counting = true;
		
		

		// Read two times. First for counting and then for reading
		while(true)
		{
			// On first pass need to calculate amounts for allocation
			// Second pass these are indices
			reader.SetActiveLumpInfo(TEXTMAP);
			// TODO Add guard not to read too much

			thingAmount = 0;
			vertexAmount = 0;
			linedefAmount = 0;
			sidedefAmount = 0;
			sectorAmount = 0;

			// Start from top

			while(reader.IsEOF() == false)
			{
				reader.ReadLine();
				// look for keywords
				if (reader.LineStartsWith("thing"))
				{
					// Start reading thing
					if (counting)
					{
						thingAmount += 1;
						read_lines_until(reader, ENTITY_END);
					}
					else
					{
						read_thing(reader);
					}
				}
				else if (reader.LineStartsWith("vertex"))
				{
					if (counting)
					{
						vertexAmount += 1;
						read_lines_until(reader, ENTITY_END);
					}
					else
					{
						read_vertex(reader);
					}
				}
				else if (reader.LineStartsWith("linedef"))
				{
					if (counting)
					{
						linedefAmount += 1;
						read_lines_until(reader, ENTITY_END);
					}
					else
					{
						read_linedef(reader);
					}
				}
				else if (reader.LineStartsWith("sidedef"))
				{
					if (counting)
					{
						sidedefAmount += 1;
						read_lines_until(reader, ENTITY_END);
					}
					else
					{
						read_sidedef(reader);
					}
				}
				else if (reader.LineStartsWith("sector"))
				{
					if (counting)
					{
						sectorAmount += 1;
						read_lines_until(reader, ENTITY_END);
					}
					else
					{
						read_sector(reader);
					}
				}
			}

			// File was read
			if (counting)
			{
				// Log_InfoF("TEXTMAP Counting done\n");
				// Read additional vertex amount for nodes
				reader.SetActiveLumpInfo(ZNODES);
				reader.ReadUntilString("XGLN");
				u32 OrgVerts = reader.ReadDWORD();
				u32 NewVertes = reader.ReadDWORD();
				// Log_InfoF("Adding %d new vertices for nodes\n", NewVertes);

				BunnyMap = new DoomMap();// (BunnySector_Map*)mgdl_AllocateGeneralMemory(sizeof(BunnySector_Map));
				map = (DoomMap*)BunnyMap;

				map->Allocate( thingAmount, sectorAmount, sidedefAmount, linedefAmount, vertexAmount + NewVertes);

				counting = false;
			}

			else
			{
				// Log_Info("TEXTMAP Reading done\n");
				// All data read
				break;
			}
		} // Counting and reading done



		// Log_Info("Reading Z nodes\n");
		// Read the nodes
		// TODO read bit by bit

		reader.SetActiveLumpInfo(ZNODES);
		reader.ReadUntilString("XGLN");
		u32 OrgVerts = reader.ReadDWORD();
		// Log_InfoF("OrgVerts %d\n", OrgVerts);
		u32 NewVertes = reader.ReadDWORD();

		Log_InfoF("NewVerts %d \n", NewVertes);
		for (int ni = 0; ni < NewVertes; ni++)
		{
			// TODO Fixed integers
			Fixed16 x = reader.ReadFixed();
			Fixed16 y = reader.ReadFixed();

			// Log_InfoF("Vert %d at (%d.%d, %d.%d)\n", ni, x.whole, x.fract,  y.whole, y.fract);
			map->vertices[vertexAmount].x = x.whole;
			map->vertices[vertexAmount].y = y.whole;
			vertexAmount++;
		}
		u32 NumSubsectors = reader.ReadDWORD();
		map->AllocateSubsectors( NumSubsectors);
		Log_InfoF("NumSubsectors %d\n", NumSubsectors);
		int segmentsRead = 0;
		for (int ni = 0; ni < NumSubsectors; ni++)
		{
			u32 NumSegs = reader.ReadDWORD();
			map->subsectors[ni].firstSegment = segmentsRead;;
			map->subsectors[ni].segmentAmount = NumSegs;
			// Log_InfoF("Subsector %d Segments; %d : First segment %d\n", ni, NumSegs, segmentsRead);
			segmentsRead += NumSegs;
		}

		u32 NumSegs = reader.ReadDWORD();
		mgdl_assert_test(NumSegs == segmentsRead);
		Log_InfoF("NumSeg %d\n", NumSegs);
		map->AllocateSegments( NumSegs);
		for (int si = 0; si < map->segmentAmount; si++)
		{
			u32 v1 = reader.ReadDWORD(); // Vertex index
			u32 partner = reader.ReadDWORD(); // Segment index
			u16 line = reader.ReadWORD(); // Linedef index
			u8 side = reader.ReadByte(); // Linedef side
			// Log_InfoF("Seg %d: V1 %d partner %d line %d side %d\n", si, v1, partner, line, side);

			map->segments[si].v1 = v1;
			map->segments[si].partnerSegment = partner;
			map->segments[si].linedef = line;
			map->segments[si].lineSide = side;
		}

		u32 NumNodes = reader.ReadDWORD();
		Log_InfoF("NumNodes : %d\n", NumNodes);
		mgdl_assert_print(NumNodes > 0, "Doom map must have at least 1 node\n");
		map->AllocateNodes( NumNodes);
		for (int ni = 0; ni < NumNodes; ni++)
		{
			if (reader.GetReadError())
			{
				break;
			}
			// Log_InfoF("Node %d\n", ni);
			// The line that splits the node
			s16 x = reader.ReadSWORD();
			s16 y = reader.ReadSWORD();
			s16 dx = reader.ReadSWORD();
			s16 dy = reader.ReadSWORD();

			// Child 0 bounding box
			s16 Top0 = reader.ReadSWORD();
			s16 Bottom0 = reader.ReadSWORD();
			s16 Left0 = reader.ReadSWORD();
			s16 Right0 = reader.ReadSWORD();
			// Child 1 bounding box
			s16 Top1 = reader.ReadSWORD();
			s16 Bottom1 = reader.ReadSWORD();
			s16 Left1 = reader.ReadSWORD();
			s16 Right1 = reader.ReadSWORD();

			// Bit 31 : 1 subsector
			// Bit 31 : 0 node
			u32 child0 = reader.ReadDWORD();
			u32 child1 = reader.ReadDWORD();

			// Log_InfoF("Child 0 %08.8x ", child0);
			if (child0 & 0x80000000)
			{
				// Log_InfoF("is subsector: %d\n", child0 & 0x7FFFFFFF);
			}
			else
			{
				// Log_InfoF("is node: %d\n", child0);
			}
			// Log_InfoF("Bounding box %d %d %d %d\n", Top0, Bottom0, Left0, Right0);

			// Log_InfoF("Child 1 %08.8x ", child1);
			if (child1 & 0x80000000)
			{
				// Log_InfoF("is subsector: %d\n", child1 & 0x7FFFFFFF);
			}
			else
			{
				// Log_InfoF("is node: %d\n", child1);
			}
			// Log_InfoF("Bounding box %d %d %d %d\n", Top1, Bottom1, Left1, Right1);

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


		CalculateGameplayInfo(map);

		return BunnyMap;
	}

	BunnySector_Map * Doom_ReadMapFromFile(const zstr& mapfilename)
	{
		return Doom_ReadMapFromFile(zstr_cstr(&mapfilename));
	}

