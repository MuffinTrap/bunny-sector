#pragma once

#include "../bunny-sector-types.h"

struct Actor;

/**
 * @brief Player is controlled with gamepad and moves in the map
 * @details Player is linked to an actor in the active map
 */
class Player
{
public:
	// TODO Id and controller etc.
	int index;

	// TODO Items and abilities

	// Movement functions

    // Drive: either from AI or input
    // All [-1,1]/
    float forwardDrive; /**< Is going forwards or backwards */
    float strafeDrive; /**< Is going sideways */
    float turnDrive; /**< Is turning on yaw axis */
    float verticalDrive; /**< Is going up or down */

    float turnVelocity; /*< How fast actor is turning */

    float turnAccelerationDegrees;
    float turnSpeedDegrees;

    // These are in OpenGL units
    float moveSpeed;
    float moveAcceleration;
    float verticalSpeedUp;
    float verticalSpeedDown;
    float verticalAccelerationUp;
    float verticalAccelerationDown;
    float standingHeight; ///< How much above ground when standing
    float climbHeight; ///< How tall elevation change can traverse
    float eyeHeightNormalized; ///< Eye height of active height


    // changed during gameplay
    float walkSpeedMultiplier;
    float turnSpeedMultiplier;

	void Init(int playerIndex, float moveSpeed, float moveAcceleration, float turnSpeed, float turnAccelerationDeg);
	void ApplyDrive(Actor* actor, float delta);
	void ApplyVerticalMove(Actor* actor, float delta);
	Viewpoint GetViewpoint(Actor* actor);
};
