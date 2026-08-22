#pragma once
#include <mgdl.h>

typedef s16 textureId; // Texture identifier instead of string

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

struct linedef
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
typedef struct linedef linedef;

struct sidedef
{
	int offsetx;
	int offsety;
	textureId texturetop;
	textureId texturebottom;
	textureId texturemiddle;

	int sector;
};
typedef struct sidedef sidedef;

// Identical
typedef Vector2 vertex;

struct sector
{
	int heightfloor;
	int heightceiling;

	textureId texturefloor;
	textureId textureceiling;
	s8 lightlevel;
	int special;
	int id;
};
typedef struct sector sector;


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
struct thing
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
typedef struct thing thing;

struct Fixed16
{
	s16 whole;
	s16 fract;
};
typedef struct Fixed16 Fixed16;

struct Segment
{
	u32 v1;
	u32 partnerSegment;
	u16 linedef;
	u8 lineSide;
};
typedef struct Segment Segment;

struct SubSector
{
	u32 segmentAmount;
	Segment* segments;

};
typedef struct SubSector SubSector;

struct DoomNode
{
	s16 x;
	s16 y;
	s16 dx;
	s16 dy;

	// Child 0 bounding box
	s16 Top0 ;
	s16 Bottom0 ;
	s16 Left0 ;
	s16 Right0 ;
	// Child 1 bounding box
	s16 Top1 ;
	s16 Bottom1 ;
	s16 Left1 ;
	s16 Right1 ;

	// Bit 31 : 1 subsector
	// Bit 31 : 0 node
	u32 child0 ;
	u32 child1 ;
};
typedef struct DoomNode DoomNode;
