
// Debug settings

bool DEBUG_PRINT = false;
bool PRINT = false;
bool RENDER_2D_WALLS = false;
int WALL_LIMIT = -1;
int REQUEST_LIMIT = -1;

// Arrays for storing top and bottom limits
int[] TOP_LIMITS(SCREEN_WIDTH);
int[] BOTTOM_LIMITS(SCREEN_WIDTH);

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
int REQUEST_AMOUNT = 128;
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

	DrawPlayerPositionAndAngle(player);
	
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
	DrawPlayerFOV();

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
		BunnySector_DrawSectorFloorOrCeiling(now.number, true, 0, 0);
		BunnySector_DrawSectorFloorOrCeiling(now.number, false, 0, 0);
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
			BunnySector_DrawWall(wall, end, neighbor.ceilingy, ceilingy, wall.picnum, otherWall.shade);
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
			// TODO Move this out here
				// Use global parameters or something to set
				// the pixel values
				// and neighbor height etc...
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
							Debug_White);
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
	RenderTopDown(0.250f);
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
