#include "dukemap.h"
#include <mgdl.h>
#include <mgdl/mgdl-vectorfunctions.h>
#include "dukemath.h"
#include "build-render.h"
#include "actor.h"
Sector* Map_GetSector(DukeMap* map, s16 sectorNumber)
{
    mgdl_assert_print((sectorNumber>= 0 && sectorNumber < map->sectorAmount),"Invalid sector for Map_GetSector");
    return &map->sectors[sectorNumber];
}

Wall* Map_GetWallInSector(DukeMap* map, s16 sector, s16 wi)
{
    Sector* s = &map->sectors[sector];
    wi += s->wallptr;
    mgdl_assert_print((wi>= 0 && wi < map->wallAmount),"Invalid wall index for Sector_GetWall");
    return &map->walls[wi];
}

Wall* Map_GetWall(DukeMap* map, s16 wallIndex)
{
    mgdl_assert_print((wallIndex>= 0 && wallIndex < map->wallAmount),"Invalid wall index for Sector_GetWall");
    return &map->walls[wallIndex];
}

Wall* Map_GetWallInSectorPtr(DukeMap* map, Sector* sector, s16 wi)
{
    wi += sector->wallptr;
    mgdl_assert_print((wi>= 0 && wi < map->wallAmount),"Invalid wall index for Sector_GetWall");
    return &map->walls[wi];
}


void Map_InitActors(DukeMap* map, Actor* players, int playerAmount)
{
    // When there are multiple players find starting sectors for all of them
    // Put player one in the official starting position
    Map_SetActorToStart(map, &players[0]);
    for (int pi = 1; pi < playerAmount; pi++)
    {
        MapSprite* startingPos = Map_FindSprite(map, SpriteLOTAG::LOTAG_Multiplayer_Start, pi);
        if (startingPos)
        {
            Log_InfoF("Found starting position for player %d\n", pi);
            players[pi].position= Vector2New(startingPos->position.x, startingPos->position.z);
            players[pi].yawRad = Math_DukeAngleToRad(startingPos->ang);
            players[pi].sectorNumber = startingPos->sectnum;
            players[pi].position.y = Map_GetSectorFloorHeight(map, players[pi].sectorNumber) + players[pi].standingHeight;
        }
        else
        {
            // If no own position found, put to starting position
            Log_InfoF("No starting position for player %d\n", pi);
            Map_InitActor(map, &players[pi]);
        }
    }
}
void Map_SetActorToStart(DukeMap* map, Actor* actor)
{
    actor->position= map->startPosition;
    actor->yawRad = Math_DukeAngleToRad(map->startAngle - 512);
    actor->sectorNumber = map->startingSector;
    actor->position.y = Map_GetSectorFloorHeight(map, map->startingSector) + actor->standingHeight;
}

void Map_SetCameraToStart(DukeMap* map, Viewpoint* camera)
{
    camera->position= Vector3New(map->startPosition.x, map->startElevation, map->startPosition.y);
    camera->yawRad = Math_DukeAngleToRad(map->startAngle - 512);
    camera->sector = map->startingSector;
    camera->position.y = Map_GetSectorFloorHeight(map, map->startingSector);
}

void Map_InitActor(DukeMap* map, Actor* player)
{
    player->position= map->startPosition;
    player->yawRad = Math_DukeAngleToRad(map->startAngle - 512);
    player->sectorNumber = map->startingSector;
    player->position.y = Map_GetSectorFloorHeight(map, map->startingSector) + player->standingHeight;
}

void Map_FindIslandSectors(DukeMap* map)
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

void Map_PrintInfo(DukeMap* map)
{
    Log_InfoF("Duke Map Version:%d Start pos:(%.2f,%.2f), Start elevation %.2f Start angle:%d Start Sector:%d\n",
              map->version,
              map->startPosition.x,
              map->startPosition.y,
              map->startElevation,
              map->startAngle,
              map->startingSector);
    Log_InfoF("Duke Map Sectors:%d Walls:%d, Sprites:%d\n", map->sectorAmount, map->wallAmount, map->spriteAmount);
    for (int i = 0; i < map->sectorAmount; i++)
    {
        Sector* S = &map->sectors[i];
        Log_InfoF("Sector n: %d Walls: %d first wall %d FloorZ %d CeilingZ %d\n", i, S->wallnum, S->wallptr, S->floory, S->ceilingy);
        Log_InfoF("Sector LOTAG: %d HITAG: %d EXTRA: %d\n", S->lotag, S->hitag, S->extra);
        Log_Info("-- Walls ---------------\n");
        for (int wi = 0; wi < S->wallnum; wi++)
        {
            Wall* w = &map->walls[S->wallptr + wi];
            Wall* w2 = &map->walls[w->point2];


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
    for (int i = 0; i < map->spriteAmount; i++)
    {
        MapSprite* s = &map->sprites[i];
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


// Are we inside a sector
// NOTE FROM DUKE SOURCE CODE
// returns 1 when inside
bool Map_IsPointInsideSectorOG(DukeMap* map, Vector2 point, int sectorNumber)
{
        Sector* sector = Map_GetSector(map, sectorNumber);
        u32 count = 0;

        for (s16 wi = 0; wi < sector->wallnum; wi++)
        {
            Wall* w = Map_GetWallInSector(map, sectorNumber, wi);
            Wall* wend = Map_GetWallEnd(map, w);
            Vector2 start = Vector2New(w->x, w->z);
            Vector2 end = Vector2New(wend->x, wend->z);

            // Check if these are different signs
            s32 testY1 = (s32)start.y - (s32)point.y;
            s32 testY2 = (s32)end.y - (s32)point.y;
            if ((testY1^testY2) < 0)
            {
                // Different signs, point.y is between

                // Test if the whole line is on the right: both are positive
                // or negative
                s32 testX1 = (s32)start.x - (s32)point.x;
                s32 testX2 = (s32)end.x - (s32)point.x;
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

bool Map_FindIntersectionWithWallUT(
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
    float t = ((x1-x3)*(y3-y4) - (y1-y3)*(x3-x4)) / ((x1-x2)*(y3-y4) - (y1-y2)*(x3-x4));
    float u = ((x1-x2)*(y1-y3) - (y1-y2)*(x1-x3)) / ((x1-x2)*(y3-y4) - (y1-y2)*(x3-x4));
    if (( 0 <= t && t <= 1.0f ) && (-1.0f <= u && u <= 0.0f))
    {
        *pointOUT = Vector2New(x1 + t*(x2-x1), y1 + t*(y2-y1));
        return true;
    }
    /*
    {
            *pointOUT = Vector2New(x3 + t*(x4-x3), y3 + t*(y4-y3));
            return true;
    }
    */
    // DEBUG write t and u instead
    *pointOUT = Vector2New(t, u);
    return false;
}

bool Map_FindIntersectionWithWall(DukeMap* map, Vector2 moveStart, Vector2 moveEnd, Wall* wall, Vector2* pointOUT)
{
    float x1 = moveStart.x;
    float z1 = moveStart.y;
    float x2 = moveEnd.x;
    float z2 = moveEnd.y;

    float x3 = wall->x;
    float z3 = wall->z;
    Wall* wend = Map_GetWallEnd(map, wall);
    float x4 = wend->x;
    float z4 = wend->z;
    return Map_FindIntersectionWithWallUT(x1, z1, x2, z2, x3, z3, x4, z4, pointOUT);
}

Vector2 Map_GetWallMiddle(DukeMap* map, Wall* w)
{
    Wall* wend = Map_GetWallEnd(map, w);
    Vector2 start = Vector2New(w->x, w->z);
    Vector2 end = Vector2New(wend->x, wend->z);
    return Vector2Add(start, Vector2Scale( Vector2Subtract(end, start), 0.5f));
}
Vector2 Map_GetWallNormal(DukeMap* map, Wall* w)
{
    Wall* wend = Map_GetWallEnd(map, w);
    Vector2 start = Vector2New(w->x, w->z);
    Vector2 end = Vector2New(wend->x, wend->z);
    Vector2 wallVector = Vector2Subtract(end, start);
    return Vector2Normalize(Vector2Rotate(wallVector, DEG2RAD*90));
}
Wall* Map_GetWallEnd(DukeMap* map, Wall* w)
{
    return &map->walls[w->point2];
}

bool Map_IsPointInsideWall(DukeMap* map, Vector2 point, Wall* wall)
{
    // negative if on the right side of wall.
    // walls go clockwise
    Wall* wend = Map_GetWallEnd(map, wall);
    Vector2 start = Vector2New(wall->x, wall->z);
    Vector2 end = Vector2New(wend->x, wend->z);

    Vector2 wallVector = Vector2Subtract(end, start);
    float crossY = Vector2CrossProduct(wallVector, Vector2Subtract(point, start));
    // DANGER Again, this code works differently TM
    return crossY < 0.0f;
}

s32 Map_GetSectorFloorHeight(DukeMap* map, s16 sectorNumber)
{
    Sector* s = Map_GetSector(map, sectorNumber);
    return s->floory;
}

s32 Map_GetSectorCeilingHeight(DukeMap* map, s16 sectorNumber)
{
    Sector* s = Map_GetSector(map, sectorNumber);
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

MapSprite* Map_FindSprite(DukeMap* map, s16 lotag, s16 hitag)
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

MapSprite* Map_GetSprite(DukeMap* map, s16 spriteIndex)
{
    mgdl_assert_print(spriteIndex >= 0 && spriteIndex < map->spriteAmount, "Invalid sprite index");
    return &map->sprites[spriteIndex];
}

void Map_MoveActorInMap(DukeMap* map, float deltaTime, Actor* inoutActor)
{
    Vector2 current = inoutActor->position;
    Vector2 destination = Actor_ApplyDrive(inoutActor, deltaTime);

    if (Vector2Equals(current, destination))
    {
        return;
    }

	Vector2 point = current;
	Vector2 endpoint = destination;

	Vector2 pointOut;
	s16 sectorOut;
	MoveResult result = Map_MovePointInMap(
		map, point, endpoint, inoutActor->sectorNumber,inoutActor->noclip,  
		&pointOut, &sectorOut);

	// Keep actor on floor and under the ceiling
    float minY = Map_GetSectorFloorHeight(map, inoutActor->sectorNumber) + inoutActor->standingHeight;

    // TODO Calculate this from rendering settings somehow
    float ceilingToEyes = inoutActor->standingHeight * (1.0f - inoutActor->eyeHeightNormalized);
    float maxY = Map_GetSectorCeilingHeight(map, inoutActor->sectorNumber) - ceilingToEyes;
    float verticalPosition = Clamp(destination.y, minY, maxY);

	inoutActor->position = pointOut;
	inoutActor->sectorNumber = sectorOut;
    inoutActor->lastResult = result;
}

MoveResult Map_MovePointInMap(DukeMap* map, 
	Vector2 start, Vector2 end, s16 sectorNumber, 
	bool ignoreCollision, 
	Vector2* positionOut, s16* sectorOut)
{

    // Keep testing until player is back inside again
    bool insideSector =  Map_IsPointInsideSectorOG(map, end, sectorNumber);
    if (insideSector == true)
    {
        // TODO does player hit head
        *positionOut = end;
        *sectorOut = sectorNumber;
        return Move_Ok;
    }
    else
    {
        // Find out where player went
        Vector2 cross;
        Sector* sector = Map_GetSector(map, sectorNumber);
        for (s16 wi = 0; wi < sector->wallnum; wi++)
        {
            Wall* w = Map_GetWallInSector(map, sectorNumber, wi);
            // Did player cross this wall
            if (Map_FindIntersectionWithWall(map, start, end, w, &cross))
            {
                // Yes
                // Is it a portal? Or is collision off?
                if (w->nextsector >= 0 || ignoreCollision)
                {
                    s16 newSector = w->nextsector;
                    bool insideNewSector = Map_IsPointInsideSectorOG(map, end, newSector);
                    if (insideNewSector)
                    {
                        *positionOut = end;
                        *sectorOut = newSector;
                        return Move_HitPortal;
                    }
                    else
                    {
                        // Player moved to neighbor's neighbor?
                        // Note push player move start point inside new sector
                        Wall* otherSide = Map_GetWall(map, w->nextwall);
                        Vector2 normal = Map_GetWallNormal(map, otherSide);
                        cross = Vector2Add(cross, normal);
                        return Map_MovePointInMap(map, cross, end, newSector, ignoreCollision, positionOut, sectorOut);
                    }
                }
                else
                {
                    Vector2 normal = Map_GetWallNormal(map, w);
                    Wall* w2 = Map_GetWallEnd(map, w);
                    // Push player back from wall
                    Vector2 hitEnd = Vector2Add(cross, normal);
                    // Slide player along the wall
                    Vector2 wstart = Vector2New(w->x, w->z);
                    Vector2 wend = Vector2New(w2->x, w2->z);
                    Vector2 slideMove = Vec2Project( Vector2Subtract(end, start), Vector2Subtract(wend, wstart));
                    Vector2 slideEnd = Vector2Add(hitEnd, slideMove);
                    bool stillInsideSector = Map_IsPointInsideSectorOG(map, slideEnd, sectorNumber);
                    if (stillInsideSector)
                    {
                        *positionOut = slideEnd;
                        *sectorOut = sectorNumber;
                        return Move_HitWall;
                    }
                    else
                    {
		    	// Did player slide through a portal?
                        MoveResult recursion = Map_MovePointInMap(
						map, hitEnd, slideEnd, sectorNumber, ignoreCollision, 
						positionOut, sectorOut);
			if (recursion != Move_Cancel) {
				return recursion;
			}
			else {
				// Cancel the whole movement chain
				break; // to After wall loop:
			}
                    }
                } // Collision with wall
            } // Found intersection
        }// Wall loop
	
	// After wall loop:
        {
            // DANGER Player has escaped: return to original position
            bool stillInsideSector = Map_IsPointInsideSectorOG(map, start, sectorNumber);
            if (stillInsideSector)
            {
                *positionOut = start;
                *sectorOut = sectorNumber;
                return Move_Cancel;
            }
            else
            {
                Vector3 xzpos = Vector3New(start.x, 0.0f, start.y);
                s16 playerSector = Map_FindSector(map, -1, xzpos);
                if (playerSector < 0)
                {
                    // Put in center of sector, MovePlayerInMap takes care of elevation
                    *positionOut = Vector2Add(sector->minXZPoint, Vector2Scale(sector->sizeXZ, 0.5f));
                    *sectorOut = sectorNumber;
                    return Move_Cancel;
                }
                else
                {
                    *positionOut = start;
                    *sectorOut = playerSector;
                    return Move_Cancel;
                }
            }
        }
    }
}

s16 Map_GetSectorNeighbor(DukeMap* map, s16 sectorNumber, s16 wallIndex)
{
    Sector* sector = Map_GetSector(map, sectorNumber);
    if (wallIndex < sector->wallnum)
    {
        Wall* w = Map_GetWallInSector(map, sectorNumber, wallIndex);
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
