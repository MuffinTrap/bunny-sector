#pragma once

enum MoveResultBit
{
    Move_Ok = 0,
    Move_HitWall = 1,
    Move_HitPortal = 2,
    Move_OnGround = 3, // Set when actor is standing on floor
    Move_Cancel = 4
};
typedef enum MoveResultBit MoveResultBit;

enum SpriteAlignment
{
    Sprite_FACE, ///< Billboard
    Sprite_WALL, ///< Not billboard, drawn like wall
    Sprite_FLOOR ///< Flat on floor or ceiling
};
typedef enum SpriteAlignment SpriteAlignment;

#define SPRITE_WALL_ALIGNED_BIT 4
#define SPRITE_FLOOR_ALIGNED_BIT 5
#define SPRITE_PIVOT_BIT 7
#define SPRITE_INVISIBLE_BIT 15
enum SpritePivot
{
    Sprite_PivotCenter, // Center is position
    Sprite_PivotFoot    // center is position + height/2
};
typedef enum SpritePivot SpritePivot;

enum SpriteLOTAG
{
    LOTAG_Multiplayer_Start = 90,
    LOTAG_Level_End = 65535
};
typedef enum SpriteLOTAG SpriteLOTAG;

struct SectorRender
{
    s16 number;
    float limitLeft; // Field of view limits or portal limits
    float limitRight;
};
typedef struct SectorRender SectorRender;

// TODO Use this instead of Player
struct Viewpoint
{
    Vector3 position;
    float yawRad;
    float pitchRad;
    s16 sector;
};
typedef struct Viewpoint Viewpoint;
