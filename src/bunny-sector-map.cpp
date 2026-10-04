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
/*
 * NOTE Stuff moved here from dukemap
void DukeMap_FindIslandSectors(DukeMap* map)
{
    if (map == nullptr)
    {
        Log_ErrorF("Map_FindIslandSectors got null pointer for map\n");
        return;
    }
    for (int i = 0; i < map->sectorAmount; i++)
    {
        Sector* S = &map->sectors[i];
        //Log_InfoF("Sector n: %d Walls: %d first wall %d FloorZ %d CeilingZ %d\n", i, S->wallnum, S->wallptr, S->floory, S->ceilingy);
        for (int wi = 0; wi < S->wallnum; wi++)
        {
            Wall* w = &map->walls[S->wallptr + wi];

            Log_InfoF("\tWall n: %d:(%d,%d) - %d:(%d,%d)\n",
                      S->wallptr+wi, w->x, w->z,
                      w->point2,    w2->x, w2->z);


            if (w->point2 == S->wallptr && wi < S->wallnum-1)
            {
                Log_InfoF("Sector %d has island\n", i);
                Log_InfoF("Wall loop: %d - %d\n", S->wallptr+wi, w->point2);
                S->extra = wi;
            }
        }
    }
}

// Point inside sector code by:
// https://stackoverflow.com/users/2608744/timepp
#define BETWEEN(p,a,b) (p >= a && p <= b || p <= a && p >= b)
bool Map_IsPointInsideSectorOG(DukeMap* map, Vector2 P, int sectorNumber)
{
    if (sectorNumber < 0)
    {
        return false;
    }
    Sector* sector = DukeMap_GetSector(map, sectorNumber);
    if (sector == nullptr)
    {
        return false;
    }
    bool inside = false;

    for (s16 wi = 0; wi < sector->wallnum; wi++)
    {
        Wall* wstart = DukeMap_GetWallInSector(map, sectorNumber, wi);
        Wall* wend = DukeMap_GetWallEnd(map, wstart);

        Vector2 A = Vector2New(wstart->x, wstart->z);
        Vector2 B = Vector2New(wend->x, wend->z);
        if ((P.x == A.x && P.y == A.y) || (P.x == B.x && P.y == B.y)) {return false;}
        if (A.y == B.y && P.y == A.y && BETWEEN(P.x, A.x, B.x)) {return false;}

        if (BETWEEN(P.y, A.y, B.y)) { // if P inside the vertical range
            // filter out "ray pass vertex" problem by treating the line a little lower
            if ((P.y == A.y && B.y >= A.y) || (P.y == B.y && A.y >= B.y)) { continue;}
            // calc cross product `PA X PB`, P lays on left side of AB if c > 0
            const float c = (A.x - P.x) * (B.y - P.y) - (B.x - P.x) * (A.y - P.y);
            if (c == 0) return false;
            if ((A.y < B.y) == (c > 0)) inside = !inside;
        }

    }
    return inside;
}
*/

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

