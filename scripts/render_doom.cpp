
bool RENDER_WALLS_DOOM = true;

// Kep Track which subsectors have been drewn
int[] DrawnSubSectors(128);
int drawnIndex = 0;

// Keeping track which ares of the screen have been drawn
class WallSegment
{
	int start;
	int end;
	WallSegment@ next;
	WallSegment@ prev;
	WallSegment()
	{
		start = 0;
		end = 0;
	}

	WallSegment(int left, int right)
	{
		start = left;
		end = right;
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

	bool MergeInto(WallSegment@ other)
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
		return true;


	}
	void InsertBetween(WallSegment@ before, WallSegment@ after)
	{
		prev = before;
		before.next = this;

		next = after;
		after.prev = this;
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
	wallSegments[0] = WallSegment(-MAX_16, -SCREEN_WIDTH/2);
	wallSegments[1] = WallSegment(SCREEN_WIDTH/2, MAX_16);
	@wallSegments[0].next =  @wallSegments[1];
	@wallSegments[1].prev =  @wallSegments[0];
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

void InsertRecursive(WallSegment@ drawn, int recursion)
{
	if (DEBUG_LOG)
	{
		mgdl_LogTextInt("  Inserrt recursion ", recursion);
		InsertNewSegment(@drawn, recursion);
	}

}

void InsertNewSegment(WallSegment@ drawn, int recursion)
{
	bool merged = false;
	WallSegment@ target = @wallSegments[0];

	while(true)
	{
		// Assume drawn to be on left of target
		if (drawn.OverlapsWith(target))
		{
			if (DEBUG_LOG)
			{
				mgdl_LogText("   overlaps!");
			}
			merged = drawn.MergeInto(target);
			if (merged)
			{
				if (DEBUG_LOG)
				{
					mgdl_LogText("   merged!");
				}
				break;
			}
		}

		// Go to next one
		if (target.next == null)
		{
			break;
		}
		else
		{
			// Drawn was on the right of target
			if (drawn.start > target.end)
			{
				@drawn.prev = @target;
			}
			if (drawn.end < target.start)
			{
				@drawn.prev = @target;
			}
			if (DEBUG_LOG)
			{
				mgdl_LogText("  try next segment");
			}
			@target = @target.next;
		}
	}
	if (merged)
	{
		// See if touches the next one
		while (target.next !is null)
		{
			if (target.OverlapsWith(target.next))
			{
				if (DEBUG_LOG)
				{
					mgdl_LogText("    touches next");
					// merge next one into this
					target.next.MergeInto(target);
				}
				break;
			}
			else
			{
				if (DEBUG_LOG)
				{
					mgdl_LogText("    not touches next");
				}
			}
			@target = @target.next;
		}
	}
	else
	{
		// TODO Does not work yet
		// Did not find overlapping one
		// Put between
		if (drawn.prev !is null)
		{
			@drawn.prev.next = @drawn;
		}
		if (drawn.next !is null)
		{
			@drawn.next.prev = @drawn;
		}
		lastWallSegment += 1;
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
			if (seg.prev == null)
			{
				mgdl_LogText("   no prev");
			}
			if (seg.next == null)
			{
				mgdl_LogText("   no next");
			}
		}
		if (IsWallSegmentFilled())
		{
			mgdl_LogText("   WALL SEGMENTS FILLED");
		}
	}
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
			glColor3f(0.2f, 1.0f, 1.0f);
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

// NOTE THIS IS CORRECT
int GetChildSide(DoomNode@ node, Vector2 v)
{
	if (node.dx == 0)
	{
		// Vertical cut
		if (v.x < node.x)
		{
			if (node.dy > 0) {return 1;}
			else {return 0;}
		}
		if (node.dy < 0) {return 1;}
		else {return 0;}
	}
	if (node.dy == 0)
	{
		// Horizontal cut
		if (v.y < node.y)
		{
			if (node.dx > 0) {return 0;}
			else {return 1;}
		}
		if (node.dx < 0) {return 0;}
		else {return 1;}
	}
	float dx = v.x - node.x;
	float dy = v.y - node.y;
	if( dx * node.dy < dy * node.dx)
	{
		return 1;
	}
	return 0;
}

int drawOrder = 0;

void DrawSubSector(DoomMap@ map, Actor@ player, DoomSubSector@ sub, int sectorIndex)
{
	BunnyV2@ pp = player.GetPosition();
	Vector2 playerPos = Vector2New(pp.x, pp.y);
	float playerAngle = player.yawRad;
	PLAYER_Y = player.elevation + 40;

	// TODO Keep track of screen free space
	DRAW_LIMIT_LEFT_CANVAS = -SCREEN_WIDTH/2;
	DRAW_LIMIT_RIGHT_CANVAS = SCREEN_WIDTH/2;

	// NOTE in doom the vertices are stored in counter
	// clockwise order, but ProcessWallTopDown excepts
	// left to right
	uint nexti= 0;
	uint firstSeg = sub.firstSegment;
	uint lastSeg = sub.firstSegment + sub.segmentAmount;
	for (uint i = firstSeg; i < lastSeg; i++)
	{
		DoomSegment@ seg = map.segments[i];

		uint next = i + 1;
		if (next >= lastSeg) { next = firstSeg;}

		DoomSegment@ partner = map.segments[next];


		DoomVertex@ v1 = map.vertices[seg.v1];
		DoomVertex@ v2 = map.vertices[partner.v1];

		/*
		glBegin(GL_LINES);
		mgdl_glColor32(Debug_Yellow);
		glVertex2f(v1.x, v1.y);
		glVertex2f(v2.x, v2.y);
		glEnd();
		*/

		Vector2 wall1 = Vector2New(v1.x, v1.y);
		Vector2 wall2 = Vector2New(v2.x, v2.y);

		Vector2 trans1 = WorldToCamera(wall1, playerPos, playerAngle);
		Vector2 trans2 = WorldToCamera(wall2, playerPos, playerAngle);

		if (RENDER_WALLS_DOOM == false)
		{
			DrawCross(Vector2New(trans1.x, trans1.y), Debug_Yellow);
			ProcessWallTopDown(trans2, trans1, player.radius, seg.neighbourSubSector >= 0);
			//if (sectorIndex == 1)
			{
			mgdl_DrawTextInt("Sub", sectorIndex, trans2.x, trans2.y-8, 8, Debug_Yellow);
			Vector2 middle = Vector2Add(trans2, Vector2Scale(Vector2Subtract(trans1, trans2), 0.5f));
			// mgdl_DrawTextInt("Seg", i, middle.x, middle.y, 8, Debug_Yellow);
			}
		}
		// Check if wall is facing away
		else if ( SeesSide(trans2, trans1))
		{
			bool draw = ProcessWall(trans2, trans1, DRAW_LIMIT_LEFT_CANVAS, DRAW_LIMIT_RIGHT_CANVAS);
			if (draw)
			{
				// Get sector info
				if (seg.linedef == 0xFFFF)
				{
					continue;
				}
				DoomLinedef@ linedef = map.linedefs[seg.linedef];
				DoomSidedef@ sidedef = map.sidedefs[linedef.sidefront];
				DoomSector@ sector = map.sectors[sidedef.sector];
				SECTOR_FLOORY = sector.heightfloor;
				SECTOR_CEILINGY = sector.heightceiling;

				SECTOR_NEIGHBOR_ID = linedef.sideback;
				if (linedef.sideback >= 0)
				{
					DoomSidedef@ back_sidedef = map.sidedefs[linedef.sideback];
					DoomSector@ back_sector = map.sectors[back_sidedef.sector];
					SECTOR_NEIGHBOR_CEILINGY = back_sector.heightceiling;
					SECTOR_NEIGHBOR_FLOORY = back_sector.heightfloor;
				}
				if (RENDER_2D_WALLS)
				{
					DrawWall2D(false);
					if (DRAW_START_X < DRAW_END_X && DRAW_END_X > DRAW_LIMIT_LEFT_CANVAS && DRAW_START_X < DRAW_LIMIT_RIGHT_CANVAS)
					{
						PushWallSegment(DRAW_START_X, DRAW_END_X);
					}
				}
				else
				{

					/*
				if (linedef.sideback < 0)
				{
					glColor3f( (1+i % 7) * 0.15f, (sub.segmentAmount) * 0.05f, 1.0f -(1+i) * 0.15f);
					// Counterclockwise!
					glVertex3f(v2.x, sector.heightfloor, v2.y);
					glVertex3f(v1.x, sector.heightfloor, v1.y);
					glVertex3f(v1.x, sector.heightceiling, v1.y);
					glVertex3f(v2.x, sector.heightceiling, v2.y);
				}
				*/
					DrawWall3D(wall1, wall2, sidedef.texturemiddle, sidedef.texturebottom, sidedef.texturetop, sector.lightlevel);

					/*
					glVertex3f(v1.x, sector.heightfloor, v1.y);
					glVertex3f(v1.x, sector.heightceiling, v1.y);

					glVertex3f(v2.x, sector.heightfloor, v2.y);
					glVertex3f(v2.x, sector.heightceiling, v2.y);
					*/
				}

				// TODO Update used screen area
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
		drawnIndex += 1;

		DrawSubSector(map, player, sub, sectorId);

	}
}

int FindSubSectorRec(DoomMap@ map, DoomNode@ node, Vector2 point)
{
	int childSide = GetChildSide(node, point);
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
	BunnyV2@ bunnypos = player.GetPosition();
	// This determines which branch is done first so that
	// eventually the players subsector is drawn first

	// Check if player even sees the other side.
	// If not, then then no need to traverse the tree in that direction

	Vector2  playerpos = Vector2New(bunnypos.x, bunnypos.y);
	int childSide = GetChildSide(node, playerpos);

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
		//DrawNodeChild(map, player, node.children[childSide ^ 1]);

	/*
	glBegin(GL_LINES);
		glColor3f(0.0f, 1.0f, 1.0f);
		glVertex2i(node.x, node.y);
		glVertex2i(node.x + node.dx * 100, node.y + node.dy * 100);
	glEnd();
	*/
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

	RENDER_WALLS_DOOM = false;
	Actor@ player = BunnySector_GetActor(0);
	DoomNode@ root = map.GetRootNode();
	DrawNode(map, player, root);

	DrawCross(Vector2New(0,0), Debug_Blue);
	DrawPlayerFOV();
	DrawPlayerPositionAndAngle(player);
	RENDER_WALLS_DOOM = true;

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
void RenderDoomMap(DoomMap@ map)
{
	if (DEBUG_LOG)
	{
		mgdl_LogText("----------- DOOM FRAME START --------------");
	}
	drawOrder = 0;

	Actor@ player = BunnySector_GetActor(0);
	if (RENDER_2D_WALLS)
	{
		glPushMatrix();

			float screen_half_width = SCREEN_WIDTH / 2.0f;
			float screen_half_height = SCREEN_HEIGHT / 2.0f;
			glTranslatef(screen_half_width, screen_half_height, 0);
	}

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

	if (RENDER_2D_WALLS)
	{
		glPopMatrix();
	}

	if (DEBUG_LOG)
	{
		mgdl_LogText("----------- DOOM FRAME END --------------");
	}
}

