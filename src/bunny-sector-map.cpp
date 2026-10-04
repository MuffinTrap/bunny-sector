#include "bunny-sector-map.h"
#include "bunny-sector-math.h"
#include "gameplay/actor.h"
#include "gameplay/actorpool.h"

zstr * BunnySector_Map::GetMapFile()
{
	return &mapfile;
}


WallInfo BunnySector_Map::GetWallInfo(int sectorIndex, int wallIndex)
{
	WallInfo wi;
	wi.start = GetWallVertexInSector(sectorIndex, wallIndex);
	wi.end = GetNextWallVertexInSector(sectorIndex, wallIndex);
	wi.normal = GetWallNormal(wi.start, wi.end);
	return wi;
}
void BunnySector_Map::MoveActors(float delta)
{
    for(int i = 0; i < actorPool->count; i++)
    {
        Actor* a = actorPool->GetActorByIndex(i);
        if (a->actorType == actor_player || a->actorType == actor_monster || a->actorType == actor_projectile)
        {
            MoveActorInMap(delta, a);
        }
    }
}

void BunnySector_Map::MoveActorInMap(float deltaTime, Actor* inoutActor)
{
    Vector2 current = inoutActor->position.vectorPosition;
    Vector2 destination = Actor_MoveOnFloor(inoutActor, deltaTime);

	Vector2 point = current;
	Vector2 endpoint = destination;

    // TODO Gravity depens on map?
    float elevationEnd = Actor_MoveVertically(inoutActor, 8024.0, deltaTime);

	Vector2 pointOut;
	s16 subSectorOut;
	u32 resultFlags = MoveActorInMapImpl(
		point,  endpoint, inoutActor->radius, inoutActor->subSectorNumber,elevationEnd,inoutActor->climbHeight,inoutActor->height, inoutActor,
		&pointOut, &subSectorOut);

	// Keep actor above floor and under the ceiling
    float minY = GetFloory(subSectorOut);
    float maxY = GetCeilingy(subSectorOut) - inoutActor->height;
    if (elevationEnd < minY)
    {
        resultFlags = Flag_SetBit(resultFlags, Move_OnGround);
    }
    float verticalPosition = Clamp(elevationEnd, minY, maxY);

	inoutActor->position.vectorPosition = pointOut;
    inoutActor->elevation = verticalPosition;
	inoutActor->subSectorNumber = subSectorOut;
    inoutActor->lastMoveResultFlags= resultFlags;
}

Vector2 BunnySector_Map::GetWallNormal(Vector2 wallStart, Vector2 wallEnd)
{
	Vector2 along = Vector2Normalize(Vector2Subtract(wallEnd, wallStart));
	return Vector2New(-along.y, along.x);
}

bool BunnySector_Map::FindIntersectionWithWall(Vector2 moveStart, Vector2 moveEnd, Vector2 wallStart, Vector2 wallEnd, Vector2* pointOUT)
{
    float x1 = moveStart.x;
    float z1 = moveStart.y;
    float x2 = moveEnd.x;
    float z2 = moveEnd.y;

    float x3 = wallStart.x;
    float z3 = wallStart.y;
    float x4 = wallEnd.x;
    float z4 = wallEnd.y;
	return FindIntersectionWithWallUT(x1,z1,x2,z2,x3,z3,x4,z4, pointOUT);
}

bool BunnySector_Map::FindIntersectionWithWallUT(
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    float x4,
    float y4,
    Vector2* pointOUT
     )
{
    float divider = ((x1-x2)*(y3-y4) - (y1-y2)*(x3-x4));
    if (divider != 0.0f)
    {
        float t = ((x1-x3)*(y3-y4) - (y1-y3)*(x3-x4)) / divider;
        float u = ((x1-x2)*(y1-y3) - (y1-y2)*(x1-x3)) / divider;
        if (( 0 <= t && t <= 1.0f ) && (-1.0f <= u && u <= 0.0f))
        {

            printf("Intersection: %f, %f\n", t, u);
            *pointOUT = Vector2New(x1 + t*(x2-x1), y1 + t*(y2-y1));
            return true;
        }
        else
        {
            *pointOUT = Vector2New(t, u);
        }
    }
    return false;
}
// Copied from raylib
// raylib.com
/**********************************************************************************************
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2026 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/
bool BunnySector_Map::CircleCollidesWithWall(Vector2 center, float radius, Vector2 p1, Vector2 p2)
{
    bool collision = false;

    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;

    if ((fabsf(dx) + fabsf(dy)) <= EPSILON)
    {
        float dx = center.x - p1.x;      // X distance between centers
        float dy = center.y - p1.y;      // Y distance between centers

        float distanceSquared = dx*dx + dy*dy; // Distance between centers squared
        float radiusSum = radius;

        collision = (distanceSquared <= (radiusSum*radiusSum));

        return collision;
    }
    else
    {
        float lengthSQ = ((dx*dx) + (dy*dy));
        float dotProduct = (((center.x - p1.x)*(p2.x - p1.x)) + ((center.y - p1.y)*(p2.y - p1.y)))/(lengthSQ);

        if (dotProduct > 1.0f) dotProduct = 1.0f;
        else if (dotProduct < 0.0f) dotProduct = 0.0f;

        float dx2 = (p1.x - (dotProduct*(dx))) - center.x;
        float dy2 = (p1.y - (dotProduct*(dy))) - center.y;
        float distanceSQ = ((dx2*dx2) + (dy2*dy2));

        if (distanceSQ <= radius*radius) collision = true;
    }

    return collision;
}

// Copied from raylib ends

float BunnySector_Map::GetDistanceToWall(Vector2 wallStart, Vector2 wallEnd, Vector2 point)
{
    const float xdiff = wallEnd.x-wallStart.x;
    const float ydiff = wallEnd.y-wallStart.y;
    if (xdiff == 0.0f && ydiff == 0.0f) {
        return Vector2Distance(point, wallStart);
    }
	const float top = fabsf( (ydiff)*point.x - (xdiff)*point.y + wallEnd.x*wallStart.y - wallEnd.y*wallStart.x);
	const float bot = sqrt( (ydiff)*(ydiff) + (xdiff)*(xdiff));
    return top/bot;
}

Actor* BunnySector_Map::GetActorByTypeAndIndex(ActorType aType, int index)
{
    return actorPool->GetActorByTypeAndIndex(aType, index);
}

Actor * BunnySector_Map::GetActorByIndex(int actorIndex)
{
    return actorPool->GetActorByIndex(actorIndex);
}



Actor* BunnySector_Map::GetActorById(int actorId)
{
    return actorPool->GetActorById(actorId);
}

void BunnySector_Map::SetActorPool(ActorPool* pool)
{
    mgdl_assert_print(pool != nullptr, "ActorPool set is null");
    actorPool = pool;
}

int BunnySector_Map::GetActorAmount()
{
    return actorPool->count;
}

void BunnySector_Map::SortMovedActors()
{
    // Keep sorting until nobody moves
    bool sortAgain = false;
    do
    {
        sortAgain = false;
        for(int i = 0; i < actorPool->count; i++)
        {
            Actor* actor = actorPool->GetActorByIndex(i);
            if (Flag_IsBitSet(actor->lastMoveResultFlags, MoveResultBit::Move_HitPortal)
                && actor->prevSubSectorNumber != actor->subSectorNumber)
            {
                actorPool->MoveByIndex(i, actor->prevSubSectorNumber, actor->subSectorNumber);
                sortAgain = true;
                break;
            }
        }

    }
    while(sortAgain);
}
// TODO Move this to ActorPool


void MapFloorVertexData_AddPolygon(MapFloorVertexData* floorData, int sectorIndex, Tesselator_BufferIndices indicesBefore, Tesselator_BufferIndices indicesAfter)
{
    floorData->floorStartIndices[sectorIndex].indexIndex = indicesBefore.indexIndex;
    u16 count = (indicesAfter.indexIndex - indicesBefore.indexIndex);
    floorData->floorStartIndices[sectorIndex].indexCount = count;
    //Log_InfoF("Sector %d: before %d After %d Count: %d\n", sectorIndex, indicesBefore.indexIndex, indicesAfter.indexIndex, count);
    // Set indices in our buffers
    floorData->floorStartIndices[sectorIndex].vertexIndex = indicesBefore.vertexIndex;
    u16 vertexCount = (indicesAfter.vertexIndex - indicesBefore.vertexIndex);
    floorData->floorStartIndices[sectorIndex].vertexCount = vertexCount;
}

