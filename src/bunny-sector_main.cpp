#include "bunny-sector_main.h"
#include "bunny-sector-map.h"
#include "bunny-sector-types.h"
#include <mgdl.h>
#include <mgdl/mgdl-script-api.h>

#include "bunny-sector-math.h"
#include "bunny-sector-materials.h"
#include "render/opengl-render.h"
#include "gameplay/actor.h"
#include "gameplay/player.h"
#include "gameplay/actorpool.h"
#include "doom/doom-map-reader.h"
#include "duke/dukemapreader.h"
#include "duke/build-render.h"

static BunnySector::Materials* materialManager;

static BunnySector_Map** mapsArray = nullptr;
static BunnySector_Map* activeMap = nullptr;
static const float MAP_AMOUNT = 4;

static RenderSettingsOpenGL defaultOpenGL;
static Viewpoint defaultView;
static Camera* defaultCamera = nullptr;
static Player player0;
static Texture* defaultChecker = nullptr;
static ActorPool actorPool; // All maps share the same actor pool

// TODO Actor pool?
// Map creates actors when it is loaded

static float bunny_UnitsToMeter = 1.0f;

static void s_AlignCameraToViewpoint(Viewpoint* info, Camera* camera)
{
	Vector3 cameraPosition = ScaleVector3ToOpenGL(info->position, &defaultOpenGL);
	float adjustedYaw = (-1.0f * info->yawRad)- DEG2RAD*90; // This is correct when angle is 0.0f
	Vector3 rotations = Vector3New(info->pitchRad, adjustedYaw, 0.0f);
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
		mapsArray = (BunnySector_Map**)mgdl_AllocateGeneralMemory(sizeof(BunnySector_Map*) * MAP_AMOUNT);
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


	// Read XML
	/* Reading the xml gives the information about materials used in the maps
	 * Build maps use picnum : s16
	 * Doom maps use names : 8 letters all caps : this is replaced by TextureIndex and names are stored in zstr array
	 * Materials have OpenGL material properties and Bunny properties, like Grass or DrawCallback id
	 * Materials are registered to OpenGL renderer which returns a MaterialId
	 * When the map is loaded,
	 * 	BunnySector knows what picnums or names it uses
	 * 	BunnySector builds a dictionary that matches the picnum or name to MaterialId when
	 * 	it registers the used materials with OpenGLRenderer
	 * When the map is drawn the picnum or name used by material is translated to MaterialId
	 * 	when drawing a wall, ceiling/floor, or sprite
	 */

	OpenGLRender_Init();
	OpenGLRender_RegisterTexture(defaultChecker); // This becomes the default texture

	// Read material definitions from a file
	materialManager = new BunnySector::Materials();
	materialManager->ReadXML("assets/materials.xml");

	BuildRender_Init();
	defaultOpenGL = GetDefaultRenderSettingsOpenGL();
	defaultView = GetDefaultCameraInfo();
	defaultCamera = GetDefaultCamera();

	actorPool.Init(MAP_ACTOR_AMOUNT);

	return true;
}

int BunnySector_LoadMap(const char* mapfilename)
{
	zstr mapname = zstr_from(mapfilename);
	int mapid;
	mapid = BunnySector_LoadMap(mapname);
	zstr_free(&mapname);
	return mapid;
}

int BunnySector_LoadMap(const zstr& mapfilename)
{
	// Do we have this map already?
	int firstFree = -1;
	for (int i = 0; i < MAP_AMOUNT; i++)
	{
		BunnySector_Map* map = mapsArray[i];
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


	BunnyMapType maptype = Map_Invalid;
	if (zstr_contains(&mapfilename, ".map"))
	{
		maptype = Map_Duke;

	}
	else if (zstr_contains(&mapfilename, ".wad"))
	{
		maptype = Map_Doom;
	}
	else
	{
		Log_ErrorF("Unknown map format in file %s\n", zstr_cstr(&mapfilename));
		return -1;
	}
	BunnySector_Map* loaded = nullptr;
	if (maptype == Map_Duke)
	{
		loaded = Duke_ReadMapFromFile(mapfilename);
		loaded->m_type = maptype;
	}
	else if (maptype == Map_Doom)
	{
		loaded = Doom_ReadMapFromFile(mapfilename);
		loaded->m_type = maptype;
		loaded->PrintInfo();
	}
	if (loaded != nullptr)
	{
		// Buffer the floor and ceiling vertices: The uvs need to be calculated first
		float unitsToMeter = maptype == Map_Doom ? DOOM_UNITS_TO_METER : DUKE_UNITS_TO_METER;
		OpenGLRender_CreateFloorBuffers(loaded, unitsToMeter);

		mapsArray[firstFree] = loaded;
		activeMap = loaded;
		return firstFree;
	}
	else
	{
		mgdl_FreeGeneralMemory(loaded);
	}
	return -1;
}


void BunnySector_StartMap(MapId mapId)
{
	if (mapId >=0 && mapId < MAP_AMOUNT)
	{
		BunnySector_Map* map = mapsArray[mapId];
		if (map != nullptr)
		{
			activeMap = map;

			if (map->m_type == Map_Duke)
			{
				RenderSettingsOpenGL_SetUnitToMeter(&defaultOpenGL, DUKE_UNITS_TO_METER);
				BunnySector_SetOpenGLUnitsToMeter(DUKE_UNITS_TO_METER);
			}
			else
			{
				RenderSettingsOpenGL_SetUnitToMeter(&defaultOpenGL, DOOM_UNITS_TO_METER);
				BunnySector_SetOpenGLUnitsToMeter(DOOM_UNITS_TO_METER);
			}
			actorPool.Clear();
			map->SetActorPool(&actorPool);
			map->CreateActors();
			map->AllocateActorCollisions(); // TODO Share with pool?
			player0.Init(0, 2024,6000, 720, 1400);
			Actor* player0Actor = activeMap->GetActorByTypeAndIndex(actor_player, 0);
			printf("BunnySector startmap put actor to %.2f, %.2f, sector %d\n", player0Actor->position.vectorPosition.x, player0Actor->position.vectorPosition.y, player0Actor->subSectorNumber);
			defaultView = player0.GetViewpoint(player0Actor);
			s_AlignCameraToViewpoint(&defaultView, defaultCamera);
		}
	}
}

static const float FIXED_STEP = 1.0f/120.0f;
static float leftOverTime = 0.0f;

// SQUARE ASPECT debugging
bool s_aspectCamera = false;
bool s_aspectView = false;

void BunnySector_UpdateMap(MapId mapId, float deltaTime)
{
	// Map move all actors and sprites etc...
	if (mapId >=0 && mapId < MAP_AMOUNT)
	{
		BunnySector_Map* map = mapsArray[mapId];
		if (map != nullptr)
		{
			deltaTime += leftOverTime;
			while (deltaTime >= FIXED_STEP)
			{
				// Note: to prevent insane delta times when debugging this is done in fixed time
				Actor* player0Actor = activeMap->GetActorByTypeAndIndex(actor_player, 0); // player0.lastActorSubSector); // Start search from where the actor was last time
				player0.ApplyDrive(player0Actor, FIXED_STEP);
				player0.ApplyVerticalMove(player0Actor, FIXED_STEP);

				map->MoveActors(FIXED_STEP);

				player0.prevActorSubSectorNumber = player0Actor->subSectorNumber;

				map->SortMovedActors();
				map->DoActorToActorCollisions();
				// NOTE TODO Call AngelScript function to tell it to process collision results
				map->RemoveDeadActors();
				map->UpdateActions(FIXED_STEP); // This can spawn new actors
				deltaTime -= FIXED_STEP;
			}
			leftOverTime = deltaTime;
		}
	}
}

BunnyMapType BunnySector_GetMapType(MapId mapid)
{

	if (mapid >=0 && mapid < MAP_AMOUNT)
	{
		BunnySector_Map* map = mapsArray[mapid];
		if (map != nullptr)
		{
			return map->m_type;
		}
	}
	return Map_Invalid;

}

void BunnySector_AlignCameraToActor(int actorId)
{
	Viewport viewPort = mgdl_GetViewport();
	// NOTE Must set GL_PROJECTION first then GL_MODELVIEW


	Actor* player0Actor = activeMap->GetActorByTypeAndIndex(actor_player, 0);
	player0Actor->subSectorNumber = activeMap->FindSubSectorV2(player0Actor->subSectorNumber, player0Actor->position.vectorPosition);
	defaultView = player0.GetViewpoint(player0Actor);
	s_AlignCameraToViewpoint(&defaultView, defaultCamera);

	float aspect =  s_aspectCamera;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(defaultCamera->fovY,
				  viewPort.width/viewPort.height,
				   defaultCamera->nearZ,
				defaultCamera->farZ);

	Camera_Apply(defaultCamera); // Sets GL_MODELVIEW


}
void BunnySector_DrawCameraInfo(float x, float y)
{
	y+=16;
	mgdl_DrawTextFloat(zstr_from("Camera x "), defaultCamera->position.x, x, y, 8, Debug_Red);
	mgdl_DrawTextFloat(zstr_from("Camera y "), defaultCamera->position.y, x, y+8, 8, Debug_Red);
	mgdl_DrawTextFloat(zstr_from("Camera z "), defaultCamera->position.z, x, y+16, 8, Debug_Red);
	mgdl_DrawTextFloat(zstr_from("Camera dx"), defaultCamera->direction.x, x, y+16+8, 8, Debug_Red);
	mgdl_DrawTextFloat(zstr_from("Camera dy "), defaultCamera->direction.y, x, y+16+16, 8, Debug_Red);
	mgdl_DrawTextFloat(zstr_from("Camera dz "), defaultCamera->direction.z, x, y+16+24, 8, Debug_Red);

}
void BunnySector_Setup3D(float viewAspect, float cameraAspect)
{
	s_aspectView = viewAspect;
	s_aspectCamera = cameraAspect;
	mgdl_glClearColor32(Debug_Black);

	//mgdl_glSetTransparency(true);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE); //  is this needed?

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glShadeModel(GL_SMOOTH);

	Viewport viewPort = mgdl_GetViewport();
	glViewport(viewPort.left, viewPort.bottom, viewPort.width, viewPort.height);
	switch(activeMap->m_type)
	{
		case Map_Invalid:
			break;
		case Map_Duke:
			RenderSettingsOpenGL_SetUnitToMeter(&defaultOpenGL, bunny_UnitsToMeter);
			OpenGLRender_SetUnitsToMeter(bunny_UnitsToMeter);
			break;
		case Map_Doom:
			RenderSettingsOpenGL_SetUnitToMeter(&defaultOpenGL, bunny_UnitsToMeter);
			OpenGLRender_SetUnitsToMeter(bunny_UnitsToMeter);
			break;
	};
}

void BunnySector_RenderMap(MapId mapId)
{
	if (mapId >=0 && mapId < MAP_AMOUNT)
	{
		BunnySector_Map* map = mapsArray[mapId];
		if (map != nullptr)
		{
			// Set up OpenGL 3D state
			glPushMatrix();

			BunnySector_Setup3D(false, false);

			BunnySector_AlignCameraToActor(0);

			BuildRender_Draw3D(&defaultView, map, &defaultOpenGL);

			glPopMatrix();
		}
	}
}

float BunnySector_GetOpenGLCameraVerticalFOVDeg()
{
	return defaultCamera->fovY;

}
void BunnySector_SetOpenGLCameraVerticalFOVDeg(float degrees)
{
	defaultCamera->fovY = degrees;
}

void BunnySector_SetPlayerDriveInput(int actorId, float forward, float strafe, float vertical, float turnYaw, float turnPitch)
{
	player0.forwardDrive = Clamp(forward, -1.0f, 1.0f);
	player0.strafeDrive = Clamp(strafe, -1.0f, 1.0f);
	player0.verticalDrive = Clamp(vertical, -1.0f, 1.0f);
	player0.turnDrive = Clamp(turnYaw, -1.0f, 1.0f);
}

void BunnySector_SetPlayerSpeeds(int actorId, float walkSpeedMultiplier, float turnSpeedMultiplier)
{
	player0.walkSpeedMultiplier = walkSpeedMultiplier;
	player0.turnSpeedMultiplier = turnSpeedMultiplier;
}

Wall* BunnySector_GetWallEnd(Wall* wall)
{
	return DukeMap_GetWallEnd((DukeMap*)activeMap, wall);
}

Actor* BunnySector_GetPlayerActor(int playerIndex)
{
	return activeMap->GetActorByTypeAndIndex(actor_player, playerIndex);
}

Actor* BunnySector_GetActorById(int actorId)
{
	return activeMap->GetActorById(actorId);
}
Actor* BunnySector_GetActorByIndex(int actorIndex)
{
	return activeMap->GetActorByIndex(actorIndex);
}
void BunnySector_StartMapDrawing()
{
	OpenGLRender_StartDrawingPolygons();
}
void BunnySector_EndMapDrawing()
{
	OpenGLRender_EndDrawingPolygons();
}

void BunnySector_SetOpenGLUnitsToMeter(float scale)
{
	bunny_UnitsToMeter = scale;
}

void BunnySector_DrawWallF(float startx, float startz, float endx, float endz, float normalx, float normalz, s32 floory, s32 ceilingy, s16 picnum, s8 shade)
{
	OpenGLRender_DrawWallV(Vector2New(startx, startz), Vector2New(endx, endz), Vector2New(normalx, normalz), floory, ceilingy, picnum, shade);
}
void BunnySector_DrawWall(Wall* start, Wall* end, s32 floory, s32 ceilingy, s16 picnum, s8 shade)
{
	//printf("BunnySector_DrawWall gets %d, %d \n", start->x, start->z);
	OpenGLRender_DrawWallV(Vector2New(start->x, start->z), Vector2New(end->x, end->z), DukeMap_GetWallNormal((DukeMap*)activeMap, start), floory, ceilingy, picnum, shade);
}

void BunnySector_StartFloorCeilingDrawing()
{
	OpenGLRender_StartDrawingFloorsFromBuffer(activeMap);
}
void BunnySector_DrawSectorFloorOrCeiling(s16 sectorNumber, bool floor)
{
	OpenGLRender_DrawFloorOrCeiling(activeMap, sectorNumber, activeMap->GetSectorShade(sectorNumber, floor), floor ? activeMap->GetFloory(sectorNumber) : activeMap->GetCeilingy(sectorNumber), activeMap->GetSectorMaterial(sectorNumber, floor), floor);
}

void BunnySector_DrawMapActors()
{
	OpenGLRender_DrawActors(activeMap, Vector2New(defaultView.position.x, defaultView.position.z));
}

#define V2_CROSS(ax, ay, bx, by)(ax * by - ay * bx)

bool BunnySector_Intersect(float a1x, float a1y, float a2x, float a2y, float b1x, float b1y, float b2x, float b2y, float& out_x, float& out_y)
{
	float AxA = V2_CROSS(a1x, a1y, a2x, a2y);
	float BxB = V2_CROSS(b1x, b1y, b2x, b2y);
	float alinex = a1x-a2x;
	float aliney = a1y-a2y;
	float blinex = b1x-b2x;
	float bliney = b1y-b2y;
	float det = V2_CROSS(alinex, aliney, blinex, bliney);
	if (det == 0.0)
	{
		 return false;
	}
	out_x = V2_CROSS(AxA, alinex, BxB, blinex)/ det;
	out_y = V2_CROSS(AxA, aliney, BxB, bliney)/ det;

	return true;
}

MaterialId BunnySector_GetMaterialId(zstr* doomTextureFilename)
{
	return materialManager->LoadMaterialByName(doomTextureFilename);
}

DoomMap* BunnySector_GetDoomMap(MapId mapId)
{
	BunnySector_Map* map = mapsArray[mapId];
	return (DoomMap*)map;
}
DukeMap* BunnySector_GetDukeMap(MapId mapId)
{
	BunnySector_Map* map = mapsArray[mapId];
	return (DukeMap*)map;
}
