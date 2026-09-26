#include "bunny-sector-types.h"


static const char* ActorTypeNames[4] {
	"Player",
	"Monster",
	"Item",
	"Projectile"
};

const char* ActorTypeToString(ActorType aType)
{
	return ActorTypeNames[(int)aType];
}
