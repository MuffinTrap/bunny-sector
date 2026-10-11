
#include "player.h"
#include "actor.h"
#include "../bunny-sector-math.h"
#include "../bunny-sector-types.h"

void Player::Init(int playerNumber)
{
	this->number = playerNumber;

	moveVelocity = 0.0f;
	turnVelocityDegrees = 0.0f;
	strafeVelocity = 0.0f;

	// Size
	eyeHeightNormalized = 1.00f;

	forwardDrive = 0.0f;
	strafeDrive = 0.0f;
	turnDrive = 0.0f;
	verticalDrive = 0.0f;

	walkSpeedMultiplier = 1.0f;
	turnSpeedMultiplier = 1.0f;

	for (int i = 0; i < 32; i++)
	{
		inventory[i].itemType = editorNumber_none;
	}
}
void Player::ConfigureActor(Actor* actor)
{
	actor->size = size;
	actor->height = standingHeight;
	actor->climbHeight = climbHeight;
}


bool Player::GiveItem(DOOM_EDITOR_NUMBER item, int amount)
{
	int firstEmpty = -1;
	for (int i = 0; i < 32; i++)
	{
		if (inventory[i].itemType == editorNumber_none && firstEmpty < 0)
		{
			firstEmpty = i;
		}
		else if (inventory[i].itemType == item)
		{
			inventory[i].amount += amount;
			return true;
		}
	}
	if (firstEmpty >= 0)
	{
		inventory[firstEmpty].amount = amount;
		inventory[firstEmpty].itemType = item;
		return true;
	}
	return false;
}
bool Player::GetItemCount(DOOM_EDITOR_NUMBER item)
{

	for (int i = 0; i < 32; i++)
	{
		if (inventory[i].itemType == item)
		{
			return inventory[i].amount;
		}
	}
	return 0;
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
		turnVelocityDegrees += turnDrive * turnSpeedMultiplier * accRad * deltaTime;
	}
	else
	{
		turnVelocityDegrees *= 0.9f;
		if (abs(turnVelocityDegrees) < deadzone)
		{
			turnVelocityDegrees = 0.0f;
		}
	}

	float tsd = Deg2Rad(turnSpeedDegrees);
	if (turnVelocityDegrees > tsd)
	{
		turnVelocityDegrees = tsd;
	}
	if (turnVelocityDegrees < -tsd)
	{
		turnVelocityDegrees = -tsd;
	}

	// Rotate
	actor->yawRad += turnVelocityDegrees * deltaTime;
	// calculate current floor direction
	Vector2 forward = FLOOR_FORWARD;
	actor->moveDirection.vectorDirection = Vector2Rotate(forward, actor->yawRad);

	// TODO Strafing
	if (forwardDrive != 0)
	{
		moveVelocity += forwardDrive * moveAcceleration * deltaTime;
	}
	else if (moveVelocity != 0)
	{
		// Slow down
		if (moveVelocity < 0)
		{
			moveVelocity += moveSlowdown * deltaTime;
			if (moveVelocity > 0)
			{
				moveVelocity = 0;
			}
		}
		else
		{

			moveVelocity -= moveSlowdown * deltaTime;
			if (moveVelocity < 0)
			{
				moveVelocity = 0;
			}
		}
	}
	actor->floorVelocity = walkSpeedMultiplier * moveVelocity;
}

Viewpoint Player::GetViewpoint(Actor* actor)
{
	Viewpoint v = actor->GetViewpoint();
	v.position.y += standingHeight;
	return v;
}

