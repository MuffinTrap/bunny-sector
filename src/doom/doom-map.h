#pragma once

#include "doom_types.h"
#include "../bunny-sector-map.h"
struct Actor;

class DoomMap : public BunnySector_Map
{
public:
	int thingAmount;
	int sectorAmount;
	int sideAmount;
	int lineAmount;
	int vertexAmount;

	DoomThing* things;
	DoomSector* sectors;
	DoomSidedef* sidedefs;
	DoomLinedef* linedefs;
	DoomVertex* vertices;

	// Node data
	int nodeAmount;
	int segmentAmount;
	int subSectorAmount;
	DoomNode* nodes; // NOTE Last one is the root node
	DoomSubSector* subsectors;
	DoomSegment* segments;

	void SetActorToStart(Actor* actor) override;
	int GetSectorAmount() override;
	int GetWallVertexAmount() override;
	int GetWallAmountInSector(int sectorIndex) override;
	int GetSectorFirstWallIndex(int sectorIndex) override;
	Vector2 GetWallVertexInSector(int sectorIndex, int wallIndex) override;
	Vector2 GetNextWallVertexInSector(int sectorIndex, int wallIndex) override;
	int GetNextWallVertexIndexInSector(int sectorIndex, int wallIndex) override;
	MaterialId GetSectorMaterial(int sectorIndex, bool floor) override;
	float GetCeilingy(int sectorIndex) override;
	float GetFloory(int sectorIndex) override;
	int FindSectorV2(int currentSector, Vector2 currentPosition) override;

	Vector2 GetSectorSize(int sectorIndex) override;
	Vector2 GetSectorMaxTexCoord(int sectorIndex) override;
	Vector2 GetSectorMinPoint(int sectorIndex) override;

	int GetNeighbourOfWall(int sectorIndex, int wallIndex) override;

	u8 GetSectorShade(int sectorIndex, bool floor) override;
	void PrintInfo() override;

	u32 MovePointInMap(
	Vector2 start, Vector2 end, float radius, s16 sectorNumber,
	float elevationEnd, float maxElevationChange, float height,
	Vector2* positionOut, s16* sectorOut) override;

	bool IsPointInsideWall(Vector2 point, Vector2 wallStart, Vector2 wallEnd) override;

	int FindSubSector(DoomNode* node, Vector2 point);

};
typedef class DoomMap DoomMap;

void DoomMap_Allocate(DoomMap* map, int thingsAmount, int sectorAmount, int sideAmount, int lineAmount, int vertexAmount);
void DoomMap_AllocateNodes(DoomMap* map, int nodeAmount);
void DoomMap_AllocateSegments(DoomMap* map, int segmentAmount);
void DoomMap_AllocateSubsectors(DoomMap* map, int subSectorAmount);

// property accessors for AngelScript
DoomThing* DoomMap_GetThing(DoomMap* map, unsigned int index);
DoomSector* DoomMap_GetSector(DoomMap* map, unsigned int index);
DoomSidedef* DoomMap_GetSidedef(DoomMap* map, unsigned int index);
DoomLinedef* DoomMap_GetLinedef(DoomMap* map, unsigned int index);
DoomVertex* DoomMap_GetVertex(DoomMap* map, unsigned int index);
DoomNode* DoomMap_GetNode(DoomMap* map, unsigned int index);
DoomSubSector* DoomMap_GetSubSector(DoomMap* map, unsigned int index);
DoomSegment* DoomMap_GetSegment(DoomMap* map, unsigned int index);

DoomNode* DoomMap_GetRootNode(DoomMap* map);
DoomNode* DoomMap_GetChildNode(DoomMap* map, ChildId id);
DoomSubSector* DoomMap_GetChildSubSector(DoomMap* map, ChildId id);

// Interface BunnySector_Map


