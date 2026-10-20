#pragma once

#include <mgdl.h>
#include "doom/doom-map.h"
#include "bunny-sector-map.h"
#include "bunny-sector-types.h"
#include "bunny-sector-items.h"
#include "bunny-sector-materials.h"

class Player;


#ifdef __cplusplus
extern "C" {
#endif

bool BunnySector_Init(mgdl_AngelScript* angel);
void BunnySector_Update(float deltaTime);

void BunnySector_StartMap(int playerAmount);
void BunnySector_UpdateActiveMap(float deltaTime);
void BunnySector_UpdateMap(int mapNumber, float deltaTime);
void BunnySector_UpdateMapPtr(BunnySector_Map* map, float deltaTime);

void BunnySector_DrawActiveMap();

GameStatus BunnySector_GetGameStatus();
void BunnySector_SetGameStatus(GameStatus status);

void BunnySector_SetRenderingCallback(asIScriptFunction* callbackFunction);
void BunnySector_SetAfterCollisionCallback(asIScriptFunction* callbackFunction);

void BunnySector_HandleDefaultCollisions();


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

// DOOM
DoomMap* BunnySector_GetDoomMap();

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
void BunnySector_DrawWallF(float startx, float startz, float endx, float endz, float normalx, float normalz, s32 floory, s32 ceilingy, s16 picnum, s8 shade, u32 linedef_flags);

void BunnySector_StartFloorCeilingDrawing();
void BunnySector_DrawSectorFloorOrCeiling(s16 sectorNumber, bool floor);

void BunnySector_DrawMapActorsForPlayer(int playerIndex);

void BunnySector_EndMapDrawing();

void BunnySector_EndFloorCeilingDrawing();

void BunnySector_SetOpenGLUnitsToMeter(float horizontalScale, float verticalScale);

#ifdef __cplusplus
}
#endif

/**
 * @brief Loads a map from file and returns map id
 * @param mapNumber Number of the map in game
 * @returns True if load succeeded
 */
bool BunnySector_LoadMap(int mapNumber);

void BunnySector_DrawCameraInfo(float x, float y);

// Manager singletons
BunnySector::ItemManager* BunnySector_GetItemManager();
BunnySector::MaterialManager* BunnySector_GetMaterialManager();


