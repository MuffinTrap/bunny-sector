#include "actor.h"
#include "build-render.h"

#include "dukemap.h"
#include "../bunny-sector-math.h"
#include "math.h"

Actor Actor_Create(ActorType aType)
{
	Actor a;
	Actor_Init(&a);
	a.actorType = aType;
	return a;
}

void Actor_Init(Actor* actor)
{
	actor->subSectorNumber = -1;
	actor->typeNumber = -1;
	actor->position.vectorPosition = Vector2Zero();
	actor->elevation = 0;
	actor->prevPosition = Vector2Zero();
	actor->floorVelocity = 0.0f;
	actor->verticalVelocity = 0.0f;
	actor->lookDirection.vectorDirection = Vector2New(1, 0);
	actor->moveDirection.vectorDirection = Vector2New(1, 0);
	actor->yawRad = 0.0f;
	actor->pitchRad = 0.0f;
	actor->radius = 0.0f;
	actor->noclip = false;
	actor->lastMoveResultFlags = 0;
	actor->actionFlags = 0;
}

Viewpoint Actor_GetViewpoint(Actor* actor)
{
	Viewpoint p;
	p.position = Vector3New(actor->position.vectorPosition.x, actor->elevation, actor->position.vectorPosition.y);
	p.sector = actor->subSectorNumber;
	p.yawRad = actor->yawRad;
	p.pitchRad = actor->pitchRad;
	return p;
}

Vector2 Actor_MoveOnFloor(Actor* actor, float delta)
{
	Vector2 floorDestination = Vector2Add(
		actor->position.vectorPosition, Vector2Scale(
				actor->moveDirection.vectorDirection,
				actor->floorVelocity * delta
				)
		);

	return floorDestination;
}

float Actor_MoveVertically(Actor* actor, float gravity, float delta)
{
	actor->verticalVelocity -= gravity * delta;

	// Limit vertical speeds
	if (actor->verticalVelocity > actor->verticalSpeedLimitUp)
	{
		actor->verticalVelocity = actor->verticalSpeedLimitUp;
	}
	else if (actor->verticalVelocity < 0)
	{
		// If falling, stop velocity when hits ground or limit max falling speed
		if (Flag_IsBitSet(actor->lastMoveResultFlags, Move_OnGround))
		{
			actor->verticalVelocity = 0.0f;
		}
		else if (actor->verticalVelocity < actor->verticalSpeedLimitDown)
		{
			actor->verticalVelocity = actor->verticalSpeedLimitDown;
		}
	}
	return actor->elevation + actor->verticalVelocity * delta;
}

/*
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

*/
BunnyV2* Actor_GetPosition(Actor* actor)
{
	return &actor->position.bunnyPosition;
}
BunnyV2 * Actor_GetFloorDirection(Actor* actor)
{
	return &actor->moveDirection.bunnyDirection;
}
void Actor_SetPosition(Actor* actor, float x, float y)
{
	actor->position.vectorPosition.x = x;
	actor->position.vectorPosition.y = y;
}

void Actor_StartAction(Actor* actor, ACTOR_ACTION_FLAGS flags)
{
	actor->actionFlags = Flag_SetAll(actor->actionFlags, flags);
}
void Actor_EndAction(Actor* actor, ACTOR_ACTION_FLAGS flags)
{
	actor->actionFlags = Flag_UnsetBit(actor->actionFlags, flags);
}
bool Actor_IsDoing(Actor* actor, ACTOR_ACTION_FLAGS flags)
{
	return Flag_IsBitSet(actor->actionFlags, flags);
}


