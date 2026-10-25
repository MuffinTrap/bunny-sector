#if defined(STUPID_WINDOWS)
#include "mgdl.angel"
#include "raymath.angel"
#include "render_utils.cpp"
#include "render_doom.cpp"

//TODO Name this file main_angel.cpp

float angel_unitstometerH = 32.0f;
float angel_unitstometerV = 22.0f;
float angel_textureScale = 112.0f;
float angel_fov = 90.0f;

// NOTE Uncomment for KDevelop intellisense to work
//#include "../src/bunny-sector_main.h"
//#include <mgdl.h>

void angelscript_init()
{
	BunnySector_SetAfterCollisionCallback(angelscript_after_collision);
	BunnySector_SetRenderingCallback(angelscript_render);

	// Match 2D render to OpenGL render
	//RenderInit(BunnySector_GetOpenGLCameraVerticalFOVDeg());
}

void angelscript_quit()
{

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

	mgdl_DrawTextFloat("Horizontal unitspm", angel_unitstometerH, text_x, NextY(), 8, Debug_Red);
	mgdl_DrawTextFloat("Vertical unitspm", angel_unitstometerV, text_x, NextY(), 8, Debug_Green);
	mgdl_DrawTextFloat("Texture scale", angel_textureScale, text_x, NextY(), 8, Debug_Yellow);

	End2D();
}
void adjustScale(float deltatime)
{
	int playerIndex =0;

		if (mgdl_IsButtonPressed(playerIndex, ButtonUp))
		{
			angel_unitstometerV += 1;
		}
		if (mgdl_IsButtonPressed(playerIndex, ButtonDown))
		{
			angel_unitstometerV -= 1;
			if (angel_unitstometerV < 1.0)
			{
				angel_unitstometerV = 1;
			}
		}
		if (mgdl_IsButtonPressed(playerIndex, ButtonLeft))
		{
			angel_textureScale -= 1;
			if (angel_textureScale < 1.0)
			{
				angel_textureScale = 1;
			}
		}
		if (mgdl_IsButtonPressed(playerIndex, ButtonRight))
		{
			angel_textureScale += 1;
		}
}

void adjustFov(float deltatime)
{
	// Minus resets the fov
	if (mgdl_IsButtonPressed(0, ButtonMinus))
	{
		SetVerticalFovDeg(angel_fov);
		BunnySector_SetOpenGLCameraVerticalFOVDeg(angel_fov);
	}

	float fovchange = 0.0f;
	if (mgdl_IsButtonPressed(0, ButtonLeft))
	{
		fovchange = 1;
	}
	else if (mgdl_IsButtonPressed(0, ButtonRight))
	{
		fovchange = -1.0f;
	}
	if (fovchange != 0.0f)
	{
		SetVerticalFovDeg(GetVerticalFovDeg() + fovchange);
		BunnySector_SetOpenGLCameraVerticalFOVDeg(BunnySector_GetOpenGLCameraVerticalFOVDeg() + fovchange);
	}
}

void angelscript_after_collision()
{
	// This is called after all collisions are registered
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
			RenderDoomMapLines(BunnySector_GetDoomMap());
		}

		//RenderMiniMapDoom(BunnySector_GetDoomMap());
		//angelscript_render_player_info();
	}

	//DrawDebugs();

	RENDER_2D_WALLS = false;
	DEBUG_DRAW = false;
	DEBUG_LOG = false;
}

void angelscript_frame(float deltatime)
{
	if (mgdl_IsButtonDown(0, Button2))
	{
		RENDER_2D_WALLS = true;
	}
	if (mgdl_IsButtonDown(0, Button1))
	{
		adjustFov(deltatime);
	//	DEBUG_DRAW = true;
	}
	else
	{
		adjustScale(deltatime);
	}

	// Check game state
}
#endif
