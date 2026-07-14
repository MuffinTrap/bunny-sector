#pragma once

#include <mgdl.h>
#include "duke/dukemap.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef int MapId;

bool BunnySector_Init();
void BunnySector_StartMap(MapId mapId);
void BunnySector_UpdateMap(MapId mapId, float deltaTime);
void BunnySector_RenderMap(MapId mapId);
void BunnySector_SetActorDriveInput(int actorId, float forward, float strafe, float vertical, float turnYaw, float turnPitch);

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

// Get data from active map
Sector* buns_GetSector(s16 sectorNumber);
Wall* buns_GetWall(s16 wallIndex);
Wall* buns_GetWallEnd(Wall* wall);
s16 buns_GetSectorAmount();
void buns_GetActorPositionV2(int actorId, buns_Vec2& out_pos);
void buns_GetActorPositionV3(int actorId, buns_Vec3& out_pos);
void buns_GetActorFloorDir(int actorId, buns_Vec2& out_dir);
Actor* buns_GetActor(int actorId);



#ifdef __cplusplus
}
#endif

/**
 * @brief Loads a map from file and returns map id
 * @param mapfilename Name of the map file
 * @returns Map id. Negative number indicates failed load and is an error code?
 */
MapId BunnySector_LoadMap(const zstr& mapfilename);
/**
 * @brief Loads a map from file and returns map id
 * @param mapfilename Name of the map file
 * @returns Map id. Negative number indicates failed load and is an error code?
 */
MapId BunnySector_LoadMap(const char* mapfilename);

