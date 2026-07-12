#include "bunny-sector_main.h"
#include <mgdl.h>

#include "duke/dukemapreader.h"
#include "duke/build-render.h"
#include "duke/opengl-render.h"
#include "duke/dukemath.h"

static DukeMap** mapsArray = nullptr;
static const float MAP_AMOUNT = 4;

static RenderSettingsOpenGL defaultOpenGL;
static CameraInfo defaultCameraInfo;
static Camera* defaultCamera;

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

	OpenGLRender_Init();
	OpenGLRender_RegisterTexture(RENDERER_PICNUM_DEFAULT, Texture_GenerateCheckerBoard());
	BuildRender_Init();
	defaultOpenGL = GetDefaultRenderSettingsOpenGL();
	defaultCameraInfo = GetDefaultCameraInfo();
	defaultCamera = GetDefaultCamera();
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
			Map_SetCameraToStart(map, &defaultCameraInfo);
		}
	}
}

static void s_SyncInfoToCamera(CameraInfo* info, Camera* camera)
{
	Vector3 cameraPosition = Vec3DukePosToOpenGL(info->position, &defaultOpenGL);
	Vector3 rotations = Vector3New(info->pitchRad, info->yawRad, 0.0f);
	Matrix rotation = MatrixRotateXYZ(rotations);
	Vector3 forward = mgdl_GetGLWorldForward();
	Vector3 cameraDir = Vector3Transform(forward, rotation);

	Camera_SetPositionV(camera, cameraPosition);
	Camera_SetDirection(camera, cameraDir);
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

			defaultCameraInfo.sector = Map_FindPlayerSector(map, defaultCameraInfo.sector, defaultCameraInfo.position);
			s_SyncInfoToCamera(&defaultCameraInfo, defaultCamera);
			Camera_Apply(defaultCamera);

			BuildRender_Draw3D(&defaultCameraInfo, map, &defaultOpenGL);

			glPopMatrix();
		}
	}
}

void BunnySector_ChangeCameraTransform(float right, float up, float forward, float yawRadiands, float pitchRadians)
{
	Vector3 translation = Vector3New(right, up, forward);
	defaultCameraInfo.position = Vector3Add(defaultCameraInfo.position, translation);
	defaultCameraInfo.pitchRad += pitchRadians;
	defaultCameraInfo.yawRad += yawRadiands;
}

