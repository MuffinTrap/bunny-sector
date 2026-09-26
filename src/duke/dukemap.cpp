#include "dukemap.h"
#include <mgdl.h>
#include <mgdl/mgdl-vectorfunctions.h>
#include "../bunny-sector-math.h"
#include "build-render.h"
#include "../gameplay/actor.h"

// Inherited functions

float DukeMap::GetCeilingy(int sectorIndex)
{
    return sectors[sectorIndex].ceilingy;
}
float DukeMap::GetFloory(int sectorIndex)
{
    return sectors[sectorIndex].floory;
}

int DukeMap::GetNextWallVertexIndexInSector(int sectorIndex, int wallIndex)
{
    Wall* w = DukeMap_GetWallInSector(this, sectorIndex, wallIndex);
    return w->point2;
}
Vector2 DukeMap::GetNextWallVertexInSector(int sectorIndex, int wallIndex)
{
    Wall* w = DukeMap_GetWallInSector(this, sectorIndex, wallIndex);
    Wall* nw = DukeMap_GetWallEnd(this, w);
    return Vector2New(nw->x, nw->z);
}
int DukeMap::GetSectorAmount()
{
    return sectorAmount;
}

int DukeMap::GetSectorFirstWallIndex(int sectorIndex)
{
    return sectors[sectorIndex].wallptr;
}
int DukeMap::GetWallAmountInSector(int sectorIndex)
{
    return sectors[sectorIndex].wallnum;
}
MaterialId DukeMap::GetSectorMaterial(int sectorIndex, bool floor)
{
    if (floor)
    {
    return sectors[sectorIndex].floorpicnum;
    }
    else
    {
    return sectors[sectorIndex].ceilingpicnum;
    }
}
int DukeMap::GetWallVertexAmount()
{
    return wallAmount;
}

Vector2 DukeMap::GetWallVertexInSector(int sectorIndex, int wallIndex)
{
    Wall* w = DukeMap_GetWallInSector(this, sectorIndex, wallIndex);
    return Vector2New(w->x, w->z);
}


int DukeMap::FindSubSectorV2(int currentSector, Vector2 currentPosition)
{
    return Map_FindSectorV2(this, currentSector, currentPosition);
}


int DukeMap::GetNeighbourOfWall(int sectorIndex, int wallIndex)
{
    return walls[sectors[sectorIndex].wallptr + wallIndex].nextsector;
}
Vector2 DukeMap::GetSectorMaxTexCoord(int sectorIndex)
{
    return sectors[sectorIndex].maxTexCoord;
}
Vector2 DukeMap::GetSectorMinPoint(int sectorIndex)
{
    return sectors[sectorIndex].minXZPoint;
}
u8 DukeMap::GetSectorShade(int sectorIndex, bool floor)
{
    if (floor){ return sectors[sectorIndex].floorshade;}
    else {return sectors[sectorIndex].ceilingshade;}
}

Vector2 DukeMap::GetSectorSize(int sectorIndex)
{
    return sectors[sectorIndex].sizeXZ;
}


int DukeMap::GetSpriteAmount()
{
    return spriteAmount;
}

Sector* DukeMap_GetSector(DukeMap* map, s16 sectorNumber)
{
    if (sectorNumber>= 0 && sectorNumber < map->sectorAmount)
    {
        return &map->sectors[sectorNumber];
    }
    return &map->sectors[0];
}

Wall* DukeMap_GetWallInSector(DukeMap* map, s16 sector, s16 wi)
{
    Sector* s = &map->sectors[sector];
    wi += s->wallptr;
    mgdl_assert_print((wi>= 0 && wi < map->wallAmount),"Invalid wall index for Sector_GetWall");
    return &map->walls[wi];
}

Wall* DukeMap_GetWall(DukeMap* map, s16 wallIndex)
{
    mgdl_assert_print((wallIndex>= 0 && wallIndex < map->wallAmount),"Invalid wall index for Sector_GetWall");
    return &map->walls[wallIndex];
}

Wall* DukeMap_GetWallInSectorPtr(DukeMap* map, Sector* sector, s16 wi)
{
    wi += sector->wallptr;
    mgdl_assert_print((wi>= 0 && wi < map->wallAmount),"Invalid wall index for Sector_GetWall");
    return &map->walls[wi];
}


void DukeMap_InitActors(DukeMap* map, Actor* players, int playerAmount)
{
    // When there are multiple players find starting sectors for all of them
    // Put player one in the official starting position
    map->SetActorToStart(&players[0]);
    for (int pi = 1; pi < playerAmount; pi++)
    {
        MapSprite* startingPos = DukeMap_FindSprite(map, SpriteLOTAG::LOTAG_Multiplayer_Start, pi);
        if (startingPos)
        {
            Log_InfoF("Found starting position for player %d\n", pi);
            players[pi].position.vectorPosition= Vector2New(startingPos->position.x, startingPos->position.z);
            players[pi].yawRad = Math_DukeAngleToRad(startingPos->ang);
            players[pi].subSectorNumber = startingPos->sectnum;
            players[pi].elevation = map->GetFloory(players[pi].subSectorNumber);
        }
        else
        {
            // If no own position found, put to starting position
            Log_InfoF("No starting position for player %d\n", pi);
            DukeMap_InitActor(map, &players[pi]);
        }
    }
}
void DukeMap::SetActorToStart(Actor* actor)
{
    actor->position.vectorPosition= startPosition;
    actor->yawRad = Math_DukeAngleToRad(startAngle);
    actor->subSectorNumber = startingSector;
    actor->elevation = GetFloory(startingSector);
}

void DukeMap_SetCameraToStart(DukeMap* map, Viewpoint* camera)
{
    camera->position= Vector3New(map->startPosition.x, map->startElevation, map->startPosition.y);
    camera->yawRad = Math_DukeAngleToRad(map->startAngle);
    camera->sector = map->startingSector;
}

void DukeMap_InitActor(DukeMap* map, Actor* player)
{
    player->position.vectorPosition= map->startPosition;
    player->yawRad = Math_DukeAngleToRad(map->startAngle);
    player->subSectorNumber = map->startingSector;
    player->elevation = DukeMap_GetSectorFloorHeight(map, map->startingSector);
}

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

            /*
            Log_InfoF("\tWall n: %d:(%d,%d) - %d:(%d,%d)\n",
                      S->wallptr+wi, w->x, w->z,
                      w->point2,    w2->x, w2->z);
                      */


            if (w->point2 == S->wallptr && wi < S->wallnum-1)
            {
                Log_InfoF("Sector %d has island\n", i);
                Log_InfoF("Wall loop: %d - %d\n", S->wallptr+wi, w->point2);
                S->extra = wi;
            }
        }
    }
}

void DukeMap::PrintInfo()
{
    Log_InfoF("Duke Map Version:%d Start pos:(%.2f,%.2f), Start elevation %.2f Start angle:%d Start Sector:%d\n",
              version,
              startPosition.x,
              startPosition.y,
              startElevation,
              startAngle,
              startingSector);
    Log_InfoF("Duke Map Sectors:%d Walls:%d, Sprites:%d\n", sectorAmount, wallAmount, spriteAmount);
    for (int i = 0; i < sectorAmount; i++)
    {
        Sector* S = &sectors[i];
        Log_InfoF("Sector n: %d Walls: %d first wall %d FloorZ %d CeilingZ %d\n", i, S->wallnum, S->wallptr, S->floory, S->ceilingy);
        Log_InfoF("Sector LOTAG: %d HITAG: %d EXTRA: %d\n", S->lotag, S->hitag, S->extra);
        Log_Info("-- Walls ---------------\n");
        for (int wi = 0; wi < S->wallnum; wi++)
        {
            Wall* w = &walls[S->wallptr + wi];
            Wall* w2 = &walls[w->point2];


            Log_InfoF("\tWall n: %d:(%d,%d) - %d:(%d,%d)\n",
                      S->wallptr+wi, w->x, w->z,
                      w->point2,    w2->x, w2->z);


            if (w->point2 == S->wallptr && wi < S->wallnum-1)
            {
                Log_Info("Sector has island\n");
                Log_InfoF("Wall loop: %d - %d\n", S->wallptr+wi, w->point2);
                S->extra = wi;
            }
        }
    }

    Log_Info("-- Sprites ---------------\n");
    for (int i = 0; i < spriteAmount; i++)
    {
        MapSprite* s = &sprites[i];
        Log_InfoF("Pos (%.0f %.0f %.0f) Angle %d Pic: %d Alignment:", s->position.x, s->position.y, s->position.z, s->ang, s->picnum);
        SpriteAlignment sa = Sprite_GetAlignment(s);
        switch(sa)
        {
            case Sprite_FACE: Log_InfoF("FACE\n"); break;
            case Sprite_WALL:Log_InfoF("WALL\n"); break;
            case Sprite_FLOOR:Log_InfoF("FLOOR\n"); break;
        };
        Log_InfoF("Tags: LOTAG: %d HITAG: %d EXTRA: %d\n", s->lotag, s->hitag, s->extra);
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


// Are we inside a sector
// NOTE FROM DUKE SOURCE CODE
// returns 1 when inside
bool Map_IsPointInsideSectorOG_1(DukeMap* map, Vector2 point, int sectorNumber)
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
        u32 count = 0;

        for (s16 wi = 0; wi < sector->wallnum; wi++)
        {
            Wall* wstart = DukeMap_GetWallInSector(map, sectorNumber, wi);
            Wall* wend = DukeMap_GetWallEnd(map, wstart);

            // Check if these are different signs
            s32 pointx = (s32)floor(point.x);
            s32 pointz = (s32)floor(point.y);

            s32 testY1 = wstart->z - pointz;
            s32 testY2 = wend->z - pointz;
            if ((testY1^testY2) < 0)
            {
                // Different signs, point.y is between

                // Test if the whole line is on the right: both are positive
                // or negative
                s32 testX1 = wstart->x - pointx;
                s32 testX2 = wend->x - pointx;
                if ((testX1^testX2) >= 0)
                {
                    // Both are on the right side: both are positive
                    // Or both are on left side : both are negative
                    // Toggle sign:
                    // 0 ^ 1 -> 1
                    // 1 ^ 1 -> 0
                    // 1 ^ 0 -> 1
                    // 0 ^ 0 -> 1
                    // Finds left: 0^1=1 then right: 1^0 = 1 : inside
                    // Finds left: 0^1=1 then left 1^1 = 0 : not inside
                    // Finds right: 0^0=0 then left 0^1 = 1 : inside
                    count ^= testX1;
                }
                else
                {
                    // Other x is left, other is right
                    // Do point on side of line test with cross product
                    // If on the right
                    //        this is negative when on right 1
                    //        y2 is positive if it was below 0 : so this is  1^0 = 1 : left
                    //        y2 is negative if it was above 1 : this is 1^1 = 0 : right
                    count ^= (testX1*testY2 - testX2*testY1)^testY2;
                }
            }
		}
		return  (count >> 31) > 0;
}

// This is from Wikipedia and works

Vector2 DukeMap_GetWallMiddle(DukeMap* map, Wall* w)
{
    Wall* wend = DukeMap_GetWallEnd(map, w);
    Vector2 start = Vector2New(w->x, w->z);
    Vector2 end = Vector2New(wend->x, wend->z);
    return Vector2Add(start, Vector2Scale( Vector2Subtract(end, start), 0.5f));
}
Vector2 DukeMap_GetWallNormal(DukeMap* map, Wall* w)
{
    Wall* wend = DukeMap_GetWallEnd(map, w);
    Vector2 start = Vector2New(w->x, w->z);
    Vector2 end = Vector2New(wend->x, wend->z);
    Vector2 wallVector = Vector2Subtract(end, start);
    return Vector2Normalize(Vector2Rotate(wallVector, DEG2RAD*90));
}
Wall* DukeMap_GetWallEnd(DukeMap* map, Wall* w)
{
    return &map->walls[w->point2];
}


bool DukeMap::IsPointInsideWall(Vector2 point, Vector2 wallStart, Vector2 wallEnd)
{
    Vector2 wallVector = Vector2Subtract(wallEnd, wallStart);
    float crossY = Vector2CrossProduct(wallVector, Vector2Subtract(point, wallStart));
    // DANGER Again, this code works differently TM
    return crossY > 0.0f;

}
bool Map_IsPointInsideWall(DukeMap* map, Vector2 point, Wall* wall)
{
    // negative if on the right side of wall.
    // walls go clockwise
    Wall* wend = DukeMap_GetWallEnd(map, wall);
    Vector2 start = Vector2New(wall->x, wall->z);
    Vector2 end = Vector2New(wend->x, wend->z);
    return map->IsPointInsideWall(point, start, end);
}

float Map_GetDistanceToWall(DukeMap* map, Wall* wall, Vector2 point)
{
    Wall* wend = DukeMap_GetWallEnd(map, wall);
    Vector2 ws = Vector2New(wall->x, wall->z);
    Vector2 we = Vector2New(wend->x, wend->z);
    return map->GetDistanceToWall(point, ws, we);
}

s32 DukeMap_GetSectorFloorHeight(DukeMap* map, s16 sectorNumber)
{
    Sector* s = DukeMap_GetSector(map, sectorNumber);
    return s->floory;
}

s32 DukeMap_GetSectorCeilingHeight(DukeMap* map, s16 sectorNumber)
{
    Sector* s = DukeMap_GetSector(map, sectorNumber);
    return s->ceilingy;
}

SpriteAlignment Sprite_GetAlignment(MapSprite* sprite)
{
    if (Flag_IsBitSet(sprite->cstat, SPRITE_WALL_ALIGNED_BIT))
    {
        return Sprite_WALL;
    }
    else if (Flag_IsBitSet(sprite->cstat, SPRITE_FLOOR_ALIGNED_BIT))
    {
        return Sprite_FLOOR;
    }
    else
    {
        return Sprite_FACE;
    }
}
SpritePivot Sprite_GetPivot(MapSprite* sprite)
{
    if (Flag_IsBitSet(sprite->cstat, (SPRITE_PIVOT_BIT)))
    {
        return Sprite_PivotCenter;
    }
    else
    {
        return Sprite_PivotFoot;
    }
}

MapSprite* DukeMap_FindSprite(DukeMap* map, s16 lotag, s16 hitag)
{
    for (int si = 0; si < map->spriteAmount; si++)
    {
        MapSprite* S = &map->sprites[si];
        if (S->lotag == lotag && S->hitag == hitag)
        {
            return S;
        }
    }
    return nullptr;
}

MapSprite* DukeMap_GetSprite(DukeMap* map, s16 spriteIndex)
{
    mgdl_assert_print(spriteIndex >= 0 && spriteIndex < map->spriteAmount, "Invalid sprite index");
    return &map->sprites[spriteIndex];
}




u32 DukeMap::MoveActorInMapImpl(
	Vector2 start, Vector2 end, float radius, s16 sectorNumber,
	float elevationEnd, float maxElevationChange, float height, Actor* actor,
	Vector2* positionOut, s16* sectorOut)
{
    u32 moveResultBitfield = 0;
    Vector2 cross;
    Sector* sector = DukeMap_GetSector(this, sectorNumber);

    // Check each wall of sector
    // First check normal walls and push player away from them
    // Then check portals and see if player crosses them

    // TODO Treat portals where elevation change is too much as walls

    for (s16 wi = 0; wi < sector->wallnum; wi++)
    {
        // Get wall start and end points
        // TODO make a function that gets the Start and Endpoint Vectors
        Wall* wall = DukeMap_GetWallInSector(this, sectorNumber, wi);
        bool treatAsWall = (wall->nextsector < 0);

        // Check if could change elevation
        if (treatAsWall == false)
        {
            s16 newSector = wall->nextsector;
            float neighborFloor = GetFloory(newSector);
            if (neighborFloor > elevationEnd + maxElevationChange)
            {
                treatAsWall = true;
            }
            else
            {
                float neighborCeiling = GetCeilingy(newSector);
                if (neighborCeiling < elevationEnd + height)
                {
                    treatAsWall = true;
                }
            }
        }
        if (treatAsWall)
        {
            float wsx = wall->x;
            float wsz = wall->z;
            Wall* w2 = DukeMap_GetWallEnd(this, wall);
            float wex = w2->x;
            float wez = w2->z;
            // Keep player away from walls
            Vector2 wstart = Vector2New(wsx, wsz);
            Vector2 wend = Vector2New(wex, wez);

            // Check if player moved so fast that went through the wall
            bool endOtherSide = IsPointInsideWall(end, wstart, wend) == false;
            if (endOtherSide)
            {
                // Find the exact intersection point and slide player along the wall
                bool intersectFound =  FindIntersectionWithWall(start, end, wstart, wend, &cross);
                if (intersectFound)
                {
                    Vector2 normal = GetWallNormal(wstart, wend);
                    // Push player back from wall
                    Vector2 hitEnd = Vector2Add(cross, normal);
                    // Slide player along the wall
                    Vector2 wstart = Vector2New(wsx, wsz);
                    Vector2 wend = Vector2New(wex, wez);
                    Vector2 slideMove = Vector2Project( Vector2Subtract(end, start), Vector2Subtract(wend, wstart));
                    end = Vector2Add(hitEnd, slideMove);

                    // TODO Push out already here?

                    moveResultBitfield = Flag_SetBit(moveResultBitfield, Move_HitWall);
                }
            }

            // Check if player is too close to wall
            // NOTE: end was maybe modified above
            if (CircleCollidesWithWall(end, radius, wstart, wend))
            {
                float distance = GetDistanceToWall(end, wstart, wend);
                if (distance < radius)
                {
                    // NOTE : Slides automagically
                    Vector2 normal = GetWallNormal(wstart, wend);
                    float intoWall = radius - distance;
                    end = Vector2Add(end, Vector2Scale(normal, intoWall));
                    moveResultBitfield = Flag_SetBit(moveResultBitfield, Move_HitWall);
                }
            }
        } // if is wall
    } // Wall loop

    // If nothing happens with portals, we stay in same sector as started
    *sectorOut = sectorNumber;

    // Check if player movement against walls made them go through portal
    // NOTE above for loop has already pushed player away from inaccessible portals
    for (s16 wi = 0; wi < sector->wallnum; wi++)
    {
        Wall* wall = DukeMap_GetWallInSector(this, sectorNumber, wi);
        if (wall->nextsector >= 0)
        {
            Wall* endWall = DukeMap_GetWallEnd(this, wall);
            Vector2 wstart = Vector2New(wall->x, wall->z);
            Vector2 wend = Vector2New(endWall->x, endWall->z);
            // Is player close to this wall?
            bool isClose = IntersectBoxV(start, end, wstart, wend);
            bool crosses = false;

            if (isClose)
            {
                // Is player on the other side of it
                bool startThisSide = IsPointInsideWall(start, wstart, wend);
                bool endOtherSide = IsPointInsideWall(end, wstart, wend) == false;
                crosses = startThisSide && endOtherSide;
            }

            if (crosses)
            {
                // If is portal and end point is on the other side
                // we can just allow player to move to next sector

                s16 newSector = wall->nextsector;
                *sectorOut = newSector;
                moveResultBitfield = Flag_SetBit(moveResultBitfield, Move_HitPortal);
            }
        } // if is portal
    }// Portal loop

    *positionOut = end;
    return moveResultBitfield;

    /*

    bool insideSector =  Map_IsPointInsideSectorOG(map, end, sectorNumber);
    if (insideSector)
    {
        // TODO does player hit head
        *positionOut = end;
        *sectorOut = sectorNumber;
        return Move_Ok;
    }
    */
}

s16 DukeMap_GetSectorNeighbor(DukeMap* map, s16 sectorNumber, s16 wallIndex)
{
    Sector* sector = DukeMap_GetSector(map, sectorNumber);
    if (wallIndex < sector->wallnum)
    {
        Wall* w = DukeMap_GetWallInSector(map, sectorNumber, wallIndex);
        return w->nextsector;
    }
    return -1;
}

s16 Map_FindSector(DukeMap* map, s16 startingSector, Vector3 position)
{
    Vector2 position2D = Vector2New(position.x, position.z);
    return Map_FindSectorV2(map, startingSector, position2D);

}
s16 Map_FindSectorV2(DukeMap* map, s16 startingSector, Vector2 position2D)
{
    if (startingSector >= 0)
    {
        if (Map_IsPointInsideSectorOG(map, position2D, startingSector))
        {
            return startingSector;
        }
    }
    for (int si = 0; si < map->sectorAmount; si++)
    {
        if (Map_IsPointInsideSectorOG(map, position2D, si))
        {
            return si;
        }
    }
    return -1;
}

void DukeMap::UpdateActions(float delta)
{
    // NOP
}

