#pragma once
#include <mgdl.h>
#include "../duke/duke_types.h"
#include "../doom/doom_types.h"

struct RenderSettings2D;
struct RenderSettingsOpenGL;
/**
 * @brief This is the player data.
 * @details It starts with the info loaded from the map
 * and then comes game specific stuff
 */



struct Actor
{
    int idNumber;
    s16 subSectorNumber;
    ActorType actorType;
    int typeNumber; //< DOOM editor number
    MaterialId texture;

    // Drive: either from AI or input
    // All [-1,1]/
    float forwardDrive; /**< Is going forwards or backwards */
    float strafeDrive; /**< Is going sideways */
    float turnDrive; /**< Is turning on yaw axis */
    float verticalDrive; /**< Is going up or down */

    // Position in duke units
    union ActorPosition
    {
        BunnyV2 bunnyPosition;
        Vector2 vectorPosition; /**< Where actor is */
    };
    ActorPosition position;

    float elevation;
    Vector2 prevPosition; /**< Place to store position before moving */

    float turnVelocity; /*< How fast actor is turning */
    Vector2 floorVelocity; /**< Where actor is trying to go */
    float verticalVelocity;
    // Directions as a normal vectors
    union ActorFloorDirection
    {
        Vector2 vectorDirection; /**< Where player is headed */
        BunnyV2 bunnyDirection; /**< Where player is headed */
    };
    ActorFloorDirection direction;

    // Rotations
    float yawRad; // Turning
    float pitchRad; // looking up and down


    float turnAccelerationDegrees;
    float turnSpeedDegrees;

    // These are in dukes
    float moveSpeed;
    float moveAcceleration;
    float verticalSpeedUp;
    float verticalSpeedDown;
    float verticalAccelerationUp;
    float verticalAccelerationDown;
    float standingHeight; ///< How much above ground when standing
    float climbHeight; ///< How tall elevation change can traverse
    float eyeHeightNormalized; ///< Eye height of active height

    // Collsions
    float radius;
    bool noclip;

    // changed during gameplay
    float walkSpeedMultiplier;
    float turnSpeedMultiplier;

    // What player is doing
    u32 actionFlags;
    
    u32 lastMoveResultFlags;
};
typedef struct Actor Actor;

Actor Actor_CreateFromViewPoint(Viewpoint point);
Actor Actor_Create(int idNumber, s16 sector, Vector3 position, float yawRad, float moveSpeed, float moveAcceleration, float turnSpeed, float turnAccelerationDeg, float standingHeight);
Actor Actor_CreatePlayer(int idNumber, float unitsToMeter);

Vector2 Actor_ApplyDrive(Actor* actor, float deltaTime);
float Actor_ApplyVerticalMove(Actor* actor, float gravity, float deltaTime);
bool IsPointInsideRect(RectF rect, Vector2 point);
Viewpoint Actor_GetViewpoint(Actor* actor);
BunnyV2* Actor_GetPosition(Actor* actor);
BunnyV2* Actor_GetFloorDirection(Actor* actor);
void Actor_SetPosition(Actor* actor, float x, float y);
void Actor_StartAction(Actor* actor, ACTOR_ACTION_FLAGS flags);
void Actor_EndAction(Actor* actor, ACTOR_ACTION_FLAGS flags);
bool Actor_IsDoing(Actor* actor, ACTOR_ACTION_FLAGS flags);
