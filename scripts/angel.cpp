#include "mgdl.angel"
#include "raymath.angel"
#include "render2d.cpp"

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

MapId testMapId;

void angelscript_init()
{
	int screenWidth = mgdl_GetScreenWidth();
	int screenHeight = mgdl_GetScreenHeight();

	BunnySector_Init();
	testMapId = BunnySector_LoadMap("assets/test_room.map");
	BunnySector_StartMap(testMapId);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

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

	Actor@ player = buns_GetActor(0);
	float playerAngle = player.yawRad;

	buns_Vec2 outPlayerDir;
	buns_GetActorFloorDir(0, outPlayerDir);

	buns_Vec2 outPlayerPos;
	buns_GetActorPositionV2(0, outPlayerPos);
	Vector2 playerPos = Vector2(outPlayerPos.x, outPlayerPos.y);

	mgdl_DrawTextFloat("Player x: ", outPlayerPos.x, 16, 16, 16, Debug_Yellow);
	mgdl_DrawTextFloat("Player y: ", outPlayerPos.y, 16, 32, 16, Debug_Yellow);
	mgdl_DrawTextFloat("Player d: ", RAD2DEG * playerAngle, 16, 48, 16, Debug_Yellow);
	mgdl_DrawTextFloat("Player fx: ", outPlayerDir.x, 16, 48+16, 16, Debug_Yellow);
	mgdl_DrawTextFloat("Player fy: ", outPlayerDir.y, 16, 48+32, 16, Debug_Yellow);
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
	}

	if (mgdl_IsButtonDown(0, ButtonLeft))
	{
		turn = -1.0f;
	}
	else if (mgdl_IsButtonDown(0, ButtonRight))
	{
		turn = 1.0f;
	}
	else
	{
		turn = 0.0f;
	}

	Vector2 wasd = mgdl_GetJoystick(0, Joystick_Nunchuk);
	forward = -wasd.y;
	turn = wasd.x;

	BunnySector_SetActorDriveInput(0, forward, strafe, vertical, turn, 0.0f);
	BunnySector_UpdateMap(testMapId, deltatime);
}

void render3d(float deltatime)
{
	BunnySector_RenderMap(testMapId);

	DrawDebugs();

}


void angelscript_frame(float deltatime)
{
	movePlayer(deltatime);
	RenderMap(deltatime);

	DrawDebugs();
}

#if USE_ANGEL_AS_CPP
#	ifdef __cplusplus
		}
#	endif
#endif
