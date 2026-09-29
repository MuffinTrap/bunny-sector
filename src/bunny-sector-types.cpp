#include "bunny-sector-types.h"


static zstr ActorTypeNames[4] {
	zstr_from("Player"),
	zstr_from("Monster"),
	zstr_from("Item"),
	zstr_from("Projectile"),
};

zstr ActorTypeToString(ActorType aType)
{
	return ActorTypeNames[(int)aType];
}
