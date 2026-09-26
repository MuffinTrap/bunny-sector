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
	virtual int GetActorAmount() = 0;
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
	virtual float GetFloory(int subSectorIndex) = 0;
	virtual float GetCeilingy(int subSectorIndex) = 0;

	virtual MaterialId GetSectorMaterial(int sectorIndex, bool floor) = 0;
	virtual u8 GetSectorShade(int sectorIndex, bool floor) = 0;

	virtual int FindSubSectorV2(int currentSubSector, Vector2 currentPosition) = 0;

	virtual int GetSpriteAmount() = 0;

	virtual u32 MoveActorInMapImpl(
	Vector2 start, Vector2 end, float radius, s16 sectorNumber,
	float elevationEnd, float maxElevationChange, float height, Actor* actor,
	Vector2* positionOut, s16* subSectorOut) = 0;

	virtual void PrintInfo() = 0;

	virtual WallInfo GetWallInfo(int sectorIndex, int wallIndex);
	virtual void UpdateActions(float delta) = 0;

	zstr* GetMapFile();
	void MoveActors(float delta);
	void MoveActorInMap(float delta,Actor* actor);
	void AllocateActors();
	virtual void CreateActors() = 0;

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

	// All the actors in this map
	// TODO Keep actors sorted by sector to make drawing and collision etc faster
	Actor* actors;
	int actorCount;

	Actor* GetActor(ActorType aType, int index);
	Actor* GetActorById(int actorId);

	//
	BunnyMapType m_type;
    zstr mapfile;
};

