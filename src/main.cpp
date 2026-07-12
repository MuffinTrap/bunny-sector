#include <mgdl.h>
#include "example.h"

static Example example;

void init()
{
    glViewport(0, 0, mgdl_GetScreenWidth(), mgdl_GetScreenHeight());

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    example.AngelInit();
}

void frame()
{

    // NOTE Use the mgdl_glClear to assure depth buffer working correctly on Wii
    mgdl_glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    example.AngelFrame();
}

void quit()
{
    // Called before program exits
    // Use this to free any resources and disconnect rocket
    example.Quit();
}

int main()
{
    mgdl_InitSystem("Bunny Sector",
        ScreenAspect::Screen4x3,
            init,
            frame,
            quit,
        FlagNone
         //| FlagFullScreen
         //| FlagSplashScreen
         //| FlagPauseUntilA
    );

    return 0;
}
