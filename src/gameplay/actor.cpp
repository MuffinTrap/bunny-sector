#include "actor.h"

#include "../bunny-sector-math.h"
#include "math.h"

Actor Actor_Create(ActorType aType)
{
	Actor a;
	a.Init();
	a.actorType = aType;
	return a;
}

void Actor::Init()
{
	subSectorNumber = -1;
	typeNumber = -1;
	position.vectorPosition = Vector2Zero();
	elevation = 0;
	prevPosition = Vector2Zero();
	floorVelocity = 0.0f;
	verticalVelocity = 0.0f;
	lookDirection.vectorDirection = Vector2New(1, 0);
	moveDirection.vectorDirection = Vector2New(1, 0);
	yawRad = 0.0f;
	pitchRad = 0.0f;
	size = 16.0f;
	height = 16.0f;
	climbHeight = 0.0f;
	noclip = false;
	lastMoveResultFlags = 0;
	actionFlags = 0;
}

Viewpoint Actor::GetViewpoint()
{
	Viewpoint p;
	p.position = Vector3New(position.vectorPosition.x, elevation, position.vectorPosition.y);
	p.sector = subSectorNumber;
	p.yawRad = yawRad;
	p.pitchRad = pitchRad;
	return p;
}

Vector2 Actor::MoveOnFloor(float delta)
{
	Vector2 floorDestination = Vector2Add(
		position.vectorPosition, Vector2Scale(
				moveDirection.vectorDirection,
				floorVelocity * delta
				)
		);

	return floorDestination;
}

float Actor::MoveVertically(float gravity, float delta)
{
	verticalVelocity -= gravity * delta;

	// Limit vertical speeds
	if (verticalVelocity > verticalSpeedLimitUp)
	{
		verticalVelocity = verticalSpeedLimitUp;
	}
	else if (verticalVelocity < 0)
	{
		// If falling, stop velocity when hits ground or limit max falling speed
		if (Flag_IsBitSet(lastMoveResultFlags, Move_OnGround))
		{
			verticalVelocity = 0.0f;
		}
		else if (verticalVelocity < verticalSpeedLimitDown)
		{
			verticalVelocity = verticalSpeedLimitDown;
		}
	}
	return elevation + verticalVelocity * delta;
}

/*
Vector3 Actor::ApplyDrive2(Actor* actor, float deltaTime)
{
	// calculate current floor direction
	Vector2 forward = FLOOR_FORWARD;
	floorDirection = Vector2Rotate(forward, yawRad);
	Vector2 strafeDirection = Vector2Rotate(floorDirection, Deg2Rad(90.0f));

	if (abs(forwardDrive) > deadzone)
	{
		Vector2 floorAcceleration = Vector2Scale(floorDirection, forwardDrive * moveAcceleration * deltaTime);
		floorVelocity = Vector2Add(floorVelocity, Vector2Scale(floorAcceleration, deltaTime));
	}
	else
	{
		floorVelocity = Vector2Scale(floorVelocity, 0.9f);
		if (Vector2Length(floorVelocity) < deadzone)
		{
			floorVelocity.x = 0.0f;
			floorVelocity.y = 0.0f;
		}
	}


	// TODO limit velocity
	if (Vector2Length(floorVelocity) > moveSpeed)
	{
		floorVelocity = Vector2Scale(Vector2Normalize(floorVelocity), moveSpeed);
	}

	// Calculate new position
	Vector2 floorDestination = Vector2New(
		position.vectorPosition.x + floorVelocity.x * deltaTime,
		position.vectorPosition.y + floorVelocity.y * deltaTime
	);

	// Apply turn drive
	if (abs(turnDrive) > deadzone)
	{
		float accRad = Deg2Rad(turnAccelerationDegrees);
		turnVelocity += turnDrive * accRad * deltaTime;
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
	yawRad += turnVelocity * deltaTime;

	if (abs(verticalDrive) > deadzone)
	{
		// Apply falling/jumping
		verticalVelocity += verticalDrive * verticalAccelerationUp * deltaTime;
	}
	else
	{
		verticalVelocity *= 0.9f;
		if (abs(verticalVelocity) < deadzone)
		{
			verticalVelocity = 0.0f;
		}
	}

	// Limit falling speed
	if (verticalVelocity > verticalSpeedUp)
	{
		verticalVelocity = verticalSpeedUp;
	}
	else if (verticalVelocity < verticalSpeedDown)
	{
		verticalVelocity = verticalSpeedDown;
	}

	// Move vertically
	float heightDestination = elevation + verticalVelocity * deltaTime;

	Vector3 destination = Vector3New(floorDestination.x, heightDestination, floorDestination.y);
	return destination;
}

*/
BunnyV2* Actor::GetPosition()
{
	return &position.bunnyPosition;
}
BunnyV2 * Actor::GetFloorDirection()
{
	return &moveDirection.bunnyDirection;
}
void Actor::SetPosition( float x, float y)
{
	position.vectorPosition.x = x;
	position.vectorPosition.y = y;
}

void Actor::StartAction( ActorActionBit flags)
{
	actionFlags = Flag_SetAll(actionFlags, flags);
}
void Actor::EndAction( ActorActionBit flags)
{
	actionFlags = Flag_UnsetBit(actionFlags, flags);
}
bool Actor::IsDoing( ActorActionBit flags)
{
	return Flag_IsBitSet(actionFlags, flags);
}


