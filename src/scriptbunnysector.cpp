#include "scriptbunnysector.h"
#include "bunny-sector_main.h"
#include <mgdl/mgdl-angelscript.h>
#include "duke/dukemap.h"
#include "duke/actor.h"

void RegisterBunnySector(mgdl_AngelScript* angel)
{
	asIScriptEngine* as_engine = angel->engine;
	as_engine->RegisterTypedef("MapId", "int");

	as_engine->RegisterGlobalFunction("bool BunnySector_Init()", asFUNCTION(BunnySector_Init), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("MapId BunnySector_LoadMap(const zstr &in mapfilename, int dukesPerUnit)", asFUNCTIONPR(BunnySector_LoadMap, (const zstr&, int), MapId), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_StartMap(MapId mapId)", asFUNCTION(BunnySector_StartMap), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_RenderMap(MapId mapId)", asFUNCTION(BunnySector_RenderMap), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_UpdateMap(MapId mapId, float deltaTime)", asFUNCTION(BunnySector_UpdateMap), asCALL_CDECL);



	// Register Map types as uninstantiable reference types

	// WALL
	as_engine->RegisterObjectType("Wall", 0, asOBJ_REF|asOBJ_NOCOUNT);
	as_engine->RegisterObjectProperty("Wall", "s32 x", asOFFSET(Wall, x));
	as_engine->RegisterObjectProperty("Wall", "s32 z", asOFFSET(Wall, z));
	as_engine->RegisterObjectProperty("Wall", "s16 nextsector", asOFFSET(Wall, nextsector));
	as_engine->RegisterObjectProperty("Wall", "s16 picnum", asOFFSET(Wall, picnum));
	as_engine->RegisterObjectProperty("Wall", "s16 nextwall", asOFFSET(Wall, nextwall));
	as_engine->RegisterObjectProperty("Wall", "s8 shade", asOFFSET(Wall, shade));

	// SECTOR
	as_engine->RegisterObjectType("Sector", 0, asOBJ_REF|asOBJ_NOCOUNT);
	as_engine->RegisterObjectProperty("Sector", "s16 wallptr", asOFFSET(Sector, wallptr));
	as_engine->RegisterObjectProperty("Sector", "s16 wallnum", asOFFSET(Sector, wallnum));
	as_engine->RegisterObjectProperty("Sector", "s32 ceilingy", asOFFSET(Sector, ceilingy));
	as_engine->RegisterObjectProperty("Sector", "s32 floory", asOFFSET(Sector, floory));

	// ACTOR
	as_engine->RegisterObjectType("Actor", 0, asOBJ_REF|asOBJ_NOCOUNT);
	as_engine->RegisterObjectProperty("Actor", "float yawRad", asOFFSET(Actor, yawRad));
	as_engine->RegisterObjectProperty("Actor", "s16 sectorNumber", asOFFSET(Actor, sectorNumber));
	as_engine->RegisterObjectProperty("Actor", "float elevation", asOFFSET(Actor, elevation));
	as_engine->RegisterObjectProperty("Actor", "bool noclip", asOFFSET(Actor, noclip));

	// Register other types
	as_engine->RegisterObjectType("buns_Vec2", sizeof(buns_Vec2), asOBJ_VALUE|asOBJ_POD);
	as_engine->RegisterObjectProperty("buns_Vec2", "float x", asOFFSET(buns_Vec2, x));
	as_engine->RegisterObjectProperty("buns_Vec2", "float y", asOFFSET(buns_Vec2, y));

	as_engine->RegisterObjectType("buns_Vec3", sizeof(buns_Vec3), asOBJ_VALUE|asOBJ_POD);
	as_engine->RegisterObjectProperty("buns_Vec3", "float x", asOFFSET(buns_Vec3, x));
	as_engine->RegisterObjectProperty("buns_Vec3", "float y", asOFFSET(buns_Vec3, y));
	as_engine->RegisterObjectProperty("buns_Vec3", "float z", asOFFSET(buns_Vec3, z));



	// Register functions to access map data
	as_engine->RegisterGlobalFunction("s16 BunnySector_GetSectorAmount()", asFUNCTION(BunnySector_GetSectorAmount), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("Sector@ BunnySector_GetSector(s16 sectorNumber)", asFUNCTION(BunnySector_GetSector), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("Wall@ BunnySector_GetWall(s16 wallIndex)", asFUNCTION(BunnySector_GetWall), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("Wall@ BunnySector_GetWallEnd(Wall@ wall)", asFUNCTION(BunnySector_GetWallEnd), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("Actor@ BunnySector_GetActor(int actorId)", asFUNCTION(BunnySector_GetActor), asCALL_CDECL);

	// Register OpenGL Drawing functions

	as_engine->RegisterGlobalFunction("void BunnySector_StartWallDrawing()", asFUNCTION(BunnySector_StartWallDrawing), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_EndWallDrawing()", asFUNCTION(BunnySector_EndWallDrawing), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_Setup3D()", asFUNCTION(BunnySector_Setup3D), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_AlignCameraToActor(int actorId)", asFUNCTION(BunnySector_AlignCameraToActor), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_DrawWallF(float startx, float starty, float endx, float endy, float normalx, float normalz, s32 floory, s32 ceilingy, s16 picnum, s8 shade)", asFUNCTION(BunnySector_DrawWallF), asCALL_CDECL);

	// NOTE These are handles = pointers, not references to value objects
	as_engine->RegisterGlobalFunction("void BunnySector_DrawWall(Wall@ start, Wall@ end, s32 floory, s32 ceilingy, s16 picnum, s8 shade)", asFUNCTION(BunnySector_DrawWall), asCALL_CDECL);

	// Register Camera functions

as_engine->RegisterGlobalFunction("float BunnySector_GetOpenGLCameraVerticalFOVDeg()", asFUNCTION(BunnySector_GetOpenGLCameraVerticalFOVDeg), asCALL_CDECL);
as_engine->RegisterGlobalFunction("void BunnySector_SetOpenGLCameraVerticalFOVDeg(float degrees)",asFUNCTION(BunnySector_SetOpenGLCameraVerticalFOVDeg), asCALL_CDECL);


	// Register Actor related functions
	as_engine->RegisterGlobalFunction("float BunnySector_GetActorRadius(int actorId)", asFUNCTION(BunnySector_GetActorRadius), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_GetActorPositionV2(int actorId, buns_Vec2 &out pos)", asFUNCTION(BunnySector_GetActorPositionV2), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_GetActorPositionV3(int actorId, buns_Vec3 &out pos)", asFUNCTION(BunnySector_GetActorPositionV2), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_GetActorFloorDir(int actorId, buns_Vec2 &out dir)", asFUNCTION(BunnySector_GetActorFloorDir), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetActorPosition(int actorId, float x, float z)", asFUNCTION(BunnySector_SetActorPosition), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetActorSpeeds(int actorId, float walkSpeedMultiplier, float turnSPeedMultiplier)", asFUNCTION(BunnySector_SetActorSpeeds), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetActorDriveInput(int actorId, float forward, float strafe, float vertical, float turnYaw, float turnPitch)", asFUNCTION(BunnySector_SetActorDriveInput), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_MoveActorFreely(int actorId, float deltaTime)", asFUNCTION(BunnySector_MoveActorFreely), asCALL_CDECL);

}
