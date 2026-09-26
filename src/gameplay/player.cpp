
#include "player.h"
#include "actor.h"
#include "../bunny-sector-math.h"
#include "../bunny-sector-types.h"

void Player::Init(int playerIndex, float moveSpeed, float moveAcceleration, float turnSpeed, float turnAccelerationDeg)
{

	float unitsToMeter = DOOM_UNITS_TO_METER;
	this->index = playerIndex;

	turnAccelerationDegrees = turnAccelerationDeg;
	turnSpeedDegrees = turnSpeed;

	this->moveSpeed = 2.0f * unitsToMeter;
	moveAcceleration = 2.0f * unitsToMeter;

	verticalSpeedUp = 89.0f * unitsToMeter;
	verticalSpeedDown = -80.2f * unitsToMeter; // DANGER
	verticalAccelerationUp = 4.0f * unitsToMeter;
	verticalAccelerationDown = 4.0f * unitsToMeter;
	turnVelocity = 0.0f;

	// Size
	standingHeight = 1.0f * unitsToMeter;
	climbHeight = standingHeight/2.0f;
	eyeHeightNormalized = 1.00f;


	forwardDrive = 0.0f;
	strafeDrive = 0.0f;
	turnDrive = 0.0f;
	verticalDrive = 0.0f;

	turnSpeedMultiplier = 1.0f;
	walkSpeedMultiplier = 1.0f;

	for (int i = 0; i < 32; i++)
	{
		inventory[i] = editorNumber_none;
	}
}

void Player::GiveItem(DOOM_EDITOR_NUMBER item)
{
	for (int i = 0; i < 32; i++)
	{
		if (inventory[i] == editorNumber_none)
		{
			inventory[i] = item;
		}
	}
}


static float deadzone = 0.01f;

void Player::ApplyVerticalMove(Actor* actor, float deltaTime)
{
	float verticalAcceleration = 0.0f;
	if (verticalDrive > deadzone)
	{
		verticalAcceleration += verticalDrive * verticalAccelerationUp;
	}
	if (verticalDrive < -deadzone)
	{
		verticalAcceleration += verticalDrive * verticalAccelerationDown;
	}

	actor->verticalVelocity += verticalAcceleration * deltaTime;
}

void Player::ApplyDrive(Actor* actor, float deltaTime)
{
	// NOTE Player standing or crouching TODO
	actor->climbHeight = climbHeight;
	actor->height = standingHeight;

	// TODO Jumping, jetpacks etc. affecting vertical movement
	actor->verticalSpeedLimitUp = verticalSpeedUp;
	actor->verticalSpeedLimitDown = verticalSpeedDown;

	// Apply turn drive
	if (abs(turnDrive) > deadzone)
	{
		float accRad = Deg2Rad(turnAccelerationDegrees);
		turnVelocity += turnDrive * turnSpeedMultiplier * accRad * deltaTime;
	}
	else
	{
		turnVelocity *= 0.9f;
		if (abs(turnVelocity) < deadzone)
		{
			turnVelocity = 0.0f;
		}
	}

	float tsd = Deg2Rad(turnSpeedDegrees);
	if (turnVelocity > tsd)
	{
		turnVelocity = tsd;
	}
	if (turnVelocity < -tsd)
	{
		turnVelocity = -tsd;
	}

	// Rotate
	actor->yawRad += turnVelocity * deltaTime;
	// calculate current floor direction
	Vector2 forward = FLOOR_FORWARD;
	actor->moveDirection.vectorDirection = Vector2Rotate(forward, actor->yawRad);

	// TODO Strafing
	actor->floorVelocity = forwardDrive * walkSpeedMultiplier * moveSpeed;
}

Viewpoint Player::GetViewpoint(Actor* actor)
{
	Viewpoint v = Actor_GetViewpoint(actor);
	v.position.y += standingHeight;
	return v;
}

