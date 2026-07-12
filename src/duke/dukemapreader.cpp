#include "dukemapreader.h"
#include "binaryreader.h"
#include <mgdl.h>
#include <stdio.h>
#include "opengl-render.h"

static const int HeightToWidth = 16;

DukeMap* ReadMapFromFile(const zstr& mapfilename)
{
    return ReadMapFromFile(zstr_cstr(&mapfilename));
}
DukeMap* ReadMapFromFile(const char* mapfilename)
{
    float scale = 1.0f/1024.0f;
    Log_InfoF("Reading map file %s\n", mapfilename);
    if (!OpenBinary(mapfilename))
    {
        Log_ErrorF("No map file %s\n", mapfilename);

        return nullptr;
    }
    DukeMap* mapPtr = (DukeMap*)malloc(sizeof(DukeMap));
    DukeMap m;
    m.mapfile = zstr_from(mapfilename);
    m.version = ReadInt32();
    s32 start_x = ReadInt32(); // X coordinate
    s32 start_z = ReadInt32(); // Z Coordinate
    s32 start_y = ReadInt32() * -1 / HeightToWidth; // Y Coordinate, flipped
    m.startPosition = Vector3New(start_x, start_y, start_z);

    m.startAngle = ReadInt16();
    m.startingSector = ReadInt16();

    m.sectorAmount = ReadUInt16();
    m.sectors = (Sector*)malloc(sizeof(Sector)*m.sectorAmount);
    for (int i = 0; i < m.sectorAmount; i++)
    {
        Sector* s = &m.sectors[i];
        s->wallptr = ReadInt16();
        s->wallnum = ReadInt16();

        // NOTE These are changed to have the same unit as width and depth

        s->ceilingy = ReadInt32()/HeightToWidth * -1; // NOTE Y is up, originally was -Z
        s->floory = ReadInt32()/HeightToWidth * -1; // NOTE Y is up, originally was -Z
        s->ceilingstat = ReadInt16();
        s->floorstat = ReadInt16();
        s->ceilingpicnum = ReadInt16();
        s->ceilingheinum = ReadInt16();
        s->ceilingshade = ReadSByte();
        s->ceilingpal = ReadByte();
        s->ceilingxpanning = ReadByte();
        s->ceilingypanning = ReadByte();
        s->floorpicnum = ReadInt16();
        s->floorheinum = ReadInt16();
        s->floorshade = ReadSByte();
        s->floorpal = ReadByte();
        s->floorxpanning = ReadByte();
        s->floorypanning = ReadByte();
        s->visibility = ReadByte();
        s->filler = ReadByte();
        s->lotag = ReadInt16();
        s->hitag = ReadInt16();
        s->extra = ReadInt16();
    }
    m.wallAmount = ReadUInt16();
    m.walls = (Wall*)malloc(sizeof(Wall) * m.wallAmount);
    for (int s = 0; s < m.wallAmount; s++)
    {
        Wall* w = &m.walls[s];
        w->x = ReadInt32();
        w->z = ReadInt32();
        w->point2 = ReadInt16();
        w->nextwall = ReadInt16();
        w->nextsector = ReadInt16();
        w->cstat = ReadInt16();
        w->picnum = ReadInt16();
        w->overpicnum = ReadInt16();
        w->shade = ReadSByte();
        w->pal = ReadByte();
        w->xrepeat = ReadByte();
        w->yrepeat = ReadByte();
        w->xpanning = ReadByte();
        w->ypanning = ReadByte();
        w->lotag = ReadInt16();
        w->hitag = ReadInt16();
        w->extra = ReadInt16();

    }
    m.spriteAmount = ReadUInt16();
    m.sprites = (MapSprite*)malloc(sizeof(MapSprite) * m.spriteAmount);
    for (int i = 0; i < m.spriteAmount; i++)
    {
        MapSprite* s = &m.sprites[i];

        s32 x = ReadInt32();
        s32 y = ReadInt32();
        s32 z = ReadInt32()/ HeightToWidth * -1;
        s->position = Vector3New(x, z, y);

        s->cstat = ReadInt16();
        s->picnum = ReadInt16();
        s->shade = ReadSByte();
        s->pal = ReadByte();
        s->clipdist = ReadByte();
        s->filler = ReadByte();
        s->xrepeat = ReadByte();
        s->yrepeat = ReadByte();
        s->xoffset = ReadSByte();
        s->yoffset = ReadSByte();
        s->sectnum = ReadInt16();
        s->statnum = ReadInt16();
        s->ang = ReadInt16();
        s->owner = ReadInt16();
        s->xvel = ReadInt16();
        s->yvel = ReadInt16();
        s->zvel = ReadInt16();
        s->lotag = ReadInt16();
        s->hitag = ReadInt16();
        s->extra = ReadInt16();
    }

    CloseBinary();

    (*mapPtr) = m;

    mapPtr->lowY = 35665;
    mapPtr->highY = -36665;

    // Build other information needed
    // Build other data needed by game
    for (int si = 0; si < mapPtr->sectorAmount; si++)
    {
        Vector2 minp = Vector2New(32000, 32000);
        Vector2 maxp = Vector2New(-32000, -32000);
        Sector* sector = &mapPtr->sectors[si];
        for (s16 wi = 0; wi < sector->wallnum; wi++)
        {
            Wall* w = &mapPtr->walls[sector->wallptr + wi];
            minp.x = minF(w->x, minp.x);
            minp.y = minF(w->z, minp.y);
            maxp.x = maxF(w->x, maxp.x);
            maxp.y = maxF(w->z, maxp.y);
        }
        // Found points : calculate tex coords
        float width = (maxp.x - minp.x) * scale;
        float height = (maxp.y - minp.y) * scale;
        float aspect = width/height;
        sector->minXZPoint = minp;
        sector->sizeXZ = Vector2Subtract(maxp, minp);
        sector->maxTexCoord.x = aspect * height;
        sector->maxTexCoord.y = 1.0 * height;
        if (sector->floory < mapPtr->lowY)
        {
            mapPtr->lowY = sector->floory;
        }
        if (sector->ceilingy > mapPtr->highY)
        {
            mapPtr->highY = sector->ceilingy;
        }
    }
    // Buffer the floor and ceiling vertices: The uvs need to be calculated first
    OpenGLRender_CreateFloorBuffers(mapPtr);

    return mapPtr;
}

