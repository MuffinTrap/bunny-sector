#pragma once
#include <mgdl.h>

#ifdef __cplusplus
    extern "C" {
#endif

#include "../bunny-sector-types.h"


void Tesselator_Init();

void Tesselator_SetBuffers(GLfloat* vertices, u32 verticeSize, GLushort* indices, u32 indicesSize);
/**
 * @brief Returns the indices before polygon
 */
Tesselator_BufferIndices Tesselator_BeginPolygon(GLfloat normal[3], RectF uvLimits);
Tesselator_BufferIndices Tesselator_BeginKnownPolygon(GLfloat normal[3], RectF uvLimits, int vertexAmount);
void Tesselator_AddVertexToPoly(GLfloat vertex[3], GLfloat uv[2]);
void Tesselator_BeginContour(Tesselator_ContourType ctype);
void Tesselator_EndContour();
/**
 * @brief Returns the indices after polygon
 */
Tesselator_BufferIndices Tesselator_EndPolygon();

void Tesselator_Deinit();

#ifdef __cplusplus
}
#endif
