#pragma once
#include <mgdl.h>
#include "../bunny-sector-types.h"


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
    Vector2 scaleXY;
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

void RenderSettingsOpenGL_SetUnitToMeter(RenderSettingsOpenGL* setting, float unitsToMeterHorizontal, float unitsToMeterVertical);

RenderSettings2D GetDefaultRenderSettings2D();
RenderSettingsOpenGL GetDefaultRenderSettingsOpenGL();
Camera* GetDefaultCamera();
Viewpoint GetDefaultCameraInfo();
