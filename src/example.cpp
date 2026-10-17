
#include "example.h"
#include <mgdl/mgdl-script-api.h>
#include "bunny-sector_main.h"
#include "scriptbunnysector.h"
#include <string>

#if defined(USE_ANGEL_AS_CPP)
#include "../scripts/angel.hpp"
#endif

bool initOk = false;

Example::Example()
{

}

void Example::AngelInit()
{
    bool initOk = false;
    // AngelScript
    angelContext = mgdl_InitAngelScript();
    if (angelContext != nullptr)
    {
        RegisterBunnySector(angelContext);

        if (mgdl_LoadAngelScriptFiles(angelContext, "scripts/angel.cpp", "scripts", "example"))
        {
            angelInitFunc = mgdl_FindAngelScriptFunction(angelContext, "void angelscript_init()");
            initOk = true;
        }
    }

	// This must be after loading the files, because the function is in there
	if (BunnySector_Init(angelContext))
	{
			// This will call bunnysector functions
			initOk = true;
            mgdl_PrepareAngelScriptFunctionCall(angelContext, angelInitFunc);
            mgdl_CommitAngelScriptFunctionCall(angelContext, angelInitFunc);
            Log_Info("Init Done!\n");
    }
    mgdl_assert_print(initOk, "BunnySector Init failed");
}

void Example::AngelFrame()
{

    // First AngelScript can do something
   // mgdl_RunAngelScriptFrame(angelContext, mgdl_GetDeltaTime());

    // Then we update and call after collision callback and rendering callback
    BunnySector_Update(mgdl_GetDeltaTime());
}


void Example::Quit()
{
}

void Example::Update()
{
    elapsedSeconds = mgdl_GetElapsedSeconds();
    deltaTime = mgdl_GetDeltaTime();

    cursorPos = WiiController_GetCursorPosition(Platform_GetController(0));
    mouseClick = WiiController_ButtonPress(Platform_GetController(0), ButtonA);
    mouseDown = WiiController_ButtonHeld(Platform_GetController(0), ButtonA);
}



