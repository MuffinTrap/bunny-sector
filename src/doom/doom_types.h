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


struct DoomSector
{
	int heightfloor;
	int heightceiling;

	textureId texturefloor;
	textureId textureceiling;
	s8 lightlevel;
	int special;
	int id;
};
typedef struct DoomSector DoomSector;


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

struct DoomSubSector
{
	u32 segmentAmount;
	DoomSegment* segments;
};
typedef struct DoomSubSector DoomSubSector;

// Segment property for AngelScript
DoomSegment* DoomSubSector_GetSegment(u32 index, DoomSubSector* obj);


bool ChildIsNode(ChildId id);

struct DoomNode
{
	s16 x;
	s16 y;
	s16 dx;
	s16 dy;

	// Child 0 bounding box
	s16 top0 ;
	s16 bottom0 ;
	s16 left0 ;
	s16 right0 ;
	// Child 1 bounding box
	s16 top1 ;
	s16 bottom1 ;
	s16 left1 ;
	s16 right1 ;

	// Bit 31 : 1 subsector
	// Bit 31 : 0 node
	ChildId child0 ;
	ChildId child1 ;
};
typedef struct DoomNode DoomNode;

