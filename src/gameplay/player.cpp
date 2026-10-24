
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

	verticalSpeedUp = 280;
	verticalSpeedDown = -280;

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
	actor->verticalSpeedLimitDown = verticalSpeedDown;
	actor->verticalSpeedLimitUp = verticalSpeedUp;
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


bool Player::ReadInput(int controllerIndex)
{
	float forward = 0;
	float strafe = 0.0f;
	float turn = 0.0f;
	float vertical = 0.0f;

	if (mgdl_IsButtonDown(controllerIndex, ButtonC))
	{
		// TODO Jump on C
	}

	Vector2 wasd = mgdl_GetJoystick(controllerIndex, Joystick_Nunchuk);
	forward = -wasd.y;
	// NOTE Strafe on Z
	if (mgdl_IsButtonDown(controllerIndex, ButtonZ))
	{
		strafe = wasd.x;
	}
	else
	{
		turn = wasd.x;
	}


	// Use button is A for now
	if (mgdl_IsButtonPressed(controllerIndex, ButtonA))
	{
		StartAction(action_use);
	}
	else 
	{
		// Note do not keep this on
		StopAction(action_use);
	}

	if (mgdl_IsButtonPressed(controllerIndex, ButtonB))
	{
		StartAction(action_shoot);
	}
	else if (mgdl_IsButtonReleased(controllerIndex, ButtonB))
	{
		StopAction(action_shoot);
	}

	if (mgdl_IsButtonPressed(controllerIndex, ButtonC))
	{
		StartAction(action_jump);
	}
	else if (mgdl_IsButtonReleased(controllerIndex, ButtonC))
	{
		StopAction(action_jump);
	}

	forwardDrive = Clamp(forward, -1.0f, 1.0f);
	strafeDrive = Clamp(strafe, -1.0f, 1.0f);
	verticalDrive = Clamp(vertical, -1.0f, 1.0f);
	turnDrive = Clamp(turn, -1.0f, 1.0f);

	return true;
}


static float deadzone = 0.2; // NOTE XBOX360

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

void Player::StartAction(ActorActionBit action)
{
	actionFlags = Flag_SetBit(actionFlags, action);
}

void Player::StopAction(ActorActionBit action)
{
	actionFlags = Flag_UnsetBit(actionFlags, action);
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
	if (abs(forwardDrive) > deadzone)
	{
		moveVelocity += forwardDrive * moveAcceleration * deltaTime;
		moveVelocity = clampF(moveVelocity, -moveSpeed, moveSpeed);

		headBobValue = sinf(mgdl_GetElapsedSeconds() * headBobFreq) * headBobAmplitude;
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

	if (abs(strafeDrive) > deadzone)
	{
		strafeVelocity += strafeDrive * strafeAcceleration * deltaTime;
		strafeVelocity = clampF(strafeVelocity, -strafeSpeed, strafeSpeed);
	}
	else if (strafeVelocity != 0)
	{
		// Slow down
		if (strafeVelocity < 0)
		{
			strafeVelocity += strafeSlowdown * deltaTime;
			if (strafeVelocity > 0)
			{
				strafeVelocity = 0;
			}
		}
		else
		{
			strafeVelocity -= strafeSlowdown * deltaTime;
			if (strafeVelocity < 0)
			{
				strafeVelocity = 0;
			}
		}
	}
	actor->floorVelocity.y = walkSpeedMultiplier * moveVelocity;
	actor->floorVelocity.x = walkSpeedMultiplier * strafeVelocity;
}

Viewpoint Player::GetViewpoint(Actor* actor)
{
	Viewpoint v = actor->GetViewpoint();
	v.position.y += standingHeight + headBobValue;

	return v;
}

