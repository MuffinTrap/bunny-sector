#include "actor.h"
#include "build-render.h"

#include "dukemap.h"
#include "dukemath.h"
#include "math.h"

Actor Actor_CreateDefaultActor(int idNumber, float unitsToMeter)
{
	Actor a;
	a.idNumber = idNumber;
	a.sectorNumber = -1;

	a.forwardDrive = 0.0f;
	a.strafeDrive = 0.0f;
	a.turnDrive = 0.0f;
	a.verticalDrive = 0.0f;

	a.position.vectorPosition = Vector2Zero();
	a.elevation = 0;
	a.prevPosition = Vector2Zero();

	a.turnVelocity = 0.0f;
	a.floorVelocity = Vector2Zero();
	a.verticalVelocity = 0.0f;
	//a.lookDirection = mgdl_GetGLWorldForward();
	a.floorDirection = Vector2New(1, 0);

	a.yawRad = 0.0f;
	a.pitchRad = 0.0f;

	a.turnAccelerationDegrees = 480.0f;
	a.turnSpeedDegrees = 340.0f; // NOTE set

	a.moveSpeed = 2.0f * unitsToMeter;
	a.moveAcceleration = 2.0f * unitsToMeter;

	a.verticalSpeedUp = 4.0f * unitsToMeter;
	a.verticalSpeedDown = -3.2f * unitsToMeter; // DANGER
	a.verticalAccelerationUp = 4.0f * unitsToMeter;
	a.verticalAccelerationDown = 4.0f * unitsToMeter;

	// Size
	a.standingHeight = 1.5f * unitsToMeter;
	a.climbHeight = a.standingHeight/2.0f;
	a.eyeHeightNormalized = 0.85f;
	a.radius = 0.5f * unitsToMeter;
	a.noclip = false;

	a.turnSpeedMultiplier = 1.0f;
	a.walkSpeedMultiplier = 1.0f;

	a.lastMoveResultFlags = 0;

	return a;
}

Viewpoint Actor_GetViewpoint(Actor* actor)
{
	Viewpoint p;
	p.position = Vector3New(actor->position.vectorPosition.x, actor->elevation, actor->position.vectorPosition.y);
	p.sector = actor->sectorNumber;
	p.yawRad = actor->yawRad;
	p.pitchRad = actor->pitchRad;
	return p;
}

static const float deadzone = 0.1f;

Vector2 Actor_ApplyDrive(Actor* actor, float deltaTime)
{

	// Apply turn drive
	if (abs(actor->turnDrive) > deadzone)
	{
		float accRad = Deg2Rad(actor->turnAccelerationDegrees);
		actor->turnVelocity += actor->turnDrive * actor->turnSpeedMultiplier * accRad * deltaTime;
	}
	else
	{
		actor->turnVelocity *= 0.9f;
		if (abs(actor->turnVelocity) < deadzone)
		{
			actor->turnVelocity = 0.0f;
		}
	}

	float tsd = Deg2Rad(actor->turnSpeedDegrees);
	if (actor->turnVelocity > tsd)
	{
		actor->turnVelocity = tsd;
	}
	if (actor->turnVelocity < -tsd)
	{
		actor->turnVelocity = -tsd;
	}

	// Rotate
	actor->yawRad += actor->turnVelocity * deltaTime;
	// calculate current floor direction
	Vector2 forward = FLOOR_FORWARD;
	actor->floorDirection = Vector2Rotate(forward, actor->yawRad);

	Vector2 floorDestination = Vector2Add(
		actor->position.vectorPosition, Vector2Scale(
				actor->floorDirection,
				actor->forwardDrive * actor->walkSpeedMultiplier * actor->moveSpeed * deltaTime
				)
		);

	Vector2 destination = floorDestination;
	return destination;
}

float Actor_ApplyVerticalMove(Actor* actor, float gravity, float deltaTime)
{
	float verticalAcceleration = gravity;
	if (actor->verticalDrive > deadzone)
	{
		// Apply falling/jumping
		if (actor->verticalDrive > 0)
		{
			verticalAcceleration += actor->verticalDrive * actor->verticalAccelerationUp;
		}
	}
	if (gravity == 0.0f && actor->verticalDrive < -deadzone)
	{
		verticalAcceleration += actor->verticalDrive * actor->verticalAccelerationDown;
	}

	actor->verticalVelocity += verticalAcceleration * deltaTime;

	// Limit vertical speeds
	if (actor->verticalVelocity > actor->verticalSpeedUp)
	{
		actor->verticalVelocity = actor->verticalSpeedUp;
	}
	else if (actor->verticalVelocity < 0)
	{
		// If falling, stop velocity when hits ground or limit max falling speed
		if (Flag_IsBitSet(actor->lastMoveResultFlags, Move_OnGround))
		{
			actor->verticalVelocity = 0.0f;
		}
		else if (actor->verticalVelocity < actor->verticalSpeedDown)
		{
			actor->verticalVelocity = actor->verticalSpeedDown;
		}
	}

	// Move vertically
	return actor->elevation + actor->verticalVelocity * deltaTime;
}

Vector3 Actor_ApplyDrive2(Actor* actor, float deltaTime)
{
	// calculate current floor direction
	Vector2 forward = FLOOR_FORWARD;
	actor->floorDirection = Vector2Rotate(forward, actor->yawRad);
	Vector2 strafeDirection = Vector2Rotate(actor->floorDirection, Deg2Rad(90.0f));

	if (abs(actor->forwardDrive) > deadzone)
	{
		Vector2 floorAcceleration = Vector2Scale(actor->floorDirection, actor->forwardDrive * actor->moveAcceleration * deltaTime);
		actor->floorVelocity = Vector2Add(actor->floorVelocity, Vector2Scale(floorAcceleration, deltaTime));
	}
	else
	{
		actor->floorVelocity = Vector2Scale(actor->floorVelocity, 0.9f);
		if (Vector2Length(actor->floorVelocity) < deadzone)
		{
			actor->floorVelocity.x = 0.0f;
			actor->floorVelocity.y = 0.0f;
		}
	}


	// TODO limit velocity
	if (Vector2Length(actor->floorVelocity) > actor->moveSpeed)
	{
		actor->floorVelocity = Vector2Scale(Vector2Normalize(actor->floorVelocity), actor->moveSpeed);
	}

	// Calculate new position
	Vector2 floorDestination = Vector2New(
		actor->position.vectorPosition.x + actor->floorVelocity.x * deltaTime,
		actor->position.vectorPosition.y + actor->floorVelocity.y * deltaTime
	);

	// Apply turn drive
	if (abs(actor->turnDrive) > deadzone)
	{
		float accRad = Deg2Rad(actor->turnAccelerationDegrees);
		actor->turnVelocity += actor->turnDrive * accRad * deltaTime;
	}
	else
	{
		actor->turnVelocity *= 0.9f;
		if (abs(actor->turnVelocity) < deadzone)
		{
			actor->turnVelocity = 0.0f;
		}
	}

	float tsd = Deg2Rad(actor->turnSpeedDegrees);
	if (actor->turnVelocity > tsd)
	{
		actor->turnVelocity = tsd;
	}
	if (actor->turnVelocity < -tsd)
	{
		actor->turnVelocity = -tsd;
	}

	// Rotate
	actor->yawRad += actor->turnVelocity * deltaTime;

	if (abs(actor->verticalDrive) > deadzone)
	{
		// Apply falling/jumping
		actor->verticalVelocity += actor->verticalDrive * actor->verticalAccelerationUp * deltaTime;
	}
	else
	{
		actor->verticalVelocity *= 0.9f;
		if (abs(actor->verticalVelocity) < deadzone)
		{
			actor->verticalVelocity = 0.0f;
		}
	}

	// Limit falling speed
	if (actor->verticalVelocity > actor->verticalSpeedUp)
	{
		actor->verticalVelocity = actor->verticalSpeedUp;
	}
	else if (actor->verticalVelocity < actor->verticalSpeedDown)
	{
		actor->verticalVelocity = actor->verticalSpeedDown;
	}

	// Move vertically
	float heightDestination = actor->elevation + actor->verticalVelocity * deltaTime;

	Vector3 destination = Vector3New(floorDestination.x, heightDestination, floorDestination.y);
	return destination;
}

DoomVertex* Actor_GetDoomPosition(Actor* actor)
{
	return &actor->position.doomPosition;
}

