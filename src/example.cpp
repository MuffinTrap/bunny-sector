
#include "example.h"
#include <mgdl/mgdl-script-api.h>
#include "bunny-sector_main.h"
#include "scriptbunnysector.h"
#include <string>

#if defined(MGDL_ROCKET)
    #include <mgdl-rocket.h>
    static ROCKET_TRACK sync_value;
#endif

#if defined(USE_ANGEL_AS_CPP)
#include <angel.hpp>
#endif

Example::Example()
{

}

void Example::AngelInit()
{
    // AngelScript
#if defined(USE_ANGEL_AS_SCRIPT)
    angelContext = mgdl_InitAngelScript();
    if (angelContext != nullptr)
    {
        RegisterBunnySector(angelContext);
        if (mgdl_LoadAngelScriptFiles(angelContext, "scripts/angel.cpp", "scripts", "example"))
        {
            mgdl_RunAngelScriptInit(angelContext);
        }
    }

#elif defined(USE_ANGEL_AS_CPP)
    angelContext = mgdl_InitAngelCpp(&angelscript_init, &angelscript_frame, &angelscript_quit);
	mgdl_RunAngelScriptInit(angelContext);
#endif
    AssetManager_PrintLoadedTextures();
}

void Example::AngelFrame()
{
    mgdl_RunAngelScriptFrame(angelContext, mgdl_GetDeltaTime());
}


void Example::Quit()
{
#ifdef MGDL_ROCKET
    Rocket_Disconnect();
#endif
}

void Example::Update()
{
    #ifdef MGDL_ROCKET

        Rocket_UpdateRow();
        if (WiiController_ButtonPress(mgdl_GetController(0), WiiButtons::Button2))
        {
            Rocket_SaveAllTracks();
        }
    #endif

    elapsedSeconds = mgdl_GetElapsedSeconds();
    deltaTime = mgdl_GetDeltaTime();

    cursorPos = WiiController_GetCursorPosition(Platform_GetController(0));
    mouseClick = WiiController_ButtonPress(Platform_GetController(0), ButtonA);
    mouseDown = WiiController_ButtonHeld(Platform_GetController(0), ButtonA);

}



