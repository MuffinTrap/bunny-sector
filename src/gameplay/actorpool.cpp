#include "actor.h"
#include "actorpool.h"
#include "actorcollision.h"
#include "../bunny-sector-math.h"
#include <mgdl.h>

void ActorPool::Init(int poolCapacity)
{
	count = 0;
	capacity = poolCapacity;
	if (actors == nullptr)
	{
		actors = (Actor*)mgdl_AllocateGeneralMemory(sizeof(Actor)*poolCapacity);
		AllocateActorCollisions();
	}
}

// TODO Do map instead ?
void ActorPool::AllocateActorCollisions()
{
    if (actorCollisionEntries == nullptr)
    {
        actorCollisionEntries = (ActorCollisionEntry*)mgdl_AllocateGeneralMemory(sizeof(ActorCollisionEntry) * MAP_ACTOR_COLLISION_ENTRY_AMOUNT);
        actorCollisionEntryCount = 0;
    }
    if (actorCollisionList == nullptr)
    {
        actorCollisionList = (Actor**)mgdl_AllocateGeneralMemory(sizeof(Actor*) * MAP_ACTOR_COLLISION_LIST_SIZE);
        actorCollisionListCount = 0;
    }
}
void ActorPool::Clear()
{
	count = 0;
}

int ActorPool::GetCollisionAmountForActor(Actor* actor)
{
	for(int i = 0; actorCollisionEntryCount; i++)
	{
		ActorCollisionEntry e = actorCollisionEntries[i];
		// TODO Should actors have a guid instead of pointer magic?
		if (e.collider == actor)
		{
			return e.collisionAmount;
		}
	}
	return 0;
}

Actor* ActorPool::GetCollisionForActor(Actor* actor, int collisionIndex)
{
	for(int i = 0; actorCollisionEntryCount; i++)
	{
		ActorCollisionEntry e = actorCollisionEntries[i];
		// TODO Should actors have a guid instead of pointer magic?
		if (e.collider == actor && collisionIndex < e.collisionAmount)
		{
			return actorCollisionList[e.collisionStartIndex + collisionIndex];
		}
	}
	return nullptr;
}

void ActorPool::DoActorToActorCollisions()
{
    // Clear lists
    actorCollisionEntryCount = 0;
    actorCollisionListCount = 0;

    // First pass:
    // Player against everything
    Actor* player0 = GetActorByTypeAndIndex(actor_player, 0);

    Actor* other = nullptr;
    int sectorStartIndex = FindSectorIndex(player0->subSectorNumber);
    int playerCollisionCount = 0;
    for(int index = 0; index < count; index++ )
    {
        other = GetActorInSectorByIndex(player0->subSectorNumber, index, sectorStartIndex);
        if (other == nullptr)
        {
            break;
        }
        if (other != player0)
        {
            if (TestActorActorCollision(player0, other))
            {
				zstr otherTyper = ActorTypeToString(other->actorType);
                printf("Player hit actor of type %s:%s\n", zstr_cstr(&otherTyper), DoomTypeToStringChar((DOOM_EDITOR_NUMBER)other->typeNumber));
                // Compile collision list: who collided with this actor
                if ( playerCollisionCount == 0)
                {
                    actorCollisionEntries[actorCollisionEntryCount].collider = player0;
                    actorCollisionEntries[actorCollisionEntryCount].collisionStartIndex = actorCollisionListCount;
                }
                // Add to list
                actorCollisionList[actorCollisionListCount] = other;
                actorCollisionListCount += 1;

                playerCollisionCount += 1;
            }
        }
    }

    // Player collided with something
    if (playerCollisionCount > 0)
    {
        actorCollisionEntries[actorCollisionEntryCount].collisionAmount = playerCollisionCount;
        actorCollisionEntryCount += 1;

        player0->lastMoveResultFlags = Flag_SetBit(player0->lastMoveResultFlags, MoveResultBit::Move_Collision);
    }

    // Projectiles agains monsters

    // NOTE only do for rendered sectors

    // Get first actor from pool
    // Get other actors on same subsector
    // Check collisions
    // Get next actor from same subsector
}

void ActorPool::RemoveDeadActors()
{
    // Keep sorting until nobody moves
    bool checkAgain = false;
    do
    {
        checkAgain = false;
        for(int i = 0; i < count; i++)
        {
            Actor* actor = GetActorByIndex(i);
            if (Flag_IsBitSet(actor->lastMoveResultFlags, MoveResultBit::Move_Dead))
            {
                RemoveAt(i);
                checkAgain = true;
                break;
            }
        }
    }
    while(checkAgain);
}

void ActorPool::Insert(Actor actor)
{
	int place;
	int sector = actor.subSectorNumber;

	count += 1;
	place = FillFromLeft(count -1, sector);
	mgdl_assert_print(place >= 0 && place < capacity, "ActorPool could not insert the actor");
	actors[place] = actor;
}

void ActorPool::Remove(Actor* actor)
{
	int index = Find(actor);
	RemoveAt(index);
}

void ActorPool::Move(Actor* actor, int oldSubSector, int newSubSector)
{
	int index = Find(actor);
	MoveByIndex(index, oldSubSector, newSubSector);
}

void ActorPool::MoveByIndex(int index, int oldSubSector, int newSubSector)
{
	if (index >= 0 && index < count)
	{
		Actor copy = actors[index];
		int place = FillFromRight(index, newSubSector);
		copy.subSectorNumber = newSubSector;

		// Dont move this again
		copy.lastMoveResultFlags = Flag_UnsetBit(copy.lastMoveResultFlags, MoveResultBit::Move_HitPortal);
		copy.prevSubSectorNumber = newSubSector;
		actors[place] = copy;
	}
}

Actor * ActorPool::GetActorInSectorByIndex(int subSectorNumber, int index, int startIndex)
{
	int start = 0;
	if (startIndex >= 0 && startIndex < count)
	{
		start = startIndex;
	}
	int indexCounter = 0;
	for (int i = start; i < count; i++)
	{
		if (actors[i].subSectorNumber == subSectorNumber)
		{
			if (indexCounter == index)
			{
				return &actors[i];
			}
			indexCounter += 1;
		}
		if (actors[i].subSectorNumber > subSectorNumber)
		{
			// End of sector
			break;
		}
	}
	return nullptr;

}

void ActorPool::RemoveAt(int index)
{
	if (index >= 0 && index < count)
	{
		int place = FillFromRight(index, -1);
		mgdl_assert_print(place == count -1, "ActorPool could not move the last actor");

		count -= 1;
	}
}

int ActorPool::FindSectorIndex(int subSectorNumber)
{
	for (int i = 0; i < count; i++)
	{
		if (actors[i].subSectorNumber == subSectorNumber)
		{
			return i;
		}
	}
	return -1;
}




Actor* ActorPool::GetActorById(int actorId)
{
    for(int i = 0; i < count; i++)
    {
        if (actors[i].idNumber == actorId)
        {
            return &actors[i];
        }
    }
    return nullptr;
}

Actor* ActorPool::GetActorByIndex(int index)
{
	if (index >= 0 && index < count)
	{
		return &actors[index];
    }
    return nullptr;
}

Actor* ActorPool::GetActorByTypeAndIndex(ActorType aType, int index)
{
    int indexCounter = 0;
    for(int i = 0; i < count; i++)
    {
        if (actors[i].actorType == aType)
        {
            if (indexCounter == index) {return &actors[i];}
            else {indexCounter += 1;}
        }
    }
    return nullptr;
}

// Private

int ActorPool::Find(Actor* actor)
{
	int index = -1;
	for (int i = 0; i < count-1; i++)
	{
		if (&actors[i] == actor)
		{
			index = i;
			break;
		}
	}
	return index;
}

int ActorPool::FillFromLeft(int emptyPlace, int stopAt)
{
	// There is nothing on the left
	if (emptyPlace == 0)
	{
		return emptyPlace;
	}
	int place = emptyPlace;

	// Start at left of empty
	// Empty place must be at index 1 at minimum
	// If all sectors are less than stopAt, place is not changed
	for (int index = emptyPlace-1; index >= 0; index--)
	{
		int mySector = actors[index].subSectorNumber;
		if (mySector > stopAt && index == 0)
		{
			// I am bigger sector than incoming and at first place
			// Move out of the way
			actors[place] = actors[index];
			place = index;
			break;
		}
		else if (mySector <= stopAt && index == emptyPlace - 1)
		{
			// I am less or same than incoming and lat last place
			// the incoming should be at empty
			place = emptyPlace;
			break;
		}

		// Look at sector of before
		int nextSubSector = actors[index-1].subSectorNumber;

		if (nextSubSector <= stopAt)
		{
			// This is where the incoming should be placed
			// move out of the way
			actors[place] = actors[index];
			place = index;
			break;
		}
		else if (nextSubSector < mySector)
		{
			// Next is less than mine: I am the last of my sector
			// Move this to place and update the place for the next group
			if (index < place)
			{
				actors[place] = actors[index];
				place = index;
			}
		}
		// else continue
	}
	return place;
}

int ActorPool::FillFromRight(int emptyPlace, int stopAt)
{
	// Shortcut: fill the last place
	int place = emptyPlace;
	if (emptyPlace == count -1)
	{
		return place;
	}
	// Start after the empty place: this is only entered if
	// there is one or more actors after empty
	for(int index = emptyPlace + 1; index < count; index++)
	{
		if (index + 1 == count)
		{
			// This is the last place (count-1), move this and stop
			actors[place] = actors[index] ;
			place = index;
			break;
		}
		// Look at the sector of next place
		int nextSubSector = actors[index+1].subSectorNumber;
		int mySector = actors[index].subSectorNumber;

		// Next is stop sector, move this and stop
		if (nextSubSector >= stopAt && stopAt >= 0) // -1 means advance until end if needed
		{
			actors[place] = actors[index] ;
			place = index;
			break;
		}
		// Next is bigger sector, move this and continue
		else if (mySector < nextSubSector)
		{
			// This is the last one in this subsector
			// move this to place
			actors[place] = actors[index] ;
			place = index;
		}
	}
	return place;
}

