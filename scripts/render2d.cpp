
// Debug settings

bool DEBUG_PRINT = false;
bool PRINT = false;
bool RENDER_2D_WALLS = false;
int WALL_LIMIT = -1;
int REQUEST_LIMIT = -1;

// File variables

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = SCREEN_WIDTH/2;

Vector2 FORWARD_2D = Vector2(1, 0);
Vector2 RIGHT_2D = Vector2Rotate(FORWARD_2D, DEG2RAD * 90);

// Arrays for storing top and bottom limits
int[] TOP_LIMITS(SCREEN_WIDTH);
int[] BOTTOM_LIMITS(SCREEN_WIDTH);

// Fov variables
float HFOVRAD = 0;
float VFOVRAD = 0; // to be set

Vector2 ViewPortSize = Vector2New(1,1); // Viewport in world units.
Vector2 CanvasSize = Vector2New(1,1); // The size of the drawing area
Vector2 CameraToCanvasConvert = Vector2New(1,1); // Transforms from Camera to Canvas. Divide by Z!

// Player view cone calculated from HFOVRAD
Vector2 frustumOrigo;
Vector2 frustumLeft;
Vector2 frustumRight;

// Near and far plane
// The distance in build maps are big. 1 meter is about 1024
float NEARZ = 1.0f;
float FARZ = 1000 * 1024.0f;

// This can be changed during gameplay
void SetVerticalFovDeg(float fovDeg)
{
	if (fovDeg >= 5.0f && fovDeg < 180.0f)
	{
		VFOVRAD = DEG2RAD * fovDeg;

		// Horizontal fov depends on vertical fov and aspect ratio
		float aspect = (CanvasSize.x/CanvasSize.y);
		HFOVRAD = VFOVRAD * aspect;
		mgdl_LogTextFloat("VFOVRAD ", VFOVRAD);
		mgdl_LogTextFloat("HFOVRAD ", HFOVRAD);

		UpdateFrustumToFov();
		// Viewport dimensions depend on FOV
		ViewPortSize = ViewportSizeFromFovAndNearZ();
	}
}

// This updates the clipping line segments
void UpdateFrustumToFov()
{
	frustumOrigo	= Vector2(0.0f, 0.0f);
	frustumLeft	= Vector2Scale(Vector2Rotate(FORWARD_2D, -HFOVRAD/2.0f), 600.0f);
	frustumRight	= Vector2Scale(Vector2Rotate(FORWARD_2D, HFOVRAD/2.0f), 600.0f);
}

// These transform the clipped coordinates to canvas for drawing
// The Y is multiplied by -1 because the world coordinates have Y up
// but on the screen the Y goes down
Vector2 ViewportToCanvas(Vector2 point)
{
	return Vector2New(
		point.x * CanvasSize.x/ViewPortSize.x,
		-point.y * CanvasSize.y/ViewPortSize.y);
}

Vector2 CameraToViewport(Vector2 point, float z)
{
	return Vector2New( 
		(point.x)/z,
		(point.y)/z);
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
	float viewWidth = viewHeight * (CanvasSize.x/CanvasSize.y);
	return Vector2New(viewWidth*2.0f, viewHeight*2.0f);
}

// Translate and rotate point to camera space
Vector2 WorldToCamera(Vector2 point, Vector2 CameraPosition, float cameraYaw)
{
	return Vector2Rotate( Vector2Subtract(point, CameraPosition), -cameraYaw);
}
// NOTE: This is to replace the above transforms when everything works
// to make code faster

/*
Vector2 GetConvertXY()
{
	float aspect=CanvasSize.x/CanvasSize.y;
	HFOVRAD=VFOVRAD*aspect;
	return Vector2New(
		CanvasSize.x/ViewPortSize.x,
		CanvasSize.y/ViewPortSize.y
	);
}
*/


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

class DrawRequest
{
	bool isSector; // Is sector, not a wall
	s16 index; // which one
	s16 wallsUnionPicnum; // if wall, the picnum. If sector how many walls following this
	// belong to this sector
	
	// If sector, the ceiling and floor of the sector
	// if wall, the offset to those of sector. If both are 0 it means the whole wall is drawn
	s32 ceilingy;
	s32 floory;

	DrawRequest()
	{
		isSector = false;
		index = -1;
	}

	DrawRequest(bool isSectorP, s16 indexP, s16 wallsUnionPicnumP, s32 ceilingyP, s32 flooryP)
	{
		isSector = isSectorP;
		index = indexP;
		wallsUnionPicnum = wallsUnionPicnumP;
		ceilingy = ceilingyP;
		floory = flooryP;
	}
}

// Collection of DrawRequests
DrawRequest[] drawRequests(128);
int drawRequestCount; // How many draw requests. Reset to 0 every frame start
int lastSectorRequest; // Index of latest sector request

void StartDrawRequestForSector(s16 index, s32 ceilingy, s32 floory)
{
	drawRequests[drawRequestCount] = DrawRequest(true, index, -1, ceilingy, floory);
	lastSectorRequest = drawRequestCount;
}
void PushDrawRequestForWall(s16 index, s16 picnum, s32 ceilingy, s32 floory)
{
	drawRequestCount += 1;
	drawRequests[drawRequestCount] = DrawRequest(false, index, picnum, ceilingy, floory);
}
// Note if no walls are drawn for this sector, the wallsUnionPicnum will be 0
void EndDrawRequestForSector()
{
	drawRequests[lastSectorRequest].wallsUnionPicnum = drawRequestCount - lastSectorRequest;
}

// Sector draw requests
class SectorRequest
{
	s16 number;
	s16 left; // Left limit in canvas units
	s16 right; // Right limit in canvas units

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
// This is used to prevent drawing sector floors and ceilings multiple times
int SECTOR_MAX = 4096;
u8[] SectorDrawTimes(SECTOR_MAX);



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

// Reset all counter related to drawing
void ResetDrawCounters()
{
	ResetRequests();
	for (int i = 0; i < SECTOR_MAX; i++)
	{
		SectorDrawTimes[i] = 0;
	}
	drawRequestCount = 0;
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
	}
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
bool Intersect(Vector2 start1, Vector2 end1, Vector2 start2, Vector2 end2, Vector2 &out intersectOut)
{
	Vector2 line1 = Vector2Subtract(start1, end1);
	Vector2 line2 = Vector2Subtract(start2, end2);
	float det = Vector2CrossProduct( line1 , line2);
	if (det == 0.0f)
	{
		 return false;
	}

	float AxA = Vector2CrossProduct(start1, end1);
	float BxB = Vector2CrossProduct(start2, end2);
	float x = Vector2CrossProduct(
			Vector2(AxA, line1.x),
			Vector2(BxB, line2.x)
		) / det;
	float y = Vector2CrossProduct(
			Vector2(AxA, line1.y),
			Vector2(BxB, line2.y)
		) / det;

	intersectOut = Vector2(x,y);
	return true;
}

// -1 is left
// 1 is right
bool ClipLineToFrustum(Vector2 start, Vector2 end, int side, Vector2 &out clipOut)
{
	if (side<0) {
		return Intersect(start, end, frustumOrigo, frustumLeft, clipOut);
	} else if (side>0) {
		return Intersect(start, end, frustumOrigo, frustumRight, clipOut);
	}
	return false;
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
	color32 color, bool isPortal, bool isTop)
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
	bool drawStrip = true;
	glBegin(GL_LINES);
	for (int x = sx; x < ex; x++)
	{
		int bufferIndex = zBufferIndexOffset + x;
		if (isPortal == false)
		{
			// When drawing walls, care about Z buffer
			drawStrip = az < ZBuffer[bufferIndex];
		}
		if (drawStrip)
		{
			// Ceiling
			/*
			mgdl_glColor32(Debug_DarkGray);
			glVertex2f(x, TOP_LIMITS[bufferIndex]);
			glVertex2f(x, ayb-1);
			*/
			// When drawing walls, dont care about limits
			float dyt = ayt;
			float dyb = ayb;
			dyt = clampf(ayt, TOP_LIMITS[bufferIndex], BOTTOM_LIMITS[bufferIndex]);
			dyb = clampf(ayb, TOP_LIMITS[bufferIndex], BOTTOM_LIMITS[bufferIndex]);
			if (isPortal)
			{
				// Drawing top part where neighbor ceiling is lower than ours
				if (isTop)
				{
					TOP_LIMITS[bufferIndex] = clampf(dyt, TOP_LIMITS[bufferIndex], SCREEN_HEIGHT/2-1);
				}
				else
				{
					// Drawing bottom part where neighbor floor is higher than ours
					BOTTOM_LIMITS[bufferIndex] = clampf(dyb, -SCREEN_HEIGHT/2, BOTTOM_LIMITS[bufferIndex]);
				}
			}

			// Wall
			mgdl_glColor32(color);
			glVertex2f(x, dyt);
			glVertex2f(x, dyb);

			// Floor
			/*
			mgdl_glColor32(Debug_Blue);
			glVertex2f(x, ayb+1);
			glVertex2f(x, BOTTOM_LIMITS[bufferIndex]);
			*/

			if (isPortal == false){
				ZBuffer[bufferIndex]=az;
			}
		}

		// Advance to next strip

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
		Vector2 clip1;

		bool isValidClip = ClipLineToFrustum(A, B, -1, clip1);
		
		if (isValidClip == false) {
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
		Vector2 clip2;

		bool isValidClip = ClipLineToFrustum(A, B,  1, clip2);

		if (isValidClip == false) {
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

void ProcessWallTopDown(Vector2 trans1, Vector2 trans2, float playerRadius, bool isPortal)
{
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




void RenderTopDownList(array<Vector2> wallpoints, int pointAmount, float scale)
{
	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;

glPushMatrix();

	glTranslatef(screen_half_width, screen_half_height, 0);
	

	Actor@ player = BunnySector_GetActor(0);
	buns_Vec2 outPlayerPos;
	buns_Vec2 outPlayerDir;
	BunnySector_GetActorPositionV2(0, outPlayerPos);
	BunnySector_GetActorFloorDir(0, outPlayerDir);

	float playerAngle = player.yawRad;
	Vector2 playerPos = Vector2New(outPlayerPos.x * scale, outPlayerPos.y * scale);
	Vector2 playerDir = Vector2New(outPlayerDir.x, outPlayerDir.y);
	float playerRadius = BunnySector_GetActorRadius(0);

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
		ProcessWallTopDown(trans1, trans2, playerRadius, false);
		
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

	glTranslatef(screen_half_width, screen_half_height, 0);
	
	mgdl_DrawTextFloat("Player x: ", outPlayerPos.x, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player y: ", outPlayerPos.y, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player d: ", RAD2DEG * playerAngle, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player vfov: ", RAD2DEG * VFOVRAD, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player gl vfov: ", BunnySector_GetOpenGLCameraVerticalFOVDeg(), text_x, NextY(), 8, Debug_Yellow);
	//mgdl_DrawTextFloat("Player fx: ", outPlayerDir.x, 16, NextY(), 8, Debug_Yellow);
	//mgdl_DrawTextFloat("Player fy: ", outPlayerDir.y, 16, NextY(), 8, Debug_Yellow);
	


	glScalef(scale, scale, 1.0f);
	
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
			ProcessWallTopDown(trans1, trans2, playerRadius,start.nextsector >= 0);

			if (PRINT && wallIndex == 1) {
				mgdl_DrawTextInt("Wall", wallIndex, text_x, NextY(), 8, Debug_White);
				mgdl_DrawTextFloat("A.z", trans1.x, text_x, NextY(), 8, Debug_White);
				mgdl_DrawTextFloat("A.x", trans1.y, text_x, NextY(), 8, Debug_White);
				mgdl_DrawTextFloat("B.z", trans2.x, text_x, NextY(), 8, Debug_White);
				mgdl_DrawTextFloat("B.x", trans2.y, text_x, NextY(), 8, Debug_White);
			}
				
		}
			
	}

	// Player view cone for fov debug
	mgdl_DrawLineV(frustumOrigo, frustumLeft, Debug_Green);
	mgdl_DrawLineV(frustumOrigo, frustumRight, Debug_Blue);

glPopMatrix();
}


void RenderMuffin()
{
	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;
	// Translate origo to screen center
	if (RENDER_2D_WALLS) {
		glPushMatrix();
		glTranslatef(screen_half_width, screen_half_height, 0);

		// Draw limits

		mgdl_DrawLine(-screen_half_width, -screen_half_height,
					  -screen_half_width, screen_half_height, Debug_White);
		mgdl_DrawLine(screen_half_width, -screen_half_height,
					  screen_half_width, screen_half_height, Debug_White);
		mgdl_DrawLine(-screen_half_width, -screen_half_height,
					  screen_half_width, -screen_half_height, Debug_White);
		mgdl_DrawLine(-screen_half_width, screen_half_height,
					  screen_half_width, screen_half_height, Debug_White);
	}
	// Set up player stuff
	///////////////////////

	Actor@ player = BunnySector_GetActor(0);

	buns_Vec2 outPlayerPos;
	BunnySector_GetActorPositionV2(0, outPlayerPos);
	float playerY = player.elevation;
	Vector2 playerPos = Vector2(outPlayerPos.x, outPlayerPos.y);

	buns_Vec2 outPlayerDir;
	BunnySector_GetActorFloorDir(0, outPlayerDir);
	Vector2 playerDir = Vector2(outPlayerDir.x, outPlayerDir.y);
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

			Vector2 trans1 = WorldToCamera(Vector2New(start.x, start.z), playerPos, playerAngle);
			Vector2 trans2 = WorldToCamera(Vector2New(end.x, end.z), playerPos, playerAngle);
			
			bool draw = ProcessWall(playerY,
				trans1,
				trans2,
				start.nextsector,
				sector.floory, sector.ceilingy,
				now.left, now.right
				);

			if (draw && !RENDER_2D_WALLS) {
				DrawWall(start, end, sector.floory, sector.ceilingy);
			}
		}
	}

	if (RENDER_2D_WALLS) {
		glPopMatrix();
		//DrawRequestDebug();
	}
	if (DEBUG_PRINT)
	{
		mgdl_LogText("------End");
	}

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
			-100, 100, -1,
			-SCREEN_WIDTH/2, SCREEN_WIDTH/2-1);
	}
glPopMatrix();

}

// Draw wall with non transformed coordinates
// TODO Put in a request instead and draw everything at once
void DrawWall(Wall@  wall, Wall@ end, s32 floory, s32 ceilingy)
{
	if (wall.nextsector >= 0){
		// Calculate top and bottom parts

		// Create wall that goes down or up to adjacent sector: Note! both sectors dont need to do this. Only lower one
		Sector@ neighbor = BunnySector_GetSector(wall.nextsector);

		// if this floor height is less than adjacent: Greate wall in between: goes up
		if (floory < neighbor.floory)
		{
			BunnySector_DrawWall(wall, end, floory, neighbor.floory, wall.picnum, wall.shade);
			//DrawQuad(startPoint, endPoint, wallNormal, floory, neighbor.floory, wall.picnum, wall.shade, 1.0f);
		}

		// Ceiling:
		// If this ceiling is higher than adjacent: Greate wall in between: goes down
		if (ceilingy > neighbor.ceilingy)
		{
			Wall@ otherWall = BunnySector_GetWall(wall.nextwall);
			BunnySector_DrawWall(wall, end, neighbor.ceilingy, ceilingy, otherWall.picnum, otherWall.shade);
			//DrawQuad(startPoint, endPoint, wallNormal, neighbor.ceilingy, ceilingy, otherWall.picnum, otherWall.shade, 1.0f);
		}
	}
	else
	{
		BunnySector_DrawWall(wall, end, floory, ceilingy, wall.picnum, wall.shade);
        //DrawQuad(startPoint, endPoint, wallNormal, floory, ceilingy, wall.picnum, wall.shade, 1.0f);
	}
}

// Returns true if this wall should be drawn
		
bool ProcessWall(float playerY,
	Vector2 trans1, Vector2 trans2,
	s16 nextsector,
	s32 floory, s32 ceilingy,
	int limitLeft, int limitRight)
{
	if (DEBUG_PRINT) {
		if (nextsector >= 0)
		{
			mgdl_LogTextInt("Portal wall to ", nextsector);
		}
		else
		{
			mgdl_LogText("Normal wall");
		}

	}
	if (PRINT) {
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

			// TODO We only really care about the X coordinates
			
			Vector2 Atop= CameraToViewport(Vector2New(ax,ceilingy-playerY),az);
			Vector2 Abot= CameraToViewport(Vector2New(ax,floory-playerY),az);
			
			Vector2 Btop = CameraToViewport(Vector2New(bx,ceilingy-playerY),bz);
			Vector2 Bbot = CameraToViewport(Vector2New(bx,floory-playerY),bz);
			
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
			if (DEBUG_PRINT)
			{
				mgdl_LogTextFloat("Acanvas x", Atop.x);
				mgdl_LogTextFloat("Acanvas y", Atop.y);
			}
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
			bool isportal = nextsector >= 0;
			if (isportal && draw) {
				if (DEBUG_PRINT) {
					mgdl_LogTextInt("Found portal to ", nextsector);
				}
			
				// Limits for drawing and the new request
				float beginx = fmaxf(Atop.x, limitLeft);
				float endx = fminf(Btop.x, limitRight);
				if (endx > beginx && CanPushRequest()) {
					PushRequest(nextsector,  beginx, endx);
					if (DEBUG_PRINT) {
						mgdl_LogTextInt("Pushed", nextsector);
						LogReq();
					}
				}
				else if (DEBUG_PRINT) {
					mgdl_LogTextFloat("Outside limits: beginx", beginx);
					mgdl_LogTextFloat("Outside limits: endx", endx);
				}
			}

			if (draw) {
				if (RENDER_2D_WALLS) {
					if (isportal) {

						Sector@ neighbor = BunnySector_GetSector(nextsector);

						// if this floor height is less than adjacent: Greate wall in between: goes up
						if (floory < neighbor.floory)
						{
							// Calculate top using neighbor floory
							Vector2 NAtop= CameraToViewport(Vector2New(ax,neighbor.floory-playerY),az);
							Vector2 NBtop= CameraToViewport(Vector2New(bx,neighbor.floory-playerY),bz);
							NAtop = ViewportToCanvas(NAtop);
							NBtop = ViewportToCanvas(NBtop);

							OutlineWall(
								NAtop.x, NAtop.y, Abot.y, az,
								NBtop.x, NBtop.y, Bbot.y, bz,
								limitLeft, limitRight,
								Debug_Blue);
						}

						// Other sector shows through here
						/*
						OutlineWall(
							Atop.x, Atop.y, Abot.y, az,
							Btop.x, Btop.y, Bbot.y, bz,
							limitLeft, limitRight,
							Debug_Red);
							*/

						// Ceiling:
						// If this ceiling is higher than adjacent: Greate wall in between: goes down
						if (ceilingy > neighbor.ceilingy)
						{
							// Calculate bottom using neighbor ceilingy
							Vector2 NAbot= CameraToViewport(Vector2New(ax,neighbor.ceilingy-playerY),az);
							Vector2 NBbot= CameraToViewport(Vector2New(bx,neighbor.ceilingy-playerY),bz);
							NAbot = ViewportToCanvas(NAbot);
							NBbot = ViewportToCanvas(NBbot);

							OutlineWall(
								Atop.x, Atop.y, NAbot.y, az,
								Btop.x, Btop.y, NBbot.y, bz,
								limitLeft, limitRight,
								Debug_Green);
						}
					}
					else {

						OutlineWall(
							Atop.x, Atop.y, Abot.y, az,
							Btop.x, Btop.y, Bbot.y, bz,
							limitLeft, limitRight,
							Debug_DarkGray);
					}
				}
				return true;
			} // Draw or not?
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

	return false;
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

float GetVerticalFovDeg()
{
	return RAD2DEG * VFOVRAD;
}

void StartFrame()
{
	// NOTE our origo is at center
	for (int i = 0; i < SCREEN_WIDTH; i++)
	{
		TOP_LIMITS[i] = -SCREEN_HEIGHT/2;
		BOTTOM_LIMITS[i] = SCREEN_HEIGHT/2-1;
	}
	ClearZBuffer();

	ResetDrawCounters();

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
	

	Vector2 inter;
	if (Intersect(A1, A2, B1, B2, inter))
	{
		mgdl_DrawLineV(A1, inter, Debug_Yellow);
		mgdl_DrawLineV(B1, inter, Debug_Red);
		DrawCross(inter, Debug_White);
	}
glPopMatrix();
}

void RenderInit(int VerticalFovDegrees)
{
	//SCREEN_WIDTH = mgdl_GetScreenWidth();
	//SCREEN_HEIGHT = mgdl_GetScreenHeight();
	CanvasSize = Vector2New(SCREEN_WIDTH, SCREEN_HEIGHT); // Canvas is where we draw
	SetVerticalFovDeg(VerticalFovDegrees);

	mgdl_LogTextFloat("Canvas aspect ratio ", CanvasSize.x/CanvasSize.y);
	mgdl_LogTextFloat("VFOV DEG", VFOVRAD * RAD2DEG);
	mgdl_LogTextFloat("HFOV DEG ", HFOVRAD * RAD2DEG);
	mgdl_LogTextFloat("Viewport width ", ViewPortSize.x);
	mgdl_LogTextFloat("Viewport height ", ViewPortSize.y);
	mgdl_LogTextFloat("Canvas width ", CanvasSize.x);
	mgdl_LogTextFloat("Canvase height ", CanvasSize.y);
}


void RenderMiniMap()
{
	Init2D_YDown();
	DrawGizmo();
	RenderTopDown(0.50f);
}

void RenderMapSoftware(float deltatime)
{
	glClearColor(0.3f, 0.2f, 0.3f, 1.0f);

	StartFrame();
	Init2D_YDown();

	// Draw debug infos
	if (mgdl_IsButtonPressed(0, Button1))
	{
		DEBUG_PRINT = true;
	}
	RENDER_2D_WALLS = true;
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
	}
	RenderMuffin();

	DEBUG_PRINT = false;
	RENDER_2D_WALLS = false;
}

void RenderMap(float deltatime)
{
	StartFrame();
	RenderMuffin();
}
