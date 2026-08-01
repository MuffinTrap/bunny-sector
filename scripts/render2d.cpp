
// Debug settings

bool DEBUG_PRINT = false;
bool PRINT = true;
int WALL_LIMIT = -1;
int REQUEST_LIMIT = -1;

// File variables

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

Vector2 FORWARD_2D = Vector2(1, 0);
Vector2 RIGHT_2D = Vector2Rotate(FORWARD_2D, DEG2RAD * 90);

// Arrays for storing top and bottom limits
int[] TOP_LIMITS(SCREEN_WIDTH);
int[] BOTTOM_LIMITS(SCREEN_WIDTH);

// Fov
float HFOVRAD = DEG2RAD * 80.0f;
float VFOVRAD = DEG2RAD * 54.0f;
Vector2 ViewPortSize = Vector2New(1,1); // Viewport in world units
Vector2 CanvasSize = Vector2New(SCREEN_WIDTH, SCREEN_HEIGHT); // Canvas is where we draw
Vector2 CameraToCanvasConvert = Vector2New(1,1); // Transforms from Camera to Canvas. Divide by Z!

// Player view cone calculated from HFOVRAD
Vector2 frustumOrigo;
Vector2 frustumLeft;
Vector2 frustumRight;

// Near and far plane
float NEARZ = 0.0001f;
float FARZ = 10 * 1024.0f;

void UpdateFrustumToFov()
{
	frustumOrigo	= Vector2(NEARZ, 0.0f);
	frustumLeft	= Vector2Scale(Vector2Rotate(FORWARD_2D, -HFOVRAD/2.0f), 600.0f);
	frustumRight	= Vector2Scale(Vector2Rotate(FORWARD_2D, HFOVRAD/2.0f), 600.0f);
}

Vector2 ViewportToCanvas(Vector2 point)
{
	return Vector2New(
		point.x * CanvasSize.x/ViewPortSize.x,
		point.y * CanvasSize.y/ViewPortSize.y);
}

Vector2 CameraToViewport(Vector2 point, float z)
{
	return Vector2New( 
		(point.x*NEARZ)/z,
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
int text_x = -SCREEN_WIDTH/2 + 8;
void ResetTextY()
{
	text_y = -SCREEN_HEIGHT/2 + 8;
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
	int picnum;
	
	// ytarget:
	// 0: Full wall. 
	// +: Goes up from floor to value. 
	// -:Goes down from ceiling to value
	s32 ytarget; 
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
int REQUEST_AMOUNT = 64;
// Ring buffer of requests
SectorRequest[] requests(REQUEST_AMOUNT);
int requestFill = 0; // How many unread

// Read and write indices
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

int SEE_LEFT = -1;
int SEE_RIGHT = 1;
int SEE_FRONT = 0;

int PlayerSeesPoint(Vector2 point)
{
	float dotF = Vector2DotProduct(Vector2Normalize(point), FORWARD_2D);
	if (dotF > 0) {
		float angle = cos(dotF);
		if (RAD2DEG * angle < HFOVRAD/2.0f) {
			return SEE_FRONT;
		}
	}
	float dotR = Vector2DotProduct(Vector2Normalize(point), RIGHT_2D);
	if (dotR > 0)	{
		return SEE_RIGHT;
	}
	else {
		return SEE_LEFT;
	}
}

bool Overlap(float a0,float a1,float b0,float b1)
{
	return(fminf(a0,a1) <= fmaxf(b0,b1) && fminf(b0,b1) <= fmaxf(a0,a1));

}
bool IntersectBoxV(Vector2 a1, Vector2 a2, Vector2 b1, Vector2 b2)
{
	return IntersectBox(a1.x, a1.y, a2.x, a2.y, b1.x, b1.y, b2.x, b2.y);
}

bool IntersectBox(float x0,float y0, float x1,float y1, float x2,float y2, float x3,float y3)
{
	return (Overlap(x0,x1,x2,x3) && Overlap(y0,y1,y2,y3));
}

bool IsValidIntersect(Vector2 p)
{
	return (p.x > -1000000 && p.y < 1000000);
}

bool PointIntersect(
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    float x4,
    float y4,
    Vector2 &out pointOUT
     )
{
	float divider = ((x1-x2)*(y3-y4) - (y1-y2)*(x3-x4));
	if (divider != 0.0f)
	{
		float t = ((x1-x3)*(y3-y4) - (y1-y3)*(x3-x4)) / divider;
		float u = ((x1-x2)*(y1-y3) - (y1-y2)*(x1-x3)) / divider;
		if (( 0 <= t && t <= 1.0f ) && (-1.0f <= u && u <= 0.0f))
		{
			pointOUT = Vector2New(x1 + t*(x2-x1), y1 + t*(y2-y1));
			return true;
		}
		else
		{
			pointOUT = Vector2New(t, u);
		}
	}
	else
	{
	}
    /*
    {
            *pointOUT = Vector2New(x3 + t*(x4-x3), y3 + t*(y4-y3));
            return true;
    }
    */
    // DEBUG write t and u instead
    return false;
}

float GetDistanceToWall(Vector2 point, Vector2 ws, Vector2 we)
{
	float top = fabsf( (we.y-ws.y)*point.x - (we.x-ws.x)*point.y + we.x*ws.y - we.y*ws.x);
	float bot = sqrt( (we.y-ws.y)*(we.y-ws.y) + (we.x-ws.x)*(we.x-ws.x));
	if (bot != 0.0f)
	{
		return top/bot;
	}
	else
	{
		return 0.0f;
	}
}

bool Map_IsPointInsideWall(Vector2 point, Vector2 ws, Vector2 we)
{
    // negative if on the right side of wall.
    // walls go clockwise
    Vector2 wallVector = Vector2Subtract(we, ws);
    float crossY = Vector2CrossProduct(wallVector, Vector2Subtract(point, ws));
    // DANGER Again, this code works differently TM
    return crossY > 0.0f;
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
		// Return very invalid point
		 return Vector2New(-1000001, 1000001);
	}
	float x = Vector2CrossProduct(
			Vector2(AxA, line1.x),
			Vector2(BxB, line2.x)
		) / det;
	float y = Vector2CrossProduct(
			Vector2(AxA, line1.y),
			Vector2(BxB, line2.y)
		) / det;

	return Vector2(x,y);
}

// -1 is left
// 1 is right
Vector2 ClipLineToFrustum(Vector2 start, Vector2 end, int side)
{
	if (side<0) {
		return Intersect(start, end, frustumOrigo, frustumLeft);
	} else if (side>0) {
		return Intersect(start, end, frustumOrigo, frustumRight);
	}
	else {
		return Vector2New(-1000001, 1000001);
	}
}

void DrawCross(Vector2 point, color32 color)
{
	int size = 10;
	mgdl_DrawLine(
		point.x - size, point.y ,
		point.x + size, point.y, color);
	mgdl_DrawLine(
		point.x, point.y - size,
		point.x, point.y + size, color);
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


void LogReq()
{
	mgdl_LogTextInt("  Read:", requestRead);
	mgdl_LogTextInt("  Write:", requestWrite);
	mgdl_LogTextInt("  Fill:", requestFill);
}

void FillWall(float ax, float ayt, float ayb, float az,
	float bx, float byt, float byb, float bz,
	int limitLeft, int limitRight,
	color32 color)
{
	if (bx == ax) {
		return;
	}
	float xchange = (bx-ax); // Whole x change, not clipped
	float xmax = fmaxf(ax, limitLeft);
	float xoutside = xmax-ax;
	
	// Clamp start and end x/C
	ax=xmax;
	bx = fminf(bx, limitRight);

	// How much y and z change on every x step
	float ytopchange = (byt-ayt)/xchange;
	float ybotchange = (byb-ayb)/xchange;
	float zchange = (bz-az)/xchange;

	// If part of wall is not on screen, need to advance y and z
	// accordingly
	if (xoutside > 0) // Wall starts outside screen
	{
		ayt += ytopchange * xoutside;
		ayb += ybotchange * xoutside;
		az  += zchange    * xoutside;
	}
	int sx = int(floor(ax));
	int ex = int(floor(bx));
	mgdl_glColor32(color);
	glBegin(GL_LINES);
	for (int x = sx; x < ex; x++)
	{
		if (az < ZBuffer[zBufferIndexOffset+x])
		{
			float dist = 1.0f - az/1024.0f;
			//glColor3f(dist, dist, dist);
			glVertex2f(x, ayt);
			glVertex2f(x, ayb);
			//mgdl_DrawLine(x,ayt,x,ayb,color);
			ZBuffer[zBufferIndexOffset+x]=az;
		}
		ayt		+=ytopchange;
		ayb		+=ybotchange;
		az		+=zchange;
	}
	glEnd();
}

void OutlineWall(float ax, float ayt, float ayb, float az,
	float bx, float byt, float byb, float bz,
	int limitLeft, int limitRight,
	color32 color)
{
	int x1 = int( fmaxf(ax, limitLeft));
	int x2 = int( fminf(bx, limitRight));
	float ytop1 = ayt;
	float ytop2 = byt;
	float ybot1 = ayb;
	float ybot2 = byb;
	float z1 = az;
	float z2 = bz;

	// How much y and z change from a to b
	float xchange = (bx-ax); // Whole x change, not clipped
	if (xchange == 0) {
		return;
	}
	float ytopchange = (ytop2-ytop1)/xchange;
	float ybotchange = (ybot2-ybot1)/xchange;
	float zchange = (z2-z1)/xchange;

	// If part of wall is not on screen, need to advance y and z
	// accordingly
	float ytopleft=ytop1;
	float ybotleft=ybot1;
	float zleft=z1;
	if (ax < x1) // Wall starts outside screen
	{
		float xoutside = x1-ax;
		ytopleft += xoutside*ytopchange;
		ybotleft += xoutside*ybotchange;
		zleft += xoutside*zchange;
	}
	// Add the change done during drawing
	float ytopright = ytopleft + (x2-x1)*ytopchange;
	float ybotright = ybotleft + (x2-x1)*ybotchange;

	// Do z check?
	mgdl_DrawLine(x1,ytopleft, x2, ytopright, color);
	mgdl_DrawLine(x1,ybotleft, x2, ybotright, color);
	mgdl_DrawLine(x1,ybotleft, x1, ybotleft, color);
	mgdl_DrawLine(x2,ytopright, x2, ybotright, color);
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

// Clip the wall to view frustum side
bool ClipWall(Vector2 A, Vector2 B, int side, bool drawDebugs, Vector2 &out point)
{
	if (side < 0) {
		// A is behind
		Vector2 clip1 = ClipLineToFrustum(A, B, -1);
		
		if (IsValidIntersect(clip1) == false) {
			if (DEBUG_PRINT) { mgdl_LogText("Invalid A intersect point");}
			return false;
		}
		
		// Clip only if the clipped point is closer to player than the end point B
		float dotclip1 = Vector2DotProduct(clip1, FORWARD_2D);
		float Bclip = Vector2DotProduct(B, FORWARD_2D);
		if (dotclip1 > 0 && dotclip1 < Bclip) {
			if (drawDebugs && PRINT) {
				mgdl_DrawLineV(A, clip1, Debug_Green);
				DrawCross(clip1, Debug_Green);
				mgdl_DrawTextFloat("A to Clip1 x:", clip1.x, text_x,NextY(),8,Debug_Green);
				mgdl_DrawTextFloat("A to Clip1 y:", clip1.y, text_x,NextY(),8,Debug_Green);
			}
			point = clip1;
			return true;
			//} else if (dotclip2 > 0){
			//	point = clip2;
		}
		else if (DEBUG_PRINT) {
			if (dotclip1 < 0) {
				mgdl_LogText("A Clip point behind");
			}
			else
			{
				mgdl_LogText("A Clip point ahead B");
			}
		}
	}
	else if (side > 0) {
		Vector2 clip2 = ClipLineToFrustum(A, B,  1);
		

		if (IsValidIntersect(clip2) == false) {
			if (DEBUG_PRINT) { mgdl_LogText("Invalid B intersect point");}
			return false;
		}
				
		float dotclip2 = Vector2DotProduct(clip2, FORWARD_2D);
		float Aclip = Vector2DotProduct(A, FORWARD_2D);
		if (dotclip2 > 0 && dotclip2 < Aclip) {
			if (drawDebugs && PRINT) {
				mgdl_DrawLineV(B, clip2, Debug_Blue);
				DrawCross(clip2, Debug_Blue);
				mgdl_DrawTextFloat("B to Clip2 x:", clip2.x, text_x,NextY(),8,Debug_Blue);
				mgdl_DrawTextFloat("B to Clip2 y:", clip2.y, text_x,NextY(),8,Debug_Blue);
			}
			point = clip2;
			return true;
			//} else if (dotclip1 > 0) {
			//	trans2 = clip1;
			//}	
		}
		else if (DEBUG_PRINT) {
			if (dotclip2 < 0) {
				mgdl_LogText("B Clip point behind");
			}
			else
			{
				mgdl_LogText("B Clip point ahead B");
			}
		}
	}
	return false;
}




void RenderTopDownList(array<Vector2> wallpoints, int pointAmount, float scale)
{
	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;

glPushMatrix();

	glTranslatef(screen_half_width, screen_half_height, 0);
	
	Actor@ player = BunnySector_GetActor(0);
	float playerAngle = player.yawRad;

	buns_Vec2 outPlayerPos;
	BunnySector_GetActorPositionV2(0, outPlayerPos);
	Vector2 playerPos = Vector2(outPlayerPos.x, outPlayerPos.y);	

	for (int wi = 0; wi < pointAmount; wi++)
	{
		int nexti = (wi + 1) % pointAmount;

		Vector2 wall1 = Vector2Scale(wallpoints[wi], scale);
		Vector2 wall2 = Vector2Scale(wallpoints[nexti], scale);

		// TOP DOWN ROTATED
		// Player is always at (0,0)
		//////////////
		Vector2 trans1 = WorldToCamera(wall1, playerPos, playerAngle);
		Vector2 trans2 = WorldToCamera(wall2, playerPos, playerAngle);
		
			bool front1 = Vector2DotProduct(trans1, FORWARD_2D) > 0;
			bool front2 = Vector2DotProduct(trans2, FORWARD_2D) > 0;
		
			bool behind = false;
			// Is wall completely behind player?
			if (!front1 && !front2)
			{
				behind = true;
			}
			// Clip _if_ needed!
			else if (front1 || front2)
			{
				Vector2 outPoint;
				if (front1==false) {
					bool clipOk = ClipWall(trans1, trans2, -1, true, outPoint);
					if (clipOk) {
						trans1 = outPoint;
					}
					else { behind = true; }
				}
				if (front2==false){
					bool clipOk = ClipWall(trans1, trans2, 1, true, outPoint);
					if (clipOk) {
						trans2 = outPoint;
					}
					else { behind = true; }
				}
			} 
				
				
			color32 wallColor = Debug_Yellow;
			if (behind ) { 
				wallColor = Debug_DarkGray;		
			}	
				
			mgdl_DrawLine(trans1.x, trans1.y,
				trans2.x, trans2.y, wallColor);
	}


	// Player view cone for fov debug
	DrawCross(Vector2Zero(), Debug_Red);
	mgdl_DrawLineV(frustumOrigo, frustumLeft, Debug_Green);
	mgdl_DrawLineV(frustumOrigo, frustumRight, Debug_Blue);

glPopMatrix();
}


void RenderTopDown(float scale)
{
	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;

glPushMatrix();


	Actor@ player = BunnySector_GetActor(0);
	float playerAngle = player.yawRad;

	buns_Vec2 outPlayerPos;
	buns_Vec2 outPlayerDir;
	BunnySector_GetActorPositionV2(0, outPlayerPos);
	BunnySector_GetActorFloorDir(0, outPlayerDir);
	Vector2 playerPos = Vector2New(outPlayerPos.x * scale, outPlayerPos.y * scale);
	Vector2 playerDir = Vector2New(outPlayerDir.x, outPlayerDir.y);
	float playerRadius = BunnySector_GetActorRadius(0);
	
	mgdl_DrawTextFloat("Player x: ", outPlayerPos.x, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player y: ", outPlayerPos.y, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player d: ", RAD2DEG * playerAngle, text_x, NextY(), 8, Debug_Yellow);
	//mgdl_DrawTextFloat("Player fx: ", outPlayerDir.x, 16, NextY(), 8, Debug_Yellow);
	//mgdl_DrawTextFloat("Player fy: ", outPlayerDir.y, 16, NextY(), 8, Debug_Yellow);
	

	glTranslatef(screen_half_width, screen_half_height, 0);
	//glScalef(scale, scale, 1.0f);
	

	s16 sectorAmount = BunnySector_GetSectorAmount();

	for (s16 sectorIndex = 0; sectorIndex < sectorAmount; sectorIndex++)
	{
	
			if (PRINT) {
				mgdl_DrawTextInt("Sector", sectorIndex, text_x, NextY(), 8, Debug_White);
			}
		Sector@ sector = BunnySector_GetSector(sectorIndex);
		for (s16 wallIndex = 0; wallIndex < sector.wallnum; wallIndex++)
		{
			Wall@ start = BunnySector_GetWall(sector.wallptr + wallIndex);
			Wall@ end = BunnySector_GetWallEnd(start);

			Vector2 wall1 = Vector2(start.x * scale, start.z * scale);
			Vector2 wall2 = Vector2(end.x * scale, end.z * scale);

			// TOP DOWN ROTATED
			// Player is always at (0,0)
			//////////////
			Vector2 trans1 = WorldToCamera(wall1, playerPos, playerAngle);
			Vector2 trans2 = WorldToCamera(wall2, playerPos, playerAngle);
			
				
			if (PRINT && wallIndex == 1) {
				mgdl_DrawTextInt("Wall", wallIndex, text_x, NextY(), 8, Debug_White);
				mgdl_DrawTextFloat("A.z", trans1.x, text_x, NextY(), 8, Debug_White);
				mgdl_DrawTextFloat("A.x", trans1.y, text_x, NextY(), 8, Debug_White);
				mgdl_DrawTextFloat("B.z", trans2.x, text_x, NextY(), 8, Debug_White);
				mgdl_DrawTextFloat("B.x", trans2.y, text_x, NextY(), 8, Debug_White);
			}
		
			
			bool front1 = Vector2DotProduct(trans1, FORWARD_2D) > 0;
			bool front2 = Vector2DotProduct(trans2, FORWARD_2D) > 0;
		
			bool behind = false;
			// Is wall completely behind player?
			if (!front1 && !front2)
			{
				behind = true;
			}
			// Clip _if_ needed!
			else if (front1 || front2)
			{
				Vector2 outPoint;
				if (front1==false) {
					bool clipOk = ClipWall(trans1, trans2, -1, true, outPoint);
					if (clipOk) {
						trans1 = outPoint;
					}
				}
				if (front2==false){
					bool clipOk = ClipWall(trans1, trans2, 1, true, outPoint);
					if (clipOk) {
						trans2 = outPoint;
					}
				}
			} 
				
			Vector2 moveEnd = Vector2Scale(FORWARD_2D, 100);
			bool isPortal =start.nextsector >= 0;
				
			color32 wallColor = Debug_White;
			if (behind ) { 
				wallColor = Debug_DarkGray;		
			}	
			else if (isPortal) {
				wallColor = Debug_Red;
				// DEBUG Draw intersection with portals
				Vector2 pointOut;
				if (PointIntersect(0, 0, moveEnd.x, moveEnd.y, trans1.x, trans1.y, trans2.x, trans2.y, pointOut))
				{
					DrawCross(pointOut, Debug_Red);
				}
				mgdl_DrawLineV(Vector2Zero(), moveEnd, Debug_Yellow);
			}

			// Debug Draw if player is inside this wall or not
			bool inside = Map_IsPointInsideWall(Vector2Zero(), trans1, trans2);
			if (inside)
			{
				Vector2 walldir = Vector2Subtract(trans2, trans1);
				Vector2 wallNormal = Vector2Normalize(Vector2Rotate(walldir , DEG2RAD * 90));

				Vector2 wallcenter = Vector2Add(trans1, Vector2Scale(walldir, 0.5));
				mgdl_DrawLineV(wallcenter, Vector2Add(wallcenter, Vector2Scale(wallNormal, 10)), wallColor);
				if (isPortal == false)
				{

					float distance = GetDistanceToWall(Vector2Zero(), trans1, trans2);
					color32 distanceColor = Debug_DarkGray;
					if (distance < playerRadius)
					{
						distanceColor = Debug_White;
					}
					mgdl_DrawLineV(Vector2Zero(), Vector2Scale( Vector2Scale(wallNormal, -1.0f), distance), distanceColor);
				}
			}

			// Debug draw if player move overlaps with wall
			if (IntersectBoxV(Vector2Zero(), moveEnd, trans1, trans2))
			{
				if (wallColor == Debug_Red) {
					wallColor = Debug_Blue;
				}
				else
				{
					 wallColor = Debug_Green;
				}
			}

			mgdl_DrawLine(trans1.x, trans1.y,
				trans2.x, trans2.y, wallColor);
		}
			
	}

	// Player view cone for fov debug
	mgdl_DrawLineV(frustumOrigo, frustumLeft, Debug_Green);
	mgdl_DrawLineV(frustumOrigo, frustumRight, Debug_Blue);

glPopMatrix();
}


void RenderMuffin(float scale)
{
	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;
	// Translate origo to screen center
glPushMatrix();

	glTranslatef(screen_half_width, screen_half_height, 0);
	//glScalef(scale, scale, 1.0f);
	// Set up player stuff
	///////////////////////
	mgdl_DrawLine(-screen_half_width, -screen_half_height,-screen_half_width, screen_half_height, Debug_Blue);
	mgdl_DrawLine(screen_half_width, -screen_half_height,screen_half_width, screen_half_height, Debug_Blue);

	Actor@ player = BunnySector_GetActor(0);

	buns_Vec2 outPlayerPos;
	BunnySector_GetActorPositionV2(0, outPlayerPos);
	float playerY = player.elevation;
	Vector2 playerPos = Vector2(outPlayerPos.x, outPlayerPos.y);

	buns_Vec2 outPlayerDir;
	BunnySector_GetActorFloorDir(0, outPlayerDir);
	Vector2 playerDir = Vector2(outPlayerDir.x, outPlayerDir.y);
	//mgdl_LogTextFloat("Player Y ", playerY);
	float playerAngle = player.yawRad;

	if (DEBUG_PRINT){
		mgdl_LogText("---------Start");
		LogReq();
	}

	// NOTE Failsafe request amount
	int requestCount = 0;
	s16 sectorAmount = BunnySector_GetSectorAmount();

	// Start processing requests
	if (DEBUG_PRINT) { mgdl_LogTextInt("Push: ", player.sectorNumber);}
	PushRequest(player.sectorNumber, -SCREEN_WIDTH/2, SCREEN_WIDTH/2-1);

	while (RequestLeft())
	{
		if (requestCount >= REQUEST_AMOUNT ||
			requestCount > sectorAmount ||
			(REQUEST_LIMIT > 0 && requestCount >= REQUEST_LIMIT)
		) {
			if (DEBUG_PRINT){
				mgdl_LogText("Hit limit");
			}
			break;
		}
		requestCount += 1;

		// Take request
		SectorRequest now = PopRequest();
		if (now.number < 0) { // Invalid request check
			continue;
		}
		Sector@ sector = BunnySector_GetSector(now.number);

		if (DEBUG_PRINT) {
			mgdl_LogTextInt("Pop", now.number);
			LogReq();
		}
		
		if (PRINT) { 
			mgdl_DrawTextInt("Sector: ", now.number, text_x, NextY(), 8, Debug_White);
		}

		// Player elevation affects perceived ceiling and floor y
		s32 ceilingy = (sector.ceilingy - playerY) * scale;
		s32 floory = (sector.floory - playerY) * scale;
				
		// Start drawing sector
		for (s16 wallIndex = 0; wallIndex < sector.wallnum; wallIndex++) {

			if (wallIndex >= WALL_LIMIT && WALL_LIMIT > 0) {
				break;
			}

			if (PRINT) {
				mgdl_DrawTextInt("Wall: ", wallIndex, text_x, NextY(),8, Debug_White);
			}

			Wall@ start = BunnySector_GetWall(sector.wallptr + wallIndex);
			Wall@ end = BunnySector_GetWallEnd(start);

			Vector2 trans1 = WorldToCamera(Vector2New(start.x * scale, start.z * scale), playerPos, playerAngle);
			Vector2 trans2 = WorldToCamera(Vector2New(end.x * scale, end.z * scale), playerPos, playerAngle);
			
			ProcessWall( wallIndex,
				trans1,
				trans2,
				floory, ceilingy,
				start.nextsector,
				now.left, now.right
				);
		}
	}
glPopMatrix();
	if (DEBUG_PRINT)
	{
		mgdl_LogText("------End");
	}
	DrawRequestDebug();
}

void RenderTICMap(Vector2[] wallpoints, int pointAmount)
{
	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;
	// Translate origo to screen center
glPushMatrix();

	glTranslatef(screen_half_width, screen_half_height, 0);
	
	mgdl_DrawTextFloat("Viewport x:", ViewPortSize.x, text_x,NextY(),8,Debug_DarkGray);
	mgdl_DrawTextFloat("Viewport y:", ViewPortSize.y, text_x,NextY(),8,Debug_DarkGray);
	mgdl_DrawTextFloat("Canvas x :", CanvasSize.x, text_x,NextY(),8,Debug_DarkGray);
	mgdl_DrawTextFloat("Canvas y :", CanvasSize.y, text_x,NextY(),8,Debug_DarkGray);
	mgdl_DrawTextFloat("NEAR Z :", NEARZ,text_x,NextY(), 8,Debug_DarkGray);
	// Set up player stuff
	///////////////////////

	Actor@ player = BunnySector_GetActor(0);

	buns_Vec2 outPlayerPos;
	BunnySector_GetActorPositionV2(0, outPlayerPos);
	float playerY = 0;
	Vector2 playerPos = Vector2(outPlayerPos.x, outPlayerPos.y);

	buns_Vec2 outPlayerDir;
	BunnySector_GetActorFloorDir(0, outPlayerDir);
	Vector2 playerDir = Vector2(outPlayerDir.x, outPlayerDir.y);

	float playerAngle = player.yawRad;
	for (int wi = 0; wi < pointAmount; wi++)
	{
		int nexti = (wi + 1) % pointAmount;
		Vector2 trans1 = WorldToCamera(wallpoints[wi], playerPos, playerAngle);
		Vector2 trans2 = WorldToCamera(wallpoints[nexti], playerPos, playerAngle);
		ProcessWall(
			wi, trans1, trans2,
			-100, 100, -1, -SCREEN_WIDTH/2, SCREEN_WIDTH/2-1);
	}
glPopMatrix();

}
		
void ProcessWall(int wallIndex,
	Vector2 trans1, Vector2 trans2,
	s32 floory, s32 ceilingy, 
	s16 nextSector, int limitLeft, int limitRight)
{
	if (DEBUG_PRINT) {
		if (nextSector >= 0)
		{
			mgdl_LogTextInt("Portal wall to ", nextSector);
		}
		else
		{
			mgdl_LogText("Normal wall");
		}

	}
	if (PRINT) {
		mgdl_DrawTextInt("Wall", wallIndex, text_x, NextY(), 8, Debug_Yellow);
		mgdl_DrawTextFloat("A.z", trans1.x, text_x, NextY(), 8, Debug_Yellow);
		mgdl_DrawTextFloat("A.x", trans1.y, text_x, NextY(), 8, Debug_Yellow);
		mgdl_DrawTextFloat("B.z", trans2.x, text_x, NextY(), 8, Debug_Yellow);
		mgdl_DrawTextFloat("B.", trans2.y, text_x, NextY(), 8, Debug_Yellow);
	}

	bool front1 = Vector2DotProduct(trans1, FORWARD_2D) > 0;
	bool front2 = Vector2DotProduct(trans2, FORWARD_2D) > 0;

	// What happens to this wall?
	bool draw = true;
	bool behind = false;
	bool clipped = false;
	bool backface = false;
	bool outsideWindow = false;

	// Is wall completely behind player?
	if (!front1 && !front2)
	{
		behind = true;
		draw = false;
		if (DEBUG_PRINT){mgdl_LogText("X behind");}
	}
	// Clip _if_ needed!
	else if (front1 || front2)
	{
		Vector2 outPoint;
		if (front1==false) {
			//mgdl_DrawText("A behind!",8, NextY(), 8, Debug_Yellow);
			bool clipOk = ClipWall(trans1, trans2, -1, false, outPoint);
			if (clipOk) {
				trans1 = outPoint;
			}
			else {
				clipped = true;
				draw = false;
				if (DEBUG_PRINT){mgdl_LogText("X A clipped");}
			} 
		}
		if (front2==false){
			//mgdl_DrawText("B behind!",8, NextY(), 8, Debug_Yellow);
			bool clipOk = ClipWall(trans1, trans2, 1, false, outPoint);
			if (clipOk) {
				trans2 = outPoint;
			}
			else {
				clipped = true;
				draw = false;
				if (DEBUG_PRINT){mgdl_LogText("X B clipped");}

			} 
		}
		
		if (draw)
		{
			// Normalize to the front of player; Z is forward
			float az =  Vector2DotProduct(trans1, FORWARD_2D);
			float bz =  Vector2DotProduct(trans2, FORWARD_2D);
			float ax =  Vector2DotProduct(trans1, RIGHT_2D);
			float bx =  Vector2DotProduct(trans2, RIGHT_2D);
			// Make sure z is not zero
			//if (az < NEARZ) { az = NEARZ; }
			//if (bz < NEARZ) { bz = NEARZ; }


			// TODO Use convert when this works
			
			Vector2 Atop= CameraToViewport(Vector2New(ax,ceilingy),az);
			Vector2 Abot= CameraToViewport(Vector2New(ax,floory),az);
			
			Vector2 Btop = CameraToViewport(Vector2New(bx,ceilingy),bz);
			Vector2 Bbot = CameraToViewport(Vector2New(bx,floory),bz);
			
			if (PRINT) {
				mgdl_DrawTextFloat("Aview x:", Atop.x, text_x, NextY(), 8, Debug_Yellow);
				mgdl_DrawTextFloat("Aview y:", Atop.y, text_x, NextY(), 8, Debug_Yellow);
				
				mgdl_DrawTextFloat("Bview x:", Btop.x, text_x, NextY(), 8, Debug_Yellow);
				mgdl_DrawTextFloat("Bview y:", Btop.y, text_x, NextY(), 8, Debug_Yellow);
			}
	
			Atop = ViewportToCanvas(Atop);
			Abot = ViewportToCanvas(Abot);
	
			Btop = ViewportToCanvas(Btop);
			Bbot = ViewportToCanvas(Bbot);
	//mgdl_LogTextFloat("Acanvas x", Atop.x);	
	//mgdl_LogTextFloat("Acanvas y", Atop.y);
			if (Atop.x > Btop.x)
			{
				backface = true;
				draw = false;

				if (DEBUG_PRINT){mgdl_LogText("X backface");}
			}
			// Check if wall is inside limits wholly or partially
			// Limits are in canvas units
			else if( Btop.x < limitLeft || Atop.x > limitRight)
			{
				outsideWindow = true;
				draw = false;
				if (DEBUG_PRINT){mgdl_LogText("X outside W");}
			}
			// Draw this wall.
			// Is it a portal?
			bool isportal = nextSector >= 0;
			color32 wallcolor = Debug_DarkGray;
			if (isportal) {
				wallcolor = Debug_Red;
				if (DEBUG_PRINT) {
					mgdl_LogTextInt("Found portal to ", nextSector);
				}
			
				// Limits for the new request 
				float beginx = fmaxf(Atop.x, limitLeft);
				float endx = fminf(Btop.x, limitRight);
				if (endx > beginx && CanPushRequest()) {
					PushRequest(nextSector,  beginx, endx);
					if (DEBUG_PRINT) {
						mgdl_LogTextInt("Pushed", nextSector);
						LogReq();
					}
				}
				else if (DEBUG_PRINT) {
					mgdl_LogTextFloat("Outside limits: beginx", beginx);
					mgdl_LogTextFloat("Outside limits: endx", endx);
				}
			}

			if (draw) {
				if (isportal){
				OutlineWall(
				Atop.x, Atop.y, Abot.y, az,
				Btop.x, Btop.y, Bbot.y, bz,
				limitLeft, limitRight,
				wallcolor);	
				}
				else {
				FillWall(
				Atop.x, Atop.y, Abot.y, az,
				Btop.x, Btop.y, Bbot.y, bz,
				limitLeft, limitRight,
				wallcolor);
				}
			} 			
		} // if one point in front
	} // is behind ?
	// Show rejection reason
	if (PRINT) {
		if (draw) { mgdl_DrawText("Draw!",text_x,NextY(),8, Debug_White);		}
		if (behind ) { mgdl_DrawText("Behind",text_x,NextY(),8,Debug_Red);		}
		if (clipped ) { mgdl_DrawText("Clipped",text_x,NextY(),8,Debug_Red);		}
		if (backface) { mgdl_DrawText("Backface",text_x,NextY(),8,Debug_Red);	}
		if (outsideWindow ) { mgdl_DrawText("Outside W",text_x,NextY(),8,Debug_Red);	}
	}
}

// Public functions
void SetNearZ(float nearz)
{
	if (nearz >= 0.0001f && nearz < FARZ)
	{
		NEARZ = nearz;
		CameraToCanvasConvert = GetConvertXY();
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
		UpdateFrustumToFov();
		CameraToCanvasConvert = GetConvertXY();
	}
}

void StartFrame()
{
	for (int i = 0; i < SCREEN_WIDTH; i++)
	{
		TOP_LIMITS[i] = 0;
		BOTTOM_LIMITS[i] = SCREEN_HEIGHT-1;
	}
	SetVerticalFovDeg(80.0f);
	ClearZBuffer();
	ResetDrawTimes();
	ResetRequests();
	UpdateFrustumToFov();
	ViewPortSize = ViewportSizeFromFovAndNearZ();
	ResetTextY();
}


bool TIC_TEST = false;
	// Smol test map
	Vector2[] wallpoints = {
		Vector2New(-640, -512),
		Vector2New(256, -512),
		Vector2New(256, 512),
		Vector2New(-640, 512),

		Vector2New(256, -512),
		Vector2New(1152, -512),
		Vector2New(1152, 512),
		Vector2New(256, 512),

		Vector2New(1152, 512),
		Vector2New(1152, 1536),
		Vector2New(256, 1536),
		Vector2New(256, 512),

	};

float elapsed = 0.0f;
void IntersectTest(float deltatime)
{
elapsed += deltatime/4.0f;
glPushMatrix();
	glTranslatef(SCREEN_WIDTH/2,SCREEN_HEIGHT/2, 0);
	
	Vector2 A1 = Vector2New(-100,-10);
	Vector2 A2 = Vector2Add(A1, Vector2Rotate( Vector2New(400, 0), -elapsed/2));
	mgdl_DrawLineV(A1, A2, Debug_Yellow);
	
	Vector2 B1 = Vector2New(10, -100);
	Vector2 B2 = Vector2Add(B1, Vector2Rotate( Vector2New(400,0),  elapsed));
	mgdl_DrawLineV(B1, B2, Debug_Red);
	
	Vector2 inter = Intersect(A1, A2, B1, B2);
	mgdl_DrawLineV(A1, inter, Debug_Yellow);
	mgdl_DrawLineV(B1, inter, Debug_Red);
	DrawCross(inter, Debug_White);
glPopMatrix();
}


void RenderMap(float deltatime)
{
	glClearColor(0.3f, 0.2f, 0.3f, 1.0f);
	Init2D_YDown();
	DrawGizmo();

	StartFrame();

	// Draw debug infos
	if (mgdl_IsButtonPressed(0, Button1))
	{
		DEBUG_PRINT = true;
	}
	
	Actor@ playerActor = BunnySector_GetActor(0);
	float playerAngle = playerActor.yawRad;

	buns_Vec2 outPlayerDir;
	BunnySector_GetActorFloorDir(0, outPlayerDir);

	buns_Vec2 outPlayerPos;
	BunnySector_GetActorPositionV2(0, outPlayerPos);
	Vector2 playerPos = Vector2(outPlayerPos.x, outPlayerPos.y);

	mgdl_DrawTextFloat("Player x: ", outPlayerPos.x, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player y: ", outPlayerPos.y, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player d: ", RAD2DEG * playerAngle, text_x, NextY(), 8, Debug_Yellow);
	
	BunnySector_SetActorSpeeds(0, 0.1f, 0.7f);
	if (false)
	{
		IntersectTest(deltatime);
	}
	else if (TIC_TEST) {
		BunnySector_MoveActorFreely(0, deltatime);
	
		Actor@ player = BunnySector_GetActor(0);
		player.noclip = true;
		RenderTICMap(wallpoints, 12);
		RenderTopDownList(wallpoints, 12, 1.0f);
	} else {
		
		BunnySector_UpdateMap(testMapId, deltatime);
		RenderMuffin(1.0f);
		RenderTopDown(1.0f);
	}

	DEBUG_PRINT = false;
}



// OLD CODE ETC.


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

*/

