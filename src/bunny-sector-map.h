#pragma once
#include "bunny-sector-types.h"
#include <mgdl.h>

// Abstract map file
struct Actor;

struct Tesselator_BufferIndices;
struct MapFloorVertexData
{
    // Store floor vertices of each sector to buffer
    // This buffer needs to hold all the vertices of every floor
    GLfloat* floorBuffer = nullptr; // All vertices of all floors: 3 position 2 uv
    static const u16 FLOOR_BUFFER_VERTEX_SIZE = 5; ///< How many floats per vertex
    u16 floorBufferSizeVertices = 0;

    GLushort* floorIndexBuffer = nullptr; // All indices of all floors
    u32 floorIndexBufferSize = 0;

    Tesselator_BufferIndices* floorStartIndices = nullptr; // Buffer end indices of each floor in vertex and index buffers: NOTE First floor starts at indices (0,0)
};
typedef struct MapFloorVertexData MapFloorVertexData;

enum BunnyMapType
{
	Map_Duke,
	Map_Doom,
	Map_Invalid
};
typedef enum BunnyMapType BunnyMapType;

class BunnySector_Map
{
public:
	virtual int GetSectorAmount() = 0;
	virtual int GetWallVertexAmount() = 0;
	virtual int GetWallAmountInSector(int sectorIndex) = 0;
	virtual int GetNextWallVertexIndexInSector(int sectorIndex, int wallIndex) = 0;
	virtual Vector2 GetNextWallVertexInSector(int sectorIndex, int wallIndex) = 0;
	virtual Vector2 GetWallVertexInSector(int sectorIndex, int wallIndex) = 0;

	virtual Vector2 GetSectorSize(int sectorIndex) = 0;
	virtual Vector2 GetSectorMaxTexCoord(int sectorIndex) = 0;
	virtual Vector2 GetSectorMinPoint(int sectorIndex) = 0;

	virtual int GetSectorFirstWallIndex(int sectorIndex) = 0;
	virtual void SetActorToStart(Actor* actor) = 0;
	virtual void MoveActorInMap(float delta,Actor* actor) = 0;
	virtual int GetNeighbourOfWall(int sectorIndex, int wallIndex) = 0;
	virtual float GetFloory(int sectorIndex) = 0;
	virtual float GetCeilingy(int sectorIndex) = 0;

	virtual MaterialId GetSectorMaterial(int sectorIndex, bool floor) = 0;
	virtual u8 GetSectorShade(int sectorIndex, bool floor) = 0;

	virtual int FindSectorV2(int currentSector, Vector2 currentPosition) = 0;

	virtual void PrintInfo() = 0;

	zstr* GetMapFile();

	// Accessed by OpenGLRenderer for tesselation and drawing
    MapFloorVertexData floorVertexData;
    float lowY;
    float highY;

	//
	BunnyMapType m_type;
    zstr mapfile;
};

