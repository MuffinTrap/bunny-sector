#pragma once

#include <mgdl.h>
#include "duke/dukemap.h"
#include "doom/doom-map.h"
#include "bunny-sector-map.h"
#include "bunny-sector-types.h"

class Player;


#ifdef __cplusplus
extern "C" {
#endif


typedef int MapId;

bool BunnySector_Init(mgdl_AngelScript* angel);
void BunnySector_Update(float deltaTime);

void BunnySector_StartMap(MapId mapId, int playerAmount);
void BunnySector_UpdateActiveMap(float deltaTime);
void BunnySector_UpdateMap(MapId mapId, float deltaTime);
void BunnySector_UpdateMapPtr(BunnySector_Map* map, float deltaTime);
BunnyMapType BunnySector_GetMapType(MapId mapid);
GameStatus BunnySector_GetGameStatus();
void BunnySector_SetGameStatus(GameStatus status);

void BunnySector_SetRenderingCallback(asIScriptFunction* callbackFunction);
void BunnySector_SetAfterCollisionCallback(asIScriptFunction* callbackFunction);


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
Actor* BunnySector_GetPlayerActor(int playerIndex);
Actor* BunnySector_GetActorById(int actorId);
Actor* BunnySector_GetActorByIndex(int actorIndex);
void BunnySector_SetActorPosition(int actorIndex, float x, float z);
void BunnySector_SetPlayerSpeeds(int playerIndex, float walkSpeedMultiplier, float turnSpeedMultiplier);
void BunnySector_SetPlayerDriveInput(int playerIndex, float forward, float strafe, float vertical, float turnYaw, float turnPitch);
int BunnySector_GetActorCollisionAmount(Actor* actor);
Actor* BunnySector_GetActorCollisionAt(Actor* actor, int index);
void BunnySector_DestroyActor(Actor* actor);

// Player functions
/**
 * @brief Gives an item to actor, returns bool if success
 */
bool BunnySector_GivePlayerItem(int playerIndex, int itemtype, int amount);
int BunnySector_GetPlayerItemCount(int playerIndex, int itemtype);
Player* BunnySector_GetPlayer(int playerIndex);
void BunnySector_StartPlayerAction(int playerIndex, ActorActionBit action);
void BunnySector_StopPlayerAction(int playerIndex, ActorActionBit action);

// Camera functions
float BunnySector_GetOpenGLCameraVerticalFOVDeg();
void BunnySector_SetOpenGLCameraVerticalFOVDeg(float degrees);

// Drawing
void BunnySector_Setup3D(float viewAspect, float cameraAspect);
void BunnySector_AlignCameraToPlayer(int playerIndex);

void BunnySector_StartMapDrawing();
void BunnySector_DrawWallF(float startx, float startz, float endx, float endz, float normalx, float normalz, s32 floory, s32 ceilingy, s16 picnum, s8 shade);
void BunnySector_DrawWall(Wall* start , Wall* end, s32 floory, s32 ceilingy, s16 picnum, s8 shade);

void BunnySector_StartFloorCeilingDrawing();
void BunnySector_DrawSectorFloorOrCeiling(s16 sectorNumber, bool floor);

void BunnySector_DrawMapActorsForPlayer(int playerIndex);

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

/**
 * @brief Loads item properties from an xml file
 * @param propertiesfile Filename of the properties
 * @returns True if the file was loaded correctly
 */
bool BunnySector_LoadItemProperties(const zstr& propertiesfile);

void BunnySector_DrawCameraInfo(float x, float y);

