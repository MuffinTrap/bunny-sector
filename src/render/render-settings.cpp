#include "render-settings.h"
static Camera* defaultCamera = nullptr;

Camera* GetDefaultCamera()
{
    if (defaultCamera == nullptr)
    {
        defaultCamera = Camera_CreateDefault();
    }
    defaultCamera->nearZ = 0.1f;
    defaultCamera->farZ = 100.0f;
    defaultCamera->fovY = 80.0f;
    defaultCamera->projection = CameraNone;
    Camera_SetMode(defaultCamera, CameraDirection);
    return defaultCamera;
}

Viewpoint GetDefaultCameraInfo()
{
    Viewpoint info;
    info.position = Vector3Zero();
    info.pitchRad = 0.0f;
    info.yawRad = 0.0f;
    info.sector = 0;
    return info;
}

RenderSettings2D GetDefaultRenderSettings2D()
{
    RenderSettings2D render2D;
    render2D.mapOffset = Vector2New(0,0);
    render2D.mapZoom = 1.0f;
    render2D.scaleXZ = 1.0f;
    render2D.collisionPoint = Vector2New(0, 0);
    render2D.collisionLength = 100.0f;
    render2D.collisionAngleDeg = 180.0f;
    render2D.movePlayer = true;
    render2D.drawOneWall = -1;
    render2D.drawOneSector = -1;
    render2D.rotateMap= true;
    render2D.centerMapToPlayer= true;
    render2D.drawPlayersAmount = 1;
    render2D.drawWallNumbers = true;
    render2D.drawSectorNumbers = true;
    return render2D;
}
RenderSettingsOpenGL GetDefaultRenderSettingsOpenGL()
{
    float unitsPerMetre = 1.0f;

    RenderSettingsOpenGL renderGL;

    renderGL.scale = 1.0f/unitsPerMetre;
    renderGL.spriteDefaultWidth = 1024;
    renderGL.spriteDefaultHeight = 8024;

    renderGL.near = 1.0f/unitsPerMetre;
    renderGL.far = 100.0f * unitsPerMetre;
    renderGL.FOVyDegrees = 80.0f;
    renderGL.aspectRatio = mgdl_GetAspectRatio();
    return renderGL;
}

void RenderSettingsOpenGL_SetUnitToMeter(RenderSettingsOpenGL* setting, float unitsToMeter)
{
    setting->scale = 1.0f/unitsToMeter;
    setting->near = 1.0f/unitsToMeter;
    setting->far = 100 * unitsToMeter;
}
