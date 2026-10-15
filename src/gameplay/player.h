#pragma once

#include "../bunny-sector-types.h"
#include "../doom/doom_types.h"

class Actor;

struct InventoryEntry
{
    DOOM_EDITOR_NUMBER itemType;
    int amount;
};

/**
 * @brief Player is controlled with gamepad and moves in the map
 * @details Player is linked to an actor in the active map
 */
class Player
{
public:
	int number; // 1 to 4

	// TODO Items and abilities

	// Movement functions

    // Drive: either from AI or input
    // All [-1,1]/
    float forwardDrive; /**< Is going forwards or backwards */
    float strafeDrive; /**< Is going sideways */
    float turnDrive; /**< Is turning on yaw axis */
    float verticalDrive; /**< Is going up or down */


    int turnSpeedDegrees;
    int turnAccelerationDegrees;
    int turnSlowdownDegrees;

    float turnVelocityDegrees; /*< How fast actor is turning */

    int moveSpeed;
    int moveAcceleration;
    int moveSlowdown;

    float moveVelocity;

    int strafeSpeed;
    int strafeAcceleration;
    int strafeSlowdown;

    float strafeVelocity;

    int jumpSpeed;
    int jumpAcceleration;
    float jumpDuration;

    float verticalSpeedUp;
    float verticalSpeedDown;
    float verticalAccelerationUp;
    float verticalAccelerationDown;

    int standingHeight; ///< How much above ground when standing
    int climbHeight; ///< How tall elevation change can traverse
    float eyeHeightNormalized; ///< Eye height of active height
    int size; // Transferred to Actor


    // changed during gameplay
    float walkSpeedMultiplier; // Affects both moving and strafing
    float turnSpeedMultiplier;

    // What the actor should be doing
    u32 actionFlags;

	int prevActorSubSectorNumber;

	InventoryEntry inventory[32];

	void Init(int playerNumber);
    void ConfigureActor(Actor* actor);
	void ApplyDrive(Actor* actor, float delta);
	void ApplyVerticalMove(Actor* actor, float delta);
	Viewpoint GetViewpoint(Actor* actor);
	bool GiveItem(DOOM_EDITOR_NUMBER item, int amount);
    bool GetItemCount(DOOM_EDITOR_NUMBER item);
    bool ReadInput(int controllerIndex);
    void StartAction(ActorActionBit action);
    void StopAction(ActorActionBit action);
};
