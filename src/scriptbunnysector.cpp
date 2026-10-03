#include "scriptbunnysector.h"
#include "bunny-sector_main.h"
#include "bunny-sector-map.h"
#include <mgdl/mgdl-angelscript.h>
#include "duke/dukemap.h"
#include "gameplay/actor.h"
#include "doom/doom_types.h"
#include <mgdl.h>

void RegisterDukeMap(mgdl_AngelScript* angel)
{

	asIScriptEngine* as_engine = angel->engine;
	// Register Duke Map types as uninstantiable reference types

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

	as_engine->RegisterGlobalFunction("Wall@ BunnySector_GetWallEnd(Wall@ wall)", asFUNCTION(BunnySector_GetWallEnd), asCALL_CDECL);

	as_engine->RegisterObjectType("DukeMap", 0, asOBJ_REF|asOBJ_NOCOUNT);
	as_engine->RegisterObjectMethod("DukeMap", "Wall@ get_walls(uint index) property", asFUNCTION(DukeMap_GetWall), asCALL_CDECL_OBJFIRST);
	as_engine->RegisterObjectMethod("DukeMap", "Sector@ get_sectors(uint index) property", asFUNCTION(DukeMap_GetSector), asCALL_CDECL_OBJFIRST);
	as_engine->RegisterObjectMethod("DukeMap", "Wall@ GetWallEnd(Wall@ start)", asFUNCTION(DukeMap_GetWallEnd), asCALL_CDECL_OBJFIRST);
	as_engine->RegisterObjectProperty("DukeMap", "s16 sectorAmount", asOFFSET(DukeMap, sectorAmount));

	as_engine->RegisterGlobalFunction("DukeMap@ BunnySector_GetDukeMap(MapId mapId)", asFUNCTION(BunnySector_GetDukeMap), asCALL_CDECL);
}


void RegisterDoomMap(mgdl_AngelScript* angel)
{
	asIScriptEngine* as_engine = angel->engine;

	as_engine->RegisterEnum("DOOM_EDITOR_NUMBER");
	as_engine->RegisterEnumValue("DOOM_EDITOR_NUMBER", "editorNumber_blue_card", (int)editorNumber_blue_card);

	as_engine->RegisterObjectType("DoomVertex", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomVertex", "float x", asOFFSET(DoomVertex, x));
		as_engine->RegisterObjectProperty("DoomVertex", "float y", asOFFSET(DoomVertex, y));

	as_engine->RegisterObjectType("DoomSegment", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomSegment", "u32 v1", asOFFSET(DoomSegment, v1));
		as_engine->RegisterObjectProperty("DoomSegment", "u32 partnerSegment", asOFFSET(DoomSegment, partnerSegment));
		as_engine->RegisterObjectProperty("DoomSegment", "u16 linedef", asOFFSET(DoomSegment, linedef));
		as_engine->RegisterObjectProperty("DoomSegment", "u8 lineSide", asOFFSET(DoomSegment, lineSide));
		as_engine->RegisterObjectProperty("DoomSegment", "int neighbourSubSector", asOFFSET(DoomSegment, neighbourSector));

	as_engine->RegisterObjectType("DoomLinedef", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomLinedef", "int sidefront", asOFFSET(DoomLinedef, sidefront));
		as_engine->RegisterObjectProperty("DoomLinedef", "int sideback", asOFFSET(DoomLinedef, sideback));
		as_engine->RegisterObjectProperty("DoomLinedef", "int v1", asOFFSET(DoomLinedef, v1));
		as_engine->RegisterObjectProperty("DoomLinedef", "int v2", asOFFSET(DoomLinedef, v2));

	as_engine->RegisterObjectType("DoomSidedef", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomSidedef", "int sector", asOFFSET(DoomSidedef, sector));
		as_engine->RegisterObjectProperty("DoomSidedef", "s16 texturetop", asOFFSET(DoomSidedef, texturetop));
		as_engine->RegisterObjectProperty("DoomSidedef", "s16 texturemiddle", asOFFSET(DoomSidedef, texturemiddle));
		as_engine->RegisterObjectProperty("DoomSidedef", "s16 texturebottom", asOFFSET(DoomSidedef, texturebottom));

	as_engine->RegisterObjectType("DoomSector", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomSector", "int heightfloor", asOFFSET(DoomSector, heightfloor));
		as_engine->RegisterObjectProperty("DoomSector", "int heightceiling", asOFFSET(DoomSector, heightceiling));
		as_engine->RegisterObjectProperty("DoomSector", "u8 lightlevel", asOFFSET(DoomSector, lightlevel));

	as_engine->RegisterObjectType("DoomThing", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomThing", "float x", asOFFSET(DoomThing, x));
		as_engine->RegisterObjectProperty("DoomThing", "float y", asOFFSET(DoomThing, y));

	as_engine->RegisterTypedef("ChildId", "uint");

	as_engine->RegisterObjectType("DoomNode", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomNode", "s16 x", asOFFSET(DoomNode, x));
		as_engine->RegisterObjectProperty("DoomNode", "s16 y", asOFFSET(DoomNode, y));
		as_engine->RegisterObjectProperty("DoomNode", "s16 dx", asOFFSET(DoomNode, dx));
		as_engine->RegisterObjectProperty("DoomNode", "s16 dy", asOFFSET(DoomNode, dy));
		as_engine->RegisterObjectMethod("DoomNode", "s16 get_bbox0(uint) property", asFUNCTION(DoomNode_GetBBox0), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomNode", "s16 get_bbox1(uint) property", asFUNCTION(DoomNode_GetBBox1), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomNode", "s16 get_bbox(uint) property", asFUNCTION(DoomNode_GetBBox), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomNode", "ChildId get_children(uint) property", asFUNCTION(DoomNode_GetChild), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomNode", "int GetChildSide(float x, float y)", asFUNCTION(DoomNode_GetChildSide), asCALL_CDECL_OBJFIRST);

		as_engine->RegisterGlobalProperty("const int BB_TOP", &BB_TOP);
		as_engine->RegisterGlobalProperty("const int BB_BOT", &BB_BOT);
		as_engine->RegisterGlobalProperty("const int BB_LFT", &BB_LFT);
		as_engine->RegisterGlobalProperty("const int BB_RGT", &BB_RGT);

	as_engine->RegisterObjectType("DoomSubSector", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomSubSector", "u32 firstSegment", asOFFSET(DoomSubSector, firstSegment));
		as_engine->RegisterObjectProperty("DoomSubSector", "u32 segmentAmount", asOFFSET(DoomSubSector, segmentAmount));


	as_engine->RegisterObjectType("DoomMap", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomMap", "int thingAmount", asOFFSET(DoomMap, thingAmount));
		as_engine->RegisterObjectProperty("DoomMap", "int sectorAmount", asOFFSET(DoomMap, sectorAmount));
		as_engine->RegisterObjectProperty("DoomMap", "int sideAmount", asOFFSET(DoomMap, sideAmount));
		as_engine->RegisterObjectProperty("DoomMap", "int lineAmount", asOFFSET(DoomMap, lineAmount));
		as_engine->RegisterObjectProperty("DoomMap", "int vertexAmount", asOFFSET(DoomMap, vertexAmount));
		as_engine->RegisterObjectProperty("DoomMap", "int segmentAmount", asOFFSET(DoomMap, segmentAmount));
		as_engine->RegisterObjectProperty("DoomMap", "int nodeAmount", asOFFSET(DoomMap, nodeAmount));
		as_engine->RegisterObjectProperty("DoomMap", "int subSectorAmount", asOFFSET(DoomMap, subSectorAmount));

		// Register DoomMap_GetX(DoomMap* map, ...) as methods of DoomMap
		as_engine->RegisterObjectMethod("DoomMap", "DoomSegment@ get_segments(uint) property", asFUNCTION(DoomMap_GetSegment), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomMap", "DoomSector@ get_sectors(uint) property", asFUNCTION(DoomMap_GetSector), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomMap", "DoomThing@ get_things(uint) property", asFUNCTION(DoomMap_GetThing), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomMap", "DoomNode@ get_nodes(uint) property", asFUNCTION(DoomMap_GetNode), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomMap", "DoomSubSector@ get_subsectors(uint) property", asFUNCTION(DoomMap_GetSubSector), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomMap", "DoomLinedef@ get_linedefs(uint) property", asFUNCTION(DoomMap_GetLinedef), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomMap", "DoomSidedef@ get_sidedefs(uint) property", asFUNCTION(DoomMap_GetSidedef), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomMap", "DoomVertex@ get_vertices(uint) property", asFUNCTION(DoomMap_GetVertex), asCALL_CDECL_OBJFIRST);

		as_engine->RegisterObjectMethod("DoomMap", "int GetActorAmount()", asFUNCTION(DoomMap_GetActorAmount), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomMap", "DoomNode@ GetRootNode()", asFUNCTION(DoomMap_GetRootNode), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomMap", "DoomNode@ GetChildNode(ChildId id)", asFUNCTION(DoomMap_GetChildNode), asCALL_CDECL_OBJFIRST);
		as_engine->RegisterObjectMethod("DoomMap", "DoomSubSector@ GetChildSubSector(ChildId id)", asFUNCTION(DoomMap_GetChildSubSector), asCALL_CDECL_OBJFIRST);

	// Functions
	as_engine->RegisterGlobalFunction("DoomMap@ BunnySector_GetDoomMap(MapId mapId)", asFUNCTION(BunnySector_GetDoomMap), asCALL_CDECL);
}

static float ANGEL_PI = M_PI;

void RegisterBunnySector(mgdl_AngelScript* angel)
{
	asIScriptEngine* as_engine = angel->engine;

	// TODO move to mgdl
	as_engine->RegisterGlobalProperty("const float M_PI", &ANGEL_PI);
	as_engine->RegisterTypedef("MapId", "int");

	as_engine->RegisterGlobalFunction("bool BunnySector_Init()", asFUNCTION(BunnySector_Init), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("MapId BunnySector_LoadMap(const zstr &in mapfilename)", asFUNCTIONPR(BunnySector_LoadMap, (const zstr&), MapId), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_StartMap(MapId mapId, int playerAmount)", asFUNCTION(BunnySector_StartMap), asCALL_CDECL);

	as_engine->RegisterEnum("ActorActionBit");
	as_engine->RegisterEnumValue("ActorActionBit", "action_use",(int)action_use);
	as_engine->RegisterEnumValue("ActorActionBit", "action_shoot",(int)action_shoot);
	as_engine->RegisterEnumValue("ActorActionBit", "action_jump", (int)action_jump);


	as_engine->RegisterEnum("GameStatus");
	as_engine->RegisterEnumValue("GameStatus", "status_menu",(int)status_menu);
	as_engine->RegisterEnumValue("GameStatus", "status_player_alive",(int)status_player_alive);
	as_engine->RegisterEnumValue("GameStatus", "status_player_dead", (int)status_player_dead);
	as_engine->RegisterEnumValue("GameStatus", "status_exit_normal", (int)status_exit_normal);

	as_engine->RegisterEnum("BunnyMapType");
	as_engine->RegisterEnumValue("BunnyMapType", "Map_Duke", (int)Map_Duke);
	as_engine->RegisterEnumValue("BunnyMapType", "Map_Doom", (int)Map_Doom);
	as_engine->RegisterEnumValue("BunnyMapType", "Map_Invalid", (int)Map_Invalid);

	as_engine->RegisterEnum("ActorType");
	as_engine->RegisterEnumValue("ActorType", "actor_player", (int)actor_player);
	as_engine->RegisterEnumValue("ActorType", "actor_item", (int)actor_item);
	as_engine->RegisterEnumValue("ActorType", "actor_monster", (int)actor_monster);
    	as_engine->RegisterEnumValue("ActorType", "actor_projectile",(int)actor_projectile);
    	as_engine->RegisterEnumValue("ActorType", "actor_decoration",(int)actor_decoration);
    	as_engine->RegisterEnumValue("ActorType", "actor_particle_emitter",(int)actor_particle_emitter);

	as_engine->RegisterGlobalFunction("zstr ActorTypeToString(ActorType atype)", asFUNCTION(ActorTypeToString), asCALL_CDECL);


	as_engine->RegisterGlobalFunction("BunnyMapType BunnySector_GetMapType(MapId mapid)", asFUNCTION(BunnySector_GetMapType), asCALL_CDECL);


	// Register callback hooks that script can connect to

	// Find custom anglescript function
	as_engine->RegisterFuncdef("void Callback()");
	as_engine->RegisterGlobalFunction("void BunnySector_SetAfterCollisionCallback(Callback @cb)", asFUNCTION(BunnySector_SetAfterCollisionCallback), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetRenderingCallback(Callback @cb)", asFUNCTION(BunnySector_SetRenderingCallback), asCALL_CDECL);


	// Register getting and setting the game state
	as_engine->RegisterGlobalFunction("GameStatus BunnySector_GetGameStatus()", asFUNCTION(BunnySector_GetGameStatus), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetGameStatus(GameStatus status)", asFUNCTION(BunnySector_SetGameStatus), asCALL_CDECL);

	// Register other types


	RegisterDoomMap(angel);
	RegisterDukeMap(angel);


	// Register functions to access map data

	// Register OpenGL Drawing functions
	as_engine->RegisterGlobalFunction("void BunnySector_SetOpenGLUnitsToMeter(float scale)", asFUNCTION(BunnySector_SetOpenGLUnitsToMeter), asCALL_CDECL);


	as_engine->RegisterGlobalFunction("void BunnySector_StartFloorCeilingDrawing()", asFUNCTION(BunnySector_StartFloorCeilingDrawing), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_DrawSectorFloorOrCeiling(s16 sectorNumber, bool floor )", asFUNCTION(BunnySector_DrawSectorFloorOrCeiling), asCALL_CDECL);

	as_engine->RegisterGlobalFunction("void BunnySector_StartMapDrawing()", asFUNCTION(BunnySector_StartMapDrawing), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_DrawMapActorsForPlayer(int playerIndex)", asFUNCTION(BunnySector_DrawMapActorsForPlayer), asCALL_CDECL);

	as_engine->RegisterGlobalFunction("void BunnySector_EndMapDrawing()", asFUNCTION(BunnySector_EndMapDrawing), asCALL_CDECL);

	as_engine->RegisterGlobalFunction("void BunnySector_Setup3D(float aspectView, float aspectCamera)", asFUNCTION(BunnySector_Setup3D), asCALL_CDECL);

	as_engine->RegisterGlobalFunction("void BunnySector_AlignCameraToPlayer(int playerIndex)", asFUNCTION(BunnySector_AlignCameraToPlayer), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_DrawWallF(float startx, float starty, float endx, float endy, float normalx, float normalz, s32 floory, s32 ceilingy, s16 picnum, s8 shade)", asFUNCTION(BunnySector_DrawWallF), asCALL_CDECL);

	// NOTE These are handles = pointers, not references to value objects
	as_engine->RegisterGlobalFunction("void BunnySector_DrawWall(Wall@ start, Wall@ end, s32 floory, s32 ceilingy, s16 picnum, s8 shade)", asFUNCTION(BunnySector_DrawWall), asCALL_CDECL);

	// Register Camera functions
	as_engine->RegisterGlobalFunction("void BunnySector_DrawCameraInfo(float x, float y)", asFUNCTION(BunnySector_DrawCameraInfo), asCALL_CDECL);

	as_engine->RegisterGlobalFunction("float BunnySector_GetOpenGLCameraVerticalFOVDeg()", asFUNCTION(BunnySector_GetOpenGLCameraVerticalFOVDeg), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetOpenGLCameraVerticalFOVDeg(float degrees)",asFUNCTION(BunnySector_SetOpenGLCameraVerticalFOVDeg), asCALL_CDECL);

	as_engine->RegisterObjectType("BunnyV2", 0, asOBJ_REF|asOBJ_NOCOUNT);
	as_engine->RegisterObjectProperty("BunnyV2", "float x", asOFFSET(BunnyV2, x));
	as_engine->RegisterObjectProperty("BunnyV2", "float y", asOFFSET(BunnyV2, y));


	// Register Actor related types and functions
	// ACTOR
	as_engine->RegisterObjectType("Actor", 0, asOBJ_REF|asOBJ_NOCOUNT);
	as_engine->RegisterObjectProperty("Actor", "float yawRad", asOFFSET(Actor, yawRad));
	as_engine->RegisterObjectProperty("Actor", "s16 sectorNumber", asOFFSET(Actor, subSectorNumber));
	as_engine->RegisterObjectProperty("Actor", "float elevation", asOFFSET(Actor, elevation));
	as_engine->RegisterObjectProperty("Actor", "bool noclip", asOFFSET(Actor, noclip));
	as_engine->RegisterObjectProperty("Actor", "float radius", asOFFSET(Actor, radius));
	as_engine->RegisterObjectProperty("Actor", "float verticalVelocity", asOFFSET(Actor, verticalVelocity));
	as_engine->RegisterObjectProperty("Actor", "int typeNumber", asOFFSET(Actor, typeNumber));
	as_engine->RegisterObjectProperty("Actor", "ActorType actorType", asOFFSET(Actor, actorType));
	as_engine->RegisterObjectMethod("Actor", "BunnyV2@ GetPosition()", asFUNCTION(Actor_GetPosition), asCALL_CDECL_OBJFIRST);
	as_engine->RegisterObjectMethod("Actor", "BunnyV2@ GetFloorDirection()", asFUNCTION(Actor_GetFloorDirection), asCALL_CDECL_OBJFIRST);
	as_engine->RegisterObjectMethod("Actor", "void SetPosition(float x, float y)", asFUNCTION(Actor_SetPosition), asCALL_CDECL_OBJFIRST);
	as_engine->RegisterObjectMethod("Actor", "bool IsDoing(ActorActionBit actionBit)", asFUNCTION(Actor_IsDoing), asCALL_CDECL_OBJFIRST);

	// ACTOR FUNCTIONS
	as_engine->RegisterGlobalFunction("Actor@ BunnySector_GetActorByIndex(int actorIndex)", asFUNCTION(BunnySector_GetActorByIndex), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("Actor@ BunnySector_GetActorById(int actorIndex)", asFUNCTION(BunnySector_GetActorById), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("Actor@ BunnySector_GetPlayerActor(int playerIndex)", asFUNCTION(BunnySector_GetPlayerActor), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetPlayerSpeeds(int playerIndex, float walkSpeedMultiplier, float turnSPeedMultiplier)", asFUNCTION(BunnySector_SetPlayerSpeeds), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetPlayerDriveInput(int actorId, float forward, float strafe, float vertical, float turnYaw, float turnPitch)", asFUNCTION(BunnySector_SetPlayerDriveInput), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("int BunnySector_GetActorCollisionAmount(Actor@ actor)", asFUNCTION(BunnySector_GetActorCollisionAmount), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("Actor@ BunnySector_GetActorCollisionAt(Actor@ actor, int index)", asFUNCTION(BunnySector_GetActorCollisionAt), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_DestroyActor(Actor@ actor)", asFUNCTION(BunnySector_DestroyActor), asCALL_CDECL);

	// Player functions
	as_engine->RegisterGlobalFunction("bool BunnySector_GivePlayerItem(int playerIndex, int itemType, int amount)", asFUNCTION(BunnySector_GivePlayerItem), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("int BunnySector_GetPlayerItemCount(int playerIndex, int itemType)", asFUNCTION(BunnySector_GetPlayerItemCount), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_StartPlayerAction(int playerIndex, ActorActionBit actionBit)", asFUNCTION(BunnySector_StartPlayerAction), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_StopPlayerAction(int playerIndex, ActorActionBit actionBit)", asFUNCTION(BunnySector_StopPlayerAction), asCALL_CDECL);
}
