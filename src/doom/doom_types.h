#pragma once
#include <mgdl.h>

typedef s16 textureId; // Texture identifier instead of string
typedef unsigned int ChildId;

enum LINEDEF_FLAG
{
	linedef_blocking = 1,  // Blocks things
	linedef_blockmonsters, // Blocks monsters
	linedef_twosided, //
	linedef_dontpegtop,  // Upper texture unpegged
	linedef_dontpegbottom, // Lower texture unpegged
	linedef_secret, // Draw as one sided wall on map
	linedef_blocksound,
	linedef_dontdraw,
	linedef_mapped // Always draw on map
};
typedef enum LINEDEF_FLAG LINEDEF_FLAG;

// Identical To Vector2;
struct DoomVertex
{
	float x;
	float y;
};
typedef struct DoomVertex DoomVertex;

struct DoomLinedef
{
	int id;
	int v1;
	int v2;

	u32 linedef_flags;

	int special;
	s8 arg0, arg1, arg2, arg3, arg4;
	int sidefront;
	int sideback;
};
typedef struct DoomLinedef DoomLinedef;

void DoomLinedef_Init(DoomLinedef* def);

struct DoomSidedef
{
	int offsetx;
	int offsety;
	textureId texturetop;
	textureId texturebottom;
	textureId texturemiddle;

	int sector;
};
typedef struct DoomSidedef DoomSidedef;

void DoomSidedef_Init(DoomSidedef* def);

struct DoomSector
{
	int heightfloor;
	int heightceiling;

	textureId texturefloor;
	textureId textureceiling;
	u8 lightlevel;
	int special;
	int id;
};
typedef struct DoomSector DoomSector;

void DoomSector_Init(DoomSector* def);

enum THING_FLAG
{
	thing_skill1 = 1,  // Blocks things
	thing_skill2, // Blocks monsters
	thing_skill3, //
	thing_skill4,  // Upper texture unpegged
	thing_ambush, // Is deaf
	thing_single, // In singleplayer
	thing_dm, // In deathmatch mode
	thing_coop, // IN coop mode
	thing_friend
};
typedef enum LINEDEF_FLAG LINEDEF_FLAG;

struct DoomThing
{
	int id;
	float x;
	float y;
	float height;
	float angleDeg; // 0 east
	int type;

	u32 thing_flags;

	int special;
	s8 arg0, arg1, arg2, arg3, arg4;
};
typedef struct DoomThing DoomThing;

void DoomThing_Init(DoomThing* def);

struct Fixed16
{
	s16 whole;
	s16 fract;
};
typedef struct Fixed16 Fixed16;

struct DoomSegment
{
	u32 v1;
	u32 partnerSegment;
	u16 linedef;
	u8 lineSide;
};
typedef struct DoomSegment DoomSegment;

void DoomSegment_Init(DoomSegment* def);

struct DoomSubSector
{
	u32 firstSegment;
	u32 segmentAmount;
};
typedef struct DoomSubSector DoomSubSector;

bool ChildIsNode(ChildId id);


extern int BB_TOP;
extern int BB_BOT;
extern int BB_LFT;
extern int BB_RGT;

struct DoomNode
{
	// Dividing line start and direction of it
	s16 x;
	s16 y;
	s16 dx;
	s16 dy;

	// Child 0 bounding box
	s16 bbox0[4];
	// Child 1 bounding box
	s16 bbox1[4];

	// Bit 31 : 1 subsector
	// Bit 31 : 0 node
	ChildId children[2];
};
typedef struct DoomNode DoomNode;
ChildId DoomNode_GetChild(DoomNode* node, unsigned int index);
s16 DoomNode_GetBBox0(DoomNode* node, unsigned int index);
s16 DoomNode_GetBBox1(DoomNode* node, unsigned int index);
s16 DoomNode_GetBBox(DoomNode* node, unsigned int index); // Combined 0-7 index

