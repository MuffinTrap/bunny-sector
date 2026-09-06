#pragma once
#include "dukemap.h"
// Forward defs


struct Camera;
class BunnySector_Map;

struct RenderSettings2D
{
    float scaleXZ;
    float mapZoom;
    Vector2 mapOffset;

    // Wall drawing debugging
    int drawOneWall; ///< if -1 no walls, if zero or positive that wall
    int drawOneSector; ///< if -1 no sectors, if zero or positive that sector

    // Collision test debugging
    Vector2 collisionPoint;
    s16 collisionInsideSector;
    float collisionLength;
    float collisionAngleDeg;
    bool movePlayer;
    bool rotateMap;
    bool centerMapToPlayer;

    int drawPlayersAmount; ///< How many players to draw
    bool drawSectorNumbers; ///< Draw sector numbers in green if they are rendered
    bool drawPortals; ///< Draw portal walls
    bool drawNormals; ///< Draw wall normals
    bool drawSprites; ///< Draw Sprites
    bool drawTreasure; ///< Draws treasure sprite regardless of other sprites
    bool drawWallNumbers;
    bool drawPortalDrawLimits; // Shows where portal limits are on the screen

    float gridSize; // Grid in OpenGL units
};
typedef struct RenderSettings2D RenderSettings2D;

// TODO Add option to not cull walls and sectors
struct RenderSettingsOpenGL
{
    float scale;
    float spriteDefaultWidth;
    float spriteDefaultHeight;

    // TODO move to camera info
    // Camera information
    float FOVyDegrees;
    float near, far;
    float aspectRatio;

    // This ratio controls how much fov increases
    // when pitch is off from 0 degrees
    float pitchToFovWidth;

};
typedef struct RenderSettingsOpenGL RenderSettingsOpenGL;

void RenderSettingsOpenGL_SetUnitToMeter(RenderSettingsOpenGL* setting, float unitsToMeter);

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Allocates memory
 */
void BuildRender_Init();

RenderSettings2D GetDefaultRenderSettings2D();
RenderSettingsOpenGL GetDefaultRenderSettingsOpenGL();
Camera* GetDefaultCamera();
Viewpoint GetDefaultCameraInfo();

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
