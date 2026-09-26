#pragma once
#include <mgdl.h>
#include "../duke/duke_types.h"
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

struct Actor
{
    int idNumber;
    s16 subSectorNumber;
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
    float radius;
    float height;
    float climbHeight; //<< How much can move upwards when colliding with stairs
    bool noclip;

    // What actor is doing
    u32 actionFlags;
    
    u32 lastMoveResultFlags;
};
typedef struct Actor Actor;

Actor Actor_CreateFromViewPoint(Viewpoint point);
void Actor_Init(Actor* actor);
Actor Actor_Create(int idNumber, s16 sector, Vector3 position, float yawRad, float radius, float height);
BunnyV2* Actor_GetPosition(Actor* actor);
BunnyV2* Actor_GetFloorDirection(Actor* actor);
void Actor_SetPosition(Actor* actor, float x, float y);
void Actor_StartAction(Actor* actor, ACTOR_ACTION_FLAGS flags);
void Actor_EndAction(Actor* actor, ACTOR_ACTION_FLAGS flags);
bool Actor_IsDoing(Actor* actor, ACTOR_ACTION_FLAGS flags);

Viewpoint Actor_GetViewpoint(Actor* actor);

Vector2 Actor_MoveOnFloor(Actor* actor, float delta);
float Actor_MoveVertically(Actor* actor, float gravity, float delta);
