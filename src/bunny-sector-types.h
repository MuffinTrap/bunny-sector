#pragma once

#define DUKE_UNITS_TO_METER 1024.0f
#define DOOM_UNITS_TO_METER 32.0f

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

typedef int MaterialId;
#define INVALID_MATERIAL_ID -1
