#pragma once
#include "dukemap.h"
// Forward defs


struct Camera;
class BunnySector_Map;

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Allocates memory
 */
void BuildRender_Init();


/** @brief Draws the map wireframe and player(s) and other information defined in the settings
 */
void BuildRender_DrawTopDown(Viewpoint* camera, DukeMap* map, RenderSettingsOpenGL* settings3D, RenderSettings2D* settings2D);

/** @brief Draws the map in 3D using OpenGL, using the functions below
 * @param player The player whose point of view is used
 * @param map The map.
 * @param settings Rendering settings
 */
void BuildRender_Draw3D(Viewpoint* camera, BunnySector_Map* map, RenderSettingsOpenGL* settings);

void BuildRender_DrawSectorWalls(Viewpoint* camera, DukeMap* map, RenderSettingsOpenGL* settings);
void BuildRender_DrawSectorFloorsAndCeilings(Viewpoint* camera, BunnySector_Map* map, RenderSettingsOpenGL* settings);
void BuildRender_DrawSprites(DukeMap* map, Viewpoint* camera, RenderSettingsOpenGL* settings);

/**
 * @brief Visualize how the sectors and portals are drawn
 */
void BuildRender_DrawSectorRequests(RenderSettingsOpenGL* settings3D);

SectorRender* BuildRender_GetDrawnSectorNumbers();
s16 BuildRender_GetDrawnSectorAmount();
bool BuildRender_WasSectorDrawn(s16 sectornumber);



#ifdef __cplusplus
}
#endif
