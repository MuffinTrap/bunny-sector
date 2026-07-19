#pragma once
		#include <mgdl/mgdl-vectorfunctions.h>
		#include <mgdl/mgdl-types.h>
#include "duke_types.h"
// Forward def
struct Actor;
struct Tesselator_BufferIndices;
struct Viewpoint;

struct MapFloorVertexData
{
    // Store floor vertices of each sector to buffer
    // This buffer needs to hold all the vertices of every floor
    GLfloat* floorBuffer = nullptr; // All vertices of all floors: 3 position 2 uv
    static const u16 FLOOR_BUFFER_VERTEX_SIZE = 5; ///< How many floats per vertex
    u16 floorBufferSizeVertices = 0;

    GLushort* floorIndexBuffer = nullptr; // All indices of all floors
    u32 floorIndexBufferSize = 0;

    Tesselator_BufferIndices* floorStartIndices = nullptr; // Buffer end indices of each floor in vertex and index buffers: NOTE First floor starts at indices (0,0)
};
typedef struct MapFloorVertexData MapFloorVertexData;

struct MapSprite
{
    // Game info

    // From file
    // s32 x, y, z; ///< Sprite position in map
    Vector3 position;
    u16 cstat; ///< Type of sprite bitfield. Use DukeMap_GetSpriteAlignment
    s16 picnum; ///< Texture index
    s8 shade;
    u8 pal;
    u8 clipdist; ///< Size in map as square, only used in FACE alignment
    u8 filler;
    u8 xrepeat, yrepeat; ///< How many pixels wide and tall : Default 64x64
    s8 xoffset, yoffset; ///< Offset of center
    s16 sectnum;
    s16 statnum; ///< Status: inactive, bullet, monster etc...
    s16 ang; ///< Facing angle
    s16 owner; // Owning player index
    s16 xvel, yvel, zvel; ///< Velocity
    u16 lotag, hitag;
    s16 extra;
};
struct Wall
{
    // GLdouble glutVertices[3]; ///< GLUT tesselation needs this
    // From file
    s32 x, z; ///< Coordinates of the left side. Right side is left side of next wall.
    s16 point2; ///< Index of next wall in sector's walls.
    s16 nextwall; ///< Index of wall on the other side or -1 if no sector there
    s16 nextsector; ///< Index of sector on the other side or -1
    u16 cstat; ///< Stats about wall
    s16 picnum; ///< Texture number
    s16 overpicnum; ///< Texture number for masked and one-way walls
    s8 shade; /// Offset to shade: darker or brighter
    u8 pal;  ///< Palette index
    u8 xrepeat, yrepeat;  ///< Repeat texture more times
    u8 xpanning, ypanning; //< Texture offset
    u16 lotag, hitag;
    s16 extra;

};
typedef struct Wall Wall;

struct Sector
{
    // From file
    s16 wallptr; /**< Index of first wall */
    s16 wallnum; /**< and amount of walls in this sector */
    s32 ceilingy;
    s32 floory; ///< Y of ceiling and floor of first point
    u16 ceilingstat, floorstat; ///< Stats about ceiling and floor
    s16 ceilingpicnum; ///< Texture of ceiling
    s16 ceilingheinum; ///< Sloping angle 0:flat, 4096: 45 degrees
    s8 ceilingshade;
    u8 ceilingpal, ceilingxpanning, ceilingypanning; ///< Palette index and texture olic ffsets
    s16 floorpicnum, floorheinum;
    s8 floorshade;
    u8 floorpal, floorxpanning, floorypanning;
    u8 visibility; ///< How distance affects shading
    u8 filler; ///< Padding byte
    u16 lotag, hitag; ///< Game specific info
    s16 extra;

    // For texture coordinates
    Vector2 minXZPoint;
    Vector2 sizeXZ;
    Vector2 maxTexCoord;
};
typedef struct Sector Sector;


struct DukeMap
{
    zstr mapfile;
    s32 version;
    Vector2 startPosition;
    float startElevation;
    s16 startAngle;

    s16 startingSector;

    s16 sectorAmount;
    Sector* sectors;
    s16 wallAmount;
    Wall* walls;
    s16 spriteAmount;
    MapSprite* sprites;

    MapFloorVertexData floorVertexData;
    float lowY;
    float highY;
};
typedef struct DukeMap DukeMap;


#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief Convert the information loaded from the file into game units and enums.
     * @param map The map to convert.
     */
void Map_ConvertToGameUnits(DukeMap* map);
void Map_FindIslandSectors(DukeMap* map);
void Map_PrintInfo(DukeMap* map);
void Map_SetCameraToStart(DukeMap* map, Viewpoint* view);
void Map_SetActorToStart(DukeMap* map, Actor* actor);
void Map_InitActor(DukeMap* map, Actor* player);
void Map_InitActors(DukeMap* map, Actor* players, int playerAmount);

Sector* Map_GetSector(DukeMap* map, s16 sectorNumber);
s32 Map_GetSectorFloorHeight(DukeMap* map, s16 sectorNumber);
s32 Map_GetSectorCeilingHeight(DukeMap* map, s16 sectorNumber);

Wall* Map_GetWallInSector(DukeMap* map, s16 sector, s16 wi);
Wall* Map_GetWallInSectorPtr(DukeMap* map, Sector* sector, s16 wi);
Wall* Map_GetWallEnd(DukeMap* map, Wall* w);
Wall* Map_GetWall(DukeMap* map, s16 wallIndex);
Vector2 Map_GetWallMiddle(DukeMap* map, Wall* w);
Vector2 Map_GetWallNormal(DukeMap* map, Wall* w);

SpriteAlignment Sprite_GetAlignment(MapSprite* sprite);
SpritePivot Sprite_GetPivot(MapSprite* sprite);
MapSprite* Map_GetSprite(DukeMap* map, s16 spriteIndex);

/**
 * @brief Searches for and returns the first sprite with matching tags
 * @param lotag Lotag of the sprite
 * @param hitag Hitag of the sprite
 * @return First matching sprite or nullptr if none found
 */
MapSprite* Map_FindSprite(DukeMap* map, s16 lotag, s16 hitag);

bool Map_IsPointInsideSectorOG(DukeMap* map, Vector2 point, int sectorNumber);
bool Map_IsPointInsideSectorRay(DukeMap* map, Vector2 point, int sectorNumber);
bool Map_IsPointInsideWall(DukeMap* map, Vector2 point, Wall* wall);
bool Map_FindIntersectionWithWall(DukeMap* map, Vector2 moveStart, Vector2 moveEnd, Wall* wall, Vector2* pointOUT);

/**
 * @brief Moves actor in map, asking actor where it is going.
 * @param map The map
 * @param deltaTime
 * @param [inout] actor The actor that is moved. The position of actor is modified. Movement result is stored in actor
*/ 
void Map_MoveActorInMap(DukeMap* map, float deltaTime, Actor* inoutActor);

MoveResult Map_MovePointInMap(DukeMap* map, Vector2 start, Vector2 end, s16 sectorNumber, bool ignoreCollision, Vector2* positionOut, s16* sectorOut);

/**
* @brief Looks for player recursively from neighbouring sectors, starting from startingSector
* @returns The sector number where player is or -1 if not inside map
*/
s16 Map_FindSector(DukeMap* map, s16 startingSector, Vector3 position);
s16 Map_FindSectorV2(DukeMap* map, s16 startingSector, Vector2 position);

s16 Map_GetSectorNeighbor(DukeMap* map, s16 sectorNumber, s16 wallIndex);

#ifdef __cplusplus
}
#endif
