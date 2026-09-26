#include "mgdl.angel"
#include "raymath.angel"
#include "render_utils.cpp"
#include "render_duke.cpp"
#include "render_doom.cpp"

float angel_unitstometer = 32.0f;

// NOTE Uncomment for KDevelop intellisense to work
//#include "../src/bunny-sector_main.h"
//#include <mgdl.h>

#if USE_ANGEL_AS_CPP
#	include <mgdl.h>
#	include <mgdl/raymath/raymath.h>
#	include <mgdl/mgdl-script-api.h>
#	include "angel.hpp"
#	include <mgdl/mgdl-angelscript.h>
#	ifdef __cplusplus
		extern "C" {
#	endif
#endif

MapId dukeMapId;
MapId doomMapId;

void angelscript_init()
{
	int screenWidth = mgdl_GetScreenWidth();
	int screenHeight = mgdl_GetScreenHeight();

	BunnySector_Init();
	doomMapId = BunnySector_LoadMap("assets/slade_test.wad");
	//dukeMapId = BunnySector_LoadMap("assets/doome1m1.map");
	BunnySector_StartMap(doomMapId);

	// Match 2D render to OpenGL render
	RenderInit(BunnySector_GetOpenGLCameraVerticalFOVDeg());

}

void angelscript_quit()
{

}

void setup_3d()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE); //  is this needed?

	// This is the other way around on Wii, but
	// hopefully OpenGX handles it
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glShadeModel(GL_SMOOTH);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	mgdl_SetGlobalAmbientColor32(Debug_White, 0.2f);

    glColor3f(1.0f, 1.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	int screenWidth = mgdl_GetScreenWidth();
	int screenHeight = mgdl_GetScreenHeight();
	float aspect = float(screenWidth)/float(screenHeight);
	float nearZ = 0.01f;
	float farZ = 100.0f;
    gluPerspective(60.0f, aspect, nearZ, farZ);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	gluLookAt(0.0f, 5.0f, 10.0f,
				 0.0f, 0.0f, 0.0f,
				 0.0f, 1.0f, 0.0);

}

void DrawDrive(int x, int y, float amount)
{
	if (amount < 0)
	{
		x += amount*100;
		amount = amount * -1.0f;
	}
	mgdl_DrawRectangle(x,y, amount * 100, 8, Debug_Yellow);
}

void DrawDebugs()
{
	int screenWidth = mgdl_GetScreenWidth();
	int screenHeight = mgdl_GetScreenHeight();

}

void movePlayer(float deltatime)
{

	float forward = 0;
	float strafe = 0.0f;
	float turn = 0.0f;
	float vertical = 0.0f;


	if (mgdl_IsButtonDown(0, ButtonUp))
	{
		vertical = 1.0f;
	}
	else if (mgdl_IsButtonDown(0, ButtonDown))
	{
		vertical = -1.0f;
	}
	else
	{
		vertical = 0.0f;
		Actor@ actor = BunnySector_GetPlayerActor(0);
		actor.verticalVelocity = 0.0f;
	}

	Vector2 wasd = mgdl_GetJoystick(0, Joystick_Nunchuk);
	forward = -wasd.y;
	turn = wasd.x;

	BunnySector_SetPlayerDriveInput(0, forward, strafe, vertical, turn, 0.0f);

}

void adjustFov(float deltatime)
{

	// Minus resets the fov
	if (mgdl_IsButtonPressed(0, ButtonMinus))
	{
		SetVerticalFovDeg(80.0f);
		BunnySector_SetOpenGLCameraVerticalFOVDeg(80.0f);
	}

	float fovchange = 0.0f;
	if (mgdl_IsButtonDown(0, ButtonLeft))
	{
		fovchange = 10.0f;
	}
	else if (mgdl_IsButtonDown(0, ButtonRight))
	{
		fovchange = -10.0f;
	}
	if (fovchange != 0.0f)
	{
		SetVerticalFovDeg(GetVerticalFovDeg() + fovchange * deltatime);
		BunnySector_SetOpenGLCameraVerticalFOVDeg(BunnySector_GetOpenGLCameraVerticalFOVDeg() + fovchange * deltatime);
	}
}
void angelscript_frame_doom(float deltatime)
{
	BunnySector_SetOpenGLUnitsToMeter(angel_unitstometer);
	BunnySector_SetPlayerSpeeds(0, 1.0f, 1.0f);
	movePlayer(deltatime);

	if (mgdl_IsButtonDown(0, ButtonZ))
	{
		// TODO Set noclip
	}
	else
	{
		BunnySector_UpdateMap(doomMapId, deltatime);
	}

	float aspect = mgdl_GetScreenWidth()/mgdl_GetScreenHeight();
	glClearColor(0.3f, 0.2f, 0.3f, 1.0f);

	if (mgdl_IsButtonDown(0, ButtonC))
	{
		DEBUG_LOG = true;
	}
	StartFrame();
	StartFrame_Doom();
	if (RENDER_2D_WALLS)
	{
		RenderDoomMapLines(BunnySector_GetDoomMap(doomMapId));
	}
	else
	{
		BunnySector_Setup3D(aspect, aspect);
		BunnySector_AlignCameraToActor(0);
		BunnySector_StartMapDrawing();
			RenderDoomMap(BunnySector_GetDoomMap(doomMapId));
			BunnySector_DrawMapActors();
		BunnySector_EndMapDrawing();
	}

	RenderMiniMapDoom(BunnySector_GetDoomMap(doomMapId));

	DrawDebugs();
}

void angelscript_frame_duke(float deltatime)
{
	BunnySector_SetPlayerSpeeds(0, 1.0f, 0.7f);
	movePlayer(deltatime);
	adjustFov(deltatime);

	if (mgdl_IsButtonDown(0, ButtonZ))
	{
		// TODO noclip
	}
	else
	{
		BunnySector_UpdateMap(dukeMapId, deltatime);
	}


	glClearColor(0.3f, 0.2f, 0.3f, 1.0f);

	StartFrame_Duke();

	DukeMap@ map = BunnySector_GetDukeMap(dukeMapId);
	// Hold down 2 to see software render result
	float aspect = mgdl_GetScreenWidth()/mgdl_GetScreenHeight();
	if (RENDER_2D_WALLS)
	{
		RenderMapSoftware(map, deltatime);
	}
	else
	{
		//BunnySector_RenderMap(dukeMapId); // This calls the old build render stuff
		BunnySector_Setup3D(aspect, aspect);
		BunnySector_AlignCameraToActor(0);
		BunnySector_StartMapDrawing();
			RenderMap(map, deltatime);
		BunnySector_EndMapDrawing();
	}

	RenderMiniMap(map);

	DrawDebugs();
}

void angelscript_frame(float deltatime)
{
	if (mgdl_IsButtonDown(0, Button2))
	{
		RENDER_2D_WALLS = true;
	}
	if (mgdl_IsButtonDown(0, Button1))
	{
		DEBUG_DRAW = true;
	}
		angelscript_frame_doom(deltatime);
		//angelscript_frame_duke(deltatime);

	RENDER_2D_WALLS = false;
	DEBUG_DRAW = false;
		DEBUG_LOG = false;
}

#if USE_ANGEL_AS_CPP
#	ifdef __cplusplus
		}
#	endif
#endif
