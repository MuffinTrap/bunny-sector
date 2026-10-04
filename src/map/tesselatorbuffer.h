#pragma once

#define VERTEX_BUFFER_VERTEX_SIZE 5

class TesselatorBuffer
{
public:
	void Init(GLfloat* vertices, u32 verticesSize, GLushort* indices, u32 indicesSize, RectF uvLimits);
	void BufferVertex(const float x, const float y, const float z, const float u, const float v);
	bool IsValid();

	static Vector2 CalculateFloorOrCeilingUV(Vector2 size, Vector2 minPoint, Vector2 maxTexCoord, Vector2 vertex, float unitsPerMeterForUV);

	u32 GetVertexIndex();
	u32 GetIndexIndex();

private:

	// These are given as parameters
	GLfloat* vertexBuffer = nullptr;
	GLushort* indexBuffer = nullptr;
	u32 vertexBufferSize = 0;
	u32 indexBufferSize = 0;
	RectF activeUVLimits;

	// Tesselation counting
	u32 vertexBufferVertexIndex = 0;
	u32 indexBufferIndex = 0;

};
