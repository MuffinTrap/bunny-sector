
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

float DEBUG_SCALE = 6.0f;

// Fov
float HFOV_MULTIPLIER = 0.41;
float VFOV_MULTIPLIER = 0.2;
int HFOV = HFOV_MULTIPLIER * SCREEN_HEIGHT;
int VFOV = 0.2f * SCREEN_HEIGHT;

float PLAYER_HALF_FOV_DEG = 45.0f;
Vector2 FORWARD_2D = Vector2(1, 0);
Vector2 RIGHT_2D = Vector2Rotate(FORWARD_2D, DEG2RAD * 90);

int SEE_LEFT = -1;
int SEE_RIGHT = 1;
int SEE_FRONT = 0;

int WALL_LIMIT = -1;
int REQUEST_LIMIT = 1;

//Actor drives
	Vector2 player = Vector2(50,50);
	float playerAngle = 0.0f;
	Vector2 playerDir = Vector2(1, 0);
	float playerY = 0.0f;


// Arrays for storing top and bottom limits
int[] TOP_LIMITS(SCREEN_WIDTH);
int[] BOTTOM_LIMITS(SCREEN_WIDTH);


// Sector draw requests
class SectorRequest
{
	int number;
	int left;
	int right;

	SectorRequest()
	{
		number = -1;
		left = SCREEN_WIDTH-1;
		right = 0;
	}

	SectorRequest(int pnum, int pleft, int pright)
	{
		number = pnum;
		left = pleft;
		right = pright;
	}
}

// List of drawn sectors
int SECTOR_MAX = 4096;
int[] SectorDrawTimes(SECTOR_MAX);

void ResetDrawTimes()
{
	for (int i = 0; i < SECTOR_MAX; i++)
	{
		SectorDrawTimes[i] = 0;
	}
}


// Sector draw requests
int REQUEST_AMOUNT = 8;
SectorRequest[] requests(REQUEST_AMOUNT);
int requestFill = 0; // How many unread

int requestRead = 0;
int requestWrite = 0;

void ResetRequests()
{
	requestRead = 0;
	requestWrite = 0;
	requestFill = 0;
	for (int i = 0; i < REQUEST_AMOUNT; i++)
	{
		requests[i].number = -1; // Make all requests invalid
	}
}

void PushRequest(int sectornumber, int left, int right)
{
	requests[requestWrite] = SectorRequest(sectornumber, left, right);
	requestWrite = (requestWrite + 1) % REQUEST_AMOUNT;
	requestFill += 1;
}

SectorRequest PopRequest()
{
	SectorRequest R = requests[requestRead];
	requestRead = (requestRead + 1) % REQUEST_AMOUNT;
	requestFill -= 1;
	return R;
}

bool RequestLeft()
{
	return requestRead != requestWrite && requestFill > 0;
}

// Can push if write will not go past read
bool CanPushRequest()
{
	return requestFill < REQUEST_AMOUNT;
}


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

// NOTE Not used
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

// NOTE Not used
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
    mgdl_InitOrthoProjection(-1.0f);

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

bool firstFrame = true;

void StartFrame()
{
	for (int i = 0; i < SCREEN_WIDTH; i++)
	{
		TOP_LIMITS[i] = 0;
		BOTTOM_LIMITS[i] = SCREEN_HEIGHT-1;
	}
	ResetDrawTimes();
	ResetRequests();

	// Only on first frame
	if (firstFrame)
	{
		buns_Vec3 outPlayerPos;
		buns_GetActorPositionV3(0, outPlayerPos);
		player= Vector2(outPlayerPos.x, outPlayerPos.z);
		playerY = outPlayerPos.y;
		firstFrame = false;
	}

	// Get player sector
	Actor@ player = buns_GetActor(0);
	// NOTE TODO
	PushRequest(player.sectorNumber, -SCREEN_WIDTH/2, SCREEN_WIDTH/2-1);

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

	playerAngle += turn * DEG2RAD * 25.0f * deltatime;
	playerDir = Vector2Rotate(FORWARD_2D, playerAngle);

	player = Vector2Add(player, Vector2Scale(playerDir, 400 * forward * deltatime));


	HFOV = HFOV_MULTIPLIER * SCREEN_HEIGHT; // TODO FIGURE out how to get this from FOV ANGLE
	VFOV = VFOV_MULTIPLIER * SCREEN_HEIGHT;

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

bool DEBUG_PRINT = false;

void LogReq()
{
	mgdl_LogTextInt("  Read:", requestRead);
	mgdl_LogTextInt("  Write:", requestWrite);
	mgdl_LogTextInt("  Fill:", requestFill);
}

void RenderMuffin()
{
	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;

	float gScale = 0.5f;



	// NOTE These need to match with the FOV
	Vector2 pnl = Vector2(0.001f, -0.001f);
	Vector2 pnr = Vector2(0.001f, 0.001f);

	Vector2 pfl = Vector2Rotate(FORWARD_2D, -PLAYER_HALF_FOV_DEG);
	Vector2 pfr = Vector2Rotate(FORWARD_2D, PLAYER_HALF_FOV_DEG);

	if (DEBUG_PRINT)
	{
		mgdl_LogText("---------Start");
		LogReq();
	}

	// NOTE Failsafe
	int requestCount = 0;
	s16 sectorAmount = buns_GetSectorAmount();
	while (RequestLeft())
	{
		// Debug draw request situation
		float rdx = 16;
		float rdy = 16;
		float rds = 16;
		float rfs = 16;
		color32 rc = Debug_Yellow;

		for(int ri = 0; ri < REQUEST_AMOUNT; ri++)
		{
			SectorRequest R = requests[ri];
			if (requestRead >= ri)
			{
				rc = Debug_Red;
			}
			else
			{
				rc = Debug_White;
			}
			if (R.number >= 0)
			{
				mgdl_DrawInt(R.number, rdx, rdy, rfs, rc);
			}
			else
			{
				mgdl_DrawText("-", rdx, rdy, rfs, rc);
			}
			rdx += rds;
		}
		rdx = 16;
		mgdl_DrawText("R", rdx + rds * requestRead, rdy+rfs, rfs, Debug_Red);
		mgdl_DrawText("W", rdx + rds * requestWrite, rdy+rfs*2, rfs, Debug_White);

		if (requestCount >= REQUEST_AMOUNT ||
			requestCount > sectorAmount ||
			(REQUEST_LIMIT > 0 && requestCount >= REQUEST_LIMIT)
		)
		{
			if (DEBUG_PRINT)
			{
				mgdl_LogText("Hit limit");
			}
			break;
		}
		requestCount += 1;

		SectorRequest now = PopRequest();
		Sector@ sector = buns_GetSector(now.number);

		if (DEBUG_PRINT)
		{
			mgdl_LogTextInt("Pop", now.number);
			LogReq();
		}

		float sectorHeight = sector.ceilingy - sector.floory;

		for (s16 wallIndex = 0; wallIndex < sector.wallnum; wallIndex++)
		{
			Wall@ start = buns_GetWall(sector.wallptr + wallIndex);
			Wall@ end = buns_GetWallEnd(start);

			Vector2 wall1 = Vector2(start.x, start.z);
			Vector2 wall2 = Vector2(end.x, end.z);

			// TOP DOWN
			/*
			//////////////
			glPushMatrix();

			glTranslatef(screen_half_width/2, screen_half_height, 0);
			glScalef(gScale/2, gScale/2, 1);

			mgdl_DrawLine(wall1.x, wall1.y, wall2.x, wall2.y, Debug_DarkGray);
			Vector2 pend = Vector2Add(player, Vector2Scale(playerDir, 5.0f));
			mgdl_DrawLine(player.x, player.y,  pend.x, pend.y, Debug_Green);
			mgdl_DrawLine(player.x, player.y,  player.x+1, player.y+1, Debug_White);

			glPopMatrix();
			*/

			// TOP DOWN ROTATED
			// Player is always at (0,0)
			//////////////

				// Transform wall points
				Vector2 wall1trans = Vector2Subtract(wall1, player);
				Vector2 wall2trans = Vector2Subtract(wall2, player);

				// My code
				Vector2 trans1 = Vector2Rotate(wall1trans, -playerAngle); // NOTE Around player means to opposite direction
				Vector2 trans2 = Vector2Rotate(wall2trans, -playerAngle); // NOTE Around player means to opposite direction

				// Save them for the minimap
				Vector2 mini1 = trans1;
				Vector2 mini2 = trans2;

				bool front1 = Vector2DotProduct(trans1, FORWARD_2D) > 0;
				bool front2 = Vector2DotProduct(trans2, FORWARD_2D) > 0;

			// PERSPECTIVE

			glPushMatrix();

			glTranslatef(screen_half_width, screen_half_height, 0);

			bool behind = false;
			bool clipped = false;

			// My code
			if (front1 || front2)
			{

				// TODO After clipping, wall cannot become longer than it was!

				Vector2 clip1 = WikiIntersect(trans1, trans2, pnl, pfl);
				Vector2 clip2 = WikiIntersect(trans1, trans2, pnr, pfr);

				float dotclip1 = Vector2DotProduct(clip1, FORWARD_2D);
				float dotclip2 = Vector2DotProduct(clip2, FORWARD_2D);

				if ( !front1)
				{
					if (dotclip1 > 0)
					{
						trans1 = clip1;
					}
					else
					{
						trans1 = clip2; // INVALID Result on purpose
					}
					clipped = true;
				}
				// If the right wall point is behind and left clip is in front ???

				//  I dont understand this one
				if (!front2)
				{
					// Sometimes it clips to left which is really
					// far away.
					// If both clips are in front, clip to closest
					if (dotclip1 > 0 && dotclip2 > 0)
					{
						if (dotclip2 < dotclip1)
						{
							trans2 = clip2;
						}
					}
					else if (dotclip1 > 0)
					{
						trans2 = clip1;
					}
					else
					{
						trans2 = clip2;
					}

					clipped = true;
				}
			}
			else
			{
				behind = true;
			}
			float z1 =  Vector2DotProduct(trans1, FORWARD_2D);
			float z2 =  Vector2DotProduct(trans2, FORWARD_2D);
			float x1 =  Vector2DotProduct(trans1, RIGHT_2D);
			float x2 =  Vector2DotProduct(trans2, RIGHT_2D);

			// Perspective transform
			// fov effect
			float zoom1x = HFOV / z1;
			float zoom1y = VFOV / z1;
			float zoom2x = HFOV / z2;
			float zoom2y = VFOV / z2;

			int screen1x = int(x1 * zoom1x);

			int screen2x = int(x2 * zoom2x);


			// Important bit. Check if fits into drawing window
			bool backface = false;
			bool outsideWindow = false;
			if (screen1x >= screen2x)
			{
				backface = true;
			}
			if( screen2x < now.left || screen1x > now.right)
			{
				outsideWindow = true;
			}

			if (!outsideWindow && !backface && !behind)
			{
				// Draw this wall.
				// Is it a portal?
				color32 wallcolor = Debug_Blue;


				bool isportal = start.nextsector >= 0;
				if (isportal)
				{
					wallcolor = Debug_Red;
					if (DEBUG_PRINT)
					{
						mgdl_LogTextInt("Found portal to ", start.nextsector);
					}
				}

				s32 ceilingy = sector.ceilingy - playerY;
				s32 floory = sector.floory - playerY;

				// Wall heights
				float screen1yb = floory * zoom1y; // Bottom
				float screen1yt = ceilingy* zoom1y; // Top
				float screen2yb = floory * zoom2y; // Bottom
				float screen2yt = ceilingy* zoom2y; // Top

				int beginx = int(fmaxf(screen1x, now.left));
				int endx = int(fminf(screen2x, now.right));

				// Linear interpolation
				for (int dx = beginx; dx <= endx; dx++)
				{
					int limitx = screen_half_width+dx;

					int yt = (dx - screen1x) * (screen2yt - screen1yt) / (screen2x - screen1x) + screen1yt;
					int yb = (dx - screen1x) * (screen2yb - screen1yb) / (screen2x - screen1x) + screen1yb;

					float limit_top =yt + screen_half_height;
					float limit_bot = yb + screen_half_height;

					int cyt = Clamp(limit_top, TOP_LIMITS[limitx], BOTTOM_LIMITS[limitx]);
					int cyb = Clamp(limit_bot, TOP_LIMITS[limitx], BOTTOM_LIMITS[limitx]);

					mgdl_DrawLine(dx, cyt- screen_half_height, dx, cyb -screen_half_height, wallcolor);
					mgdl_DrawLine(dx, cyt- screen_half_height, dx, cyb -screen_half_height, wallcolor);
				}

				mgdl_DrawLine(beginx, screen1yt, endx, screen2yt, wallcolor); // Top
				mgdl_DrawLine(beginx, screen1yb, endx, screen2yb, wallcolor); // Bottom
				mgdl_DrawLine(beginx, screen1yt, beginx, screen1yb, Debug_White); // left
				mgdl_DrawLine(endx, screen2yt, screen2x, screen2yb, wallcolor); // right
				if (isportal)
				{

					mgdl_DrawTextInt("Screen1x: ", screen1x, 0, 128, 16, Debug_Yellow);
					mgdl_DrawTextInt("Screen2x: ", screen2x, 0, 128 + 16, 16, Debug_Yellow);
					mgdl_DrawTextInt("Beginx: ", beginx, 0, 128 + 16*2, 16, Debug_Yellow);
					mgdl_DrawTextInt("Endx: ", endx, 0, 128 + 16*3, 16, Debug_Yellow);
				}


				if (isportal && endx > beginx && CanPushRequest())
				{
					PushRequest(start.nextsector,  beginx, endx);

					if (DEBUG_PRINT)
					{
						mgdl_LogTextInt("Pushed", start.nextsector);
						LogReq();
					}
				}
			}

			glPopMatrix();

			// Rejection debug

			glPushMatrix();
			glTranslatef(screen_half_width, screen_half_height, 0);
			glScalef(gScale/2, gScale/2, 1);

			if (!front1 && !front2)
			{
				// Behind
				mgdl_DrawLine(mini1.x, mini1.y,
							  mini2.x, mini2.y, Debug_Red);
			}
			else if (backface)
			{
				mgdl_DrawLine(mini1.x, mini1.y,
							  mini2.x, mini2.y, Debug_Magenta);

			}
			else if (outsideWindow)
			{
				mgdl_DrawLine(mini1.x, mini1.y,
							  mini2.x, mini2.y, Debug_Yellow);

			}
			else
			{
				mgdl_DrawLine(mini1.x, mini1.y,
							  mini2.x, mini2.y, Debug_White);

			}
			if (clipped)
			{
				mgdl_DrawLine(trans1.x, trans1.y,
							  trans2.x, trans2.y, Debug_Green);

			}


			Vector2 pend2 = Vector2Scale(FORWARD_2D, 5.0f);
			mgdl_DrawLine(0,0,  pend2.x, pend2.y, Debug_Green);
			mgdl_DrawLine(0,0,  2, 2, Debug_White);
			mgdl_DrawLine(0,0, pfl.x * 600, pfl.y * 600, Debug_Yellow);
			mgdl_DrawLine(0,0, pfr.x * 600, pfr.y * 600, Debug_Yellow);

			glPopMatrix();
		}
	}
	if (DEBUG_PRINT)
	{
		mgdl_LogText("------End");
	}

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

	//RenderBisqwit();

	glPushMatrix();
	//glTranslatef(0, 100, 0);
	RenderMuffin();
	glPopMatrix();






	mgdl_InitOrthoProjection(1.0f);
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
