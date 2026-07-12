#include "bunny-sector_main.h"
#include <mgdl.h>

#include "duke/dukemapreader.h"
#include "duke/build-render.h"
#include "duke/opengl-render.h"
#include "duke/dukemath.h"
#include "duke/actor.h"

static DukeMap** mapsArray = nullptr;
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
		defaultChecker = Texture_GenerateCheckerBoard();
	}

	OpenGLRender_Init();
	OpenGLRender_RegisterTexture(RENDERER_PICNUM_DEFAULT, defaultChecker);
	BuildRender_Init();
	defaultOpenGL = GetDefaultRenderSettingsOpenGL();
	defaultView = GetDefaultCameraInfo();
	defaultCamera = GetDefaultCamera();
	demoActor = Actor_CreateDefaultActor(0);
	return true;
}

int BunnySector_LoadMap(const char* mapfilename)
{
	zstr mapname = zstr_from(mapfilename);
	int mapid = BunnySector_LoadMap(mapname);
	zstr_free(&mapname);
	return mapid;
}

int BunnySector_LoadMap(const zstr& mapfilename)
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

	DukeMap* loaded = ReadMapFromFile(mapfilename);
	if (loaded != nullptr)
	{
		mapsArray[firstFree] = loaded;
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
			defaultView = Actor_GetViewpoint(&demoActor);
			s_AlignCameraToViewpoint(&defaultView, defaultCamera);
		}
	}
}


void BunnySector_UpdateMap(MapId mapId, float deltaTime)
{
	// Map move all actors and sprites etc...
	if (mapId >=0 && mapId < MAP_AMOUNT)
	{
		DukeMap* map = mapsArray[mapId];
		if (map != nullptr)
		{
			Map_MoveActorInMap(map, deltaTime, &demoActor);
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

			demoActor.sectorNumber = Map_FindSector(map, demoActor.sectorNumber, demoActor.position);
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

