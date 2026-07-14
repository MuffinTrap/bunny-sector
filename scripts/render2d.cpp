
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

float DEBUG_SCALE = 6.0f;

float PLAYER_HALF_FOV_DEG = 45.0f;
Vector2 FORWARD_2D = Vector2(1, 0);
Vector2 RIGHT_2D = Vector2Rotate(FORWARD_2D, DEG2RAD * 90);

int SEE_LEFT = -1;
int SEE_RIGHT = 1;
int SEE_FRONT = 0;

int WALL_LIMIT = -1;

//Actor drives
	Vector2 player = Vector2(50,50);
	float playerAngle = 0.0f;
	Vector2 playerDir = Vector2(1, 0);

// Arrays for storing top and bottom limits
int[] TOP_LIMITS(SCREEN_WIDTH);
int[] BOTTOM_LIMITS(SCREEN_WIDTH);

int PlayerSeesPoint(Vector2 point)
{
	float dotF = Vector2DotProduct(Vector2Normalize(point), FORWARD_2D);
	if (dotF > 0)
	{
		float angle = cos(dotF);
		if (RAD2DEG * angle < PLAYER_HALF_FOV_DEG)
		{
			return SEE_FRONT;
		}
	}
	float dotR = Vector2DotProduct(Vector2Normalize(point), RIGHT_2D);
	if (dotR > 0)
	{
		return SEE_RIGHT;
	}
	else
	{
		return SEE_LEFT;
	}
}

Vector2 WikiIntersect(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4)
{
	float xtop = Vector2CrossProduct(p1, p2)*(p3.x - p4.x) - (p1.x - p2.x) * Vector2CrossProduct(p3, p4);
	float ytop = Vector2CrossProduct(p1, p2)*(p3.y - p4.y) - (p1.y - p2.y) * Vector2CrossProduct(p3, p4);
	float det = (p1.x - p2.x)*(p3.y - p4.y) - (p1.y - p2.y)*(p3.x - p4.x);

	if (det == 0.0)
	{
		return Vector2Zero();
	}

	return Vector2(xtop/det, ytop/det);

}

Vector2 Intersect(Vector2 start1, Vector2 end1, Vector2 start2, Vector2 end2)
{
	float x = Vector2CrossProduct(start1, end1);
	float y = Vector2CrossProduct(start2, end2);
	Vector2 line1 = Vector2Subtract(start1, end1);
	Vector2 line2 = Vector2Subtract(start2, end2);
	float det = Vector2CrossProduct( line1 , line2);
	x = Vector2CrossProduct( Vector2(x, line1.x), Vector2(y, line2.x)) / det;
	y = Vector2CrossProduct( Vector2(x, line1.y), Vector2(y, line2.y)) / det;

	return Vector2(x,y);
}

Vector2 ClipBehindPlayer(float ax, float ay, float bx, float by)
{
	// Near plane of camera
	float px1 = 1;
	float py1 = 1;
	float px2 = 200;
	float py2 = 1;

	float a = (px1-px2) * (ay - py2) - (py1 - py2) * (ax - px2);
	float b = (py1-py2) * (ax - bx) - (px1 - px2) * (ay - by);

	float t = a/b;

	ax = ax - (t * (bx - ax));
	ay = ay - (t * (by - ay));
	return Vector2(ax, ay);
}

void DrawCross(float x, float y, color32 color)
{
	int size = 10;
	mgdl_DrawLine(
		SCREEN_WIDTH/2 + x - size, SCREEN_HEIGHT/2 + y ,
		SCREEN_WIDTH/2 + x + size, SCREEN_HEIGHT/2 + y, color);
	mgdl_DrawLine(
		SCREEN_WIDTH/2 + x, SCREEN_HEIGHT/2 + y - size,
		SCREEN_WIDTH/2 + x, SCREEN_HEIGHT/2 + y + size, color);
}

bool isBehind(Vector2 point)
{
	return Vector2DotProduct(FORWARD_2D, point) < 0.0f;
}

void Init2D_YDown()
{
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    // Y increases down: 2D default
	int screenWidth = mgdl_GetScreenWidth();
	int screenHeight = mgdl_GetScreenHeight();
    glOrtho(0.0, screenWidth, screenHeight,0.0, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// NOTE: This is from the OpenGL red book. The purpose is to have the vertices
	// in the middle of the screen pixels
	glTranslatef(0.375f, 0.375f, 0.0f);
}

void DrawGizmo()
{
	float coordSize = 16;
	// Coordinates!
	mgdl_DrawLineGradient(16, 16, 16 + FORWARD_2D.x * coordSize, 16 + FORWARD_2D.y * coordSize, Debug_Green, Debug_White);
	mgdl_DrawLineGradient(16, 16, 16 + RIGHT_2D.x * coordSize, 16 + RIGHT_2D.y * coordSize, Debug_Red, Debug_White);

	Vector2 positiveAngle = Vector2Rotate(FORWARD_2D, mgdl_GetElapsedSeconds());
	mgdl_DrawLineGradient(16, 16, 16 + positiveAngle.x * coordSize, 16 + positiveAngle.y * coordSize, Debug_Red, Debug_White);
}

void StartFrame()
{
	for (int i = 0; i < SCREEN_WIDTH; i++)
	{
		TOP_LIMITS[i] = 0;
		BOTTOM_LIMITS[i] = SCREEN_HEIGHT;
	}
}

void MovePlayer(float deltatime)
{
	float forward = 0;
	float strafe = 0.0f;
	float turn = 0.0f;
	float vertical = 0.0f;
	if (mgdl_IsButtonDown(0, ButtonUp))
	{
		vertical = 1.0f;
	}
	else if (mgdl_IsButtonDown(0, ButtonDown))
	{
		vertical = -1.0f;
	}
	else
	{
		vertical = 0.0f;
	}

	if (mgdl_IsButtonDown(0, ButtonLeft))
	{
		turn = -1.0f;
	}
	else if (mgdl_IsButtonDown(0, ButtonRight))
	{
		turn = 1.0f;
	}
	else
	{
		turn = 0.0f;
	}

	Vector2 wasd = mgdl_GetJoystick(0, Joystick_Nunchuk);
	forward = -wasd.y;
	turn = wasd.x;

	playerAngle += turn * DEG2RAD * 90.0f * deltatime;
	playerDir = Vector2Rotate(FORWARD_2D, playerAngle);

	player = Vector2Add(player, Vector2Scale(playerDir, 100 * forward * deltatime));

}

// NOTE Negative is behind
bool isbehindz(float z)
{
	return z < 0;

}

void RenderBisqwit()
{

	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;
	// One wall in front of player
	Vector2 wall1 = Vector2(70, 20);
	Vector2 wall2 = Vector2(70, 70);

	mgdl_DrawLine(wall1.x, wall1.y, wall2.x, wall2.y, Debug_Yellow);

	Vector2 Bisqwitend = Vector2Add(player, Vector2((cos(playerAngle)*5), sin(playerAngle)*5));
	mgdl_DrawLine(player.x, player.y,  Bisqwitend.x, Bisqwitend.y, Debug_Red);

	glPushMatrix();
		glTranslatef(screen_half_width, 50, 0);

		// Transform wall points
		Vector2 wall1trans = Vector2Subtract(wall1, player);
		Vector2 wall2trans = Vector2Subtract(wall2, player);


		// Bisqwit code
		float btw1y = wall1trans.x * cos(playerAngle) + wall1trans.y * sin(playerAngle);
		float btw2y = wall2trans.x * cos(playerAngle) + wall2trans.y * sin(playerAngle);
		float btw1x = wall1trans.x * sin(playerAngle) - wall1trans.y * cos(playerAngle);
		float btw2x = wall2trans.x * sin(playerAngle) - wall2trans.y * cos(playerAngle);

		mgdl_DrawLine(-btw1x, -btw1y,
					  -btw2x, -btw2y, Debug_Red);

		mgdl_DrawLine(0,0,  0, -5, Debug_Red);
		mgdl_DrawLine(0,0,  2, 2, Debug_White);

	glPopMatrix();
// FIRST PERSON VIEW
	// Camera is at 0,0
	// ////////////////
	glPushMatrix();
	glTranslatef(screen_half_width * 1.5f, 50,0);

		// fov effect
		float zoom = 16.0f;



	// Clip
	// if at least one wall is in front of player
	if (btw1y > 0 || btw2y > 0)
	{
		Vector2 playerNearLeft = Vector2(-0.001f, 0.001f);
		Vector2 playerNearRight = Vector2(0.001f, 0.001f);

		Vector2 playerFarLeft = Vector2(-20.001f, 5.001f);
		Vector2 playerFarRight = Vector2(20.001f, 5.001f);
		Vector2 btw1 = Vector2(btw1x, btw1y);
		Vector2 btw2 = Vector2(btw2x, btw2y);
		Vector2 clip1 = WikiIntersect(btw1, btw2, playerNearLeft, playerFarLeft);
		Vector2 clip2 = WikiIntersect(btw1, btw2, playerNearRight, playerFarRight);

		// The trick here is that the Intersect function treats both lines as
		// infinitely long
		// If the left wall point is behind and left clipping point is in front of the player
		// then left point was clipped ok. If the clip was behind, it is moved to the right clipping
		// point, which is on the right side of player. This invalidates the wall in next step
		if ( btw1y <= 0)
		{
			if (clip1.y > 0)
			{
				btw1 = clip1;
			}
			else
			{
				btw1 = clip2; // INVALID Result on purpose
			}
		}
		// If the right wall point is behind and left clip is in front ???

		//  I dont understand this one
		if (btw2y <= 0)
		{
			if (clip1.y > 0)
			{
				btw2 = clip1;
			}
			else
			{
				btw2 = clip2;
			}
		}

		// Take new values
		btw1x = btw1.x;
		btw1y = btw1.y;
		btw2x = btw2.x;
		btw2y = btw2.y;

		float screen1x = -btw1x * zoom / btw1y;
		float screen1yb = 50/ btw1y; // Bottom
		float screen1yt = -50/ btw1y; // Top

		float screen2x = -btw2x * zoom/ btw2y;
		float screen2yb = 50/ btw2y; // Bottom
		float screen2yt = -50/ btw2y; // Top

		mgdl_DrawLine(screen1x, screen1yt, screen2x, screen2yt, Debug_Red); // Top
		mgdl_DrawLine(screen1x, screen1yb, screen2x, screen2yb, Debug_Red); // Bottom
		mgdl_DrawLine(screen1x, screen1yt, screen1x, screen1yb, Debug_Yellow); // left
		mgdl_DrawLine(screen2x, screen2yt, screen2x, screen2yb, Debug_Red); // right
	}
	glPopMatrix();
}

void RenderMuffin()
{
	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;

	// One wall in front of player
	Vector2 wall1 = Vector2(70, 20);
	Vector2 wall2 = Vector2(70, 70);

	// TOP DOWN
	//////////////

	mgdl_DrawLine(wall1.x, wall1.y, wall2.x, wall2.y, Debug_Yellow);
	Vector2 pend = Vector2Add(player, Vector2Scale(playerDir, 5.0f));
	mgdl_DrawLine(player.x, player.y,  pend.x, pend.y, Debug_Green);
	mgdl_DrawLine(player.x, player.y,  player.x+1, player.y+1, Debug_White);

	// TOP DOWN ROTATED
	// Player is always at (0,0)
	//////////////

	glPushMatrix();
		glTranslatef(screen_half_width, 50, 0);

		// Transform wall points
		Vector2 wall1trans = Vector2Subtract(wall1, player);
		Vector2 wall2trans = Vector2Subtract(wall2, player);

		// My code
		Vector2 trans1 = Vector2Rotate(wall1trans, -playerAngle); // NOTE Around player means to opposite direction
		Vector2 trans2 = Vector2Rotate(wall2trans, -playerAngle); // NOTE Around player means to opposite direction

		mgdl_DrawLine(trans1.x, trans1.y,
					  trans2.x, trans2.y, Debug_Yellow);



		Vector2 pend2 = Vector2Scale(FORWARD_2D, 5.0f);
		mgdl_DrawLine(0,0,  pend2.x, pend2.y, Debug_Green);
		mgdl_DrawLine(0,0,  2, 2, Debug_White);



	glPopMatrix();

	// PERSPECTIVE

	glPushMatrix();
	glTranslatef(screen_half_width * 1.5f, 50,0);

		// fov effect
		float zoom = 16.0f;

	// My code
	bool front1 = Vector2DotProduct(trans1, FORWARD_2D) > 0;
	bool front2 = Vector2DotProduct(trans2, FORWARD_2D) > 0;
	if (front1 || front2)
	{
		Vector2 pnl = Vector2(0.001f, -0.001f);
		Vector2 pnr = Vector2(0.001f, 0.001f);

		Vector2 pfl = Vector2(5.0f, -20.001f);
		Vector2 pfr = Vector2(5.0f, 20.001f);
		Vector2 clip1 = WikiIntersect(trans1, trans2, pnl, pfl);
		Vector2 clip2 = WikiIntersect(trans1, trans2, pnr, pfr);

		if ( !front1)
		{
			if (Vector2DotProduct(clip1, FORWARD_2D) > 0)
			{
				trans1 = clip1;
			}
			else
			{
				trans1 = clip2; // INVALID Result on purpose
			}
		}
		// If the right wall point is behind and left clip is in front ???

		//  I dont understand this one
		if (!front2)
		{
			if (Vector2DotProduct(clip1, FORWARD_2D) > 0)
			{
				trans2 = clip1;
			}
			else
			{
				trans2 = clip2;
			}
		}
		float z1 =  Vector2DotProduct(trans1, FORWARD_2D);
		float z2 =  Vector2DotProduct(trans2, FORWARD_2D);
		float x1 =  Vector2DotProduct(trans1, RIGHT_2D);
		float x2 =  Vector2DotProduct(trans2, RIGHT_2D);

		float screen1x = x1 * zoom / z1;
		float screen1yb = 50/ z1; // Bottom
		float screen1yt = -50/ z1; // Top

		float screen2x = x2 * zoom/ z2;
		float screen2yb = 50/ z2; // Bottom
		float screen2yt = -50/ z2; // Top

		mgdl_DrawLine(screen1x, screen1yt, screen2x, screen2yt, Debug_Blue); // Top
		mgdl_DrawLine(screen1x, screen1yb, screen2x, screen2yb, Debug_Blue); // Bottom
		mgdl_DrawLine(screen1x, screen1yt, screen1x, screen1yb, Debug_White); // left
		mgdl_DrawLine(screen2x, screen2yt, screen2x, screen2yb, Debug_Blue); // right
	}

	glPopMatrix();
}

void RenderMap(float deltatime)
{

	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;

	glClearColor(0.3f, 0.2f, 0.3f, 1.0f);
	Init2D_YDown();
	DrawGizmo();
	StartFrame();

	MovePlayer(deltatime);

	RenderBisqwit();

	glPushMatrix();
	glTranslatef(0, 100, 0);
	RenderMuffin();
	glPopMatrix();






	mgdl_InitOrthoProjection();
	//mgdl_DrawTextInt("Player x")
	//mgdl_DrawTextInt("Player x")
}

void OLD()
{




	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;
	float fov = 300;

	Actor@ player = buns_GetActor(0);
	buns_Vec3 outPlayerPos;
	buns_GetActorPositionV3(0, outPlayerPos);
	Vector2 playerPos = Vector2(-1000, 100);
	float playerElevation = outPlayerPos.y;
	buns_Vec2 outPlayerDir;
	buns_GetActorFloorDir(0, outPlayerDir);
	Vector2 playerDir = Vector2(outPlayerDir.x, outPlayerDir.y);
	float playerAngle = mgdl_GetElapsedSeconds()/ 2.0f;
	playerDir = Vector2(1,0);

	/*
	Vector2 playerViewLeftStart = Vector2Scale(Vector2Rotate( playerDir, DEG2RAD * -45.0f), 0.01f);
	Vector2 playerViewLeftEnd = Vector2Scale(Vector2Rotate( playerDir, DEG2RAD * -45.0f), SCREEN_WIDTH);

	Vector2 playerViewRightStart = Vector2Scale(Vector2Rotate( playerDir, DEG2RAD * 45.0f), 0.01f);
	Vector2 playerViewRightEnd = Vector2Scale(Vector2Rotate( playerDir, DEG2RAD * 45.0f), SCREEN_WIDTH);
	*/

	Vector2 playerViewLeftStart = Vector2Scale(Vector2Rotate( playerDir, DEG2RAD * -45.0f), 0.01f);
	Vector2 playerViewLeftEnd = Vector2Scale(Vector2Rotate( playerDir, DEG2RAD * -45.0f), SCREEN_WIDTH);

	Vector2 playerViewRightStart = Vector2Scale(Vector2Rotate( playerDir, DEG2RAD * 45.0f), 0.01f);
	Vector2 playerViewRightEnd = Vector2Scale(Vector2Rotate( playerDir, DEG2RAD * 45.0f), SCREEN_WIDTH);


	s16 sectorAmount = buns_GetSectorAmount();
	for (s16 i = 0; i < sectorAmount; i++)
	{
		Sector@ sector = buns_GetSector(i);
		float sectorHeight = sector.ceilingy - sector.floory;

		for (s16 wallIndex = 0; wallIndex < sector.wallnum; wallIndex++)
		{
			if (wallIndex >= WALL_LIMIT && WALL_LIMIT > 0)
			{
				break;
			}
			Wall@ start = buns_GetWall(sector.wallptr + wallIndex);
			Wall@ end = buns_GetWallEnd(start);

			// Center to player


			Vector2 wallLeft = Vector2(start.x - playerPos.x, start.z - playerPos.y);
			Vector2 wallRight = Vector2( end.x - playerPos.x, end.z - playerPos.y);

			// DEBUG
			mgdl_DrawLine(
				0,
				0,
				 playerDir.x * 5,
				 playerDir.y * 5,
				Debug_Yellow);

			// Draw the wall
			/*
			mgdl_DrawLineGradient(
				 screen_half_width + wallLeft.x/DEBUG_SCALE,
				 screen_half_height + wallLeft.y/DEBUG_SCALE,
				 screen_half_width + wallRight.x/DEBUG_SCALE,
				 screen_half_height + wallRight.y/DEBUG_SCALE,
				Debug_Blue, Debug_White);
				*/

			// Rotate wall points around player
			Vector2 rotatedLeft = Vector2Rotate(wallLeft, playerAngle);
			Vector2 rotatedRight = Vector2Rotate(wallRight, playerAngle);

			// Wall point is either
			// Behind
			// Outside view : left or right
			// Inside view

			// player view
			mgdl_DrawLine(screen_half_width, screen_half_height,
			screen_half_width + playerViewLeftEnd.x,
			screen_half_height + playerViewLeftEnd.y,
			Debug_DarkGray);

			mgdl_DrawLine(screen_half_width, screen_half_height,
			screen_half_width + playerViewRightEnd.x,
			screen_half_height + playerViewRightEnd.y,
			Debug_White);
			// ////

			int dirLeft = PlayerSeesPoint(rotatedLeft);
			int dirRight = PlayerSeesPoint(rotatedRight);
			// Is whole wall behind
			if (isBehind(rotatedLeft) && isBehind(rotatedRight))
			{
				continue;
			}
			else
			{
				// Both on the left or both on the right : wall cannot be seen
				if ((dirLeft == SEE_LEFT && dirRight == SEE_LEFT) || (dirLeft == SEE_RIGHT && dirRight == SEE_RIGHT))
				{
					continue;
				}
				if (dirLeft == SEE_FRONT && dirRight == SEE_FRONT)
				{
					// Both in front
				}
				else
				{
					// Do both clips
					Vector2 leftClip = WikiIntersect(rotatedLeft, rotatedRight, playerViewLeftStart, playerViewLeftEnd);
					Vector2 rightClip = WikiIntersect(rotatedLeft, rotatedRight, playerViewRightStart, playerViewRightEnd);

					if (!isBehind(leftClip))
					{
						DrawCross(leftClip.x/DEBUG_SCALE, leftClip.y/DEBUG_SCALE, Debug_Red);
					}
					if (!isBehind(rightClip))
					{
						DrawCross(rightClip.x/DEBUG_SCALE, rightClip.y/DEBUG_SCALE, Debug_Green);
					}
					// Right is visible or to the right, left needs to be clipped
					if (dirLeft == SEE_LEFT)
					{
						//rotatedLeft = leftClip;
					}
					// Left is visible or to the left, right needs to be clipped
					if (dirRight == SEE_RIGHT)
					{
						rotatedRight = rightClip;
					}

					// After clipping the points can end up behind player
					if (isBehind(rotatedLeft) && isBehind(rotatedRight))
					{
						continue;
					}
					// Bisqwit : checks if wall has turned around after clipping
				}
			}
			if (dirLeft == SEE_FRONT)
			{
				DrawCross(rotatedLeft.x/DEBUG_SCALE, rotatedLeft.y/DEBUG_SCALE, Debug_Yellow);
			}
			else if (dirLeft == SEE_LEFT)
			{
				DrawCross(rotatedLeft.x/DEBUG_SCALE, rotatedLeft.y/DEBUG_SCALE, Debug_Red);
			}
			if (dirRight == SEE_FRONT)
			{
				DrawCross(rotatedLeft.x/DEBUG_SCALE, rotatedLeft.y/DEBUG_SCALE, Debug_Blue);
			}
			else if (dirRight == SEE_RIGHT)
			{
				DrawCross(rotatedLeft.x/DEBUG_SCALE, rotatedLeft.y/DEBUG_SCALE, Debug_Green);
			}

				//DrawCross(rotatedLeft.x /DEBUG_SCALE, rotatedLeft.y/DEBUG_SCALE, Debug_Red);
				 //DrawCross(rotatedRight.x /DEBUG_SCALE, rotatedRight.y/DEBUG_SCALE, Debug_Red);
			// DEBUG

			// Draw wall
			mgdl_DrawLineGradient(
				screen_half_width + rotatedLeft.x/DEBUG_SCALE,
				 screen_half_height + rotatedLeft.y/DEBUG_SCALE,
				screen_half_width + rotatedRight.x/DEBUG_SCALE,
				 screen_half_height + rotatedRight.y/DEBUG_SCALE,
				 Debug_DarkGray,
				 Debug_White);
			/*

			mgdl_DrawLine(screen_half_width, screen_half_height,
			screen_half_width + FORWARD_2D.x * 5, screen_half_height + FORWARD_2D.y * 5, Debug_Green);

			// Z is distance from player
			float leftZ = rotatedLeft.y;
			float rightZ = rotatedRight.y;

			// Clip to camera frustum

			// Wall height
			float leftHeight = (sectorHeight / leftZ) * fov;
			float rightHeight = (sectorHeight / rightZ) * fov;

			// Convert to screen space
			float leftScreenX = (rotatedLeft.x / leftZ) * fov;
			float leftScreenY = (SCREEN_HEIGHT + playerElevation ) / leftZ;
			float rightScreenX = (rotatedRight.x / rightZ) * fov;
			float rightScreenY = (SCREEN_HEIGHT + playerElevation ) / rightZ;

			// Wall bottom
			float leftLevel = (sector.floory / leftZ) * fov;
			float rightLevel = (sector.floory / rightZ) * fov;
			leftScreenY -= leftLevel;
			rightScreenY -= rightLevel;

			glPushMatrix();
			glTranslatef(screen_half_width, screen_half_height, 0.0f);

			// Top of wall
			mgdl_DrawLine(leftScreenX, leftScreenY + leftHeight,
						  rightScreenX, rightScreenY + rightHeight,
						  Debug_Red);

			// Bottom of wall
			mgdl_DrawLine(leftScreenX, leftScreenY,
						  rightScreenX, rightScreenY,
						  Debug_Red);

			// Left
			mgdl_DrawLine(leftScreenX, leftScreenY + leftHeight,
						  leftScreenX, leftScreenY,
						  Debug_Red);
			// Right
			mgdl_DrawLine(rightScreenX, rightScreenY + rightHeight,
						  rightScreenX, rightScreenY,
						  Debug_Red);


			glPopMatrix();
			*/


		}
	}

}
