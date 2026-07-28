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
void BunnySector_MoveActorFreely(int actorId, float deltatime);

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
Sector* buns_GetSector(s16 sectorNumber);
Wall* buns_GetWall(s16 wallIndex);
Wall* buns_GetWallEnd(Wall* wall);
s16 buns_GetSectorAmount();
void buns_GetActorPositionV2(int actorId, buns_Vec2& out_pos);
void buns_GetActorPositionV3(int actorId, buns_Vec3& out_pos);
void buns_GetActorFloorDir(int actorId, buns_Vec2& out_dir);
void buns_SetActorPosition(int actorId, float x, float z);
Actor* buns_GetActor(int actorId);



#ifdef __cplusplus
}
#endif

/**
 * @brief Loads a map from file and returns map id
 * @param mapfilename Name of the map file
 * @param dukesPerUnit All dimensions are divided by this to convert to meters. 1024 is a good default.
 * @returns Map id. Negative number indicates failed load and is an error code?
 */
MapId BunnySector_LoadMap(const zstr& mapfilename, int dukesPerUnit);
/**
 * @brief Loads a map from file and returns map id
 * @param mapfilename Name of the map file
 * @param dukesPerUnit All dimensions are divided by this to convert to meters. 1024 is a good default.
 * @returns Map id. Negative number indicates failed load and is an error code?
 */
MapId BunnySector_LoadMap(const char* mapfilename, int dukesPerUnit);

