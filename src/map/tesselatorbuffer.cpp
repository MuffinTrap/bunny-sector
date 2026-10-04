#include <mgdl.h>
#include "tesselatorbuffer.h"

void TesselatorBuffer::Init(GLfloat* vertices, u32 verticesSize, GLushort* indices, u32 indicesSize, RectF uvLimits)
{

	vertexBuffer = vertices;
	indexBuffer = indices;
	vertexBufferSize = verticesSize;
	indexBufferSize = indicesSize;
    activeUVLimits = uvLimits;

    vertexBufferVertexIndex = 0;
    indexBufferIndex = 0;
    mgdl_assert_print(indexBuffer != nullptr && vertexBuffer != nullptr, "Tesselator received null pointers for buffer addresses");
}

u32 TesselatorBuffer::GetIndexIndex()
{
	return indexBufferIndex;
}
u32 TesselatorBuffer::GetVertexIndex()
{
	return vertexBufferVertexIndex;
}

bool TesselatorBuffer::IsValid()
{
    mgdl_assert_print(indexBuffer != nullptr && vertexBuffer != nullptr, "Tesselator has no buffers set, cannot start polygon");
	return true;
}




/**
 * @brief Put a vertex in the output buffer and set the indice of it in index buffer
 */
void TesselatorBuffer::BufferVertex(const float x, const float y, const float z, const float u, const float v)
{
#ifdef TESS_DEBUG
    Log_InfoF("Buffer vertex got C(%.2f %.2f, %.2f), TX(%.2f, %.2f)\n", x, y, z, u, v);
#endif
    static const float tolerance = 0.9f; // Duke units are integers, so this can be quite large
    static const float uvTolerance = 0.001f; // This is way smaller because values usually are under 10
    // Is this vertex already in the buffer?
    bool found = false;
    GLushort index = 0;
    // Incoming vertex
    Vector2 V = Vector2New(x,z);
    Vector2 TX = Vector2New(u,v);
    for (int i = 0; i < vertexBufferVertexIndex; i++)
    {
        GLfloat* vertex = &vertexBuffer[i * VERTEX_BUFFER_VERTEX_SIZE];

        // Existing vertex
        Vector2 ex = Vector2New(vertex[0], vertex[2]);
        float d = Vector2Length( Vector2Subtract(ex, V));
        if (d < tolerance)
        {
            Vector2 exTx = Vector2New(vertex[3], vertex[4]);
            float dtx = Vector2Length( Vector2Subtract(exTx, TX));
            if (dtx < uvTolerance)
            {
#               ifdef TESS_DEBUG
                    Log_InfoF("Found at index %d\n", i);
#               endif
                found = true;
                index = i;
                break;
            }
        }
    }
    if (found)
    {
        if (indexBufferIndex < indexBufferSize)
        {
            indexBuffer[indexBufferIndex] = index;
            indexBufferIndex++;
        }
        else
        {
            indexBufferIndex++;
            Log_ErrorF("Tesselator ran out of space in the index buffer, needs at least %d indices\n", indexBufferIndex);
        }
    }
    else
    {
        //Log_InfoF("New vertex to index %d\n", vertexBufferVertexIndex);
        mgdl_assert_print(vertexBufferSize > vertexBufferVertexIndex, "Tesselator ran out of space in vertex buffer");
        GLfloat* vertex = &vertexBuffer[vertexBufferVertexIndex * VERTEX_BUFFER_VERTEX_SIZE];
        vertex[0] = x;
        vertex[1] = y;
        vertex[2] = z;

        // TODO calculate these later?
        vertex[3] = activeUVLimits.x + u * activeUVLimits.w;
        vertex[4] = activeUVLimits.y + v * activeUVLimits.h;


        if (indexBufferIndex < indexBufferSize)
        {
            indexBuffer[indexBufferIndex] = vertexBufferVertexIndex;
            indexBufferIndex++;
        }
        else
        {
            indexBufferIndex++;
            Log_ErrorF("Tesselator ran out of space in the index buffer, needs at least %d indices\n", indexBufferIndex);
        }
        vertexBufferVertexIndex += 1;
    }
}

/*
 * @brief Calculate the uv coordinates of a floor or ceiling vertex in a sector
 */
Vector2 TesselatorBuffer::CalculateFloorOrCeilingUV(Vector2 size, Vector2 minPoint, Vector2 maxTexCoord, Vector2 vertex, float unitsPerMeterForUV)
{
    float xrange = size.x;
    float zrange = size.y;
    float xdiff = vertex.x - minPoint.x;
    float zdiff = vertex.y - minPoint.y;
    // NOTE The texture will repeat like crazy because these are duke units
    float tx = xdiff/xrange * maxTexCoord.x;
    float tz = zdiff/zrange * maxTexCoord.y;
    return Vector2New(tx/unitsPerMeterForUV, tz/unitsPerMeterForUV);
}
