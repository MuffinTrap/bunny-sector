#pragma once
#include <mgdl.h>
#include "../bunny-sector-types.h"


#define DOOM_UNITS_TO_METER 32.0f
#define DOOM_UNITS_TO_METER_H 32.0f // NOTE only used in map loading
#define DOOM_UNITS_TO_METER_V 22.0f // NOTE this is only used in map loading

typedef unsigned int ChildId;

#define DOOM_SIDE_FRONT 0
#define DOOM_SIDE_BACK 1
#define DOOM_INVALID_LINEDEF 65535
#define DOOM_NO_LINE_NEIGHBOR  666666
#define DOOM_TICK_DURATION_SECONDS 0.0285714285714 // 1/35
#define DOOM_SPEED_TO_UNITS 0.125 // 1/8

// Sector is used for something
enum DoomSectorUse
{
	sector_none = 0,
	sector_door
};


enum LINEDEF_SPECIAL_ACTION    // Arguments
{
	special_none = 0,
	// 1-8 polyobj
	// 9 line horizon
	special_door_close = 10, // Door sector, speed
	special_door_open = 11, // Door sector, speed
	special_door_raise = 12, // 0: Door sector, 1: speed, 2: delay (ticks)
	special_door_locked_raise = 13, // Door sector, speed, delay (ticks), 3: Lock( see DOOM_LOCK_VALID_KEY

	special_exit_normal = 243 // Map exit, 0: start position index for next map

};

enum DOOM_LOCK_VALID_KEY
{
	Lock_none = 0,
	Lock_red_key_card = 1,
	Lock_blue_key_card = 2,
	Lock_yellow_key_card = 3,

	Lock_red_skull_key = 4,
	Lock_blue_skull_key = 5,
	Lock_yellow_skull_key = 6,

	Lock_any_key = 100,
	Lock_all_keys = 101,
};

enum LINEDEF_FLAG // Value is the bit index
{
	// 0 - 3
	linedef_blocking = 0,  // Blocks things
	linedef_blockmonsters, // Blocks monsters
	linedef_twosided, //
	linedef_dontpegtop,  // Upper texture unpegged

	// 4 - 7
	linedef_dontpegbottom, // Lower texture unpegged
	linedef_secret, // Draw as one sided wall on map
	linedef_blocksound,
	linedef_dontdraw,

	// 8 - 11
	linedef_mapped, // Always draw on map
	linedef_repeat_special, // Can be activated multiple times
	linedef_activate_player_use, // Activated by player use
	linedef_activate_monster_cross, // Activated by monster crossing

	// 12 - 15
	linedef_activate_player_push, // Activated by player hit :
	linedef_activate_monsters, // Can be activated by monsters
	linedef_blockplayers,     // 14
	linedef_blockeverything, // 15

	// 16 Own invention for doors
	linedef_activate_player_cross,

	// Extra tags which take more than one bit
	linedef_activate_projectile_impact, // Activated by projectile hit 10 + 11
	linedef_activate_projectile_cross, // Activated by projectile crossing 10 + 12
	linedef_activate_player_use_back // Activated by player use both ways 12 + 11

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

	int special; /*<< This the action (Exit, Open Door etc..) of this line */
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
	MaterialId texturetop;
	MaterialId texturebottom;
	MaterialId texturemiddle;

	int sector;
};
typedef struct DoomSidedef DoomSidedef;

void DoomSidedef_Init(DoomSidedef* def);

struct DoomSector
{
	int heightfloor;
	int heightceiling;

	MaterialId texturefloor;
	MaterialId textureceiling;
	u8 lightlevel;
	float xscaleceiling; // NOTE Divide by this amount
	float yscaleceiling;
	int special;
	int id; // Tag

	DoomSectorUse usecase;
	int doorOpenHeight;
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
	int type; // This is the editor number

	u32 thing_flags;

	int special;
	s8 arg0, arg1, arg2, arg3, arg4;

	// Extra info
	MaterialId texture;
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

	// Not in file but added for convenience
	int neighbourSector;
};
typedef struct DoomSegment DoomSegment;

void DoomSegment_Init(DoomSegment* def);

struct DoomSubSector
{
	u32 firstSegment;
	u32 segmentAmount;

	// Not in file, but added for convenience
	int sector;
	Vector2 minXZPoint;
	Vector2 sizeXZ;
	Vector2 maxTexCoord;
};
typedef struct DoomSubSector DoomSubSector;

bool ChildIsNode(ChildId id);


extern int BB_TOP;
extern int BB_BOT;
extern int BB_LFT;
extern int BB_RGT;

class DoomNode
{
public:
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

	ChildId GetChild( unsigned int index);
	int GetChildSide(float px, float py);
	s16 GetBBox0( unsigned int index);
	s16 GetBBox1( unsigned int index);
	s16 GetBBox( unsigned int index); // Combined 0-7 index
	bool PointInsideBox( Vector2 point, int childIndex);
};

float DoomSpeedToUnits(int doomSpeed);


