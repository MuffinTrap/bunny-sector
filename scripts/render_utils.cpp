
bool DEBUG_DRAW = false;
bool DEBUG_LOG = false;
bool RENDER_2D_WALLS = false;

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;
// Stuff used by both Doom and Duke render

Vector2 FORWARD_2D = Vector2(1, 0);
Vector2 RIGHT_2D = Vector2Rotate(FORWARD_2D, DEG2RAD * 90);


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


// Output of ProcessWall when doing 2D DRAWING
// These are in screen space units
Vector2 A_XZ;
Vector2 B_XZ;

// Parameters needed by DrawWall2D

// Always needed
float PLAYER_Y;
float SECTOR_FLOORY;
float SECTOR_CEILINGY;

// If drawing a portal
int SECTOR_NEIGHBOR_ID;
int SECTOR_NEIGHBOR_FLOORY;
int SECTOR_NEIGHBOR_CEILINGY;

// Always needed but can be same for all
int DRAW_LIMIT_LEFT_CANVAS;
int DRAW_LIMIT_RIGHT_CANVAS;

// Arrays for storing top and bottom limits
int[] DRAW_LIMIT_TOPS(SCREEN_WIDTH);
int[] DRAW_LIMIT_BOTTOMS(SCREEN_WIDTH);

// Written by OutlineWall
int OUTLINE_START_X;
int OUTLINE_END_X;

// Written by DrawWall2D
int DRAW_START_X;
int DRAW_END_X;

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
	float aspect = (CanvasSize.x/CanvasSize.y);
	float viewWidth = atan(aspect * viewHeight);
	return Vector2New(viewWidth*2.0f, viewHeight*2.0f);
}

// Translate and rotate point to camera space
Vector2 WorldToCamera(Vector2 point, Vector2 CameraPosition, float cameraYaw)
{
	return Vector2Rotate( Vector2Subtract(point, CameraPosition), -cameraYaw);
}
// NOTE: This is to replace the above transforms when everything works
// to make code faster

Vector2 GetConvertXY()
{
	float aspect=CanvasSize.x/CanvasSize.y;
	HFOVRAD=VFOVRAD*aspect;
	return Vector2New(
		CanvasSize.x/ViewPortSize.x,
		CanvasSize.y/ViewPortSize.y
	);
}

// WALL DRAWING
// /////////////


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

	OUTLINE_START_X = x1;
	OUTLINE_END_X = x2;
}


// MATH FUNCTIONS
// //////////////

int NormalizeAngleDeg(int deg)
{
	deg = deg % 360;
	if (deg < 0) { return deg + 360;}
	else{ return deg; }
}
float NormalizeAngleRad(float rad)
{
	while(rad < 0)
	{
		rad += M_PI * 2;
	}
	while(rad >= M_PI * 2)
	{
		rad -= M_PI * 2;
	}
	return rad;
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

bool isBehind(Vector2 point)
{
	return Vector2DotProduct(FORWARD_2D, point) < 0.0f;
}

// Clip the wall to view frustum side
bool ClipWall(Vector2 A, Vector2 B, int side, bool drawDebugs, Vector2 &out point)
{
	if (side < 0) {
		// A is behind
		Vector2 clip1;

		bool isValidClip = ClipLineToFrustum(A, B, -1, clip1);

		if (isValidClip == false) {
			if (DEBUG_LOG) { mgdl_LogText("Invalid A intersect point");}
			return false;
		}

		// Clip only if the clipped point is closer to player than the end point B
		float dotclip1 = Vector2DotProduct(clip1, FORWARD_2D);
		float Bclip = Vector2DotProduct(B, FORWARD_2D);
		if (dotclip1 > 0 && dotclip1 < Bclip) {
			if (drawDebugs && DEBUG_LOG) {
				mgdl_DrawLineV(A, clip1, Debug_Green);
				DrawCross(clip1, Debug_Green);
				mgdl_DrawTextFloat("A to Clip1 x:", clip1.x, text_x,NextY(),8,Debug_Green);
				mgdl_DrawTextFloat("A to Clip1 y:", clip1.y, text_x,NextY(),8,Debug_Green);
			}
			point = clip1;
			return true;
		}
		else if (DEBUG_LOG) {
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
			if (DEBUG_LOG) { mgdl_LogText("Invalid B intersect point");}
			return false;
		}

		float dotclip2 = Vector2DotProduct(clip2, FORWARD_2D);
		float Aclip = Vector2DotProduct(A, FORWARD_2D);
		if (dotclip2 > 0 && dotclip2 < Aclip) {
			if (drawDebugs && DEBUG_LOG) {
				mgdl_DrawLineV(B, clip2, Debug_Blue);
				DrawCross(clip2, Debug_Blue);
				mgdl_DrawTextFloat("B to Clip2 x:", clip2.x, text_x,NextY(),8,Debug_Blue);
				mgdl_DrawTextFloat("B to Clip2 y:", clip2.y, text_x,NextY(),8,Debug_Blue);
			}
			point = clip2;
			return true;
		}
		else if (DEBUG_LOG) {
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

// OCCLUSION CALCUATIONS



void ProcessWallTopDown(Vector2 trans1, Vector2 trans2, float playerRadius, bool isPortal)
{
	bool front1 = Vector2DotProduct(trans1, FORWARD_2D) > 0;
	bool front2 = Vector2DotProduct(trans2, FORWARD_2D) > 0;

	bool behind = false;
	bool clipped = false;
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
			else
			{
				clipped = true;
			}
		}
		if (front2==false){
			bool clipOk = ClipWall(trans1, trans2, 1, true, outPoint);
			if (clipOk) {
				trans2 = outPoint;
			}
			else
			{
				clipped = true;
			}
		}
	}

	Vector2 moveEnd = Vector2Scale(FORWARD_2D, 100);

	color32 wallColor = Debug_Green;
	if (behind ) {
		wallColor = Debug_DarkGray;
	}
	else if (clipped)
	{
		wallColor = Debug_Blue;
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

		Vector2 wallcenter = Vector2Add(trans1, Vector2Scale(walldir, 1.0));
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
	/*
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
	*/
	glBegin(GL_LINES);
		mgdl_glColor32(wallColor);
		glVertex2f(trans1.x, trans1.y);
		glVertex2f(trans2.x, trans2.y);
	glEnd();

	//mgdl_DrawLine(trans1.x, trans1.y,
				  //trans2.x, trans2.y, wallColor);
}


// Returns true if this wall should be drawn



bool ProcessWall( Vector2 trans1, Vector2 trans2,
	int limitLeft, int limitRight)
{
	if (DEBUG_DRAW) {
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
		if (DEBUG_LOG){mgdl_LogText("X behind");}
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
				if (DEBUG_LOG){mgdl_LogText("X A clipped");}
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
				if (DEBUG_LOG){mgdl_LogText("X B clipped");}

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
			Vector2 Atop= CameraToViewport(Vector2New(ax,0),az);

			Vector2 Btop = CameraToViewport(Vector2New(bx,0),bz);
			Atop = ViewportToCanvas(Atop);

			Btop = ViewportToCanvas(Btop);

			// TODO We only really care about the X coordinates
			if (Atop.x > Btop.x)
			{
				backface = true;
				draw = false;
				if (DEBUG_LOG){mgdl_LogText("X backface");}
			}

			if (draw) {
				A_XZ.x = ax;
				A_XZ.y = az;
				B_XZ.x = bx;
				B_XZ.y = bz;
			} // Draw or not?
		} // if one point in front
	} // is behind ?

	// Show rejection reason
	if (DEBUG_DRAW) {
		if (draw) { mgdl_DrawText("Draw!",text_x,NextY(),8, Debug_White);		}
		if (behind ) { mgdl_DrawText("Behind",text_x,NextY(),8,Debug_Red);		}
		if (clipped ) { mgdl_DrawText("Clipped",text_x,NextY(),8,Debug_Red);		}
		if (backface) { mgdl_DrawText("Backface",text_x,NextY(),8,Debug_Red);	}
		if (outsideWindow ) { mgdl_DrawText("Outside W",text_x,NextY(),8,Debug_Red);	}
	}

	return draw;
}

// RENDER SETUP AND STATE

void StartFrame()
{
	ResetTextY();
	// NOTE our origo is at center
	for (int i = 0; i < SCREEN_WIDTH; i++)
	{
		DRAW_LIMIT_TOPS[i] = -SCREEN_HEIGHT/2;
		DRAW_LIMIT_BOTTOMS[i] = SCREEN_HEIGHT/2-1;
	}
	Init2D_YDown();
}

void Init2D_YDown()
{
	mgdl_InitOrthoProjection(-1.0f);
}


// DEBUG DRAWING
// //////////////

void DrawPlayerFOV()
{
	// Player view cone for fov debug
	mgdl_DrawLineV(frustumOrigo, frustumLeft, Debug_Green);
	mgdl_DrawLineV(frustumOrigo, frustumRight, Debug_Blue);
}

void DrawPlayerPositionAndAngle(Actor@ actor)
{
	buns_Vec2 outp;
	BunnySector_GetActorPositionV2(0, outp);
	mgdl_DrawTextFloat("Player x: ", outp.x, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player y: ", outp.y, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player elevation: ", actor.elevation, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player d: ", RAD2DEG * actor.yawRad, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player vfov: ", RAD2DEG * VFOVRAD, text_x, NextY(), 8, Debug_Yellow);
	mgdl_DrawTextFloat("Player gl vfov: ", BunnySector_GetOpenGLCameraVerticalFOVDeg(), text_x, NextY(), 8, Debug_Yellow);
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

void DrawWall2D(bool fill)
{
	float ax = A_XZ.x;
	float az = A_XZ.y;
	float bx = B_XZ.x;
	float bz = B_XZ.y;
	float playerY = PLAYER_Y;
	float sectorceilingy = SECTOR_CEILINGY;
	float sectorfloory = SECTOR_FLOORY;
	bool isportal = SECTOR_NEIGHBOR_ID >= 0;
	int nextsector = SECTOR_NEIGHBOR_ID;
	int limitLeft = DRAW_LIMIT_LEFT_CANVAS;
	int limitRight = DRAW_LIMIT_RIGHT_CANVAS;

	Vector2 Atop= CameraToViewport(Vector2New(ax,sectorceilingy-playerY),az);
	Vector2 Abot= CameraToViewport(Vector2New(bx,sectorfloory-playerY),az);

	Vector2 Btop = CameraToViewport(Vector2New(bx,sectorceilingy-playerY),bz);
	Vector2 Bbot = CameraToViewport(Vector2New(bx,sectorfloory-playerY),bz);

	if (DEBUG_DRAW) {
		mgdl_DrawTextFloat("Aview x:", Atop.x, text_x, NextY(), 8, Debug_Yellow);
		mgdl_DrawTextFloat("Aview y:", Atop.y, text_x, NextY(), 8, Debug_Yellow);

		mgdl_DrawTextFloat("Bview x:", Btop.x, text_x, NextY(), 8, Debug_Yellow);
		mgdl_DrawTextFloat("Bview y:", Btop.y, text_x, NextY(), 8, Debug_Yellow);
	}

	Atop = ViewportToCanvas(Atop);
	Abot = ViewportToCanvas(Abot);

	Btop = ViewportToCanvas(Btop);
	Bbot = ViewportToCanvas(Bbot);
	if (DEBUG_LOG)
	{
		mgdl_LogTextFloat("Acanvas x", Atop.x);
		mgdl_LogTextFloat("Acanvas y", Atop.y);
	}

	if (isportal) {
		float neighborfloory = SECTOR_NEIGHBOR_FLOORY;
		float neighborceilingy = SECTOR_NEIGHBOR_CEILINGY;

		// if this floor height is less than adjacent: Greate wall in between: goes up
		if (sectorfloory < neighborfloory)
		{
			// Calculate top using neighbor floory
			Vector2 NAtop= CameraToViewport(Vector2New(ax,neighborfloory-playerY),az);
			Vector2 NBtop= CameraToViewport(Vector2New(bx,neighborfloory-playerY),bz);
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
		 *						OutlineWall(
		 *							Atop.x, Atop.y, Abot.y, az,
		 *							Btop.x, Btop.y, Bbot.y, bz,
		 *							limitLeft, limitRight,
		 *							Debug_Red);
		 */

		// Ceiling:
		// If this ceiling is higher than adjacent: Greate wall in between: goes down
		if (sectorceilingy > neighborceilingy)
		{
			// Calculate bottom using neighbor ceilingy
			Vector2 NAbot= CameraToViewport(Vector2New(ax,neighborceilingy-playerY),az);
			Vector2 NBbot= CameraToViewport(Vector2New(bx,neighborceilingy-playerY),bz);
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
			Debug_White);

		DRAW_START_X = OUTLINE_START_X;
		DRAW_END_X = OUTLINE_END_X;
	}

}
void DrawWall2DFill(float ax, float ayt, float ayb, float az,
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
			dyt = clampf(ayt, DRAW_LIMIT_TOPS[bufferIndex], DRAW_LIMIT_BOTTOMS[bufferIndex]);
			dyb = clampf(ayb, DRAW_LIMIT_TOPS[bufferIndex], DRAW_LIMIT_BOTTOMS[bufferIndex]);
			if (isPortal)
			{
				// Drawing top part where neighbor ceiling is lower than ours
				if (isTop)
				{
					DRAW_LIMIT_TOPS[bufferIndex] = clampf(dyt, DRAW_LIMIT_TOPS[bufferIndex], SCREEN_HEIGHT/2-1);
				}
				else
				{
					// Drawing bottom part where neighbor floor is higher than ours
					DRAW_LIMIT_BOTTOMS[bufferIndex] = clampf(dyb, -SCREEN_HEIGHT/2, DRAW_LIMIT_BOTTOMS[bufferIndex]);
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

void DrawWall3D(Vector2 left, Vector2 right, s16 picnumMiddle, s16 picnumBottom, s16 picnumTop, s8 shade)
{
	Vector2 wallNormal = Vector2RightNormal(left, right);
	if (SECTOR_NEIGHBOR_ID >= 0)
	{
		// Calculate top and bottom parts

		// Create wall that goes down or up to adjacent sector: Note! both sectors dont need to do this. Only lower one

		// if this floor height is less than adjacent: Greate wall in between: goes up
		if (SECTOR_FLOORY < SECTOR_NEIGHBOR_FLOORY)
		{
			//BunnySector_DrawWall(wall, end, floory, neighbor.floory, wall.picnum, wall.shade);
			BunnySector_DrawWallF(left.x, left.y, right.x, right.y, wallNormal.x, wallNormal.y, SECTOR_FLOORY, SECTOR_NEIGHBOR_FLOORY, picnumBottom, shade);
		}


		// Ceiling:
		// If this ceiling is higher than adjacent: Greate wall in between: goes down
		if (SECTOR_CEILINGY > SECTOR_NEIGHBOR_CEILINGY)
		{
			//Wall@ otherWall = BunnySector_GetWall(wall.nextwall);
			//DrawQuad(startPoint, endPoint, wallNormal, neighbor.ceilingy, ceilingy, otherWall.picnum, otherWall.shade, 1.0f);
			BunnySector_DrawWallF(left.x, left.y, right.x, right.y, wallNormal.x, wallNormal.y, SECTOR_CEILINGY, SECTOR_NEIGHBOR_CEILINGY, picnumTop, shade);
		}

		if (picnumMiddle >= 0)
		{
			// TODO Masked portal from higher floor to lower ceiling
			BunnySector_DrawWallF(left.x, left.y, right.x, right.y, wallNormal.x, wallNormal.y, MaxI(SECTOR_FLOORY, SECTOR_NEIGHBOR_FLOORY),
			MinI(SECTOR_CEILINGY, SECTOR_NEIGHBOR_CEILINGY),
			picnumMiddle, shade);
		}
	}
	else
	{
		//BunnySector_DrawWall(wall, end, floory, ceilingy, wall.picnum, wall.shade);
        //DrawQuad(startPoint, endPoint, wallNormal, floory, ceilingy, wall.picnum, wall.shade, 1.0f);
		BunnySector_DrawWallF(left.x, left.y, right.x, right.y, wallNormal.x, wallNormal.y, SECTOR_CEILINGY, SECTOR_FLOORY, picnumMiddle, shade);
	}
}
