#pragma once
#include <mgdl.h>
#include "../doom/doom_types.h"

struct RenderSettings2D;
struct RenderSettingsOpenGL;
/**
 * @brief This is the actor data
 * @details Actors are anything that exists and moves in the map:
 * Players
 * Monsters
 * Items
 * Decorations
 * Projectiles
 * Special particles
 *
 * It starts with the info loaded from the map
 * and then comes game specific stuff
 */

class Actor
{
public:
    int idNumber;
    s16 subSectorNumber;
    s16 prevSubSectorNumber;
    ActorType actorType;
    int typeNumber; //< DOOM editor number
    MaterialId texture;

    // Position in map units
    union ActorPosition
    {
        BunnyV2 bunnyPosition;
        Vector2 vectorPosition; /**< Where actor is */
    };
    ActorPosition position;

    // OpenGL y coordinate
    float elevation;

    Vector2 prevPosition; /**< Place to store position before moving */

    float floorVelocity; /**< Where actor is trying to go */
    float floorSpeedLimit;

    float verticalVelocity;
    float verticalSpeedLimitUp;
    float verticalSpeedLimitDown;

    // Directions as a normal vectors
    union ActorFloorDirection
    {
        Vector2 vectorDirection; /**< Where player is headed */
        BunnyV2 bunnyDirection; /**< Where player is headed */
    };
    ActorFloorDirection lookDirection;
    ActorFloorDirection moveDirection;

    // Rotations
    float yawRad; // Turning
    float pitchRad; // looking up and down

    // Collisions
    float size;
    float height;
    float climbHeight; //<< How much can move upwards when colliding with stairs
    bool noclip;

    // Drawing
    int renderWidth;
    int renderHeight;

    // What actor is doing
    u32 actionFlags;
    
    u32 lastMoveResultFlags;

    static Actor Create(int idNumber, s16 sector, Vector3 position, float yawRad, float radius, float height);
    static Actor CreateFromViewPoint(Viewpoint point);

    void Init();
    BunnyV2* GetPosition();
    BunnyV2* GetFloorDirection();
    void SetPosition( float x, float y);
    void StartAction( ActorActionBit flags);
    void EndAction( ActorActionBit flags);
    bool IsDoing( ActorActionBit flags);

    Viewpoint GetViewpoint();

    Vector2 MoveOnFloor( float delta);
    float MoveVertically( float gravity, float delta);
};

