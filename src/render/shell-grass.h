#pragma once

#include <mgdl.h>

struct ShellGrass
{
	u8 shellAmount;
	Texture* texture;
	float uvRepeat;
	float height;
};

typedef void (*BufferDrawFunction)(void);

ShellGrass* CreateGrass(u16 widthPixels, u16 heightPixels, u8 shellAmount, float height, float uvRepeat);
void DrawGrass(ShellGrass* grass, color32 color, float minIntensity, float maxIntensity, float size, float ystepoffset);

void DrawGrassOnPolygonBuffer(ShellGrass* grass, color32 color, Vector3 growDirection, float height, float minIntensity, BufferDrawFunction drawFunction, float activeScale);
