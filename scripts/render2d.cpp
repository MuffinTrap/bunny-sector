
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

bool DEBUG_PRINT = false;
float DEBUG_SCALE = 6.0f;

Vector2 FORWARD_2D = Vector2(1, 0);
Vector2 RIGHT_2D = Vector2Rotate(FORWARD_2D, DEG2RAD * 90);

int SEE_LEFT = -1;
int SEE_RIGHT = 1;
int SEE_FRONT = 0;

int WALL_LIMIT = -1;
int REQUEST_LIMIT = -1;

// Arrays for storing top and bottom limits
int[] TOP_LIMITS(SCREEN_WIDTH);
int[] BOTTOM_LIMITS(SCREEN_WIDTH);

// Fov
int HFOVRAD = DEG2RAD * 105.0f;
int VFOVRAD = DEG2RAD * 60.0f;
Vector2 ViewPortSize = Vector2New(1,1); // Viewport in world units
Vector2 CanvasSize = Vector2New(SCREEN_WIDTH, SCREEN_HEIGHT); // Canvas is where we draw
Vector2 CameraToCanvasConvert = Vector2New(1,1); // Transforms from Camera to Canvas. Divide by Z!

// Near and far plane
float NEARZ = 0.01f;
float FARZ = 1024.0f * 200; // 1024 dukes is 1 meter

Vector2 ViewportToCanvas(Vector2 point)
{
	return Vector2New(point.x * CanvasSize.x/ViewPortSize.x,
					  point.y * CanvasSize.y/ViewPortSize.y);
}

Vector2 CameraToViewport(Vector2 point, float z)
{
	if (z < NEARZ) {z = NEARZ;}
	return Vector2New( (point.x*NEARZ)/z,
		   (point.y*NEARZ)/z);
}

Vector2 ViewportSizeFromFovAndNearZ()
{
	float viewHeight = tan(VFOVRAD/2.0f)*NEARZ;
	float viewWidth = tan(HFOVRAD/2.0f)*NEARZ;
	return Vector2New(viewHeight*2.0f, viewWidth*2.0f);
}

// Translate and rotate point to camera space
Vector2 WorldToCamera(Vector2 point, Vector2 CameraPosition, float cameraYaw)
{
	return Vector2Rotate( Vector2Subtract(point, CameraPosition), -cameraYaw);

}

Vector2 GetConvertXY()
{
	float aspect=CanvasSize.x/CanvasSize.y;
	HFOVRAD=VFOVRAD*aspect;
	ViewPortSize = ViewportSizeFromFovAndNearZ();
	return Vector2New(
		NEARZ*CanvasSize.x/ViewPortSize.x,
		NEARZ*CanvasSize.y/ViewPortSize.y
	);
}


// Array for storing Z buffer
int[] ZBuffer(SCREEN_WIDTH);
int zBufferIndexOffset = SCREEN_WIDTH/2;

void ClearZBuffer()
{
	for(int i = 0; i < SCREEN_WIDTH; i++)
	{
		ZBuffer[i] = FARZ;
	}
}

// Layouting
int text_y = 8;
void ResetTextY()
{
	text_y = 8;
}
int NextY()
{
	text_y += 8;
	return text_y;
}


// NOTE
// This is the stuff that is eventually fed to the OpenGL renderer

class WallDraw
{
	s16 index;
	int beginx;
	int beginy_top;
	int beginy_bottom;

	int endx;
	int endy_top;
	int endy_bottom;

	int picnum;
}

class RenderingResults
{
	// What sectors were drawn
	s16[] SectorsDrawn(4096);
	s16 SectorsDrawnAmount;
	// What walls were drawn
	WallDraw[] WallsDrawn(256);
	s16 WallsDrawnAmount;
}





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
		if (RAD2DEG * angle < HFOVRAD/2.0f)
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

// NOTE This is wonky!
Vector2 Wonky_Intersect(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4)
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

// NOTE This works!
Vector2 Intersect(Vector2 start1, Vector2 end1, Vector2 start2, Vector2 end2)
{
	float AxA = Vector2CrossProduct(start1, end1);
	float BxB = Vector2CrossProduct(start2, end2);
	Vector2 line1 = Vector2Subtract(start1, end1);
	Vector2 line2 = Vector2Subtract(start2, end2);
	float det = Vector2CrossProduct( line1 , line2);
	if (det == 0.0f)
	{
		 return Vector2Zero();
	}
	float x = Vector2CrossProduct(
			Vector2(AxA, line1.x),
			Vector2(BxB, line2.x)
		) / det;
	float y = Vector2CrossProduct(
			Vector2(AxA, line1.y),
			Vector2(y, line2.y)
		) / det;

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
	float coordSize = 64;
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
		BOTTOM_LIMITS[i] = SCREEN_HEIGHT-1;
	}
	ClearZBuffer();
	ResetDrawTimes();
	ResetRequests();

	ResetTextY();
}

/*
void RenderBisqwit()
{


	Actor@ player = buns_GetActor(0);
	float playerAngle = player.yawRad;
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
*/


void LogReq()
{
	mgdl_LogTextInt("  Read:", requestRead);
	mgdl_LogTextInt("  Write:", requestWrite);
	mgdl_LogTextInt("  Fill:", requestFill);
}

void RenderTopDown()
{
	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;


	glPushMatrix();

		glTranslatef(screen_half_width, screen_half_height, 0);
		glScalef(0.25f, 0.25f, 1.0f);


	Actor@ player = buns_GetActor(0);
	float playerAngle = player.yawRad;

	buns_Vec2 outPlayerPos;
	buns_GetActorPositionV2(0, outPlayerPos);
	Vector2 playerPos = Vector2(outPlayerPos.x, outPlayerPos.y);


	// NOTE These need to match with the FOV
	Vector2 pnl = Vector2(0.001f, -0.001f);
	Vector2 pnr = Vector2(0.001f, 0.001f);
	Vector2 pfl = Vector2Scale(Vector2Rotate(FORWARD_2D, -HFOVRAD/2.0f), 600.0f);
	Vector2 pfr = Vector2Scale(Vector2Rotate(FORWARD_2D, HFOVRAD/2.0f), 600.0f);

	s16 sectorAmount = buns_GetSectorAmount();

	for (s16 sectorIndex = 0; sectorIndex < sectorAmount; sectorIndex++)
	{
		Sector@ sector = buns_GetSector(sectorIndex);
		for (s16 wallIndex = 0; wallIndex < sector.wallnum; wallIndex++)
		{
			Wall@ start = buns_GetWall(sector.wallptr + wallIndex);
			Wall@ end = buns_GetWallEnd(start);

			Vector2 wall1 = Vector2(start.x, start.z);
			Vector2 wall2 = Vector2(end.x, end.z);

			wall1 = Vector2Scale(wall1, 1.f);
			wall2 = Vector2Scale(wall2, 1.f);

			// TOP DOWN ROTATED
			// Player is always at (0,0)
			//////////////

			// Transform wall points
			Vector2 wall1trans = Vector2Subtract(wall1, playerPos);
			Vector2 wall2trans = Vector2Subtract(wall2, playerPos);

			// My code
			Vector2 trans1 = Vector2Rotate(wall1trans, -playerAngle); // NOTE Around player means to opposite direction
			Vector2 trans2 = Vector2Rotate(wall2trans, -playerAngle); // NOTE Around player means to opposite direction

			if (start.nextsector < 0)
			{
				mgdl_DrawLine(trans1.x, trans1.y,
							  trans2.x, trans2.y, Debug_White);
			}
			else
			{
				mgdl_DrawLine(trans1.x, trans1.y,
							  trans2.x, trans2.y, Debug_Red);
			}
		}
	}

	// Player view cone debug
	Vector2 pend2 = Vector2Scale(FORWARD_2D, 5.0f);
	mgdl_DrawLine(0,0,  pend2.x, pend2.y, Debug_Green);
	mgdl_DrawLine(0,0,  2, 2, Debug_White);
	mgdl_DrawLine(0,0, pfl.x, pfl.y, Debug_Green);
	mgdl_DrawLine(0,0, pfr.x, pfr.y, Debug_Blue);

	glPopMatrix();
}

Vector2 FillWall(int ax, int ayt, int ayb, float az,
			  int bx, int byt, int byb, float bz,
			  int limitLeft, int limitRight,
			  color32 color)
{
	int x1 = int( fmaxf(ax, limitLeft));
	int x2 = int( fminf(bx, limitRight));
	int ytop1 = ayt;
	int ytop2 = byt;
	int ybot1 = ayb;
	int ybot2 = byb;
	float z1 = az;
	float z2 = bz;

	// How much y and z change from a to b
	float xchange = (bx-ax); // Whole x change, not clipped
	float ytopchange = (ytop2-ytop1)/xchange;
	float ybotchange = (ybot2-ybot1)/xchange;
	float zchange = (z2-z1)/xchange;

	// If part of wall is not on screen, need to advance y and z
	// accordingly
	float ytop=ytop1;
	float ybot=ybot1;
	float z=z1;
	if (ax < x1) // Wall starts outside screen
	{
		float xoutside = x1-ax;
		ytop += xoutside*ytopchange;
		ybot += xoutside*ybotchange;
		z += xoutside*zchange;
	}
	float ytopleft=ytop;
	float ybotleft=ybot;
	for (int x = x1; x<x2; x++)
	{
		if (z < ZBuffer[zBufferIndexOffset+x])
		{
			mgdl_DrawLine(x,ytop,x,ybot,color);
			ZBuffer[zBufferIndexOffset+x]=z;
		}
		ytop		+=ytopchange;
		ybot		+=ybotchange;
		z		+=zchange;
	}
	mgdl_DrawLine(x1,ytopleft, x2, ytop, Debug_White);
	mgdl_DrawLine(x1,ybotleft, x2, ybot, Debug_White);
	mgdl_DrawLine(x1,ybotleft, x1, ybotleft, Debug_White);
	mgdl_DrawLine(x2,ytop, x2, ybot, Debug_White);
	return Vector2New(x1,x2);
}

void DrawRequestDebug()
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
}

void RenderMuffin()
{
	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;
	// Translate origo to screen center
	glPushMatrix();

	glTranslatef(screen_half_width, screen_half_height, 0);
	// Set up player stuff
	///////////////////////

	Actor@ player = buns_GetActor(0);

	buns_Vec2 outPlayerPos;
	buns_GetActorPositionV2(0, outPlayerPos);
	float playerY = player.elevation;
	Vector2 playerPos = Vector2(outPlayerPos.x, outPlayerPos.y);

	buns_Vec2 outPlayerDir;
	buns_GetActorFloorDir(0, outPlayerDir);
	Vector2 playerDir = Vector2(outPlayerDir.x, outPlayerDir.y);

	float playerAngle = player.yawRad;


	// NOTE These need to match with the FOV

	Vector2 pnl = Vector2(0.001f, -0.001f);
	Vector2 pnr = Vector2(0.001f, 0.001f);
	Vector2 pfl = Vector2Scale(Vector2Rotate(FORWARD_2D, -HFOVRAD/2.0f), 600.0f);
	Vector2 pfr = Vector2Scale(Vector2Rotate(FORWARD_2D, HFOVRAD/2.0f), 600.0f);

	if (DEBUG_PRINT)
	{
		mgdl_LogText("---------Start");
		LogReq();
	}

	// NOTE Failsafe request amount
	int requestCount = 0;
	s16 sectorAmount = buns_GetSectorAmount();

	//
	PushRequest(player.sectorNumber, -SCREEN_WIDTH/2, SCREEN_WIDTH/2-1);

	while (RequestLeft())
	{
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

		// Take request
		SectorRequest now = PopRequest();
		Sector@ sector = buns_GetSector(now.number);

		if (DEBUG_PRINT)
		{
			mgdl_LogTextInt("Pop", now.number);
			LogReq();
		}

		// Start drawing sector
		float sectorHeight = (sector.ceilingy - sector.floory)/4.0f;

		for (s16 wallIndex = 0; wallIndex < sector.wallnum; wallIndex++)
		{
			if (wallIndex >= WALL_LIMIT && WALL_LIMIT > 0)
			{
				break;
			}
			Wall@ start = buns_GetWall(sector.wallptr + wallIndex);
			Wall@ end = buns_GetWallEnd(start);

			Vector2 wall1 = Vector2(start.x, start.z);
			Vector2 wall2 = Vector2(end.x, end.z);

			// Transform wall points
			/*
			 * Vector2 wall1trans = Vector2Subtract(wall1, playerPos);
			 * Vector2 wall2trans = Vector2Subtract(wall2, playerPos);
			 *
			 * Vector2 trans1 = Vector2Rotate(wall1trans, -playerAngle); // NOTE Around player means to opposite direction
			 * Vector2 trans2 = Vector2Rotate(wall2trans, -playerAngle); // NOTE Around player means to opposite direction
			 */

			Vector2 trans1 = WorldToCamera(wall1, playerPos, playerAngle);
			Vector2 trans2 = WorldToCamera(wall2, playerPos, playerAngle);

			mgdl_DrawTextInt("Wall", wallIndex, 8, NextY(), 8, Debug_Yellow);
			mgdl_DrawTextFloat("A.x", trans1.x, 8, NextY(), 8, Debug_Yellow);
			mgdl_DrawTextFloat("B.x", trans2.x, 8, NextY(), 8, Debug_Yellow);

			bool front1 = Vector2DotProduct(trans1, FORWARD_2D) > 0;
			bool front2 = Vector2DotProduct(trans2, FORWARD_2D) > 0;

			bool draw = true;
			bool behind = false;
			bool clipped = false;
			bool backface = false;
			bool outsideWindow = false;

			Vector2 clip1 = Intersect(trans1, trans2, pnl, pfl);
			Vector2 clip2 = Intersect(trans1, trans2, pnr, pfr);

			float dotclip1 = Vector2DotProduct(clip1, FORWARD_2D);
			float dotclip2 = Vector2DotProduct(clip2, FORWARD_2D);


			// Clip if needed
			if (!front1 && !front2)
			{
				behind = true;
				draw = false;
			}
			// Clip if needed
			else if (front1 || front2)
			{
				if (!front1)
				{
					if (dotclip1 > 0)
					{
						mgdl_DrawLineV(trans1,clip1,Debug_Green);
						DrawCross(clip1.x, clip1.y, Debug_Green);
						mgdl_DrawTextFloat("A to Clip1 x", clip1.x, 8,NextY(),8,Debug_Green);
						trans1 = clip1;
					}
					else
					{
						clipped = true;
						draw = false;
					}
				}
				if (!front2)
				{
					// Sometimes it clips to left which is really
					// far away.
					// If both clips are in front, clip to closest
					if (dotclip2 > 0)
					{
						mgdl_DrawLineV(trans2,clip2,Debug_Green);
						mgdl_DrawTextFloat("B to Clip2 x", clip1.x, 8,NextY(),8,Debug_White);
						DrawCross(clip2.x, clip2.y, Debug_Blue);
						trans2 = clip2;
					}
					else
					{
						clipped = true;
						draw = false;
					}
				}
			} // clip test

			if (draw)
			{
				// Normalize to the front of player; Z is forward
				float az =  Vector2DotProduct(trans1, FORWARD_2D);
				float bz =  Vector2DotProduct(trans2, FORWARD_2D);
				float ax =  Vector2DotProduct(trans1, RIGHT_2D);
				float bx =  Vector2DotProduct(trans2, RIGHT_2D);
				// Make sure z is not zero
				if (az < NEARZ) { az = NEARZ;}
				if (bz < NEARZ) { bz = NEARZ;}

				// Draw this wall.
				// Is it a portal?
				bool isportal = start.nextsector >= 0;
				color32 wallcolor = Debug_Blue;
				if (isportal)
				{
					wallcolor = Debug_Red;
					if (DEBUG_PRINT)
					{
						mgdl_LogTextInt("Found portal to ", start.nextsector);
					}
				}

				// Player elevation affects ceiling and floor y
				s32 ceilingy = (sector.ceilingy - playerY)/2.0f;
				s32 floory = (sector.floory - playerY)/2.0f;

				// TODO Use convert when this works
				Vector2 Atop = CameraToViewport(Vector2New(ax,ceilingy),az);
				Vector2 Abot = CameraToViewport(Vector2New(ax,floory),az);

				Vector2 Btop = CameraToViewport(Vector2New(bx,ceilingy),bz);
				Vector2 Bbot = CameraToViewport(Vector2New(bx,floory),bz);

				Atop = ViewportToCanvas(Atop);
				Abot = ViewportToCanvas(Abot);

				Btop = ViewportToCanvas(Btop);
				Bbot = ViewportToCanvas(Bbot);

				if (Atop.x > Btop.x)
				{
					backface = true;
					draw = false;
				}
				// Limits are in canvas units
				if( Btop.x < now.left || Atop.x > now.right)
				{
					outsideWindow = true;
					draw = false;
				}

				if (draw)
				{
					// Need to return the actual draw area
					Vector2 startAndEndX = FillWall(Atop.x, Atop.y, Abot.y, az,
													Btop.x, Btop.y, Bbot.y, bz,
													now.left, now.right,
													wallcolor);

					float beginx = startAndEndX.x;
					float endx = startAndEndX.y;
					if (isportal && endx > beginx && CanPushRequest())
					{
						PushRequest(start.nextsector,  beginx, endx);

						if (DEBUG_PRINT)
						{
							mgdl_LogTextInt("Pushed", start.nextsector);
							LogReq();
						}
					} // is portal
				} // if draw
			} // if draw
		} // walls loop
	} // while request left

	glPopMatrix();
	if (DEBUG_PRINT)
	{
		mgdl_LogText("------End");
	}
	//DrawRequestDebug();
}

// Public functions

void SetNearZ(float nearz)
{
	if (nearz >= 0.0001f && nearz < FARZ)
	{
		NEARZ = nearz;

	}
}
void SetFarZ(float farz)
{
	if (farz > NEARZ)
	{
		FARZ = farz;
	}
}

void SetVerticalFovDeg(float fovDeg)
{
	if (fovDeg >= 5.0f && fovDeg < 90.0f)
	{
		VFOVRAD = DEG2RAD * fovDeg;
		HFOVRAD = VFOVRAD * (CanvasSize.x/CanvasSize.y);
	}
}


void RenderMap(float deltatime)
{
	glClearColor(0.3f, 0.2f, 0.3f, 1.0f);
	Init2D_YDown();
	ClearZBuffer();
	CameraToCanvasConvert = GetConvertXY();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE); //  is this needed?


	DrawGizmo();

	StartFrame();
	RenderTopDown();
	RenderMuffin();
}
