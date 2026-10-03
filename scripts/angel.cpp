#include "mgdl.angel"
#include "raymath.angel"
#include "render_utils.cpp"
#include "render_duke.cpp"
#include "render_doom.cpp"

//TODO Name this file main_angel.cpp

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

	BunnySector_SetAfterCollisionCallback(angelscript_after_collision);
	BunnySector_SetRenderingCallback(angelscript_render);

	doomMapId = BunnySector_LoadMap("assets/slade_test.wad");
	//dukeMapId = BunnySector_LoadMap("assets/doome1m1.map");

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

void Start2D()
{
	int screenWidth = mgdl_GetScreenWidth();
	int screenHeight = mgdl_GetScreenHeight();

glPushMatrix();

	float screen_half_width = screenWidth / 2.0f;
	float screen_half_height = screenHeight / 2.0f;
	glTranslatef(screen_half_width, screen_half_height, 0);

}
void End2D()
{
	glPopMatrix();

}

void DrawDebugs()
{
	Start2D();

	End2D();
}

void movePlayer(int playerIndex, float deltatime)
{

	float forward = 0;
	float strafe = 0.0f;
	float turn = 0.0f;
	float vertical = 0.0f;


	if (mgdl_IsButtonDown(playerIndex, ButtonUp))
	{
		vertical = 1.0f;
	}
	else if (mgdl_IsButtonDown(playerIndex, ButtonDown))
	{
		vertical = -1.0f;
	}
	else
	{
		vertical = 0.0f;
		Actor@ actor = BunnySector_GetPlayerActor(playerIndex);
		actor.verticalVelocity = 0.0f;
	}

	Vector2 wasd = mgdl_GetJoystick(playerIndex, Joystick_Nunchuk);
	forward = -wasd.y;
	turn = wasd.x;

	BunnySector_SetPlayerDriveInput(playerIndex, forward, strafe, vertical, turn, 0.0f);

	// Use button is A for now
	if (mgdl_IsButtonDown(playerIndex, ButtonA))
	{
		BunnySector_StartPlayerAction(playerIndex, action_use);
	}
	else if (mgdl_IsButtonReleased(playerIndex, ButtonA))
	{
		BunnySector_StopPlayerAction(playerIndex, action_use);
	}

	if (mgdl_IsButtonDown(playerIndex, ButtonB))
	{
		BunnySector_StartPlayerAction(playerIndex, action_shoot);
	}
	else if (mgdl_IsButtonReleased(playerIndex, ButtonB))
	{
		BunnySector_StopPlayerAction(playerIndex, action_shoot);
	}

	if (mgdl_IsButtonDown(playerIndex, ButtonC))
	{
		BunnySector_StartPlayerAction(playerIndex, action_jump);
	}
	else if (mgdl_IsButtonReleased(playerIndex, ButtonC))
	{
		BunnySector_StopPlayerAction(playerIndex, action_jump);
	}

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
	movePlayer(0, deltatime);
}

void angelscript_after_collision()
{
	// This is called after all collisions are registered
	Actor@ player0 = BunnySector_GetPlayerActor(0);
	int collisions = BunnySector_GetActorCollisionAmount(player0);
	if (collisions > 0)
	{
		mgdl_LogTextInt("Player hit this many other actors ", collisions);
	}
	for (int i = 0; i < collisions; i++)
	{
		Actor@ other = BunnySector_GetActorCollisionAt(player0, i);
		mgdl_LogText("Player collision with ");
		mgdl_LogText(ActorTypeToString(other.actorType));
		BunnySector_GivePlayerItem(0, editorNumber_blue_card, 1);
		BunnySector_DestroyActor(other);
	}
}

void angelscript_render_menu()
{
	Start2D();
	mgdl_DrawText("Menu", text_x, NextY(32), 32, Debug_LightYellow);
	mgdl_DrawText("Press (A) to start", text_x, NextY(), 16, Debug_LightBlue);
	End2D();
	if (mgdl_IsButtonPressed(0, ButtonA))
	{
		BunnySector_StartMap(doomMapId, 1);
		BunnySector_SetGameStatus(status_player_alive);
	}
}

void angelscript_render_exit_menu()
{
	Start2D();
		// Map is over
		mgdl_DrawText("Exit map", text_x, NextY(32), 32, Debug_LightYellow);
		mgdl_DrawText("Press (A) and (B) together to restart", text_x, NextY(), 8, Debug_LightBlue);

	End2D();

	if (mgdl_IsButtonDown(0, ButtonA) && mgdl_IsButtonDown(0, ButtonB))
	{
		BunnySector_StartMap(doomMapId, 1);
		BunnySector_SetGameStatus(status_player_alive);
	}
}

void angelscript_render_player_info()
{
	Start2D();
		int bluecards = BunnySector_GetPlayerItemCount(0, editorNumber_blue_card);
		mgdl_DrawTextInt("Blue cards", bluecards, text_x, NextY(), 8, Debug_LightBlue);
		Actor@ player0 = BunnySector_GetPlayerActor(0);
		if (player0.IsDoing(action_use))
		{
			mgdl_DrawText("Player is using", text_x, NextY(16), 16, Debug_LightBlue);
		}
	End2D();
}

void angelscript_render()
{
	float aspect = mgdl_GetScreenWidth()/mgdl_GetScreenHeight();
	glClearColor(0.3f, 0.2f, 0.3f, 1.0f);

	if (mgdl_IsButtonDown(0, ButtonC))
	{
		DEBUG_LOG = true;
	}

	StartFrame();

	GameStatus status = BunnySector_GetGameStatus();
	if (status == status_player_alive)
	{
		StartFrame_Doom();
		if (RENDER_2D_WALLS)
		{
			RenderDoomMapLines(BunnySector_GetDoomMap(doomMapId));
		}
		else
		{
			BunnySector_Setup3D(aspect, aspect);
			BunnySector_AlignCameraToPlayer(0);
			BunnySector_StartMapDrawing();
			RenderDoomMap(BunnySector_GetDoomMap(doomMapId));
			BunnySector_DrawMapActorsForPlayer(0);
			BunnySector_EndMapDrawing();
		}

		RenderMiniMapDoom(BunnySector_GetDoomMap(doomMapId));
		angelscript_render_player_info();
	}
	else if (status == status_menu)
	{
		angelscript_render_menu();
	}
	else if (status == status_exit_normal)
	{
		angelscript_render_exit_menu();
	}

	DrawDebugs();

	RENDER_2D_WALLS = false;
	DEBUG_DRAW = false;
		DEBUG_LOG = false;
}

void angelscript_frame_duke(float deltatime)
{
	BunnySector_SetPlayerSpeeds(0, 1.0f, 0.7f);
	movePlayer(0, deltatime);
	adjustFov(deltatime);



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
		BunnySector_AlignCameraToPlayer(0);
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

		GameStatus status = BunnySector_GetGameStatus();
		if (status == status_player_alive)
		{
			angelscript_frame_doom(deltatime);
		}
		//angelscript_frame_duke(deltatime);

	// Check game state

}
bool actest = true;


#if USE_ANGEL_AS_CPP
#	ifdef __cplusplus
		}
#	endif
#endif
