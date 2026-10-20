#if defined(_WIN32)
#	define _CRT_SECURE_NO_WARNINGS
#endif
#include <mgdl.h>

#include <mgdl/mgdl-script-api.h>

#include "bunny-sector-types.h"
#include "bunny-sector-math.h"
#include "bunny-sector_main.h"
#include "bunny-sector-map.h"
#include "bunny-sector-materials.h"
#include "bunny-sector-items.h"
#include "bunny-sector-players.h"
#include "bunny-sector-game.h"

#include "render/opengl-render.h"
#include "render/render-settings.h"

#include "gameplay/actor.h"
#include "gameplay/player.h"
#include "gameplay/actorpool.h"

#include "doom/doom-map-reader.h"
#include "doom/doom_render.h"

#if defined(USE_ANGEL_AS_CPP)
#	include "../scripts/angel.hpp"
#endif

static BunnySector::MaterialManager* materialManager;
static BunnySector::ItemManager* itemManager;
static PlayerManager* playerManager;
static Game game;

static RenderSettingsOpenGL defaultOpenGL;

static Camera* defaultCamera = nullptr;
static Viewpoint defaultView;

static Player players[4];
static Viewpoint playerViews[4];
static Camera* playerCameras[4];
static int activePlayerAmount = 1;

static Texture* defaultChecker = nullptr;

// All maps share the same actor pool
// Map creates actors when it is loaded
static ActorPool actorPool;

static asIScriptFunction* AfterCollisionCallback;
static asIScriptFunction* RenderingCallback;
static mgdl_AngelScript* angelContenxt;

static float bunny_UnitsToMeter_horizontal = 1.0f;
static float bunny_UnitsToMeter_vertical = 1.0f;

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

bool BunnySector_Init(mgdl_AngelScript* angel)
{
	angelContenxt = angel;

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
	materialManager = new BunnySector::MaterialManager();
	bool materialsloaded = materialManager->ReadXML("config/materials.xml");

	itemManager = new BunnySector::ItemManager();
	bool itemsloaded = itemManager->LoadXML("config/items.xml");

	playerManager = new PlayerManager();
	bool playersloaded = playerManager->ReadXML("config/players.xml");

	defaultOpenGL = GetDefaultRenderSettingsOpenGL();
	defaultView = GetDefaultCameraInfo();
	defaultCamera = GetDefaultCamera();


	for (int i = 0; i < 4; i++)
	{
		playerCameras[i] = GetDefaultCamera();
		playerViews[i] = GetDefaultCameraInfo();
	}

	actorPool.Init(MAP_ACTOR_AMOUNT);

	// Needs Camara to be set
	InitDoomRender(BunnySector_GetOpenGLCameraVerticalFOVDeg());

	// Read game configuration
	bool gameloaded = game.ReadXML("config/game.xml");

	return (materialsloaded && itemsloaded && playersloaded && gameloaded);
}

void BunnySector_SetRenderingCallback(asIScriptFunction* callbackFunction)
{
	RenderingCallback = callbackFunction;
}

void DoRenderCallback()
{
	mgdl_PrepareAngelScriptFunctionCall(angelContenxt, RenderingCallback);
	mgdl_CommitAngelScriptFunctionCall(angelContenxt, RenderingCallback);
}

void DoCollisionCallback()
{
	mgdl_PrepareAngelScriptFunctionCall(angelContenxt, AfterCollisionCallback);
	mgdl_CommitAngelScriptFunctionCall(angelContenxt, AfterCollisionCallback);
}

void BunnySector_HandleDefaultCollisions()
{
	for (int i = 0; i < activePlayerAmount; i++)
	{
		Actor* playerActor = actorPool.GetActorByTypeAndIndex(ActorType::actor_player, i);
		// Handle all collisions that script did not handle
		int collisions = actorPool.GetCollisionAmountForActor(playerActor);
		for (int i = 0; i < collisions; i++)
		{
			Actor* other = actorPool.GetCollisionForActor(playerActor, i);
			if (other->actorType == actor_item)
			{
				BunnySector_GivePlayerItem(i, other->typeNumber, 1); // TODO how many?
				BunnySector_DestroyActor(other);
			}
		}
	}
}

void BunnySector_SetAfterCollisionCallback(asIScriptFunction* callbackFunction)
{
	AfterCollisionCallback = callbackFunction;
}

void BunnySector_StartMap(int playerAmount)
{
	BunnySector_Map* map = game.GetActiveMap();
	if (map != nullptr)
	{
		activePlayerAmount = playerAmount;

		RenderSettingsOpenGL_SetUnitToMeter(&defaultOpenGL, DOOM_UNITS_TO_METER, DOOM_UNITS_TO_METER);
		BunnySector_SetOpenGLUnitsToMeter(DOOM_UNITS_TO_METER, DOOM_UNITS_TO_METER_V);
		actorPool.Clear();
		map->SetActorPool(&actorPool);
		map->CreateActors();
		map->ClearActions();
		// NOTE player numbers, not index
		for (int i = 0; i < activePlayerAmount; i++)
		{
			int playerNumber = i + 1;
			int playerIndex = i;
			playerManager->LoadPlayer(&players[playerIndex], playerNumber);
			Actor* playerActor = map->GetActorByTypeAndIndex(actor_player, playerIndex);
			players[i].ConfigureActor(playerActor);
			printf("BunnySector startmap put actor to %.2f, %.2f, sector %d\n", playerActor->position.vectorPosition.x, playerActor->position.vectorPosition.y, playerActor->subSectorNumber);
			playerViews[i] = players[i].GetViewpoint(playerActor);

			s_AlignCameraToViewpoint(&playerViews[i], playerCameras[i]);
		}

		game.StartActiveMap();
	}
}

static const float FIXED_STEP = 1.0f/120.0f;
static float leftOverTime = 0.0f;

// SQUARE ASPECT debugging
bool s_aspectCamera = false;
bool s_aspectView = false;

void BunnySector_SetGameStatus(GameStatus status)
{
	game.status = status;
}

GameStatus BunnySector_GetGameStatus()
{
	return game.status;
}

void BunnySector_Update(float deltaTime)
{
	switch(game.status)
	{
		case status_menu:
			if (game.UpdateMenu())
			{
				if (game.LoadMap(game.GetFirstMapNumber()))
				{
					// TODO Get player amount from game
					BunnySector_StartMap(1); // NOTE Sets actor pool and creates players

					// TODO Show map preview and name before starting
					game.StartActiveMap();
				}
				else
				{
					Log_Error("Game could not load first map");
				}
			}
			break;
		case status_player_alive:
		{

			BunnySector_Map* activeMap = game.GetActiveMap();
			if (activeMap != nullptr)
			{
				BunnySector_UpdateMapPtr(activeMap, deltaTime);
				BunnySector_DrawActiveMap();
			}
		}

			break;
		case status_player_dead:
			// NOTE Draw something but don't unload the map, player might revive
			break;
		case status_exit_normal:
			if (game.UpdateMapEndMenu())
			{
				int nextMapNumber = game.GetNextMapNumber(game.GetActiveMap()->number);
				if (nextMapNumber >= 0)
				{
					if (game.LoadMap(nextMapNumber))
					{
						game.StartActiveMap();
					}
				}
			}
			break;
	}

	// Draw after updating
	DoRenderCallback();
}

void BunnySector_UpdateMapPtr(BunnySector_Map* map, float deltaTime)
{
	if (map != nullptr)
	{

		for (int pi = 0; pi < activePlayerAmount; pi++)
		{
			players[pi].ReadInput(pi);
			// Check if player is trying to use something
			Actor* playerActor = map->GetActorByTypeAndIndex(actor_player, pi); // player0.lastActorSubSector); // Start search from where the actor was last time
			if (playerActor->IsDoing(action_use))
			{
				Rayhit usehit = map->CastRayInMap(playerActor->position.vectorPosition, playerActor->moveDirection.vectorDirection, playerActor->height, playerActor->size, playerActor->subSectorNumber);
				if (usehit.hit)
				{
					map->TryUseWall(playerActor, usehit.wallIndex);
				}
			}
		}

		deltaTime += leftOverTime;
		while (deltaTime >= FIXED_STEP)
		{
			for (int pi = 0; pi < activePlayerAmount; pi++)
			{
				// Note: to prevent insane delta times when debugging this is done in fixed time
				Actor* playerActor = map->GetActorByTypeAndIndex(actor_player, pi); // player0.lastActorSubSector); // Start search from where the actor was last time
				players[pi].ApplyDrive(playerActor, FIXED_STEP);
				players[pi].ApplyVerticalMove(playerActor, FIXED_STEP);
				playerActor->actionFlags = players[pi].actionFlags;
				players[pi].prevActorSubSectorNumber = playerActor->subSectorNumber;

			}
			map->MoveActors(FIXED_STEP);
			map->SortMovedActors();

			actorPool.DoActorToActorCollisions();
			DoCollisionCallback();
			BunnySector_HandleDefaultCollisions();
			actorPool.RemoveDeadActors();

			MapUpdateResult upresult = map->UpdateActions(FIXED_STEP); // This can spawn new actors
			if (upresult)
			{
				game.status = status_exit_normal;
				// TODO What is the next map?
			}
			deltaTime -= FIXED_STEP;
		}
		leftOverTime = deltaTime;
	}
}


// NOTE
// DRAWING FUNCTIONS

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

	RenderSettingsOpenGL_SetUnitToMeter(&defaultOpenGL, bunny_UnitsToMeter_horizontal, bunny_UnitsToMeter_vertical);
	OpenGLRender_SetUnitsToMeter(bunny_UnitsToMeter_horizontal, bunny_UnitsToMeter_vertical);
}

void BunnySector_AlignCameraToPlayer(int playerIndex)
{
	// TODO Different viewports for each player

	BunnySector_Map* activeMap = game.GetActiveMap();

	Viewport viewPort = mgdl_GetViewport();
	// NOTE Must set GL_PROJECTION first then GL_MODELVIEW


	Actor* playerActor = activeMap->GetActorByTypeAndIndex(actor_player, playerIndex);
	playerActor->subSectorNumber = activeMap->FindSubSectorV2(playerActor->subSectorNumber, playerActor->position.vectorPosition);
	playerViews[playerIndex] = players[playerIndex].GetViewpoint(playerActor);
	s_AlignCameraToViewpoint(&playerViews[playerIndex], playerCameras[playerIndex]);

	float aspect =  s_aspectCamera;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// TODO Amount of players changes the viewport / copy from ALASA
	// Skew the aspect just a bit to get the doom rendering
	gluPerspective(defaultCamera->fovY,
				  (viewPort.width/viewPort.height),
				   defaultCamera->nearZ,
				defaultCamera->farZ);

	Camera_Apply(playerCameras[playerIndex]); // Sets GL_MODELVIEW
}

void BunnySector_StartMapDrawing()
{
	OpenGLRender_StartDrawingPolygons();
}
void BunnySector_EndMapDrawing()
{
	OpenGLRender_EndDrawingPolygons();
}

void DrawSkyBox()
{
	Camera_SetPositionV(defaultCamera, Vector3Zero());
	Camera_SetDirection(defaultCamera, mgdl_GetGLWorldForward());

	Viewport viewPort = mgdl_GetViewport();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// TODO Amount of players changes the viewport / copy from ALASA
	gluPerspective(defaultCamera->fovY,
				  viewPort.width/viewPort.height,
				   defaultCamera->nearZ,
				defaultCamera->farZ);

	Camera_Apply(defaultCamera);

	// Don't write the Skybox to depth buffer so that everything is drawn on top
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);
	glCullFace(GL_FRONT);
	OpenGLRender_DrawSkybox(game.GetActiveMap());
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);


}

void BunnySector_DrawActiveMap()
{
	float aspect = mgdl_GetScreenWidth()/mgdl_GetScreenHeight();
	BunnySector_Setup3D(aspect, aspect);
	for (int pi = 0; pi < activePlayerAmount; pi++)
	{
		// Set camera to center for skybox
		DrawSkyBox();

		BunnySector_AlignCameraToPlayer(0);
		BunnySector_StartMapDrawing();
			RenderDoomMap( (DoomMap*)game.GetActiveMap());
			BunnySector_DrawMapActorsForPlayer(0);
		BunnySector_EndMapDrawing();
	}
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

float BunnySector_GetOpenGLCameraVerticalFOVDeg()
{
	return defaultCamera->fovY;

}
void BunnySector_SetOpenGLCameraVerticalFOVDeg(float degrees)
{
	defaultCamera->fovY = degrees;
}

void BunnySector_SetPlayerDriveInput(int playerIndex, float forward, float strafe, float vertical, float turnYaw, float turnPitch)
{
	players[playerIndex].forwardDrive = Clamp(forward, -1.0f, 1.0f);
	players[playerIndex].strafeDrive = Clamp(strafe, -1.0f, 1.0f);
	players[playerIndex].verticalDrive = Clamp(vertical, -1.0f, 1.0f);
	players[playerIndex].turnDrive = Clamp(turnYaw, -1.0f, 1.0f);
}

void BunnySector_SetPlayerSpeeds(int playerIndex, float walkSpeedMultiplier, float turnSpeedMultiplier)
{
	players[playerIndex].walkSpeedMultiplier = walkSpeedMultiplier;
	players[playerIndex].turnSpeedMultiplier = turnSpeedMultiplier;
}

void BunnySector_StartPlayerAction(int playerIndex, ActorActionBit action)
{
	players[playerIndex].StartAction(action);
}

void BunnySector_StopPlayerAction(int playerIndex, ActorActionBit action)
{
	players[playerIndex].StopAction(action);
}

Actor* BunnySector_GetPlayerActor(int playerIndex)
{

	BunnySector_Map* activeMap = game.GetActiveMap();
	return activeMap->GetActorByTypeAndIndex(actor_player, playerIndex);
}

void BunnySector_DestroyActor(Actor* actor)
{
	actor->lastMoveResultFlags = Flag_SetBit(actor->lastMoveResultFlags, MoveResultBit::Move_Dead);
}

bool BunnySector_GivePlayerItem(int playerIndex, int itemType, int amount)
{
	return players[playerIndex].GiveItem((DOOM_EDITOR_NUMBER)itemType, amount);
}

int BunnySector_GetPlayerItemCount(int playerIndex, int itemtype)
{
	return players[playerIndex].GetItemCount((DOOM_EDITOR_NUMBER)itemtype);
}

Player* BunnySector_GetPlayer(int playerIndex)
{
	return &players[playerIndex];
}

Actor* BunnySector_GetActorById(int actorId)
{

	BunnySector_Map* activeMap = game.GetActiveMap();
	return activeMap->GetActorById(actorId);
}
Actor* BunnySector_GetActorByIndex(int actorIndex)
{

	BunnySector_Map* activeMap = game.GetActiveMap();
	return activeMap->GetActorByIndex(actorIndex);
}

int BunnySector_GetActorCollisionAmount(Actor* actor)
{
	return actorPool.GetCollisionAmountForActor(actor);
}

Actor* BunnySector_GetActorCollisionAt(Actor* actor, int index)
{
	return actorPool.GetCollisionForActor(actor, index);
}


void BunnySector_SetOpenGLUnitsToMeter(float horizontalScale, float verticalScale)
{
	bunny_UnitsToMeter_horizontal = horizontalScale;
	bunny_UnitsToMeter_vertical = verticalScale;
}

void BunnySector_DrawWallF(float startx, float startz, float endx, float endz, float normalx, float normalz, s32 floory, s32 ceilingy, s16 picnum, s8 shade, u32 linedef_flags)
{
	OpenGLRender_DrawWallV(Vector2New(startx, startz), Vector2New(endx, endz), Vector2New(normalx, normalz), floory, ceilingy, picnum, shade, linedef_flags);
}

void BunnySector_StartFloorCeilingDrawing()
{
	BunnySector_Map* activeMap = game.GetActiveMap();
	OpenGLRender_StartDrawingFloorsFromBuffer(activeMap);
}
void BunnySector_DrawSectorFloorOrCeiling(s16 sectorNumber, bool floor)
{

	BunnySector_Map* activeMap = game.GetActiveMap();
	OpenGLRender_DrawFloorOrCeiling(activeMap, sectorNumber, activeMap->GetSectorShade(sectorNumber, floor), floor ? activeMap->GetFloory(sectorNumber) : activeMap->GetCeilingy(sectorNumber), activeMap->GetSectorMaterial(sectorNumber, floor), floor);
}

void BunnySector_DrawMapActorsForPlayer(int playerIndex)
{

	BunnySector_Map* activeMap = game.GetActiveMap();
	Viewpoint point = playerViews[playerIndex];
	OpenGLRender_DrawActors(activeMap, Vector2New(point.position.x, point.position.z));
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

DoomMap* BunnySector_GetDoomMap()
{
	BunnySector_Map* activeMap = game.GetActiveMap();
	return (DoomMap*)activeMap;
}

BunnySector::ItemManager* BunnySector_GetItemManager() { return itemManager;}
BunnySector::MaterialManager* BunnySector_GetMaterialManager() { return materialManager;}
