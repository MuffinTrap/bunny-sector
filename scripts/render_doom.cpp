
bool ChildIsNode(ChildId childId)
{
	return (childId & 0x80000000 ) == 0;
}
bool ChildIsSector(ChildId childId)
{
	return (childId & 0x80000000 ) > 0;
}


// What angle from player to given point
float PointToAngle(Vector2 p, Vector2 v)
{
	Vector2 delta = Vector2Subtract(v, p);
	return atan2(delta.y, delta.x);
}

bool SeesSide(Vector2 sideA, Vector2 sideB, Vector2 p, float yawRad)
{
	float angle1 = PointToAngle(p, sideA);
	float angle2 = PointToAngle(p, sideB);

	// Angle between the two points from players view
	float span = NormalizeAngleRad(angle1 - angle2);
	// Normalize so that player looks at angle 0
	angle1 -= yawRad;


	float span1 = NormalizeAngleRad(angle1 + HFOVRAD/2.0f);
	if (span1 > HFOVRAD)
	{
		if (span1 >= span + HFOVRAD)
		{
			return false;
		}
	}
	return true;
}

// Is the bounding box clipping or inside the player's FOV?
bool PlayerSeesNode(Actor@ player, s16 top, s16 bot, s16 left, s16 right)
{
	// Which side of the box the player is (quaranteed to be outside)
	DoomVertex@ dp = player.GetDoomPosition();
	Vector2 p = Vector2New(dp.x, dp.y);

	// Vertices of the corners relative to player
	Vector2 A = WorldToCamera(Vector2New(left, bot), p, player.yawRad);
	Vector2 B = WorldToCamera(Vector2New(left, top), p, player.yawRad);
	Vector2 C = WorldToCamera(Vector2New(right, top), p, player.yawRad);
	Vector2 D = WorldToCamera(Vector2New(right, bot), p, player.yawRad);
	/*  B - C
	 *  |   |
	 *  A - D
	 */


	int SAME = 0;
	int LEFT = -1;
	int RIGHT = 1;
	int ABOVE = -1;
	int BELOW = 1;
	int sides = 1; // How many sides player sees
	Vector2 side1A;
	Vector2 side1B;
	Vector2 side2A;
	Vector2 side2B;

	// Test player should see side B-A
	// Angle mismatch
	if (SeesSide(A, B, p, player.yawRad))
	{
		mgdl_DrawLineV(B, A, Debug_White);
	}

	int xdiff = SAME;
	int ydiff = SAME;
	if (p.y > top)
	{
		ydiff = ABOVE;
		mgdl_DrawLine(0, 0, B.x, B.y, Debug_White);
		mgdl_DrawLine(0, 0, C.x, C.y, Debug_White);
	}
	else if (p.y < bot)
	{
		ydiff = BELOW;
		mgdl_DrawLine(0, 0, A.x, A.y, Debug_Yellow);
		mgdl_DrawLine(0, 0, D.x, D.y, Debug_Yellow);
	}
	if (p.x < left)
	{
		xdiff = LEFT;
		mgdl_DrawLine(0, 0, B.x, B.y, Debug_Blue);
		mgdl_DrawLine(0, 0, A.x, A.y, Debug_Blue);
	}
	else if (p.x > right)
	{
		xdiff = RIGHT;
		mgdl_DrawLine(0, 0, C.x, C.y, Debug_Green);
		mgdl_DrawLine(0, 0, D.x, D.y, Debug_Green);
	}
	if (xdiff == LEFT)
	{
		// Sides go from left to right from players point of view
		side1A = B;
		side1B = A;
		if (ydiff == ABOVE)
		{
			side2A = C;
			side2B = B;
			sides = 2;
		}
		else if (ydiff == BELOW)
		{
			side2A = A;
			side2B = D;
			sides = 2;
		}
	}
	else if (xdiff == RIGHT)
	{
		side1A = D;
		side1B = C;
		if (ydiff == ABOVE)
		{
			side2A = C;
			side2B = B;
			sides = 2;
		}
		else if (ydiff == BELOW)
		{
			side2A = A;
			side2B = D;
			sides = 2;
		}
	}
	else
	{
		if (ydiff == ABOVE)
		{
			side1A = C;
			side1B = B;
		}
		else if (ydiff == BELOW)
		{
			side1A = A;
			side1B = D;
		}
	}

	// Always test side 1
	bool see2 = false;
	bool see1 = SeesSide(side1A, side1B, p, player.yawRad);
	if ( see1 == false && sides == 2)
	{
		// Test side 2 if available
		see2 = SeesSide(side2A, side2B, p, player.yawRad);
	}
	// no side 2 to test, not visible

	if (see1 == false && see2 == false)
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
		glBegin(GL_LINES);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex2f(A.x, A.y);
			glVertex2f(B.x, B.y);

			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex2f(B.x, B.y);
			glVertex2f(C.x, C.y);

			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex2f(C.x, C.y);
			glVertex2f(D.x, D.y);

			glColor3f(1.0f, 0.0f, 1.0f);
			glVertex2f(D.x, D.y);
			glVertex2f(A.x, A.y);
		glEnd();
	}
	return see1 || see2;
}

// NOTE Same as Map_IsInsideWall?
bool IsOnBackSide(DoomNode@ node, DoomVertex@ v)
{
	float dx = v.x - node.x;
	float dy = v.y - node.y;
	return dx * node.dy - dy * node.dx <= 0;
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
	bool onChild1Area = IsOnBackSide(node, playerpos);

	// Check if player even sees the other side.
	// If not, then then no need to traverse the tree in that direction

	if (onChild1Area)
	{
		DrawNodeChild(map, player, node.child1);

		// Do we see the other child at all
		if (PlayerSeesNode(player, node.top0, node.bottom0, node.left0, node.right0))
		{
			DrawNodeChild(map, player, node.child0);
		}
	}
	else
	{
		DrawNodeChild(map, player, node.child0);
		if (PlayerSeesNode(player, node.top1, node.bottom1, node.left1, node.right1))
		{
			DrawNodeChild(map, player, node.child1);
		}
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
	//DrawNode(map, player, root);


	DrawCross(Vector2New(0,0), Debug_Blue);
	DrawPlayerFOV();
	DrawPlayerPositionAndAngle(player);

	// Testing the player sees node
	PlayerSeesNode (player, 40, 20, 70, 90);

glPopMatrix();

}

