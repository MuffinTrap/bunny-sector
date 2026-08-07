#include "bunny-sector_main.h"
#include <mgdl.h>

#include "duke/dukemapreader.h"
#include "duke/build-render.h"
#include "duke/opengl-render.h"
#include "duke/dukemath.h"
#include "duke/actor.h"

static DukeMap** mapsArray = nullptr;
static DukeMap* activeMap = nullptr;
static const float MAP_AMOUNT = 4;

static RenderSettingsOpenGL defaultOpenGL;
static Viewpoint defaultView;
static Camera* defaultCamera = nullptr;
static Actor demoActor;
static Texture* defaultChecker = nullptr;

static void s_AlignCameraToViewpoint(Viewpoint* info, Camera* camera)
{
	Vector3 cameraPosition = Vec3DukePosToOpenGL(info->position, &defaultOpenGL);
	Vector3 rotations = Vector3New(info->pitchRad, info->yawRad, 0.0f);
	Matrix rotation = MatrixRotateXYZ(rotations);
	Vector3 forward = mgdl_GetGLWorldForward();
	Vector3 cameraDir = Vector3Transform(forward, rotation);

	Camera_SetPositionV(camera, cameraPosition);
	Camera_SetDirection(camera, cameraDir);
}

bool BunnySector_Init()
{
	if (mapsArray == nullptr)
	{
		mapsArray = (DukeMap**)mgdl_AllocateGeneralMemory(sizeof(DukeMap*) * MAP_AMOUNT);
		for (int i = 0; i < MAP_AMOUNT; i++)
		{
			mapsArray[i] = nullptr;
		}
	}

	if (defaultChecker == nullptr)
	{
		defaultChecker = Texture_GenerateCheckerBoard(false);
		Texture_SetFilterModeMag(defaultChecker, Nearest);
		Texture_SetFilterModeMin(defaultChecker, Linear);
	}

	OpenGLRender_Init();
	OpenGLRender_RegisterTexture(RENDERER_PICNUM_DEFAULT, defaultChecker);
	// Get rest of the textures from file
	OpenGLRender_ReadMaterialsXML("assets/materials.xml");
	BuildRender_Init();
	defaultOpenGL = GetDefaultRenderSettingsOpenGL();
	defaultView = GetDefaultCameraInfo();
	defaultCamera = GetDefaultCamera();
	demoActor = Actor_CreateDefaultActor(0);
	return true;
}

int BunnySector_LoadMap(const char* mapfilename, int dukesPerUnit)
{
	zstr mapname = zstr_from(mapfilename);
	int mapid = BunnySector_LoadMap(mapname, dukesPerUnit);
	zstr_free(&mapname);
	return mapid;
}

int BunnySector_LoadMap(const zstr& mapfilename, int dukesPerUnit)
{
	// Do we have this map already?
	int firstFree = -1;
	for (int i = 0; i < MAP_AMOUNT; i++)
	{
		DukeMap* map = mapsArray[i];
		if (map != nullptr)
		{
			if (zstr_eq(&map->mapfile, &mapfilename))
			{
				return i;
			}
		}
		else if (firstFree < 0)
		{
			firstFree = i;
		}
	}

	activeMap = ReadMapFromFile(mapfilename, dukesPerUnit);
	if (activeMap  != nullptr)
	{
		mapsArray[firstFree] = activeMap ;
		return firstFree;
	}
	return -1;
}


void BunnySector_StartMap(MapId mapId)
{
	if (mapId >=0 && mapId < MAP_AMOUNT)
	{
		DukeMap* map = mapsArray[mapId];
		if (map != nullptr)
		{
			Map_SetActorToStart(map, &demoActor);
			printf("BunnySector startmap put actor to %.2f, %.2f\n", demoActor.position.x, demoActor.position.y);
			defaultView = Actor_GetViewpoint(&demoActor);
			s_AlignCameraToViewpoint(&defaultView, defaultCamera);
		}
	}
}

static const float FIXED_STEP = 1.0f/120.0f;
static float leftOverTime = 0.0f;

void BunnySector_UpdateMap(MapId mapId, float deltaTime)
{
	// Map move all actors and sprites etc...
	if (mapId >=0 && mapId < MAP_AMOUNT)
	{
		DukeMap* map = mapsArray[mapId];
		if (map != nullptr)
		{
			deltaTime += leftOverTime;
			while (deltaTime >= FIXED_STEP)
			{
				// Note: to prevent insane delta times when debugging this is done in fixed time
				Map_MoveActorInMap(map, FIXED_STEP, &demoActor);
				deltaTime -= FIXED_STEP;
			}
			leftOverTime = deltaTime;
		}
	}
}

void BunnySector_RenderMap(MapId mapId)
{
	if (mapId >=0 && mapId < MAP_AMOUNT)
	{
		DukeMap* map = mapsArray[mapId];
		if (map != nullptr)
		{
			// Set up OpenGL 3D state
			mgdl_glClearColor32(Debug_Black);
			glPushMatrix();
	            
			//mgdl_glSetTransparency(true);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glDepthMask(GL_TRUE); //  is this needed?
			
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glShadeModel(GL_SMOOTH);

			Viewport viewPort = mgdl_GetViewport();
			glViewport(viewPort.left, viewPort.bottom, viewPort.width, viewPort.height);

			// NOTE Must set GL_PROJECTION first then GL_MODELVIEW
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			gluPerspective(defaultCamera->fovY,
                                    (float)viewPort.width/(float)viewPort.height,
                                    defaultCamera->nearZ,
                                    defaultCamera->farZ);

			demoActor.sectorNumber = Map_FindSectorV2(map, demoActor.sectorNumber, demoActor.position);
			defaultView = Actor_GetViewpoint(&demoActor);

			s_AlignCameraToViewpoint(&defaultView, defaultCamera);
			Camera_Apply(defaultCamera);

			BuildRender_Draw3D(&defaultView, map, &defaultOpenGL);

			glPopMatrix();
		}
	}
}

void BunnySector_SetActorDriveInput(int actorId, float forward, float strafe, float vertical, float turnYaw, float turnPitch)
{
	demoActor.forwardDrive = Clamp(forward, -1.0f, 1.0f);
	demoActor.strafeDrive = Clamp(strafe, -1.0f, 1.0f);
	demoActor.verticalDrive = Clamp(vertical, -1.0f, 1.0f);
	demoActor.turnDrive = Clamp(turnYaw, -1.0f, 1.0f);
}

void BunnySector_MoveActorFreely(int actorId, float deltatime)
{
	demoActor.position = Actor_ApplyDrive(&demoActor, deltatime);
}

void BunnySector_SetActorSpeeds(int actorId, float walkSpeedMultiplier, float turnSpeedMultiplier)
{
	demoActor.walkSpeedMultiplier = walkSpeedMultiplier;
	demoActor.turnSpeedMultiplier = turnSpeedMultiplier;
}

Sector* BunnySector_GetSector(s16 sectorNumber)
{
	Sector* sp = Map_GetSector(activeMap, sectorNumber);
	//Log_InfoF("Get sector %d floory %d ceilingy %d\n", sectorNumber, sp->floory, sp->ceilingy);
	return Map_GetSector(activeMap, sectorNumber);
}
Wall* BunnySector_GetWall(s16 wallIndex)
{
	return Map_GetWall(activeMap, wallIndex);
}
Wall* BunnySector_GetWallEnd(Wall* wall)
{
	return Map_GetWallEnd(activeMap, wall);
}
s16 BunnySector_GetSectorAmount()
{
	return activeMap->sectorAmount;
}

float BunnySector_GetActorRadius(int actorId)
{
	return demoActor.radius;
}
void BunnySector_GetActorPositionV2(int actorId, buns_Vec2& out_pos)
{
	out_pos.x = demoActor.position.x;
	out_pos.y = demoActor.position.y;
}
void BunnySector_SetActorPosition(int actorId, float x, float z)
{
	demoActor.position.x = x;
	demoActor.position.y = z;
}
void BunnySector_GetActorFloorDir(int actorId, buns_Vec2& out_dir)
{
	out_dir.x = demoActor.floorDirection.x;
	out_dir.y = demoActor.floorDirection.y;
}
void BunnySector_GetActorPositionV3(int actorId, buns_Vec3& out_pos)
{
	out_pos.x = demoActor.position.x;
	out_pos.y = demoActor.elevation;
	out_pos.z = demoActor.position.y;
}

Actor* BunnySector_GetActor(int actorId)
{
	return &demoActor;
}

#define V2_CROSS(ax, ay, bx, by)(ax * by - ay * bx)

bool BunnySector_Intersect(double a1x, double a1y, double a2x, double a2y, double b1x, double b1y, double b2x, double b2y, buns_Vec2& out_point)
{

	double AxA = V2_CROSS(a1x, a1y, a2x, a2y);
	double BxB = V2_CROSS(b1x, b1y, b2x, b2y);
	double alinex = a1x-a2x;
	double aliney = a1y-a2y;
	double blinex = b1x-b2x;
	double bliney = b1y-b2y;
	double det = V2_CROSS(alinex, aliney, blinex, bliney);
	if (det == 0.0)
	{
		 return false;
	}
	double x = V2_CROSS(AxA, alinex, BxB, blinex)/ det;
	double y = V2_CROSS(AxA, aliney, BxB, bliney)/ det;

	out_point.x = x;
	out_point.y = y;

	return true;
}
