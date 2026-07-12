#include "scriptbunnysector.h"
#include "bunny-sector_main.h"
#include <mgdl/mgdl-angelscript.h>

void RegisterBunnySector(mgdl_AngelScript* angel)
{
	asIScriptEngine* as_engine = angel->engine;
	as_engine->RegisterTypedef("MapId", "int");

	as_engine->RegisterGlobalFunction("bool BunnySector_Init()", asFUNCTION(BunnySector_Init), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("MapId BunnySector_LoadMap(const zstr &in mapfilename)", asFUNCTIONPR(BunnySector_LoadMap, (const zstr&), MapId), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_StartMap(MapId mapId)", asFUNCTION(BunnySector_StartMap), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_RenderMap(MapId mapId)", asFUNCTION(BunnySector_RenderMap), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_UpdateMap(MapId mapId, float deltaTime)", asFUNCTION(BunnySector_UpdateMap), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetActorDriveInput(int actorId, float forward, float strafe, float vertical, float turnYaw, float turnPitch)", asFUNCTION(BunnySector_SetActorDriveInput), asCALL_CDECL);
}
