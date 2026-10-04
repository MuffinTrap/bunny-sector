#pragma once



#define SPRITE_WALL_ALIGNED_BIT 4
#define SPRITE_FLOOR_ALIGNED_BIT 5
#define SPRITE_PIVOT_BIT 7
#define SPRITE_INVISIBLE_BIT 15

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

