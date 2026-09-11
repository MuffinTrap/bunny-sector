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

// Accurate intersection
bool buns_Intersect(float a1x, float a1y,
	float a2x, float a2y,
	float b1x, float b1y,
	float b2x, float b2y,
	float& out_x,
	float& out_y);


// Get data from active map

// DUKE
Wall* BunnySector_GetWallEnd(Wall* wall);

// DOOM
DoomMap* BunnySector_GetDoomMap(MapId mapId);
DukeMap* BunnySector_GetDukeMap(MapId mapId);

// Actor functions
Actor* BunnySector_GetActor(int actorId);
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

void BunnySector_StartMapDrawing();
void BunnySector_DrawWallF(float startx, float startz, float endx, float endz, float normalx, float normalz, s32 floory, s32 ceilingy, s16 picnum, s8 shade);
void BunnySector_DrawWall(Wall* start , Wall* end, s32 floory, s32 ceilingy, s16 picnum, s8 shade);

void BunnySector_StartFloorCeilingDrawing();
void BunnySector_DrawSectorFloorOrCeiling(s16 sectorNumber, bool floor);

void BunnySector_EndMapDrawing();

void BunnySector_EndFloorCeilingDrawing();

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

