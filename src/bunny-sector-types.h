#pragma once

#define DUKE_UNITS_TO_METER 1024.0f
#define DOOM_UNITS_TO_METER 32.0f
#define MAP_ACTOR_AMOUNT 128
#define MAP_ACTOR_COLLISION_LIST_SIZE 64
#define MAP_ACTOR_COLLISION_ENTRY_AMOUNT 32

#include <mgdl.h>

enum ActorType
{
    actor_player,
    actor_monster,
	actor_item,
    actor_projectile,
    actor_decoration,
    actor_particle
};

enum ACTOR_ACTION_FLAGS
{
	action_use = 0,
	action_shoot,
	action_jump
};

enum MoveResultBit
{
    Move_Ok = 0,
    Move_HitWall = 1,
    Move_HitPortal = 2,
    Move_OnGround = 3, // Set when actor is standing on floor
    Move_Cancel = 4,
	Move_Collision = 5, // Set when actor collided with other actor
	Move_Dead = 6 // Set when actor should be removed
};
typedef enum MoveResultBit MoveResultBit;

enum MapMaterialType
{
	Material_Texture = 0, // Normal texture material
	Material_Grass = 1,  // Draw multiple shells of grass
	Material_Function = 2, // Use a custom function to draw on the area
	Material_SpriteModel = 3, // Draw a mesh instead of a texture for a sprite
	Material_SpriteAnimated = 4 // Animate a sprite sheet on sprite
};
typedef enum MapMaterialType MapMaterialType;

// Extension of normal material
struct MapMaterial
{
	Material* mgdlMaterial;
	MapMaterialType type;

	union MaterialData
	{
		s32 functionName; // If type is function, frame index if type is Animated sprite
		s32 spriteFrame;
		Mesh* meshPtr;		// If type is SpriteModel
	};

	union MaterialParameter
	{
		float grassLength; // If type is grass
		float frameDuration; // If type is Animated sprite
		float meshScale; // If type is Sprite Mesh
	};

	MaterialData data;
	MaterialParameter parameter;
};
typedef struct MapMaterial MapMaterial;

typedef int MaterialId;
#define INVALID_MATERIAL_ID -1


struct WallInfo
{
	Vector2 start;
	Vector2 end;
	Vector2 normal;
};
typedef struct WallInfo WallInfo;

struct Viewpoint
{
    Vector3 position;
    float yawRad;
    float pitchRad;
    s16 sector;
};
typedef struct Viewpoint Viewpoint;

const char* ActorTypeToString(ActorType aType);

struct BunnyV2
{
	float x;
	float y;
};
typedef struct BunnyV2 BunnyV2;

#if defined(USE_ANGEL_AS_CPP)
// NOTE Should we do typedef BunnyV2 Vector2 ?

#endif
