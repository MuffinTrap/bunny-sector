#include "tesselator.h"
#include "tesselatorbuffer.h"

// TODO Use some other tesselation library with more memory management options

GLUtesselator* tesselator = nullptr;
static TesselatorBuffer tessBuffer;
static RectF activeUVLimits;

#ifdef MGDL_PLATFORM_WINDOWS
#define _GLUfuncptr void(*)()
#endif

// TESSELATION CALLBACKS
// /////////////////////

// Ring buffer for tesselation input
// This buffer needs to hold all the vertices of a sector floor or ceiling
// for tesselation, so it is large
#define TESSELATION_BUFFER_SIZE_DOUBLES (3*128) // Divisible by three for the ring buffering to work; 3 doubles per vertex
#define TESSELATION_BUFFER_SIZE_BYTES (TESSELATION_BUFFER_SIZE_DOUBLES * sizeof(double))
static GLdouble* tesselationBuffer = nullptr;
static int tesselationBufferIndexDoubles = 0;

// Ring buffer for tesselation combine
// This is needed if two vertices are identical, but hopefully it is not needed
#define COMBINE_BUFFER_SIZE_DOUBLES (3*9) // Divisible by three for the ring buffering to work; 3 doubles per vertex
#define COMBINE_BUFFER_SIZE_BYTES (COMBINE_BUFFER_SIZE_DOUBLES * sizeof(double))
static GLdouble* combineRingBuffer = nullptr;
static int CombineBufferIndexDoubles = 0;



#ifndef CALLBACK
#define CALLBACK
#endif

void CALLBACK tessBegin(GLenum which)
{
    //Log_InfoF("Tesselation start mode: %s \n", which == GL_TRIANGLES ? "Triangles" : "Not triangles");
}

// This puts a new vertex into the buffer: called after gluTessEndPolygon
void CALLBACK tessVertex(GLvoid* vertex)
{

    const GLdouble* coordinates = (GLdouble*)vertex;
    // Log_InfoF("Tesselation vertex C(%.2f %.2f, %.2f), TX(%.2f, %.2f, %.2f)\n", coordinates[0], coordinates[1], coordinates[2], coordinates[3], coordinates[4], coordinates[5]);
    tessBuffer.BufferVertex(
        (GLfloat)coordinates[0], (GLfloat)coordinates[1], (GLfloat)coordinates[2],
        (GLfloat)coordinates[3], (GLfloat)coordinates[4]);
}

void CALLBACK tessCombine(GLdouble coords[3], GLdouble* vertex_data[4], GLfloat weight[4], GLdouble **dataOut)
{
    //Log_InfoF("Tesselation combine vertex: %.2f, %.2f\n", coords[0], coords[2]);
    if (CombineBufferIndexDoubles + 6 >= COMBINE_BUFFER_SIZE_DOUBLES)
    {
        CombineBufferIndexDoubles = 0;
    }
    // Reads 6 doubles
    GLdouble* vertex = &combineRingBuffer[CombineBufferIndexDoubles];

    // Coordinates of the combined vertex
    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    vertex[3] = 0.0f;
    vertex[4] = 0.0f;
    vertex[5] = 0.0f;
    /*  This causes crashes so don't do it
     *    for (int i = 3; i < 6; i++)
     *    {
     *        vertex[i] = weight[0] * vertex_data[0][i] +
     *                    weight[1] * vertex_data[1][i] +
     *                    weight[2] * vertex_data[2][i] +
     *                    weight[3] * vertex_data[3][i];
}
*/
    *dataOut = vertex;
    CombineBufferIndexDoubles = (CombineBufferIndexDoubles + 6) % COMBINE_BUFFER_SIZE_DOUBLES;
}

void CALLBACK tessEnd(void)
{

}

void CALLBACK tessError(GLenum errorCode)
{
    const GLubyte* str;
    str = gluErrorString(errorCode);
    Log_ErrorF("Tesselation error: %s\n", str);
}

void CALLBACK tessEdgeFlag(GLboolean flag)
{
    #ifndef GEKKO
    glEdgeFlag(flag);
    #endif
}

void Tesselator_Init()
{
    if (tesselator == nullptr)
    {
        tesselator = gluNewTess();
        mgdl_assert_print(tesselator != nullptr, "No Glut tesselator!");

        gluTessCallback(tesselator, GLU_TESS_BEGIN, (_GLUfuncptr)tessBegin);
        gluTessCallback(tesselator, GLU_TESS_VERTEX, (_GLUfuncptr)tessVertex);
        gluTessCallback(tesselator, GLU_TESS_END, (_GLUfuncptr)tessEnd);
        gluTessCallback(tesselator, GLU_TESS_ERROR, (_GLUfuncptr)tessError);
        gluTessCallback(tesselator, GLU_TESS_EDGE_FLAG, (_GLUfuncptr)tessEdgeFlag); // this makes tess only submit triangles
        gluTessCallback(tesselator, GLU_TESS_COMBINE, (_GLUfuncptr)tessCombine);
        if (tesselationBuffer == nullptr)
        {
            tesselationBuffer = (GLdouble*)mgdl_AllocateGraphicsMemory(TESSELATION_BUFFER_SIZE_BYTES);
        }
        if (combineRingBuffer == nullptr)
        {
            combineRingBuffer = (GLdouble*)mgdl_AllocateGraphicsMemory(COMBINE_BUFFER_SIZE_BYTES);
        }
    }
}

void Tesselator_SetBuffers(GLfloat* vertices, u32 verticesSize, GLushort* indices, u32 indicesSize)
{
    tessBuffer.Init(vertices, verticesSize, indices, indicesSize, activeUVLimits);
}
/**
 * @brief Returns the starting index in vertices buffer
 */
Tesselator_BufferIndices Tesselator_BeginPolygon(GLfloat normal[3], RectF uvLimits)
{
    mgdl_assert_test (tessBuffer.IsValid());

    activeUVLimits = uvLimits;
    gluTessNormal(tesselator, normal[0], normal[1], normal[2]);
    gluTessBeginPolygon(tesselator, NULL);

    tesselationBufferIndexDoubles = 0; // Start from beginning

    Tesselator_BufferIndices indices;
    indices.indexCount = 0;
    indices.indexIndex = tessBuffer.GetIndexIndex();
    indices.vertexCount = 0;
    indices.vertexIndex = tessBuffer.GetVertexIndex();
    return indices;
}
void Tesselator_BeginContour(Tesselator_ContourType ctype)
{
    gluTessBeginContour(tesselator);

}
void Tesselator_EndContour()
{
    gluTessEndContour(tesselator);

}
void Tesselator_AddVertexToPoly(GLfloat vertex[3], GLfloat uv[2])
{

    // Tesselation
    // NOTE DANGER Must be counter clockwise
    //Log_InfoF("Tesselation vertex AddToPoly C(%.2f %.2f, %.2f), TX(%.2f, %.2f)\n", vertex[0], vertex[1], vertex[2], uv[0], uv[1]);
    // TODO Send normal too, but maybe not with every vertex?
    tesselationBuffer[tesselationBufferIndexDoubles + 0] = vertex[0];
    tesselationBuffer[tesselationBufferIndexDoubles + 1] = vertex[1];
    tesselationBuffer[tesselationBufferIndexDoubles + 2] = vertex[2];
    tesselationBuffer[tesselationBufferIndexDoubles + 3] = uv[0];
    tesselationBuffer[tesselationBufferIndexDoubles + 4] = uv[1];
    tesselationBuffer[tesselationBufferIndexDoubles + 5] = 0.0f;

    // NOTE  Always put the same address for both, even when their data is different
    // glutess does the pointer arithmetic itself.
    gluTessVertex(tesselator,
                  &tesselationBuffer[tesselationBufferIndexDoubles], &tesselationBuffer[tesselationBufferIndexDoubles]);

    /*
    Log_InfoF("Tesselation buffer at %d C(%.2f %.2f, %.2f), TX(%.2f, %.2f, %.2f)\n",
              tesselationBufferIndexDoubles,
              tesselationBuffer[tesselationBufferIndexDoubles+0], tesselationBuffer[tesselationBufferIndexDoubles+1], tesselationBuffer[tesselationBufferIndexDoubles+2],
              tesselationBuffer[tesselationBufferIndexDoubles+3], tesselationBuffer[tesselationBufferIndexDoubles+4], tesselationBuffer[tesselationBufferIndexDoubles+5] );
              */


    tesselationBufferIndexDoubles = (tesselationBufferIndexDoubles + 6) % TESSELATION_BUFFER_SIZE_DOUBLES;

}
/**
 * @brief Returns the amount of triangles in the polygon
 */
Tesselator_BufferIndices Tesselator_EndPolygon()
{
    mgdl_CacheFlushRange(tesselationBuffer, TESSELATION_BUFFER_SIZE_BYTES);
    gluTessEndPolygon(tesselator);
    Tesselator_BufferIndices indices;
    indices.indexIndex = tessBuffer.GetIndexIndex();
    indices.vertexIndex = tessBuffer.GetVertexIndex();
    // Log_InfoF("Tesselator end polygon to vertex %d, index %d\n", vertexBufferVertexIndex, indexBufferIndex);
    return indices;
}

void Tesselator_Deinit()
{
    gluDeleteTess(tesselator);
    mgdl_FreeGraphicsMemory(tesselationBuffer);
    mgdl_FreeGraphicsMemory(combineRingBuffer);
}
