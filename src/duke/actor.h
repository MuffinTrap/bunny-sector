#pragma once
#include <mgdl.h>
#include "duke_types.h"

struct DukeMap;
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
    s16 sectorNumber;

    // Drive: either from AI or input
    // All [-1,1]/
    float forwardDrive; /**< Is going forwards or backwards */
    float strafeDrive; /**< Is going sideways */
    float turnDrive; /**< Is turning on yaw axis */
    float verticalDrive; /**< Is going up or down */

    // Position in duke units
    Vector2 position; /**< Where actor is */
    float elevation;
    Vector2 prevPosition; /**< Place to store position before moving */

    float turnVelocity; /*< How fast actor is turning */
    Vector2 floorVelocity; /**< Where actor is trying to go */
    float verticalVelocity;
    // Directions as a normal vectors
    Vector2 floorDirection; /**< Where player is headed */

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
    float kneelingHeight; ///< How much above ground when kneeling/crouching
    float eyeHeightNormalized; ///< Eye height of active height

    // Collsions
    float radius;
    bool noclip;

    // changed during gameplay
    float walkSpeedMultiplier;
    float turnSpeedMultiplier;
    
    u32 lastMoveResultFlags;
};
typedef struct Actor Actor;

Actor Actor_CreateFromViewPoint(Viewpoint point);
Actor Actor_Create(int idNumber, s16 sector, Vector3 position, float yawRad, float moveSpeed, float moveAcceleration, float turnSpeed, float turnAccelerationDeg, float standingHeight);
Actor Actor_CreateDefaultActor(int idNumber);

Vector2 Actor_ApplyDrive(Actor* actor, float deltaTime);
void Player_UpdateMove(Actor* player, WiiController* controller, RenderSettings2D* settings2D, RenderSettingsOpenGL* settingsGL, DukeMap* map, int amountPlayers);
bool IsPointInsideRect(RectF rect, Vector2 point);
Viewpoint Actor_GetViewpoint(Actor* actor);
