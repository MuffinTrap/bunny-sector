
bool RENDER_TOPDOWN = false;
const int DOOM_SIDE_FRONT = 0;
const int DOOM_SIDE_BACK = 0;

// Kep Track which subsectors have been drewn
int[] DrawnSubSectors(128);
int drawnIndex = 0;

// Keeping track which ares of the screen have been drawn
class WallSegment
{
	int start;
	int end;
	int nextIndex;
	int prevIndex;
	int myIndex;
	WallSegment()
	{
		start = 0;
		end = 0;
		nextIndex = -1;
		prevIndex = -1;
		myIndex = -1;
	}

	WallSegment(int left, int right, int prev, int next, int index)
	{
		start = left;
		end = right;
		prevIndex = prev;
		nextIndex = next;
		myIndex = index;
	}

	bool OverlapsWith(WallSegment@ other)
	{
// Overlap:  Determine whether the two number ranges overlap.
// #define Overlap(a0,a1,b0,b1) (map_min(a0,a1) <= map_max(b0,b1) && map_min(b0,b1) <= map_max(a0,a1))

		if (MinI(start, end) <= MaxI(other.start, other.end) && MinI(other.start, other.end) <= MaxI(start, end))
		{
			return true;
		}
		return false;

	}

	void MergeInto(WallSegment@ other)
	{
		if (DEBUG_LOG)
		{
			mgdl_LogText("  try merge into");
			mgdl_LogTextInt("  other.start", other.start);
			mgdl_LogTextInt("  other.end", other.end);
		}
		if (start >= other.start && end <= other.end)
		{
			// This is inside the other!
		}
		if (start < other.start)
		{
			other.start = start;
		}
		if (end > other.end)
		{
			other.end = end;
		}
	}
	void Drop()
	{
		if (prevIndex >= 0)
		{
			WallSegment@ prevSegment = @wallSegments[prevIndex];
			prevSegment.nextIndex = nextIndex;
		}
		if (nextIndex >= 0)
		{
			WallSegment@ nextSegment = @wallSegments[nextIndex];
			nextSegment.prevIndex = prevIndex;
		}
	}
}

int WALL_SEGMENT_AMOUNT = 64;
WallSegment[] wallSegments(WALL_SEGMENT_AMOUNT);
int lastWallSegment = 0;
// Linked list


int MAX_16 = 0x7FFF;

void InitWallSegments()
{

}

void ResetWallSegments()
{
	wallSegments[0] = WallSegment(-MAX_16, -SCREEN_WIDTH/2, -1, 1, 0);
	wallSegments[1] = WallSegment(SCREEN_WIDTH/2, MAX_16, 0, -1, 1);
	lastWallSegment = 2;
}

// This is complicated, do later
void PushWallSegment(int startx, int endx)
{
	if (DEBUG_LOG)
	{
		mgdl_LogTextInt("Push segment start ", startx);
		mgdl_LogTextInt("Push segment end ", endx);
	}
	WallSegment@ drawn = @wallSegments[lastWallSegment];
	drawn.start = startx;
	drawn.end = endx;
	InsertNewSegment(drawn, 0);
}

// Called after drawing a wall
/*
 * Start from the first segment ; this is the target
 * 1. if incoming segment overlaps with target -> merge incoming to target -> BREAK
 * 2. if incoming segment is before the target -> add it to segments -> BREAK
 * 3. if incoming segment is after the target -> take next segment as target and repeat
 *
 * if incoming was merged:
 * 	4. check if target now overlaps with the next one
 * 		5. if it overlaps: merge next one into target. Take the next next one as new next and repeat
 * else : STOP
 *
 * */

void InsertNewSegment(WallSegment@ drawn, int recursion)
{
	bool merged = false;
	int prevTargetIndex = -1;
	int targetIndex = 0;
	while(true)
	{
		WallSegment@ target = @wallSegments[targetIndex];
		// 1. Try merge
		if (drawn.OverlapsWith(target))
		{
			if (DEBUG_LOG)
			{
				mgdl_LogText("   overlaps!");
			}
			drawn.MergeInto(target);
			merged = true;
			if (DEBUG_LOG)
			{
				mgdl_LogText("   merged!");
			}
			break;
		} // 2. Insert?
		else if (drawn.end < target.start)
		{
			drawn.myIndex = lastWallSegment;
			drawn.prevIndex = prevTargetIndex;
			if (prevTargetIndex >= 0)
			{
				WallSegment@ previous = @wallSegments[prevTargetIndex];
				previous.nextIndex = lastWallSegment;
			}
			WallSegment@ next = @wallSegments[targetIndex];
			next.prevIndex = lastWallSegment;
			drawn.nextIndex = targetIndex;

			wallSegments[lastWallSegment] = drawn;
			lastWallSegment += 1;
			if (lastWallSegment >= WALL_SEGMENT_AMOUNT)
			{
				mgdl_LogText("Too many wall segments!");
			}
			break;
		}
		// 3. Try next one
		else if (drawn.start > target.end)
		{
			if (DEBUG_LOG)
			{
				mgdl_LogText("  try next segment");
			}
			prevTargetIndex += 1;
			targetIndex += 1;
			continue;
		}
	}


	if (merged)
	{
		WallSegment@ target = @wallSegments[targetIndex];
		int nextIndex = target.nextIndex;
		// See if touches the next one
		while (nextIndex >= 0)
		{
			// 4. Check if merge reaches next one
			WallSegment@ nextSegment = @wallSegments[nextIndex];
			if (target.OverlapsWith(nextSegment))
			{
				if (DEBUG_LOG)
				{
					mgdl_LogText("    touches next");
					// merge next one into this
					nextSegment.MergeInto(target);
				}
				// 5.
				nextSegment.Drop();
				nextIndex = nextSegment.nextIndex;
			}
			else
			{
				if (DEBUG_LOG)
				{
					mgdl_LogText("    not touches next");
				}
				break;
			}
		}
	}

	// DEBUG
	if (DEBUG_LOG && recursion == 0)
	{
		for (int i = 0; i < lastWallSegment; i++)
		{
			WallSegment@ seg = @wallSegments[i];
			mgdl_LogTextInt("Wall seg ", i);
			mgdl_LogTextInt("   start", seg.start);
			mgdl_LogTextInt("   end", seg.end);
			mgdl_LogTextInt("   <- prev", seg.prevIndex);
			mgdl_LogTextInt("   next ->", seg.nextIndex);
		}
		if (IsWallSegmentFilled())
		{
			mgdl_LogText("   WALL SEGMENTS FILLED");
		}
	}
}

bool IsWallSegmentOccluded(int startx, int endx)
{
	int nextIndex = 0;
	while(nextIndex >= 0)
	{
		WallSegment@ seg = @wallSegments[nextIndex];
		if (startx >= seg.start && endx <= seg.end)
		{
			return true;
		}
		nextIndex = seg.nextIndex;
	}
	return false;
}

bool IsWallSegmentFilled()
{
	if (wallSegments[0].start <= -MAX_16 + 1 && wallSegments[0].end >= MAX_16-1)
	{
		return true;
	}
	return false;
}


bool ChildIsNode(ChildId childId)
{
	return (childId & 0x80000000 ) == 0;
}
bool ChildIsSector(ChildId childId)
{
	return (childId & 0x80000000 ) > 0;
}

// NOTE Sides are in player view space
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
		if (DEBUG_DRAW)
		{
			DrawCross(sideA, Debug_Red);
		}
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
		if (DEBUG_DRAW)
		{
			DrawCross(sideB, Debug_Red);
		}
	}
	else
	{
		angleB = atan2(sideB.y, sideB.x);
	}
	// Negative angle to the left, positive to right
	float LL = -HFOVRAD/2.0f;
	float RL = HFOVRAD/2.0f;
	if (DEBUG_DRAW)
	{
		mgdl_DrawTextFloat("Angle to LL ", RAD2DEG*LL, text_x, NextY(),8, Debug_Green);
		mgdl_DrawTextFloat("Angle to RL ", RAD2DEG*RL, text_x, NextY(),8, Debug_Blue);
		mgdl_DrawTextFloat("Angle to A ", RAD2DEG*angleA, text_x, NextY(), 8, Debug_Yellow);
	}
	if (angleA <= LL)
	{
		// A over the left limit: -HFOVRAD/2

			float span = (angleB - angleA);
		if (DEBUG_DRAW)
		{
			mgdl_DrawTextFloat("Span ", RAD2DEG*span, text_x, NextY(),8, Debug_Green);
			mgdl_DrawTextFloat("Angle to B ", RAD2DEG*angleB, text_x, NextY(),8, Debug_Green);
			mgdl_DrawTextFloat("Angle to A + span ", RAD2DEG*angleB, text_x, NextY(),8, Debug_Green);
		}

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

// Is the bounding box clipping or inside the player's FOV?
bool PlayerSeesNode(Actor@ player, s16 top, s16 bot, s16 left, s16 right)
{
	// Which side of the box the player is (quaranteed to be outside)

	// Used for rotating the points and checking
	// world space relation to the box
	BunnyV2@ dp = player.GetPosition();
	Vector2 pp = Vector2New(dp.x, dp.y);

	// Vertices of the corners relative to player
	Vector2 A = WorldToCamera(Vector2New(left, bot), pp, player.yawRad);
	Vector2 B = WorldToCamera(Vector2New(left, top), pp, player.yawRad);
	Vector2 C = WorldToCamera(Vector2New(right, top), pp, player.yawRad);
	Vector2 D = WorldToCamera(Vector2New(right, bot), pp, player.yawRad);
	/*  A - D
	 *  |   |
	 *  B - C
	 */


	int SAME = 0;
	int LEFT = -1;
	int RIGHT = 1;
	int ABOVE = -1;
	int BELOW = 1;
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


	bool see1 = SeesSide(sideA, sideB);
	if (DEBUG_DRAW)
	{
		DrawCross(A, Debug_Yellow);
		if (see1 == false)
		{
			// Draw Cross over rejected bounding box
			glBegin(GL_LINES);
			glColor3f(0.0f, 0.5f, 0.5f);
			glVertex2f(A.x, A.y);
			glVertex2f(C.x, C.y);
			glVertex2f(B.x, B.y);
			glVertex2f(D.x, D.y);

			// Show the test side
			glColor3f(0.8f, 1.0f, 1.0f);
			glVertex2f(sideA.x, sideA.y);
			glVertex2f(sideB.x, sideB.y);
			glEnd();
		}
		else
		{
			// Draw outline around seen box
			glBegin(GL_LINE_LOOP);
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex2f(A.x, A.y);
			glVertex2f(B.x, B.y);

			glVertex2f(C.x, C.y);

			glVertex2f(D.x, D.y);

			glVertex2f(A.x, A.y);
			glEnd();
		}
	}

	return see1;
}

int GetWallSide(Vector2 wstart, Vector2 wend, Vector2 point)
{
	Vector2 delta = Vector2Subtract(wend, wstart);

	if (delta.x == 0)
	{
		// Vertical cut
		if (point.x < wstart.x)
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
		if (point.y < wstart.y)
		{
			if (delta.x > 0) {return 0;}
			else {return 1;}
		}
		if (delta.x < 0) {return 0;}
		else {return 1;}
	}
	float dxp = point.x - wstart.x;
	float dyp = point.y - wstart.y;
	if( dxp * delta.y < dyp * delta.x)
	{
		return 1;
	}
	return 0;

}


int drawOrder = 0;

void DrawSubSectorTopDown(DoomMap@ map, Actor@ player, DoomSubSector@ sub, int sectorIndex)
{

	BunnyV2@ pp = player.GetPosition();
	Vector2 playerPos = Vector2New(pp.x, pp.y);
	float playerAngle = player.yawRad;
	PLAYER_Y = player.elevation + 40;

	// NOTE in doom the vertices are stored in counter
	// clockwise order, but ProcessWallTopDown excepts
	// left to right
	uint nexti= 0;
	uint firstSeg = sub.firstSegment;
	uint lastSeg = sub.firstSegment + sub.segmentAmount;
	float middlex = 0;
	float middley = 0;
	for (uint i = firstSeg; i < lastSeg; i++)
	{
		DoomSegment@ seg = map.segments[i];

		// This segment is just for nodes. No need to do anything for it
		if (seg.linedef == 0xFFFF)
		{
			continue;
		}

		uint next = i + 1;
		if (next >= lastSeg) { next = firstSeg;}

		DoomSegment@ partner = map.segments[next];
		DoomVertex@ v1 = map.vertices[seg.v1];
		DoomVertex@ v2 = map.vertices[partner.v1];

		Vector2 wall1 = Vector2New(v1.x, v1.y);
		Vector2 wall2 = Vector2New(v2.x, v2.y);

		Vector2 trans1 = WorldToCamera(wall1, playerPos, playerAngle);
		Vector2 trans2 = WorldToCamera(wall2, playerPos, playerAngle);

		DrawCross(Vector2New(trans1.x, trans1.y), Debug_Yellow);
		bool isPortal = seg.neighbourSubSector >= 0;
		ProcessWallTopDown(trans1, trans2, player.radius, isPortal); // NOTE: FLIP_THE_Y changes this

		middlex += trans1.x;
		middley += trans1.y;

	}
	mgdl_DrawTextInt("Subs", sectorIndex, middlex/sub.segmentAmount, middley/sub.segmentAmount, 8, Debug_Yellow);
}

void DrawSubSector(DoomMap@ map, Actor@ player, DoomSubSector@ sub, int sectorIndex)
{
	BunnyV2@ pp = player.GetPosition();
	Vector2 playerPos = Vector2New(pp.x, pp.y);
	float playerAngle = player.yawRad;
	PLAYER_Y = player.elevation + 40;

	// NOTE in doom the vertices are stored in counter
	// clockwise order, but ProcessWallTopDown excepts
	// left to right
	uint nexti= 0;
	uint firstSeg = sub.firstSegment;
	uint lastSeg = sub.firstSegment + sub.segmentAmount;
	for (uint i = firstSeg; i < lastSeg; i++)
	{
		DoomSegment@ seg = map.segments[i];

		// This segment is just for nodes. No need to do anything for it
		if (seg.linedef == 0xFFFF)
		{
			continue;
		}

		uint next = i + 1;
		if (next >= lastSeg) { next = firstSeg;}

		DoomSegment@ partner = map.segments[next];
		DoomVertex@ v1 = map.vertices[seg.v1];
		DoomVertex@ v2 = map.vertices[partner.v1];

		Vector2 wall1 = Vector2New(v1.x, v1.y);
		Vector2 wall2 = Vector2New(v2.x, v2.y);

		Vector2 trans1 = WorldToCamera(wall1, playerPos, playerAngle);
		Vector2 trans2 = WorldToCamera(wall2, playerPos, playerAngle);

		// Check if wall is facing away
		// NOTE Portals can be either way around and these checks don't work
		DoomLinedef@ linedef = map.linedefs[seg.linedef];
		bool isPortal = seg.neighbourSubSector >= 0;
		if (isPortal || SeesSide(trans1, trans2)) // NOTE FLIP_THE_Y affets this check
		{
			// NOTE FLIP_THE_Y affets this call
			bool draw = ProcessWall(trans1, trans2, DRAW_LIMIT_LEFT_CANVAS, DRAW_LIMIT_RIGHT_CANVAS);
			if ((isPortal || draw) && CANVAS_BX > DRAW_LIMIT_LEFT_CANVAS && CANVAS_AX < DRAW_LIMIT_RIGHT_CANVAS)
			{
				// Which side are we on of the linedef
				DoomVertex@ lineStart = map.vertices[linedef.v2];
				DoomVertex@ lineEnd = map.vertices[linedef.v1];
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

				DoomSidedef@ sidedef = map.sidedefs[frontSide];

				// Occlusion test and book keeping
				if (sidedef.texturemiddle >= 0)
				{
					if (IsWallSegmentOccluded(CANVAS_AX, CANVAS_BX))
					{
						continue;
					}
					PushWallSegment(CANVAS_AX, CANVAS_BX);
				}

				DoomSector@ sector = map.sectors[sidedef.sector];
				SECTOR_FLOORY = sector.heightfloor;
				SECTOR_CEILINGY = sector.heightceiling;

				SECTOR_NEIGHBOR_ID = backSide;
				if (backSide >= 0)
				{
					DoomSidedef@ back_sidedef = map.sidedefs[backSide];
					DoomSector@ back_sector = map.sectors[back_sidedef.sector];
					SECTOR_NEIGHBOR_CEILINGY = back_sector.heightceiling;
					SECTOR_NEIGHBOR_FLOORY = back_sector.heightfloor;
				}
				if (RENDER_2D_WALLS)
				{
					DrawWall2D(false);
				}
				else
				{

					// NOTE FLIP_THE_Y affets this call
					DrawWall3D(wall2, wall1, sidedef.texturemiddle, sidedef.texturebottom, sidedef.texturetop, sector.lightlevel);
				}
			}
		}
	}
	drawOrder += 1;
}

void DrawNodeChild(DoomMap@ map, Actor@ player, ChildId id)
{
	if (ChildIsNode(id))
	{
		DrawNode(map, player, map.GetChildNode(id));
	}
	else
	{
		DoomSubSector@ sub =  map.GetChildSubSector(id);

		// TODO better way
		int sectorId = id & 0x7fffffff;
		DrawnSubSectors[drawnIndex] = sectorId;

		if (RENDER_TOPDOWN)
		{
			DrawSubSectorTopDown(map, player, sub, sectorId);
		}
		else if (IsWallSegmentFilled() == false)
		{
			DrawSubSector(map, player, sub, sectorId);
		}

		drawnIndex += 1;
	}
}

int FindSubSectorRec(DoomMap@ map, DoomNode@ node, Vector2 point)
{
	int childSide = node.GetChildSide(point.x, point.y);
	if (ChildIsNode(node.children[childSide]))
	{
		return FindSubSectorRec(map, map.nodes[node.children[childSide]], point);
	}
	else
	{
		return node.children[childSide] & 0x7fffffff;
	}
}


void DrawNode(DoomMap@ map, Actor@ player, DoomNode@ node)
{
	if (IsWallSegmentFilled())
	{
		return;
	}
	BunnyV2@ bunnypos = player.GetPosition();
	// This determines which branch is done first so that
	// eventually the players subsector is drawn first

	// Check if player even sees the other side.
	// If not, then then no need to traverse the tree in that direction
	if (RENDER_TOPDOWN)
	{
		BunnyV2@ pp = player.GetPosition();
		Vector2 playerPos = Vector2New(pp.x, pp.y);
		float playerAngle = player.yawRad;
		Vector2 ns = Vector2New(node.x, node.y);
		Vector2 ne = Vector2New(node.x + node.dx, node.y + node.dy);

		Vector2 trans1 = WorldToCamera(ns, playerPos, playerAngle);
		Vector2 trans2 = WorldToCamera(ne, playerPos, playerAngle);

		glBegin(GL_LINES);
		mgdl_glColor32(Debug_Blue);
		glVertex2f(trans1.x + 1, trans1.y + 1);
		glVertex2f(trans2.x + 1, trans2.y + 1);
		glEnd();
	}

	int childSide = node.GetChildSide(bunnypos.x, bunnypos.y);

	DrawNodeChild(map, player, node.children[childSide]);

	// The other child:
	int indexExtra = (childSide^1) * 4;

	// Do we see the other child at all
	if (PlayerSeesNode(player,
		node.bbox[indexExtra + BB_TOP],
		node.bbox[indexExtra + BB_BOT],
		node.bbox[indexExtra + BB_LFT],
		node.bbox[indexExtra + BB_RGT]))
	{
		DrawNodeChild(map, player, node.children[childSide ^ 1]);
	}
}

void RenderMiniMapDoom(DoomMap@ map)
{
	Init2D_YDown();
glPushMatrix();

	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;
	glTranslatef(screen_half_width, screen_half_height, 0);
	float scale = 1.00f;
	glScalef(scale, scale, 1.0f);

	// Draw Nodes and bounding boxes
	Actor@ player = BunnySector_GetActor(0);

	DoomNode@ root = map.GetRootNode();
	RENDER_TOPDOWN = true;
	if (map.nodeAmount > 0)
	{
		DrawNode(map, player, root);
	}
	else
	{
		DoomSubSector@ sub =  map.GetChildSubSector(0);
		DrawSubSector(map, player, sub, 0);
	}
	RENDER_TOPDOWN = false;

	DrawCross(Vector2New(0,0), Debug_Blue);
	DrawPlayerFOV();
	DrawPlayerPositionAndAngle(player);

	// Testing the player sees node
	//PlayerSeesNode (player, 60, 10, 70, 220);

	BunnyV2@ bp = player.GetPosition();
	Vector2 pp = Vector2New(bp.x, bp.y);

	mgdl_DrawTextInt("Player subsec", FindSubSectorRec(map, root, pp), text_x, NextY(), 8, Debug_Red);
	//mgdl_DrawTextFloat("Units to meter ", angel_unitstometer, text_x, NextY(), 16, Debug_Red);
	BunnySector_DrawCameraInfo(text_x, NextY());
glPopMatrix();
}

void StartFrame_Doom()
{
	for (int i = 0; i < 128; i++)
	{
	 DrawnSubSectors[i] = -1;
	}
	drawnIndex = 0;
	ResetWallSegments();
}

void RenderDoomMapLines(DoomMap@ map)
{
	Init2D_YDown();
	glPushMatrix();

		float screen_half_width = SCREEN_WIDTH / 2.0f;
		float screen_half_height = SCREEN_HEIGHT / 2.0f;
		glTranslatef(screen_half_width, screen_half_height, 0);

	Actor@ player = BunnySector_GetActor(0);

	// Draw Nodes and bounding boxes
	DoomNode@ root = map.GetRootNode();
	DrawNode(map, player, root);

	glPopMatrix();
}

void RenderDoomMap(DoomMap@ map)
{
	if (DEBUG_LOG)
	{
		mgdl_LogText("----------- DOOM FRAME START --------------");
	}
	drawOrder = 0;

	Actor@ player = BunnySector_GetActor(0);

	// Draw Nodes and bounding boxes
	DoomNode@ root = map.GetRootNode();
	DrawNode(map, player, root);

	// Draw all floors and ceilings
	BunnySector_StartFloorCeilingDrawing();
	for (int i = 0; i < drawnIndex; i++)
	{
		BunnySector_DrawSectorFloorOrCeiling(DrawnSubSectors[i], true);
		BunnySector_DrawSectorFloorOrCeiling(DrawnSubSectors[i], false);
	}

	if (DEBUG_LOG)
	{
		mgdl_LogText("----------- DOOM FRAME END --------------");
	}
}

