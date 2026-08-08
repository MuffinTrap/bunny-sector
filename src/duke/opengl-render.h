#pragma once
#include <mgdl/mgdl-types.h>
#include <mgdl/mgdl-material.h>
#include <mgdl/mgdl-color.h>
#include "dukemap.h"
#include "shell-grass.h"
struct RenderSettingsOpenGL;
struct DukeMap;
struct Sector;
struct Wall;
struct Texture;

#define RENDERER_PICNUM_DEFAULT 0

#ifdef __cplusplus
extern "C" {
#endif

enum MapMaterialType
{
	Material_Texture = 0, // Normal texture material
	Material_Grass = 1,  // Draw multiple shells of grass
	Material_Function = 2, // Use a custom function to draw on the area
	Material_SpriteModel = 3, // Draw a mesh instead of a texture for a sprite
	Material_SpriteAnimated = 4 // Animate a sprite sheet on sprite
};
typedef enum MapMaterialType MapMaterialType;

// Extension of normal material
struct MapMaterial
{
	Material* mgdlMaterial;
	MapMaterialType type;

	union MaterialData
	{
		s32 functionName; // If type is function, frame index if type is Animated sprite
		s32 spriteFrame;
		Mesh* meshPtr;		// If type is SpriteModel
	};

	union MaterialParameter
	{
		float grassLength; // If type is grass
		float frameDuration; // If type is Animated sprite
		float meshScale; // If type is Sprite Mesh
	};

	MaterialData data;
	MaterialParameter parameter;
};
typedef struct MapMaterial MapMaterial;

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
bool OpenGLRender_RegisterTexture(s16 picnum, Texture* texture);
void OpenGLRender_ReadMaterialsXML(const char* materialsfile);
bool OpenGLRender_RegisterMaterial(s16 picnum, Material* material, MapMaterialType materialType);
bool OpenGLRender_RegisterMapMaterial(s16 picnum, MapMaterial* material);

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
void OpenGLRender_CreateFloorBuffers(DukeMap* map);
/**
 * @brief Tesselates a floor of sector.
 */
void OpenGLRender_TesselateFloor(DukeMap* map, u16 sectorIndex);
void OpenGLRender_StopCountingFloorBufferSize();

/**
 * @brief Exports all geometry to obj file
 */
void OpenGLRender_WriteToObj(DukeMap* map, const char* filename, RenderSettingsOpenGL* settings);

void OpenGLRender_StartObjExport(DukeMap* map, const char* filename, RenderSettingsOpenGL* settings);
void OpenGLRender_StartFillingWallBuffer(DukeMap* map);
void OpenGLRender_BufferWalls(DukeMap* map);



/**
 * @brief Sets up rendering state to draw floors and ceilings from a buffer
 */
void OpenGLRender_StartDrawingFloorsFromBuffer(DukeMap* map);
void OpenGLRender_DrawFloorOrCeiling(DukeMap* map, s16 sectorIndex, bool floor);

void OpenGLRender_Line2(int x1, int z1, int x2, int z2);
void OpenGLRender_Line3(Vector3 start, Vector3 end);

void OpenGLRender_AnimateSprites();

Texture* OpenGLRender_GetTexture(s16 picnum);
void OpenGLRender_SetColor(color32 oc);
void OpenGLRender_DrawDot(Vector2 point, float size, color32 color);

#ifdef __cplusplus
}
#endif
