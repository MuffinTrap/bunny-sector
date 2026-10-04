#include "polytesselator.h"

#include <list>

PolyTesselator::PolyTesselator(GLfloat* vertices, u32 verticesSize, GLushort* indices, u32 indicesSize)
{
	tessBuffer.Init(vertices, verticesSize, indices, indicesSize, RectF_Create(0,0, 1.0f, 1.0f));
}

Tesselator_BufferIndices PolyTesselator::BeginPolygon(GLfloat normal[3], RectF uvLimits, int vertexCount)
{
	this->normal[0] = normal[0];
	this->normal[1] = normal[1];
	this->normal[2] = normal[2];

    Tesselator_BufferIndices indices;
    indices.indexCount = 0;
    indices.indexIndex = tessBuffer.GetIndexIndex();
    indices.vertexCount = 0;
    indices.vertexIndex = tessBuffer.GetVertexIndex();

	activePoly = TPPLPoly();
	activePoly.Init(vertexCount);
	activePolyVertexIndex = 0;

	return indices;
}

void PolyTesselator::BeginContour(Tesselator_ContourType ctype)
{
	activePoly.SetHole(ctype == contour_hole);
}

void PolyTesselator::AddVertexToPoly(GLfloat vertex[3], GLfloat uv[2])
{
	activePoly[activePolyVertexIndex].x = vertex[0];
	activePoly[activePolyVertexIndex].y = vertex[2]; // NOTE This is OpenGL so floor is x-z
	activePolyVertexIndex += 1;
}

void PolyTesselator::EndContour()
{

}

Tesselator_BufferIndices PolyTesselator::EndPolygon(Vector2 polygonSize, Vector2 polygonMinPoint, Vector2 polygonMaxUV, float unitsPerMeterUV)
{
	resultPolys.clear();
	// NOTE Flip order of active
	//activePoly.Invert();
	partitionObject.Triangulate_OPT(&activePoly, &resultPolys);
	// Write results to buffer
	std::list<TPPLPoly>::iterator iter;
	for (iter = resultPolys.begin(); iter != resultPolys.end(); iter++)
	{
		for (int i = 0; i < iter->GetNumPoints(); i++)
		{
			float x = iter->GetPoint(i).x;
			float y= iter->GetPoint(i).y;

			Vector2 uv = tessBuffer.CalculateFloorOrCeilingUV(polygonSize, polygonMinPoint, polygonMaxUV, Vector2New(x, y), unitsPerMeterUV);
			tessBuffer.BufferVertex(x, 0.0f, y, uv.x, uv.y);
			// NOTE                 X  Y     Z  because OpenGL
		}
	}

    Tesselator_BufferIndices indices;
    indices.indexCount = 0;
    indices.indexIndex = tessBuffer.GetIndexIndex();
    indices.vertexCount = 0;
    indices.vertexIndex = tessBuffer.GetVertexIndex();

	return indices;
}









