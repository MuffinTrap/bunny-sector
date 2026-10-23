#pragma once
#include <mgdl.h>
#include "shell-grass.h"
struct RenderSettingsOpenGL;
struct Wall;
struct Texture;
class BunnySector_Map;

#define RENDERER_DEFAULT_MATERIAL_ID 0

struct FogParameters
{
	color32 color;
	int intensityPercentage;
};

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
void OpenGLRender_StartDrawingPolygons();

	void OpenGLRender_DrawWallV(Vector2 start, Vector2 end, Vector2 normalXZ, s32 floorY, s32 ceilingY, s16 picnum, s8 shade, u32 linedef_flags);

	void OpenGLRender_DrawActors(BunnySector_Map* map, Vector2 cameraPosition);
	void OpenGLRender_DrawSprite(Vector3 position, float width, float height, float spriteAngle, Vector2 playerPosition, SpriteAlignment alignment, SpritePivot pivot, s16 picnum, s8 brightnessOffset)
;
	void OpenGLRender_DrawSkybox(BunnySector_Map* map);

	void OpenGLRender_DrawQuad(Vector2 start, Vector2 end, Vector2 normalXZ, float floorY, float ceilingY, s16 picnum, s8 brightnessOffset, float uvScale);
/**
 * @brief Finalizes the drawing
 */
void OpenGLRender_EndDrawingPolygons();


// Tesselate and store all floors to buffer
void OpenGLRender_CreateFloorBuffers(BunnySector_Map* map);
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

void OpenGLRender_SetFogEnabled(bool enabled);
void OpenGLRender_SetFogLimits(float start, float end);
void OpenGLRender_SetFogColor(color32 color);
void OpenGLRender_SetFogDensityPercent(int opacity);

void OpenGLRender_SetSkyboxEnabled(bool enabled);
void OpenGLRender_SetSkyboxColors(color32 zenith, color32 horizon, color32 nadir);

/**
 * @brief Sets up rendering state to draw floors and ceilings from a buffer
 */
void OpenGLRender_StartDrawingFloorsFromBuffer(BunnySector_Map* map);
void OpenGLRender_DrawFloorOrCeiling(BunnySector_Map* map, int sectorIndex, u8 shade, float ycoord, MaterialId materialId, bool floor);

void OpenGLRender_Line2(int x1, int z1, int x2, int z2);
void OpenGLRender_Line3(Vector3 start, Vector3 end);

void OpenGLRender_AnimateSprites();

MapMaterial* OpenGLRender_GetMaterialForMaterialId(MaterialId picnum);
void OpenGLRender_SetColor(color32 oc);
void OpenGLRender_DrawDot(Vector2 point, float size, color32 color);

void OpenGLRender_SetUnitsToMeter(float unitsToMeterHorizontal, float unitsToMeterVertical);
void OpenGLRender_SetTextureScale(float scale);

s16 OpenGLRender_GetPicnumForName(zstr* textureName);

#ifdef __cplusplus
}
#endif
