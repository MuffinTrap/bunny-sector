#pragma once

#include <mgdl.h>
#include "duke/dukemap.h"
#include "doom/doom-map.h"
#include "bunny-sector-map.h"
#include "bunny-sector-types.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef int MapId;

bool BunnySector_Init();
void BunnySector_StartMap(MapId mapId);
void BunnySector_UpdateMap(MapId mapId, float deltaTime);
void BunnySector_RenderMap(MapId mapId);
BunnyMapType BunnySector_GetMapType(MapId mapid);


MaterialId BunnySector_GetMaterialId(zstr* doomTextureFilename);

struct buns_Vec2
{
	float x;
	float y;
};
typedef struct buns_Vec2 buns_Vec2;
struct buns_Vec3
{
	float x;
	float y;
	float z;
};
typedef struct buns_Vec3 buns_Vec3;

// Accurate intersection with doubles
bool buns_Intersect(double a1x, double a1y, 
	double a2x, double a2y, 
	double b1x, double b1y, 
	double b2x, double b2y, 
	buns_Vec2& out_point);


// Get data from active map

// DUKE
Sector* BunnySector_GetSector(s16 sectorNumber);
Wall* BunnySector_GetWall(s16 wallIndex);
Wall* BunnySector_GetWallEnd(Wall* wall);
s16 BunnySector_GetSectorAmount();

// DOOM
DoomMap* BunnySector_GetDoomMap(MapId mapId);

// Actor functions
Actor* BunnySector_GetActor(int actorId);
void BunnySector_GetActorPositionV2(int actorId, buns_Vec2& out_pos);
void BunnySector_GetActorPositionV3(int actorId, buns_Vec3& out_pos);
void BunnySector_GetActorFloorDir(int actorId, buns_Vec2& out_dir);
float BunnySector_GetActorRadius(int actorId);
void BunnySector_SetActorPosition(int actorId, float x, float z);
void BunnySector_SetActorSpeeds(int actorId, float walkSpeedMultiplier, float turnSpeedMultiplier);
void BunnySector_SetActorDriveInput(int actorId, float forward, float strafe, float vertical, float turnYaw, float turnPitch);
void BunnySector_MoveActorFreely(int actorId, float deltatime);

// Camera functions
float BunnySector_GetOpenGLCameraVerticalFOVDeg();
void BunnySector_SetOpenGLCameraVerticalFOVDeg(float degrees);

// Drawing
void BunnySector_Setup3D(float viewAspect, float cameraAspect);
void BunnySector_AlignCameraToActor(int actorId);
void BunnySector_StartWallDrawing();
void BunnySector_DrawWallF(float startx, float startz, float endx, float endz, float normalx, float normalz, s32 floory, s32 ceilingy, s16 picnum, s8 shade);
void BunnySector_DrawWall(Wall* start , Wall* end, s32 floory, s32 ceilingy, s16 picnum, s8 shade);

void BunnySector_DrawSectorFloorOrCeiling(s16 sectorNumber, bool floor, s16 picnum, s8 shade);
void BunnySector_EndWallDrawing();

void BunnySector_SetOpenGLUnitsToMeter(float scale);

#ifdef __cplusplus
}
#endif

/**
 * @brief Loads a map from file and returns map id
 * @param mapfilename Name of the map file
 * @param dukesPerUnit All dimensions are divided by this to convert to meters. 1024 is a good default.
 * @returns Map id. Negative number indicates failed load and is an error code?
 */
MapId BunnySector_LoadMap(const zstr& mapfilename);
/**
 * @brief Loads a map from file and returns map id
 * @param mapfilename Name of the map file
 * @param dukesPerUnit All dimensions are divided by this to convert to meters. 1024 is a good default.
 * @returns Map id. Negative number indicates failed load and is an error code?
 */
MapId BunnySector_LoadMap(const char* mapfilename);

void BunnySector_DrawCameraInfo(float x, float y);

