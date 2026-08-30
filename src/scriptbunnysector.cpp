#include "scriptbunnysector.h"
#include "bunny-sector_main.h"
#include "bunny-sector-map.h"
#include <mgdl/mgdl-angelscript.h>
#include "duke/dukemap.h"
#include "duke/actor.h"
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

	as_engine->RegisterGlobalFunction("s16 BunnySector_GetSectorAmount()", asFUNCTION(BunnySector_GetSectorAmount), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("Sector@ BunnySector_GetSector(s16 sectorNumber)", asFUNCTION(BunnySector_GetSector), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("Wall@ BunnySector_GetWall(s16 wallIndex)", asFUNCTION(BunnySector_GetWall), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("Wall@ BunnySector_GetWallEnd(Wall@ wall)", asFUNCTION(BunnySector_GetWallEnd), asCALL_CDECL);
}

void RegisterDoomMap(mgdl_AngelScript* angel)
{
	asIScriptEngine* as_engine = angel->engine;

	as_engine->RegisterObjectType("DoomVertex", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomVertex", "float x", asOFFSET(DoomVertex, x));
		as_engine->RegisterObjectProperty("DoomVertex", "float y", asOFFSET(DoomVertex, y));

	as_engine->RegisterObjectType("DoomSegment", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomSegment", "u32 v1", asOFFSET(DoomSegment, v1));
		as_engine->RegisterObjectProperty("DoomSegment", "u32 partnerSegment", asOFFSET(DoomSegment, partnerSegment));
		as_engine->RegisterObjectProperty("DoomSegment", "u16 linedef", asOFFSET(DoomSegment, linedef));
		as_engine->RegisterObjectProperty("DoomSegment", "u8 lineSide", asOFFSET(DoomSegment, lineSide));

	as_engine->RegisterObjectType("DoomLinedef", 0, asOBJ_REF|asOBJ_NOCOUNT);
		as_engine->RegisterObjectProperty("DoomLinedef", "int sidefront", asOFFSET(DoomLinedef, sidefront));
		as_engine->RegisterObjectProperty("DoomLinedef", "int sideback", asOFFSET(DoomLinedef, sideback));

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
	as_engine->RegisterGlobalFunction("void BunnySector_StartMap(MapId mapId)", asFUNCTION(BunnySector_StartMap), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_RenderMap(MapId mapId)", asFUNCTION(BunnySector_RenderMap), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_UpdateMap(MapId mapId, float deltaTime)", asFUNCTION(BunnySector_UpdateMap), asCALL_CDECL);


	as_engine->RegisterEnum("BunnyMapType");
	as_engine->RegisterEnumValue("BunnyMapType", "Map_Duke", (int)Map_Duke);
	as_engine->RegisterEnumValue("BunnyMapType", "Map_Doom", (int)Map_Doom);
	as_engine->RegisterEnumValue("BunnyMapType", "Map_Invalid", (int)Map_Invalid);

	as_engine->RegisterGlobalFunction("BunnyMapType BunnySector_GetMapType(MapId mapid)", asFUNCTION(BunnySector_GetMapType), asCALL_CDECL);


	// Register other types
	as_engine->RegisterObjectType("buns_Vec2", sizeof(buns_Vec2), asOBJ_VALUE|asOBJ_POD);
	as_engine->RegisterObjectProperty("buns_Vec2", "float x", asOFFSET(buns_Vec2, x));
	as_engine->RegisterObjectProperty("buns_Vec2", "float y", asOFFSET(buns_Vec2, y));

	as_engine->RegisterObjectType("buns_Vec3", sizeof(buns_Vec3), asOBJ_VALUE|asOBJ_POD);
	as_engine->RegisterObjectProperty("buns_Vec3", "float x", asOFFSET(buns_Vec3, x));
	as_engine->RegisterObjectProperty("buns_Vec3", "float y", asOFFSET(buns_Vec3, y));
	as_engine->RegisterObjectProperty("buns_Vec3", "float z", asOFFSET(buns_Vec3, z));


	RegisterDoomMap(angel);
	RegisterDukeMap(angel);


	// Register functions to access map data

	// Register OpenGL Drawing functions
	as_engine->RegisterGlobalFunction("void BunnySector_SetOpenGLUnitsToMeter(float scale)", asFUNCTION(BunnySector_SetOpenGLUnitsToMeter), asCALL_CDECL);


	as_engine->RegisterGlobalFunction("void BunnySector_DrawSectorFloorOrCeiling(s16 sectorNumber, bool floor, s16 picnum, s8 shade)", asFUNCTION(BunnySector_DrawSectorFloorOrCeiling), asCALL_CDECL);

	as_engine->RegisterGlobalFunction("void BunnySector_StartWallDrawing()", asFUNCTION(BunnySector_StartWallDrawing), asCALL_CDECL);

	as_engine->RegisterGlobalFunction("void BunnySector_EndWallDrawing()", asFUNCTION(BunnySector_EndWallDrawing), asCALL_CDECL);

	as_engine->RegisterGlobalFunction("void BunnySector_Setup3D(float aspectView, float aspectCamera)", asFUNCTION(BunnySector_Setup3D), asCALL_CDECL);

	as_engine->RegisterGlobalFunction("void BunnySector_AlignCameraToActor(int actorId)", asFUNCTION(BunnySector_AlignCameraToActor), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_DrawWallF(float startx, float starty, float endx, float endy, float normalx, float normalz, s32 floory, s32 ceilingy, s16 picnum, s8 shade)", asFUNCTION(BunnySector_DrawWallF), asCALL_CDECL);

	// NOTE These are handles = pointers, not references to value objects
	as_engine->RegisterGlobalFunction("void BunnySector_DrawWall(Wall@ start, Wall@ end, s32 floory, s32 ceilingy, s16 picnum, s8 shade)", asFUNCTION(BunnySector_DrawWall), asCALL_CDECL);

	// Register Camera functions
as_engine->RegisterGlobalFunction("void BunnySector_DrawCameraInfo(float x, float y)", asFUNCTION(BunnySector_DrawCameraInfo), asCALL_CDECL);

as_engine->RegisterGlobalFunction("float BunnySector_GetOpenGLCameraVerticalFOVDeg()", asFUNCTION(BunnySector_GetOpenGLCameraVerticalFOVDeg), asCALL_CDECL);
as_engine->RegisterGlobalFunction("void BunnySector_SetOpenGLCameraVerticalFOVDeg(float degrees)",asFUNCTION(BunnySector_SetOpenGLCameraVerticalFOVDeg), asCALL_CDECL);


	// Register Actor related types and functions
	// ACTOR
	as_engine->RegisterObjectType("Actor", 0, asOBJ_REF|asOBJ_NOCOUNT);
	as_engine->RegisterObjectProperty("Actor", "float yawRad", asOFFSET(Actor, yawRad));
	as_engine->RegisterObjectProperty("Actor", "s16 sectorNumber", asOFFSET(Actor, sectorNumber));
	as_engine->RegisterObjectProperty("Actor", "float elevation", asOFFSET(Actor, elevation));
	as_engine->RegisterObjectProperty("Actor", "bool noclip", asOFFSET(Actor, noclip));
	as_engine->RegisterObjectProperty("Actor", "float radius", asOFFSET(Actor, radius));
	as_engine->RegisterObjectMethod("Actor", "DoomVertex@ GetDoomPosition()", asFUNCTION(Actor_GetDoomPosition), asCALL_CDECL_OBJFIRST);

	// ACTOR FUNCTIONS
	as_engine->RegisterGlobalFunction("Actor@ BunnySector_GetActor(int actorId)", asFUNCTION(BunnySector_GetActor), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("float BunnySector_GetActorRadius(int actorId)", asFUNCTION(BunnySector_GetActorRadius), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_GetActorPositionV2(int actorId, buns_Vec2 &out pos)", asFUNCTION(BunnySector_GetActorPositionV2), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_GetActorPositionV3(int actorId, buns_Vec3 &out pos)", asFUNCTION(BunnySector_GetActorPositionV2), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_GetActorFloorDir(int actorId, buns_Vec2 &out dir)", asFUNCTION(BunnySector_GetActorFloorDir), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetActorPosition(int actorId, float x, float z)", asFUNCTION(BunnySector_SetActorPosition), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetActorSpeeds(int actorId, float walkSpeedMultiplier, float turnSPeedMultiplier)", asFUNCTION(BunnySector_SetActorSpeeds), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_SetActorDriveInput(int actorId, float forward, float strafe, float vertical, float turnYaw, float turnPitch)", asFUNCTION(BunnySector_SetActorDriveInput), asCALL_CDECL);
	as_engine->RegisterGlobalFunction("void BunnySector_MoveActorFreely(int actorId, float deltaTime)", asFUNCTION(BunnySector_MoveActorFreely), asCALL_CDECL);

}
