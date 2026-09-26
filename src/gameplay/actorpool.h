
#pragma once

struct Actor;

class ActorPool
{
public:
	int count;
	void Init(int poolCapacity);
	void Clear();

	void Insert(Actor actor);
	void Remove(Actor* actor);
	void RemoveAt(int index);
	void Move(Actor* actor, int oldSubSector, int newSubSector);
	void MoveByIndex(int index, int oldSubSector, int newSubSector);
	Actor* GetActorById(int actorId);
	Actor* GetActorByIndex(int actorIndex);
	Actor* GetActorByTypeAndIndex(ActorType aType, int index);
	Actor* GetActorInSectorByIndex(int subSectorNumber, int index, int startIndex);

	int FindSectorIndex(int subSectorNumber);
private:
	int capacity;
	Actor* actors = nullptr;

	/**
	 * @brief Move actors from left of the place to fill it
	 * @returns Index of the place that is free after moving
	 */
	int FillFromLeft(int emptyPlace, int stopAt);
	/**
	 * @brief Move actors from right of the place to fill it
	 * @returns Index of the place that is free after moving
	 */
	int FillFromRight(int emptyPlace, int stopAt);

	/**
	 * @brief Finds index of actor
	 * @returns Index or -1 if not found
	 */
	int Find(Actor* actor);
};
