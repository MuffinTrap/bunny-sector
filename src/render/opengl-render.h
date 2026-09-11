#pragma once
#include <mgdl/mgdl-types.h>
#include <mgdl/mgdl-material.h>
#include <mgdl/mgdl-color.h>
#include "dukemap.h"
#include "shell-grass.h"
struct RenderSettingsOpenGL;
struct Wall;
struct Texture;
class BunnySector_Map;

#define RENDERER_PICNUM_DEFAULT 0

#ifdef __cplusplus
extern "C" {
#endif

#include "../bunny-sector-types.h"

/**
 * @brief Init the renderer
 */
void OpenGLRender_Init();

void OpenGLRender_Deinit();

/**
 * @brief Registers a texture to be used when a picnum is drawn
 * @details This will create a new Sprite with one frame and store the sprite pointer in array
 * @param picnum What picnum does this Sprite match
 * @param texture The texture to use
 * @returns True if registering succeeded: there was space in array
 */
bool OpenGLRender_RegisterDefaultTexture(Texture* texture);
MaterialId OpenGLRender_RegisterTexture(Texture* texture);
MaterialId OpenGLRender_RegisterMaterial(Material* material, MapMaterialType materialType);
MaterialId OpenGLRender_RegisterMapMaterial(MapMaterial* material);
MaterialId OpenGLRender_GetMapMaterialId(MapMaterial* material);

void OpenGLRender_SetShellGrass(ShellGrass* grass);

/**
 * @brief Sets up the rendering state for drawing walls and sprites
 */
void OpenGLRender_StartDrawingPolygons(float scaleXYZ);

	void OpenGLRender_DrawWallV(Vector2 start, Vector2 end, Vector2 normalXZ, s32 floorY, s32 ceilingY, s16 picnum, s8 shade);

	void OpenGLRender_DrawWall(DukeMap* map, Wall* w, float floorY, float ceilingY, RenderSettingsOpenGL* settings);
	void OpenGLRender_DrawSprite(Vector3 position, float width, float height, float spriteAngle, float playerAngle, SpriteAlignment alignment, SpritePivot pivot, s16 picnum, s8 brightnessOffset);

	void OpenGLRender_DrawQuad(Vector2 start, Vector2 end, Vector2 normalXZ, float floorY, float ceilingY, s16 picnum, s8 brightnessOffset, float scale);
/**
 * @brief Finalizes the drawing
 */
void OpenGLRender_EndDrawingPolygons();


// Tesselate and store all floors to buffer
void OpenGLRender_CreateFloorBuffers(BunnySector_Map* map, float unitsPerMeterForUV);
/**
 * @brief Tesselates a floor of sector.
 */
void OpenGLRender_TesselateFloor(BunnySector_Map* map, u16 sectorIndex, float unitsPerMeterForUV);
void OpenGLRender_StopCountingFloorBufferSize();

/**
 * @brief Exports all geometry to obj file
 */
void OpenGLRender_WriteToObj(BunnySector_Map* map, const char* filename, RenderSettingsOpenGL* settings);

void OpenGLRender_StartObjExport(BunnySector_Map* map, const char* filename, RenderSettingsOpenGL* settings);
void OpenGLRender_StartFillingWallBuffer(BunnySector_Map* map);
void OpenGLRender_BufferWalls(BunnySector_Map* map);



/**
 * @brief Sets up rendering state to draw floors and ceilings from a buffer
 */
void OpenGLRender_StartDrawingFloorsFromBuffer(BunnySector_Map* map);
void OpenGLRender_DrawFloorOrCeilingDuke(DukeMap* map, s16 sectorIndex, bool floor);
void OpenGLRender_DrawFloorOrCeiling(BunnySector_Map* map, int sectorIndex, u8 shade, float ycoord, MaterialId materialId, bool floor);

void OpenGLRender_Line2(int x1, int z1, int x2, int z2);
void OpenGLRender_Line3(Vector3 start, Vector3 end);

void OpenGLRender_AnimateSprites();

Texture* OpenGLRender_GetTexture(s16 picnum);
void OpenGLRender_SetColor(color32 oc);
void OpenGLRender_DrawDot(Vector2 point, float size, color32 color);

void OpenGLRender_SetUnitsToMeter(float unitsToMeter);

s16 OpenGLRender_GetPicnumForName(zstr* textureName);

#ifdef __cplusplus
}
#endif
