#pragma once

struct Actor;

struct ActorCollisionEntry
{
	Actor* collider;
	int collisionAmount;
	int collisionStartIndex;
	bool handledByGame; /*<< Game sets this to true if it handled these collisions. If false the game will do default actions*/
};
