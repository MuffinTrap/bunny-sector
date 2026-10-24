#include "doom_render.h"
#include "doom_render_util.h"
#include "../bunny-sector-types.h"
#include "doom_types.h"
#include "../gameplay/actor.h"
#include "../bunny-sector_main.h"
#include "../bunny-sector-math.h"
#include "block_segment.h"

// Statics

static float HFOVRAD = 0;
static float VFOVRAD = 0; // to be set

// TODO Get from mgdl

static int DRAW_LIMIT_LEFT_CANVAS = -SCREEN_WIDTH/2;
static int DRAW_LIMIT_RIGHT_CANVAS = SCREEN_WIDTH/2;

// Renderer state

static Vector2 ViewPortSize = Vector2New(1,1); // Viewport in world units.
static Vector2 CanvasSize = Vector2New(1,1); // The size of the drawing area
static Vector2 CameraToCanvasConvert = Vector2New(1,1); // Transforms from Camera to Canvas. Divide by Z!

// Player view cone calculated from HFOVRAD
static Vector2 frustumOrigo;
static Vector2 frustumLeft;
static Vector2 frustumRight;

static Vector2 FORWARD_2D = Vector2New(1, 0);
static Vector2 RIGHT_2D = Vector2Rotate(FORWARD_2D, DEG2RAD * 90);

const int MAX_SUBSECTORS_DRAWN = 128;
static int DrawnSubSectors[MAX_SUBSECTORS_DRAWN];
static int subSectorDrawCount = 0;

// Global function parameters

float gp_SectorFloorY;
float gp_SectorCeilingY;
float gp_SectorNeighbour;
float gp_SectorNeighbourFloorY;
float gp_SectorNeighbourCeilingY;

int gp_CanvasAx; // in canvas units
int gp_CanvasBx;

void ResetGlobalRenderVariables()
{
	gp_SectorFloorY = -16000;
	gp_SectorCeilingY = 16000;
	gp_CanvasAx = -SCREEN_WIDTH;
	gp_CanvasBx = -SCREEN_WIDTH;
}


// Viewport size is calculated from FOV
// Tan(VFOV/2) gives the length of the side opposite to angle
// when Z is 1
// tan(VFOV/2) = opposite/adjacent(Z)
// Same for horizontal fov
//                + view top
//                |
//                |
//       VFOV/2   |
//  camera --------
//       VFOV/2   |
//                |
//                |
//                + view bottom
Vector2 ViewportSizeFromFovAndNearZ()
{
	float viewHeight = tan(VFOVRAD/2.0f);
	float aspect = (CanvasSize.x/CanvasSize.y);
	float viewWidth = atan(aspect * viewHeight);
	return Vector2New(viewWidth*2.0f, viewHeight*2.0f);
}

void UpdateFrustumToFov()
{
	frustumOrigo	= Vector2New(0.0f, 0.0f);
	frustumLeft	= Vector2Scale(Vector2Rotate(FORWARD_2D, -HFOVRAD/2.0f), 600.0f);
	frustumRight	= Vector2Scale(Vector2Rotate(FORWARD_2D, HFOVRAD/2.0f), 600.0f);
}

void SetVerticalFovDeg(float fovDeg)
{
	if (fovDeg >= 5.0f && fovDeg < 180.0f)
	{
		VFOVRAD = DEG2RAD * fovDeg;

		// Horizontal fov depends on vertical fov and aspect ratio
		float aspect = (CanvasSize.x/CanvasSize.y);
		HFOVRAD = VFOVRAD * aspect;

		UpdateFrustumToFov();
		// Viewport dimensions depend on FOV
		ViewPortSize = ViewportSizeFromFovAndNearZ();
	}
}


// Utils

Vector2 WorldToCamera(Vector2 point, Vector2 CameraPosition, float cameraYaw)
{
	return Vector2Rotate( Vector2Subtract(point, CameraPosition), -cameraYaw);
}


// These transform the clipped coordinates to canvas for drawing
Vector2 GetConvertXY()
{
	return Vector2New(
		CanvasSize.x/ViewPortSize.x,
		CanvasSize.y/ViewPortSize.y * -1.0f
	);
}
Vector2 CameraToViewport(Vector2 point, float z)
{
	return Vector2New(
		(point.x)/z,
		(point.y)/z);
}
// The Y is multiplied by -1 because the world coordinates have Y up
// but on the screen the Y goes down
Vector2 ViewportToCanvas(Vector2 point)
{
	return Vector2New(
		point.x * CanvasSize.x/ViewPortSize.x,
		-point.y * CanvasSize.y/ViewPortSize.y);
}

void InitDoomRender(float verticalFovDegrees)
{
	CanvasSize = Vector2New(SCREEN_WIDTH, SCREEN_HEIGHT); // Canvas is where we draw
	SetVerticalFovDeg(verticalFovDegrees);
	CameraToCanvasConvert = GetConvertXY();
}

bool SeesSide(Vector2 sideA, Vector2 sideB)
{
	if (sideA.x < 0 && sideB.x < 0)
	{
		// A and B are behind player
		return false;
	}
	// NOTE if very close to side and
	// angle to A is more than 180, atan2 will flip around
	// Clamp to -pi
	float angleA = 0.0f;
	if (sideA.y > 0 && sideA.x < 0)
	{
		// A is on the right side and behind: clamp to -pi
		angleA = -M_PI;
	}
	else
	{
		angleA = atan2(sideA.y, sideA.x);
	}
	float angleB = 0.0f;
	if (sideB.y < 0 && sideB.x < 0)
	{
		// B is on the left side and behind, clamp to M_PI
		sideB.x = M_PI;
	}
	else
	{
		angleB = atan2(sideB.y, sideB.x);
	}
	// Negative angle to the left, positive to right
	float LL = -HFOVRAD/2.0f;
	float RL = HFOVRAD/2.0f;
	if (angleA <= LL)
	{
		// A over the left limit: -HFOVRAD/2
		float span = (angleB - angleA);

		// Angle between the two points from players view
		// if span > 0, then seeing the front of side
		if (angleA + span > LL)
		{
			// but B is inside view or over to right
			return true;
		}
		else
		{
			return false;
		}
	}
	else{
		// A inside left limit
		// Side is visible if A is also inside right limit
		return angleA < RL;
	}
}

static const int SIDE_LEFT = -1;
static const int SIDE_RIGHT = 1;

bool Intersect(Vector2 start1, Vector2 end1, Vector2 start2, Vector2 end2, Vector2 & intersectOut)
{
	const Vector2 line1 = Vector2Subtract(start1, end1);
	const Vector2 line2 = Vector2Subtract(start2, end2);
	const float det = Vector2CrossProduct( line1 , line2);
	if (det == 0.0f)
	{
		 return false;
	}

	const float AxA = Vector2CrossProduct(start1, end1);
	const float BxB = Vector2CrossProduct(start2, end2);
	const float x = Vector2CrossProduct(
			Vector2New(AxA, line1.x),
			Vector2New(BxB, line2.x)
		) / det;
	const float y = Vector2CrossProduct(
			Vector2New(AxA, line1.y),
			Vector2New(BxB, line2.y)
		) / det;

	intersectOut = Vector2New(x,y);
	return true;
}

/**
 * @brief This function clips a wall agains the view frustum
 */
bool ClipWall(Vector2 A, Vector2 B, int side, Vector2 &point)
{
	Vector2 other;
	Vector2 frustumSide;
	if (side == SIDE_LEFT) {
		frustumSide = frustumLeft;
		other = B;
	}
	else
	{
		frustumSide = frustumRight;
		other = A;
	}

	const Vector2 line1 = Vector2Subtract(A, B);
	const Vector2 line2 = Vector2Subtract(frustumOrigo, frustumSide);
	const float det = Vector2CrossProduct( line1 , line2);
	if (det == 0.0f)
	{
		 return false;
	}

	const float AxA = Vector2CrossProduct(A, B);
	const float BxB = Vector2CrossProduct(frustumOrigo, frustumSide);
	const float x = Vector2CrossProduct(
			Vector2New(AxA, line1.x),
			Vector2New(BxB, line2.x)
		) / det;
	const float y = Vector2CrossProduct(
			Vector2New(AxA, line1.y),
			Vector2New(BxB, line2.y)
		) / det;

	Vector2 clip = Vector2New(x,y);

	// Clip only if the clipped point is closer to player than the end point B
	float dotclip = Vector2DotProduct(clip, FORWARD_2D);
	float otherClip = Vector2DotProduct(other, FORWARD_2D);
	if (dotclip > 0 && dotclip < otherClip) {
		point = clip;
		return true;
	}
	return false;
}

/**
 * @brief This function is the most difficult part
 * @details The purpose of this is to calculate where the wall segment
 * will be drawn on the screen and provide those cordinates to occlusion
 * culling that is recorded with BlockSegments
 */
bool ProcessWall( Vector2 trans1, Vector2 trans2)
{
	// Are the two end points in front of the player?
	const bool front1 = Vector2DotProduct(trans1, FORWARD_2D) > 0;
	const bool front2 = Vector2DotProduct(trans2, FORWARD_2D) > 0;

	// Is wall behind?
	if (!front1 && !front2)
	{
		return false;
	}
	else if (front1 || front2)
	{
		// Wall is partially seen maybe.
		Vector2 outPoint;
		int side = 0;
		if (front1==false) {
			side = SIDE_LEFT;
		}
		else if (front2==false){
			side = SIDE_RIGHT;

		}
		// NOTE Only clip is one front is false
		bool clipOk = true;
		if (side != 0)
		{
			clipOk = ClipWall(trans1, trans2, side, outPoint);
		}



		// TODO Inline ClipWall









		if (clipOk)
		{
			if (front1 == false)
			{
				trans1 = outPoint;
			}
			else if (front2==false){
				trans2 = outPoint;
			}

			float az =  Vector2DotProduct(trans1, FORWARD_2D);
			float bz =  Vector2DotProduct(trans2, FORWARD_2D);
			float ax =  Vector2DotProduct(trans1, RIGHT_2D);
			float bx =  Vector2DotProduct(trans2, RIGHT_2D);

			Vector2 Atop= ViewportToCanvas(CameraToViewport(Vector2New(ax,0),az));
			Vector2 Btop = ViewportToCanvas(CameraToViewport(Vector2New(bx,0),bz));

			if (Atop.x < Btop.x)
			{
				gp_CanvasAx = Atop.x;
				gp_CanvasBx = Btop.x;
				return true;
			}
		}
	}

	return false;
}

// Forwards


void DrawNode(DoomMap& map, Actor& player, DoomNode& node);

void DrawWall3D(Vector2 left, Vector2 right, s16 picnumMiddle, s16 picnumBottom, s16 picnumTop, s8 shade, u32 linedef_flags)
{

	// TODO Cache these and submit all at once

	Vector2 wallNormal = Vector2RightNormal(left, right);
	if (gp_SectorNeighbour >= 0)
	{
		// Calculate top and bottom parts

		// Create wall that goes down or up to adjacent sector: Note! both sectors dont need to do this. Only lower one

		// if this floor height is less than adjacent: Greate wall in between: goes up
		if (gp_SectorFloorY < gp_SectorNeighbourFloorY && picnumBottom >= 0)
		{
			//BunnySector_DrawWall(wall, end, floory, neighbor.floory, wall.picnum, wall.shade);
			BunnySector_DrawWallF(left.x, left.y, right.x, right.y, wallNormal.x, wallNormal.y, gp_SectorFloorY, gp_SectorNeighbourFloorY,  picnumBottom, shade, linedef_flags);
		}


		// Ceiling:
		// If this ceiling is higher than adjacent: Greate wall in between: goes down
		if (gp_SectorCeilingY > gp_SectorNeighbourCeilingY && picnumTop >= 0)
		{
			//Wall@ otherWall = BunnySector_GetWall(wall.nextwall);
			//DrawQuad(startPoint, endPoint, wallNormal, neighbor.ceilingy, ceilingy, otherWall.picnum, otherWall.shade, 1.0f);
			BunnySector_DrawWallF(left.x, left.y, right.x, right.y, wallNormal.x, wallNormal.y, gp_SectorNeighbourCeilingY, gp_SectorCeilingY, picnumTop, shade, linedef_flags);
		}

		if (picnumMiddle >= 0)
		{
			// TODO Masked portal from higher floor to lower ceiling
			BunnySector_DrawWallF(left.x, left.y, right.x, right.y,
								  wallNormal.x, wallNormal.y, MaxI(gp_SectorFloorY, gp_SectorNeighbourFloorY),
									MinI(gp_SectorCeilingY, gp_SectorNeighbourCeilingY),
									picnumMiddle, shade, linedef_flags);
		}
	}
	else if (picnumMiddle >= 0)
	{
		//BunnySector_DrawWall(wall, end, floory, ceilingy, wall.picnum, wall.shade);
        //DrawQuad(startPoint, endPoint, wallNormal, floory, ceilingy, wall.picnum, wall.shade, 1.0f);
		BunnySector_DrawWallF(left.x, left.y, right.x, right.y, wallNormal.x, wallNormal.y, gp_SectorFloorY, gp_SectorCeilingY, picnumMiddle, shade, linedef_flags);
	}
}


void DrawSubSector(DoomMap& map, Actor& player, DoomSubSector& sub, int sectorIndex)
{
	//printf("-- Drawing subsector %d\n", sectorIndex);
	Vector2 playerPos = player.position.vectorPosition;
	float playerAngle = player.yawRad;

	// NOTE in doom the vertices are stored in counter
	// clockwise order, but ProcessWallTopDown excepts
	// left to right

	// TODO Optimize by remembering the previous point

	u32 firstSeg = sub.firstSegment;
	u32 lastSeg = sub.firstSegment + sub.segmentAmount;
	for (u32 i = firstSeg; i < lastSeg; i++)
	{
		DoomSegment& seg = map.segments[i];

		//printf("---- Drawing Segment %d\n", i);
		// This segment is just for nodes. No need to do anything for it
		if (seg.linedef == 0xFFFF)
		{
			continue;
		}

		u32 next = i + 1;
		if (next >= lastSeg) { next = firstSeg;}

		DoomSegment& partner = map.segments[next];
		DoomVertex& v1 = map.vertices[seg.v1];
		DoomVertex& v2 = map.vertices[partner.v1];

		Vector2 wall1 = Vector2New(v1.x, v1.y);
		Vector2 wall2 = Vector2New(v2.x, v2.y);

		Vector2 trans1 = WorldToCamera(wall1, playerPos, playerAngle);
		Vector2 trans2 = WorldToCamera(wall2, playerPos, playerAngle);

		// Check if wall is facing away
		// NOTE Portals can be either way around and these checks don't work
		DoomLinedef& linedef = map.linedefs[seg.linedef];
		bool isPortal = seg.neighbourSector >= 0; // NOTE 0 is valid subsector number
		if (isPortal || SeesSide(trans1, trans2)) // NOTE FLIP_THE_Y affets this check
		{
			// NOTE FLIP_THE_Y affets this call
			bool draw = ProcessWall(trans1, trans2);
			if (draw || isPortal)
			{
				//printf("----  Segment %d After ProcessWall AX %d BX %d\n", i, gp_CanvasAx, gp_CanvasBx);
			}

			// ProcessWall writes gb_CanvasAx and gb_CanvasBx

			if ((isPortal || draw) && gp_CanvasBx > DRAW_LIMIT_LEFT_CANVAS && gp_CanvasAx < DRAW_LIMIT_RIGHT_CANVAS)
			{
				// Which side are we on of the linedef
				// NOTE FLIPPED DANGER
				DoomVertex& lineStart = map.vertices[linedef.v2];
				DoomVertex& lineEnd = map.vertices[linedef.v1];

				int wallSide = GetWallSide(Vector2New(lineStart.x, lineStart.y), Vector2New(lineEnd.x, lineEnd.y), playerPos);
				int frontSide = 0;
				int backSide = 0;
				if (wallSide == DOOM_SIDE_FRONT)
				{
					frontSide = linedef.sidefront;
					backSide = linedef.sideback;
				}
				else
				{
					frontSide = linedef.sideback;
					backSide = linedef.sidefront;
				}
				if (frontSide < 0)
				{
					// Error
					continue;
				}

				DoomSidedef& sidedef = map.sidedefs[frontSide];


				DoomSector& sector = map.sectors[sidedef.sector];
				gp_SectorFloorY = sector.heightfloor;
				gp_SectorCeilingY = sector.heightceiling;

				gp_SectorNeighbour = backSide;
				if (backSide >= 0)
				{
					DoomSidedef& back_sidedef = map.sidedefs[backSide];
					DoomSector& back_sector = map.sectors[back_sidedef.sector];
					gp_SectorNeighbourCeilingY = back_sector.heightceiling;
					gp_SectorNeighbourFloorY = back_sector.heightfloor;
				}

				// Occlusion test and book keeping
				{
					if (IsSegmentBlocked(gp_CanvasAx, gp_CanvasBx))
					{
						continue;
					}
					if (isPortal)
					{
						if (gp_SectorNeighbourFloorY >= gp_SectorNeighbourCeilingY - 0.1f) // Check if a door is closed
						{
							//printf("---- Closed door X\n");
							PushBlockSegment(gp_CanvasAx, gp_CanvasBx, isPortal);
						}
					}
					else
					{
						PushBlockSegment(gp_CanvasAx, gp_CanvasBx, isPortal);
					}

					// NOTE FLIP_THE_Y affets this call
					//printf("---- Draw wall 3D %.1f %.1f\n", wall2.x, wall1.x);
					DrawWall3D(wall1, wall2, sidedef.texturemiddle, sidedef.texturebottom, sidedef.texturetop, sector.lightlevel, linedef.linedef_flags);
				}
			}
		}
	}
}

void DrawNodeChild(DoomMap& map, Actor& player, ChildId id)
{
	// Only do occlusion test in first person view
	if (IsScreenBlocked())
	{

		if (ChildIsNode(id) == false)
		{
			int sectorId = id & 0x7fffffff;
			//printf("xxx Will not draw subsector %d\n",sectorId );
		}
		return;
	}

	if (ChildIsNode(id))
	{
		DrawNode(map, player, *map.GetChildNode(id));
	}
	else
	{
		DoomSubSector* sub =  map.GetChildSubSector(id);

		// TODO better way
		int sectorId = id & 0x7fffffff;
		DrawSubSector(map, player, *sub, sectorId);
		DrawnSubSectors[subSectorDrawCount] = sectorId;
		subSectorDrawCount += 1;
	}
}

bool PlayerSeesNode(Actor& player, s16 top, s16 bot, s16 left, s16 right)
{
	// Which side of the box the player is (quaranteed to be outside)

	// Used for rotating the points and checking
	// world space relation to the box
	Vector2 pp = player.position.vectorPosition;

	// Vertices of the corners relative to player
	Vector2 A = WorldToCamera(Vector2New(left, bot), pp, player.yawRad);
	Vector2 B = WorldToCamera(Vector2New(left, top), pp, player.yawRad);
	Vector2 C = WorldToCamera(Vector2New(right, top), pp, player.yawRad);
	Vector2 D = WorldToCamera(Vector2New(right, bot), pp, player.yawRad);
	/*  A - D
	 *  |   |
	 *  B - C
	 */


	const int SAME = 0;
	const int LEFT = -1;
	const int RIGHT = 1;
	const int ABOVE = -1;
	const int BELOW = 1;
	Vector2 sideA;
	Vector2 sideB;

	int xdiff = SAME;
	int ydiff = SAME;
	if (pp.y < bot)
	{
		ydiff = ABOVE;
	}
	else if (pp.y > top)
	{
		ydiff = BELOW;
	}
	if (pp.x < left)
	{
		xdiff = LEFT;
	}
	else if (pp.x > right)
	{
		xdiff = RIGHT;
	}
	if (xdiff == LEFT)
	{
		// Sides go from left to right from players point of view
		if (ydiff == ABOVE)
		{
			sideA = D;
			sideB = B;
		}
		else if (ydiff == BELOW)
		{
			sideA = A;
			sideB = C;
		}
		else
		{
			sideA = A;
			sideB = B;
		}
	}
	else if (xdiff == RIGHT)
	{
		if (ydiff == ABOVE)
		{
			sideA = C;
			sideB = A;
		}
		else if (ydiff == BELOW)
		{
			sideA = B;
			sideB = D;
		}
		else
		{
			sideA = C;
			sideB = D;
		}
	}
	else
	{
		if (ydiff == ABOVE)
		{
			sideA = D;
			sideB = A;
		}
		else if (ydiff == BELOW)
		{
			sideA = B;
			sideB = C;
		}
	}

	return SeesSide(sideA, sideB);
}

void DrawNode(DoomMap& map, Actor& player, DoomNode& node)
{
	Vector2 bunnypos = player.position.vectorPosition;
	// This determines which branch is done first so that
	// eventually the players subsector is drawn first

	int childSide = node.GetChildSide(bunnypos.x, bunnypos.y);

	DrawNodeChild(map, player, node.children[childSide]);

	// The other child:
	int indexExtra = (childSide^1) * 4;

	// Do we see the other child at all
	if (PlayerSeesNode(player,
		node.GetBBox(indexExtra + BB_TOP),
		node.GetBBox(indexExtra + BB_BOT),
		node.GetBBox(indexExtra + BB_LFT),
		node.GetBBox(indexExtra + BB_RGT)))
	{
		DrawNodeChild(map, player, node.children[childSide ^ 1]);
	}
}

void RenderDoomMap(DoomMap* map)
{

	ResetGlobalRenderVariables();
	for (int i = 0; i < MAX_SUBSECTORS_DRAWN; i++)
	{
	 DrawnSubSectors[i] = -1;
	}
	ResetBlockSegments();
	subSectorDrawCount = 0;

	//printf("---- START FRAME -----\n");

	// Draw Skybox gradient

Actor* player = BunnySector_GetPlayerActor(0);
	DoomNode* root = map->GetRootNode();
	DrawNode(*map, *player, *root);

	BunnySector_StartFloorCeilingDrawing();

	for (int i = 0; i < subSectorDrawCount; i++)
	{
		// TODO Draw both
		BunnySector_DrawSectorFloorOrCeiling(DrawnSubSectors[i], true);
		BunnySector_DrawSectorFloorOrCeiling(DrawnSubSectors[i], false);
	}

//	printf("---- END FRAME -----\n");
}

