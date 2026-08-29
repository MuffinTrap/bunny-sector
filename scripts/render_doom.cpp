
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
	}
	else
	{
		angleA = atan2(sideA.y, sideA.x);
	}
	float angleB = 0.0f;
	if (sideB.y < 0 && sideB.x < 0)
	{
		sideB.x = M_PI;

	}
	else
	{
		angleB = atan2(sideB.y, sideB.x);
	}
	if (angleB < angleA)
	{
		// Side is facing away
		return false;
	}
	// Negative angle to the left, positive to right
	float LL = -HFOVRAD/2.0f;
	float RL = HFOVRAD/2.0f;
	if (DEBUG_PRINT)
	{
		mgdl_DrawTextFloat("Angle to LL ", RAD2DEG*LL, text_x, NextY(),8, Debug_Green);
		mgdl_DrawTextFloat("Angle to RL ", RAD2DEG*RL, text_x, NextY(),8, Debug_Blue);
		mgdl_DrawTextFloat("Angle to A ", RAD2DEG*angleA, text_x, NextY(), 8, Debug_Yellow);
	}
	if (angleA <= LL)
	{
		// A over the left limit: -HFOVRAD/2


		// Angle between the two points from players view
		// if span > 0, then seeing the front of side
		float span = (angleB - angleA);
		if (DEBUG_PRINT)
		{
			mgdl_DrawTextFloat("Span ", RAD2DEG*span, text_x, NextY(),8, Debug_Green);
			mgdl_DrawTextFloat("Angle to B ", RAD2DEG*angleB, text_x, NextY(),8, Debug_Green);
			mgdl_DrawTextFloat("Angle to A + span ", RAD2DEG*angleB, text_x, NextY(),8, Debug_Green);
		}
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
	DoomVertex@ dp = player.GetDoomPosition();
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

	DrawCross(A, Debug_White);

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
	if (see1 == false)
	{
		// Draw Cross over rejected bounding box
		glBegin(GL_LINES);
			glColor3f(0.5f, 1.0f, 1.0f);
			glVertex2f(A.x, A.y);
			glVertex2f(C.x, C.y);
			glVertex2f(B.x, B.y);
			glVertex2f(D.x, D.y);
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

	return see1;
}

// NOTE Same as Map_IsInsideWall?
int GetChildSide(DoomNode@ node, DoomVertex@ v)
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
			if (node.dx > 0) {return 1;}
			else {return 0;}
		}
		if (node.dx < 0) {return 1;}
		else {return 0;}
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

void DrawSubSector(DoomMap@ map, Actor@ player, DoomSubSector@ sub)
{
	DoomVertex@ pp = player.GetDoomPosition();
	Vector2 playerPos = Vector2New(pp.x, pp.y);
	float playerAngle = player.yawRad;

	// NOTE in doom the vertices are stored in counter
	// clockwise order, but ProcessWallTopDown excepts
	// left to right
	for (uint i = 0; i < sub.segmentAmount; i++)
	{
		uint next = (i+1)%sub.segmentAmount;
		DoomSegment@ seg = sub.segments[i];
		DoomSegment@ partner = sub.segments[next];

		DoomVertex@ v1 = map.vertices[seg.v1];
		DoomVertex@ v2 = map.vertices[partner.v1];

		/*
		glBegin(GL_LINES);
		mgdl_glColor32(pcolor);
		glVertex2f(v1.x, v1.y);
		glVertex2f(v2.x, v2.y);
		glEnd();
		*/

		Vector2 wall1 = Vector2New(v1.x, v1.y);
		Vector2 wall2 = Vector2New(v2.x, v2.y);

		Vector2 trans1 = WorldToCamera(wall1, playerPos, playerAngle);
		Vector2 trans2 = WorldToCamera(wall2, playerPos, playerAngle);

		DrawCross(Vector2New(trans1.x, trans1.y), Debug_White);
		if (drawOrder == 0)
		{
			ProcessWallTopDown(trans2, trans1, player.radius, false);
		}

		// Check if wall is facing away
		if (SeesSide(trans2, trans1))
		{
			ProcessWall(trans2, trans1, -1, -2.0f, 2.0f);
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
		DrawSubSector(map, player, sub);
	}
}

void DrawNode(DoomMap@ map, Actor@ player, DoomNode@ node)
{
	DoomVertex@ playerpos = player.GetDoomPosition();
	// This determines which branch is done first so that
	// eventually the players subsector is drawn first

	// Check if player even sees the other side.
	// If not, then then no need to traverse the tree in that direction

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

	/*
	glBegin(GL_LINES);
		glColor3f(0.0f, 1.0f, 1.0f);
		glVertex2i(node.x, node.y);
		glVertex2i(node.x + node.dx * 100, node.y + node.dy * 100);
	glEnd();
	*/
}

void RenderDoomMap(DoomMap@ map)
{
	drawOrder = 0;
	ResetTextY();

	glClearColor(0.2f, 0.001f, 0.01f, 1.0f);
	Init2D_YDown();

	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;

	Actor@ player = BunnySector_GetActor(0);
	DoomVertex@ playerpos = player.GetDoomPosition();


glPushMatrix();
	u32 va = map.vertexAmount;
	int vv = va;
	mgdl_DrawTextInt("Vertices x: ", vv, 8, 8, 8, Debug_Yellow);

	glTranslatef(screen_half_width, screen_half_height, 0);
	float scale = 1.00f;
	glScalef(scale, scale, 1.0f);

	// Draw Nodes and bounding boxes
	DoomNode@ root = map.GetRootNode();
	DrawNode(map, player, root);


	DrawCross(Vector2New(0,0), Debug_Blue);
	DrawPlayerFOV();
	DrawPlayerPositionAndAngle(player);

	// Testing the player sees node
	//PlayerSeesNode (player, 60, 10, 70, 120);

glPopMatrix();

}

