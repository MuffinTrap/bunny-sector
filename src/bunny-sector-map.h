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

// Macros from bisqwit
#define map_min(a,b)             (((a) < (b)) ? (a) : (b)) // min: Choose smaller of two scalars.
#define map_max(a,b)             (((a) > (b)) ? (a) : (b)) // max: Choose greater of two scalars.
#define map_clamp(a, mi,ma)      map_min(map_max(a,mi),ma)         // clamp: Clamp value into set range.
#define vxs(x0,y0, x1,y1)    ((x0)*(y1) - (x1)*(y0))   // vxs: Vector cross product
// Overlap:  Determine whether the two number ranges overlap.
#define Overlap(a0,a1,b0,b1) (map_min(a0,a1) <= map_max(b0,b1) && map_min(b0,b1) <= map_max(a0,a1))
// IntersectBox: Determine whether two 2D-boxes intersect.
#define IntersectBox(x0,y0, x1,y1, x2,y2, x3,y3) (Overlap(x0,x1,x2,x3) && Overlap(y0,y1,y2,y3))
#define IntersectBoxV(v0, v1, v2, v3) (Overlap(v0.x,v1.x,v2.x,v3.x) && Overlap(v0.y,v1.y,v2.y,v3.y))

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
	virtual int GetNeighbourOfWall(int sectorIndex, int wallIndex) = 0;
	virtual float GetFloory(int sectorIndex) = 0;
	virtual float GetCeilingy(int sectorIndex) = 0;

	virtual MaterialId GetSectorMaterial(int sectorIndex, bool floor) = 0;
	virtual u8 GetSectorShade(int sectorIndex, bool floor) = 0;

	virtual int FindSectorV2(int currentSector, Vector2 currentPosition) = 0;

	virtual u32 MovePointInMap(
	Vector2 start, Vector2 end, float radius, s16 sectorNumber,
	float elevationEnd, float maxElevationChange, float height,
	Vector2* positionOut, s16* sectorOut) = 0;

	virtual void PrintInfo() = 0;

	virtual WallInfo GetWallInfo(int sectorIndex, int wallIndex);

	zstr* GetMapFile();
	void MoveActorInMap(float delta,Actor* actor);

    virtual bool IsPointInsideWall(Vector2 point, Vector2 wallStart, Vector2 wallEnd) = 0;

	bool FindIntersectionWithWall(Vector2 moveStart, Vector2 moveEnd, Vector2 wallStart, Vector2 wallEnd, Vector2* pointOUT);

	bool FindIntersectionWithWallUT(
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    float x4,
    float y4,
    Vector2* pointOUT);

	static Vector2 GetWallNormal(Vector2 wallStart, Vector2 wallEnd);
	static bool CircleCollidesWithWall(Vector2 center, float radius, Vector2 p1, Vector2 p2);
	static float GetDistanceToWall(Vector2 wallStart, Vector2 wallEnd, Vector2 point);

	// Accessed by OpenGLRenderer for tesselation and drawing
    MapFloorVertexData floorVertexData;
    float lowY;
    float highY;

	//
	BunnyMapType m_type;
    zstr mapfile;
};

