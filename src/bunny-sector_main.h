#pragma once

#include <mgdl.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef int MapId;

bool BunnySector_Init();
void BunnySector_StartMap(MapId mapId);
void BunnySector_UpdateMap(MapId mapId, float deltaTime);
void BunnySector_RenderMap(MapId mapId);
void BunnySector_SetActorDriveInput(int actorId, float forward, float strafe, float vertical, float turnYaw, float turnPitch);


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

