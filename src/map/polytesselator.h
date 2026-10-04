#pragma once

#include <mgdl.h>
#include "../bunny-sector-types.h"
#include "../polypartition/polypartition.h"
#include "tesselatorbuffer.h"
#include <list>

class PolyTesselator
{
public:
	/**
	 * @brief Sets the buffer memory where the vertices should be written
	 * @param vertices Address of vertices in (x,y,z),(u,v) configuration
	 * @param verticesSize Size of vertices in floats
	 * @param indices Address of indices
	 * @param indicesSize Size of indices in shorts
	 */
	PolyTesselator(GLfloat* vertices, u32 verticesSize, GLushort* indices, u32 indicesSize);

	/**
	 * @brief Start a new polygon with a shared normal
	 * @param normal Normal of all points
	 * @param uvLimits Min and max of uv coordinates
	 * @param vertexCount How many vertices does this polygon have
	 * @returns Where the polygon starts in indices and vertices
	 */
	Tesselator_BufferIndices BeginPolygon(GLfloat normal[3], RectF uvLimits, int vertexCount = 0);

	/**
	 * @brief Begin a contour.
	 * @param ctype Is this the outline or a hole
	 */
	void BeginContour(Tesselator_ContourType ctype);

	/**
	 * @brief Adds a vertex to previously started polygon
	 * @param vertex X, Y, Z
	 * @param uv U and V
	 */
	void AddVertexToPoly(GLfloat vertex[3], GLfloat uv[2]);

	/**
	 * @brief End a contour in polygon
	 */
	void EndContour();

	/**
	 * @brief End the polygon
	 * @details The caller should compare the indices returned by BeginPolygon and EndPolygon to calculate
	 * the polygon size
	 * @returns The indices after the polygon was completed.
	 */
	Tesselator_BufferIndices EndPolygon(Vector2 polygonSize, Vector2 polygonMinPoint, Vector2 polygonMaxUV, float unitsPerMeterUV);

	void DeInit();

private:
	// Tracking
	GLfloat normal[3];

	TPPLPoly activePoly;
	int activePolyVertexIndex;

	TPPLPartition partitionObject;

	std::list<TPPLPoly> inputPolys, resultPolys;
	TesselatorBuffer tessBuffer;
};
