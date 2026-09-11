#include "doom-map.h"
#include "../gameplay/actor.h"
#include "../bunny-sector-math.h"

 void DoomMap_Allocate(DoomMap* map, int thingsAmount, int sectorAmount, int sideAmount, int lineAmount, int vertexAmount)
{
	map->thingAmount = thingsAmount;
	map->sectorAmount = sectorAmount;
	map->sideAmount = sideAmount;
	map->lineAmount = lineAmount;
	map->vertexAmount = vertexAmount;

	map->things = (DoomThing*)mgdl_AllocateGeneralMemory(thingsAmount * sizeof(DoomThing));
	map->sectors = (DoomSector*)mgdl_AllocateGeneralMemory(sectorAmount * sizeof(DoomSector));
	map->linedefs = (DoomLinedef*)mgdl_AllocateGeneralMemory(lineAmount * sizeof(DoomLinedef));
	map->sidedefs = (DoomSidedef*)mgdl_AllocateGeneralMemory(sideAmount * sizeof(DoomSidedef));
	map->vertices = (DoomVertex*)mgdl_AllocateGeneralMemory(vertexAmount * sizeof(DoomVertex));

}

int DoomMap::GetNeighbourOfWall(int sectorIndex, int wallIndex)
{
	DoomSubSector* sub = &subsectors[sectorIndex];
	DoomSegment* seg = &segments[sub->firstSegment + wallIndex];
	DoomLinedef* linde = &linedefs[seg->linedef];
	if (linde->sideback >= 0)
	{
		DoomSidedef* side = &sidedefs[linde->sideback];
		return side->sector;
	}
	return -1;
}

u8 DoomMap::GetSectorShade(int sectorIndex, bool floor)
{
	return sectors[subsectors[sectorIndex].sector].lightlevel;
}



void DoomMap_AllocateNodes(DoomMap* map, int nodeAmount)
{
	map->nodeAmount = nodeAmount;
	map->nodes = (DoomNode*)mgdl_AllocateGeneralMemory(nodeAmount * sizeof(DoomNode));
}
void DoomMap_AllocateSegments(DoomMap* map, int segmentAmount)
{
	map->segmentAmount = segmentAmount;
	map->segments = (DoomSegment*)mgdl_AllocateGeneralMemory(segmentAmount * sizeof(DoomSegment));
}
void DoomMap_AllocateSubsectors(DoomMap* map, int subSectorAmount)
{
	map->subSectorAmount = subSectorAmount;
	map->subsectors = (DoomSubSector*)mgdl_AllocateGeneralMemory(subSectorAmount * sizeof(DoomSubSector));
}

void SetActorToStart(DoomMap* map, Actor* actor)
{

}
void DoomMap::SetActorToStart(Actor* actor)
{
	// Find thing 0
	actor->position.vectorPosition.x = things[0].x;
	actor->position.vectorPosition.y = things[0].y;
	actor->elevation = 0.0f;
	actor->yawRad = DEG2RAD * things[0].angleDeg;
	actor->sectorNumber = FindSectorV2(0, actor->position.vectorPosition);
}


void DoomMap::PrintInfo()
{
	printf("Doom map: things: %d, sectors %d, sides %d, lines %d, vertices %d\n", thingAmount, sectorAmount, sideAmount, lineAmount, vertexAmount);

	for (int i = 0; i < thingAmount; i++)
	{
		printf("Thing %d (%.2f, %.2f)\n", i, things[i].x, things[i].y);
	}

	for (int i = 0; i < vertexAmount; i++)
	{
		printf("Vertex %d (%.2f, %.2f)\n", i, vertices[i].x, vertices[i].y);
	}

	for (int i = 0; i < sectorAmount; i++)
	{
		printf("Sector %d id: %d floor %d, ceiling %d\n", i,
			   sectors[i].id,
			   sectors[i].heightfloor,
			   sectors[i].heightceiling
			   );
	}
	for (int i = 0; i < sideAmount; i++)
	{
		printf("Side def %d sector: %d texture mid: %d\n", i,
			   sidedefs[i].sector, sidedefs[i].texturemiddle);
	}
	for (int i = 0; i < lineAmount; i++)
	{
		printf("Line def %d. id %d from: %d to: %d front side %d back side %d\n", i,
		linedefs[i].id,
		linedefs[i].v1,
		linedefs[i].v2,
		linedefs[i].sidefront,
		linedefs[i].sideback
		);
	}
	for (int i = 0; i < segmentAmount; i++)
	{
		printf("Segment %d. V1: %d, partner: %d, linedef: %d, side %d\n",
			   i,
			   segments[i].v1,
			   segments[i].partnerSegment,
			   segments[i].linedef,
			   segments[i].lineSide
		);
	}

}

float DoomMap::GetCeilingy(int sectorIndex)
{
	return sectors[subsectors[sectorIndex].sector].heightceiling;
}
float DoomMap::GetFloory(int sectorIndex)
{
	return sectors[subsectors[sectorIndex].sector].heightfloor;
}

Vector2 DoomMap::GetWallVertexInSector(int sectorIndex, int wallIndex)
{
	DoomSubSector* ds = &subsectors[sectorIndex];
	// TODO Should this check for partner?
	DoomSegment* seg = &segments[ds->firstSegment + wallIndex];
	DoomVertex* dv = &vertices[seg->v1];
	return Vector2New(dv->x, dv->y);
}
Vector2 DoomMap::GetNextWallVertexInSector(int sectorIndex, int wallIndex)
{
	DoomSubSector* ds = &subsectors[sectorIndex];
	// TODO Should this check for partner?
	DoomSegment* seg = &segments[ds->firstSegment + ((wallIndex+1) % ds->segmentAmount)];
	DoomVertex* dv = &vertices[seg->v1];
	return Vector2New(dv->x, dv->y);
}


int DoomMap::GetNextWallVertexIndexInSector(int sectorIndex, int wallIndex)
{
	DoomSubSector* ds = &subsectors[sectorIndex];
	return ds->firstSegment + ((wallIndex+1) % ds->segmentAmount);
}

int DoomMap::GetSectorAmount()
{
	return subSectorAmount;
}
int DoomMap::GetSectorFirstWallIndex(int sectorIndex)
{
	DoomSubSector* ds = &subsectors[sectorIndex];
	return ds->firstSegment;
}
MaterialId DoomMap::GetSectorMaterial(int sectorIndex, bool floor)
{
	if (floor)
	{
	return sectors[subsectors[sectorIndex].sector].texturefloor;
	}
	else
	{
	return sectors[subsectors[sectorIndex].sector].textureceiling;
	}
}


int DoomMap::GetWallAmountInSector(int sectorIndex)
{
	return subsectors[sectorIndex].segmentAmount;
}
int DoomMap::GetWallVertexAmount()
{
	return segmentAmount;
}

int GetChildSide(DoomNode* node, Vector2 v)
{
	if (node->dx == 0)
	{
		// Vertical cut
		if (v.x < node->x)
		{
			if (node->dy > 0) {return 1;}
			else {return 0;}
		}
		if (node->dy < 0) {return 1;}
		else {return 0;}
	}
	if (node->dy == 0)
	{
		// Horizontal cut
		if (v.y < node->y)
		{
			if (node->dx > 0) {return 0;}
			else {return 1;}
		}
		if (node->dx < 0) {return 0;}
		else {return 1;}
	}
	float dx = v.x - node->x;
	float dy = v.y - node->y;
	if( dx * node->dy < dy * node->dx)
	{
		return 1;
	}
	return 0;
}

int DoomMap::FindSubSector(DoomNode* node, Vector2 point)
{
	int childSide = GetChildSide(node, point);
	if (ChildIsNode(node->children[childSide]))
	{
		return FindSubSector(&nodes[node->children[childSide]], point);
	}
	else
	{
		return node->children[childSide] & 0x7fffffff;
	}
}

int DoomMap::FindSectorV2(int currentSector, Vector2 currentPosition)
{
	// TODO BSD traversal
	return FindSubSector(&nodes[nodeAmount-1], currentPosition);
}
 Vector2 DoomMap::GetSectorMaxTexCoord(int sectorIndex)
{
	return subsectors[sectorIndex].maxTexCoord;
}
 Vector2 DoomMap::GetSectorMinPoint(int sectorIndex)
{
	return subsectors[sectorIndex].minXZPoint;
}
Vector2 DoomMap::GetSectorSize(int sectorIndex)
{
	return subsectors[sectorIndex].sizeXZ;
}


u32 DoomMap::MovePointInMap(
	Vector2 start, Vector2 end, float radius, s16 sectorNumber,
	float elevationEnd, float maxElevationChange, float height,
	Vector2* positionOut, s16* sectorOut)
{
    u32 moveResultBitfield = 0;
    Vector2 cross;
    DoomSubSector* sector = &subsectors[sectorNumber];

    // Check each wall of sector
    // First check normal walls and push player away from them
    // Then check portals and see if player crosses them

    // TODO Treat portals where elevation change is too much as walls

    for (s16 wi = 0; wi < sector->segmentAmount; wi++)
    {
        // Get wall start and end points
        // TODO make a function that gets the Start and Endpoint Vectors
        DoomSegment* wall = &segments[sector->firstSegment + wi];
        bool treatAsWall = (wall->neighbourSubSector < 0);

        // Check if could change elevation
        if (treatAsWall == false)
        {
            s16 newSector = wall->neighbourSubSector;
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
			DoomVertex* w2 = &vertices[wall->v1];
			DoomSegment* wall2 = &segments[sector->firstSegment + ((wi + 1) % sector->segmentAmount)];
            DoomVertex* w1 = &vertices[wall2->v1];
            // Keep player away from walls
            Vector2 wstart = Vector2New(w1->x, w1->y);
            Vector2 wend = Vector2New(w2->x, w2->y);

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

    // Check each wall of sector
    // First check normal walls and push player away from them
    // Then check portals and see if player crosses them

    // TODO Treat portals where elevation change is too much as walls

    for (s16 wi = 0; wi < sector->segmentAmount; wi++)
    {
        // Get wall start and end points
        // TODO make a function that gets the Start and Endpoint Vectors
        DoomSegment* wall = &segments[sector->firstSegment + wi];
        if (wall->neighbourSubSector >= 0)
        {
			DoomSegment* wall2 = &segments[sector->firstSegment + ((wi + 1) % sector->segmentAmount)];
			DoomVertex* wp1 = &vertices[wall->v1];
			DoomVertex* wp2 = &vertices[wall2->v1];
            Vector2 wstart = Vector2New(wp1->x, wp1->y);
            Vector2 wend = Vector2New(wp2->x, wp2->y);
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
                s16 newSector = wall->neighbourSubSector;
                *sectorOut = newSector;
			*sectorOut = FindSectorV2(0, end); // Always find the sector again
                moveResultBitfield = Flag_SetBit(moveResultBitfield, Move_HitPortal);
            }
        } // if is portal
    }// Portal loop

    *positionOut = end;
    return moveResultBitfield;
}


bool DoomMap::IsPointInsideWall(Vector2 point, Vector2 wallStart, Vector2 wallEnd)
{

    Vector2 wallVector = Vector2Subtract(wallEnd, wallStart);
    float crossY = Vector2CrossProduct(wallVector, Vector2Subtract(point, wallStart));
    // DANGER Again, this code works differently TM
    return crossY > 0.0f;
	/*
	Vector2 delta = Vector2Subtract(wallEnd, wallStart);

	if (delta.x == 0)
	{
		// Vertical cut
		if (point.x < wallStart.x)
		{
			if (delta.y > 0) {return 1;}
			else {return 0;}
		}
		if (delta.y < 0) {return 1;}
		else {return 0;}
	}
	if (delta.y == 0)
	{
		// Horizontal cut
		if (point.y < wallStart.y)
		{
			if (delta.x > 0) {return 1;}
			else {return 0;}
		}
		if (delta.x < 0) {return 1;}
		else {return 0;}
	}
	float dx = point.x - wallStart.x;
	float dy = point.y - wallStart.y;
	if( dx * delta.y < dy * delta.x)
	{
		return 1;
	}
	return 0;
	*/
}


DoomThing* DoomMap_GetThing(DoomMap* map, unsigned int index) { return &map->things[index];}
DoomSector* DoomMap_GetSector(DoomMap* map, unsigned int index) { return &map->sectors[index];}
DoomSidedef* DoomMap_GetSidedef(DoomMap* map, unsigned int index) { return &map->sidedefs[index];}
DoomLinedef* DoomMap_GetLinedef(DoomMap* map, unsigned int index) { return &map->linedefs[index];}
DoomVertex* DoomMap_GetVertex(DoomMap* map, unsigned int index) { return &map->vertices[index];}
DoomNode* DoomMap_GetNode(DoomMap* map, unsigned int index) { return &map->nodes[index];}
DoomSubSector* DoomMap_GetSubSector(DoomMap* map, unsigned int index) { return &map->subsectors[index];}
DoomSegment* DoomMap_GetSegment(DoomMap* map, unsigned int index) { return &map->segments[index];}

DoomNode * DoomMap_GetRootNode(DoomMap* map) { return &map->nodes[map->nodeAmount-1]; }

DoomNode* DoomMap_GetChildNode(DoomMap* map, ChildId id) {return &map->nodes[id];}
DoomSubSector* DoomMap_GetChildSubSector(DoomMap* map, ChildId id) { return &map->subsectors[(id & 0x7fffffff)];}
