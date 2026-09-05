#pragma once

#include "duke/dukemap.h"
#include "doom/doom-map.h"

// Abstract map file

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

	union MapPointer
	{
		DukeMap* dukeMap;
		DoomMap* doomMap;
	};

public:
	zstr* GetMapFile();
	int GetSectorAmount();
	int GetWallVertexAmount();
	int GetWallAmountInSector(int sectorIndex);
	int GetNextWallVertexIndex(int sectorIndex, int wallIndex);
	Vector2 GetNextWallVertex(int sectorIndex, int wallIndex);
	Vector2 GetWallVertexInSector(int sectorIndex, int wallIndex);

    Vector2 GetSectorSize(int sectorIndex);
    Vector2 GetSectorMaxTexCoord(int sectorIndex);
    Vector2 GetSectorMinPoint(int sectorIndex);
	int GetSectorFirstWallIndex(int sectorIndex);
	void SetActorToStart(Actor* actor);
	void MoveActorInMap(float delta,Actor* actor);
	int GetNeighbourOfWall(int sectorIndex, int wallIndex);
	float GetFloory(int sectorIndex);
	float GetCeilingy(int sectorIndex);

	// Accessed by OpenGLRenderer
    MapFloorVertexData floorVertexData;
	MapPointer mapPtr;
	BunnyMapType m_type;
    zstr mapfile;
};
typedef class BunnySector_Map BunnySector_Map;

