#include "mgdl.angel"
#include "raymath.angel"

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


void angelscript_frame(float deltatime)
{
	float xmove = 0.0f;
	float ymove = 0.0f;
	float turnMove = 0.0f;
	if (mgdl_IsButtonDown(0, ButtonUp))
	{
		ymove = 100.0f;
	}
	else if (mgdl_IsButtonDown(0, ButtonDown))
	{
		ymove = -100.0f;
	}
	if (mgdl_IsButtonDown(0, ButtonLeft))
	{
		xmove = -10.0f;
	}
	else if (mgdl_IsButtonDown(0, ButtonRight))
	{
		xmove = 10.0f;
	}
	BunnySector_ChangeCameraTransform(xmove*deltatime,ymove*deltatime,0,deltatime,0);
	BunnySector_RenderMap(testMapId);
}

#if USE_ANGEL_AS_CPP
#	ifdef __cplusplus
		}
#	endif
#endif
