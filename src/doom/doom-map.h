#pragma once

#include "doom_types.h"
#include "../bunny-sector-map.h"
#include "../bunny-sector-types.h"

class Actor;

#define DOOM_MAP_ACTION_AMOUNT 32

struct DoomMapAction
{
	float startTime;
	float accumulation;
	int state;
	DoomLinedef* trigger;
};

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

	// Active actions
	DoomMapAction* actions;
	int actionCount;

	void SetActorToStart(Actor* actor) override;
	int GetSectorAmount() override;
	int GetWallVertexAmount() override;
	int GetWallAmountInSector(int sectorIndex) override;
	int GetSectorFirstWallIndex(int sectorIndex) override;
	Vector2 GetWallVertexInSector(int sectorIndex, int wallIndex) override;
	Vector2 GetNextWallVertexInSector(int sectorIndex, int wallIndex) override;
	int GetNextWallVertexIndexInSector(int sectorIndex, int wallIndex) override;
	MaterialId GetSectorMaterial(int sectorIndex, bool floor) override;
	float GetCeilingy(int subSectorIndex) override;
	float GetFloory(int subSectorIndex) override;
	int FindSubSectorV2(int currentSector, Vector2 currentPosition) override;

	Vector2 GetSectorSize(int sectorIndex) override;
	Vector2 GetSectorMaxTexCoord(int sectorIndex) override;
	Vector2 GetSectorMinPoint(int sectorIndex) override;

	int GetNeighbourOfWall(int sectorIndex, int wallIndex) override;

	void CreateActors() override;

	int GetSpriteAmount() override;

	u8 GetSectorShade(int sectorIndex, bool floor) override;
	void PrintInfo() override;
	MapUpdateResult UpdateActions(float delta) override;

	u32 MoveActorInMapImpl(Vector2 start, Vector2 end, float size, s16 sectorNumber, float elevationEnd, float maxElevationChange, float height, Actor* actor, Vector2* positionOut, s16* subSectorOut);

	void StartLinedefAction(DoomLinedef* line, Actor* actor, bool crossed);

	bool IsPointInsideWall(Vector2 point, Vector2 wallStart, Vector2 wallEnd) override;
	float GetSectorCeilingy(int sectorIndex);
	float GetSectorFloory(int sectorIndex);

	void AddActor(ActorType actorType, int typeNumber, int id, Vector2 position, float angleDeg);

	int FindSubSector(DoomNode* node, Vector2 point);

	void StartAction(DoomLinedef* trigger);
	void ClearActions() override;

	bool OpenDoorSector(int sectorIndex, DoomSector* sector, int heightChange);
	bool CloseDoorSector(DoomSector* sector, int heightChange);

	bool DoOpenDoorAction(DoomMapAction* act, float delta);
	bool DoCloseDoorAction(DoomMapAction* act, float delta);
	void Allocate(int thingsAmount, int sectorAmount, int sideAmount, int lineAmount, int vertexAmount);
	void AllocateNodes(int nodeAmount);
	void AllocateSegments(int segmentAmount);
	void AllocateSubsectors(int subSectorAmount);

// property accessors for AngelScript
DoomThing* GetThing(unsigned int index);
DoomSector* GetSector(unsigned int index);
DoomSidedef* GetSidedef(unsigned int index);
DoomLinedef* GetLinedef(unsigned int index);
DoomVertex* GetVertex(unsigned int index);
DoomNode* GetNode(unsigned int index);
DoomSubSector* GetSubSector(unsigned int index);
DoomSegment* GetSegment(unsigned int index);

DoomNode* GetRootNode();
DoomNode* GetChildNode(ChildId id);
int GetActorAmount() override;
DoomSubSector* GetChildSubSector(ChildId id);


};
typedef class DoomMap DoomMap;


// Interface BunnySector_Map


