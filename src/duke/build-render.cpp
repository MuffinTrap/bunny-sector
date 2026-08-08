// Renders a duke map in 2D
#include <mgdl.h>
#include <mgdl/mgdl-vector.h>
#include <mgdl/mgdl-color.h>
#include <mgdl/mgdl-vectorfunctions.h>


#include "build-render.h"
#include "dukemap.h"
#include "dukemath.h"
#include "duke_types.h"
#include "opengl-render.h"

// Overlap:  Determine whether the two number ranges overlap.
#define Overlap(a0,a1,b0,b1) (min(a0,a1) <= max(b0,b1) && min(b0,b1) <= max(a0,a1))
// IntersectBox: Determine whether two 2D-boxes intersect.
#define IntersectBox(x0,y0, x1,y1, x2,y2, x3,y3) (Overlap(x0,x1,x2,x3) && Overlap(y0,y1,y2,y3))

// How many portals can be waiting for drawing
#define MAX_PORTAL_QUEUE 32
static SectorRender* renderQueue = nullptr; // Circular buffer of render requests
static s16 renderQueueInserts = 0;
// These point to renderQueue
static SectorRender* head;
static SectorRender* tail;

#define MAX_SECTOR_DRAW_TIMES 4
static u8* sectorDrawTimes = nullptr; // NOTE How many times each should be drawn. Additional times come from requests

static const s16 MAX_SECTORS = 4096; // How many sectors were in the last map loaded

static Camera* defaultCamera = nullptr;

Camera* GetDefaultCamera()
{
    if (defaultCamera == nullptr)
    {
        defaultCamera = Camera_CreateDefault();
    }
    defaultCamera->nearZ = 0.0001f;
    defaultCamera->farZ = 100.0f;
    defaultCamera->fovY = 77.7f;
    defaultCamera->projection = CameraNone;
    Camera_SetMode(defaultCamera, CameraDirection);
    return defaultCamera;
}

Viewpoint GetDefaultCameraInfo()
{
    Viewpoint info;
    info.position = Vector3Zero();
    info.pitchRad = 0.0f;
    info.yawRad = 0.0f;
    info.sector = 0;
    return info;
}

RenderSettings2D GetDefaultRenderSettings2D()
{
    RenderSettings2D render2D;
    render2D.mapOffset = Vector2New(0,0);
    render2D.mapZoom = 1.0f;
    render2D.scaleXZ = 1.0f;
    render2D.collisionPoint = Vector2New(0, 0);
    render2D.collisionLength = 100.0f;
    render2D.collisionAngleDeg = 180.0f;
    render2D.movePlayer = true;
    render2D.drawOneWall = -1;
    render2D.drawOneSector = -1;
    render2D.rotateMap= true;
    render2D.centerMapToPlayer= true;
    render2D.drawPlayersAmount = 1;
    render2D.drawWallNumbers = true;
    render2D.drawSectorNumbers = true;
    return render2D;
}
RenderSettingsOpenGL GetDefaultRenderSettingsOpenGL()
{
    float dukeUnitsPerMetre = 1024.0f;// NOTE CHECKED
    float texCoordPerMetre = 1.0f; // NOTE CHECKED

    RenderSettingsOpenGL renderGL;

    renderGL.scale = 1.0f/dukeUnitsPerMetre;
    renderGL.spriteDefaultWidth = 1024;
    renderGL.spriteDefaultHeight = 8024;

    renderGL.near = 0.1f;
    renderGL.far = 100.0f;
    renderGL.FOVyDegrees = 77.7f + 10.0f; // This is the culling fov
    renderGL.aspectRatio = mgdl_GetAspectRatio();
    return renderGL;
}

SectorRender* BuildRender_GetDrawnSectorNumbers()
{
    return renderQueue;
}
s16 BuildRender_GetDrawnSectorAmount()
{
    return renderQueueInserts;
}
bool BuildRender_WasSectorDrawn(s16 sectornumber)
{
    return sectorDrawTimes[sectornumber] > 0;
}

// TODO give renderer inteface so can use other render than OpenGL
void BuildRender_Init()
{
    if (renderQueue == nullptr)
    {
        renderQueue = (SectorRender*)mgdl_AllocateGeneralMemory(sizeof(SectorRender) * MAX_PORTAL_QUEUE);
    }

    // init again if more is needed than last time
    if (sectorDrawTimes == nullptr)
    {
        sectorDrawTimes = (u8*)mgdl_AllocateGraphicsMemory(sizeof(u8) * MAX_SECTORS);

    }

    renderQueueInserts = 0;

}

void BuildRender_ExportCurrentMapToObj(DukeMap* map, const char* filename, RenderSettingsOpenGL* settings)
{
    OpenGLRender_WriteToObj(map, filename, settings);
}

void BuildRender_DrawSprites(DukeMap* map, Viewpoint* player, RenderSettingsOpenGL* settings)
{
    // Draw all the sprites from renderer sectors
    for (int si = 0; si < map->spriteAmount; si++)
    {
        MapSprite* sprite = &map->sprites[si];
        if (BuildRender_WasSectorDrawn(sprite->sectnum)
            && !Flag_IsBitSet(sprite->cstat, SPRITE_INVISIBLE_BIT))
        {
            float scaleAspect = (float)sprite->xrepeat / (float)sprite->yrepeat;
            // The size comes from the size of the texture somehow
            float spriteSize = settings->spriteDefaultWidth;
            float spriteHeight = settings->spriteDefaultWidth * scaleAspect;

            OpenGLRender_DrawSprite(sprite->position, spriteSize, spriteHeight,
                                    Math_DukeAngleToRad(sprite->ang), player->yawRad,
                                    Sprite_GetAlignment(sprite), Sprite_GetPivot(sprite),
                                    sprite->picnum, sprite->shade);
        }
    }
}

void BuildRender_Draw3D(Viewpoint* camera, DukeMap* map, RenderSettingsOpenGL* settings)
{
        OpenGLRender_StartDrawingPolygons(settings->scale);
            BuildRender_DrawSectorWalls(camera, map, settings);
            BuildRender_DrawSectorFloorsAndCeilings(camera, map, settings);
            BuildRender_DrawSprites(map, camera, settings);
        OpenGLRender_EndDrawingPolygons();


    OpenGLRender_AnimateSprites();
}

// If there is already a request for w->nextsector
// combine the limits: otherwise it will be drawn only
// partially and the other requests are skipped
static bool ShouldAddRequest(SectorRender* head, SectorRender* tail, Wall* w, float newLimitLeft, float newLimitRight)
{
    bool addRequest = false;
    bool passedHead = false;
    int steps = 0; // Safety measure

    // Start from first request. Current request is tail-1
    SectorRender* lookAhead = (tail);

    // Look through the buffer until at tail-1
    while(lookAhead != (tail-1) && steps < MAX_PORTAL_QUEUE)
    {
        // Check when going past requests and start to wrap around
        if (passedHead)
        {
            if (sectorDrawTimes[w->nextsector] == 0)
            {
                // There was no request for it and
                // The nextsector has newer been drawn, do it now
                addRequest = true;
                break;
            }
        }
        else if (lookAhead == head)
        {
            passedHead = true;
        }

        if (lookAhead->number == w->nextsector)
        {
            if (!passedHead)
            {
                // This request is waiting, increase it's limits if
                // they were smaller than new ones
                lookAhead->limitLeft = minF(lookAhead->limitLeft, newLimitLeft);
                lookAhead->limitRight = maxF(lookAhead->limitRight, newLimitRight);
                // No need to add, since it is already waiting
                addRequest = false;
            }
            else
            {
                // This is a request that has been processed.
                // Resubmit if new limits are bigger
                if (lookAhead->limitLeft > newLimitLeft || lookAhead->limitRight < newLimitRight)
                {
                    // If they were, add the request again
                    lookAhead->limitLeft = minF(lookAhead->limitLeft, newLimitLeft);
                    lookAhead->limitRight = maxF(lookAhead->limitRight, newLimitRight);
                    // If has passed draw limit, decrease times by one
                    if (sectorDrawTimes[w->nextsector] >= MAX_SECTOR_DRAW_TIMES)
                    {
                        sectorDrawTimes -= 1;
                    }
                    addRequest = true;
                }

            }
            //Log_InfoF("Combined sector request: S %d |%.2f - %.2f|\n", w->nextsector, newLimitLeft, newLimitRight);

            // Stop looking
            break;
        }
        steps += 1;
        lookAhead += 1;

        if (lookAhead  == renderQueue + MAX_PORTAL_QUEUE)
        {
            // At the end, loop to start
            lookAhead = renderQueue;
        }
    }
    return addRequest;
}


void BuildRender_DrawSectorWalls(Viewpoint* player, DukeMap* map, RenderSettingsOpenGL* settings)
{
    for (int i = 0; i < map->sectorAmount ; i++)
    {
        sectorDrawTimes[i] = 0;
    }
    for (int i = 0; i < MAX_PORTAL_QUEUE; i++)
    {
        renderQueue[i].number = -8012; // Max sector number is 4096
    }

    // DANGER
    // When camera looks up or down, need to draw with wider
    // limits, otherwise wall are left undrawn
    // Increase fow when abs pitch is close to M_PI
    float addRelative = sin( abs(player->pitchRad));
    float betweenDegrees = 180.0f - settings->FOVyDegrees;
    float drawFovDegrees = settings->FOVyDegrees + addRelative * betweenDegrees;

    // Perspective projection values to cull walls that player
    // does not see
    float top = settings->near * tan( Deg2Rad(drawFovDegrees/2.0f));
    float right = top * settings->aspectRatio;
    float left = -right;


    // No items in buffer
    head = renderQueue;
    tail = renderQueue;

    Vector2 playerPos2 = Vector2New(player->position.x, player->position.z);

    // Put player sector draw request at tail
    *head = (SectorRender){player->sector, left, right};
    renderQueueInserts++;

    // Circular buffer pointer arithmetics
    // Move the head forward or loop around
    if ( ( head += 1) == renderQueue + MAX_PORTAL_QUEUE)
    {
        head = renderQueue;
    }

    // Draw a sector and put more sectors to queue for drawing
    do {
        // Take next request from buffer:
        SectorRender request = (*tail);
        // Mark as done in queue

        // Move tail to next one
        if ( ( tail += 1) == renderQueue + MAX_PORTAL_QUEUE)
        {
            tail = renderQueue;
        }
        // If this is drawn for maximum amount of times, skip it
        if (sectorDrawTimes[request.number] >= MAX_SECTOR_DRAW_TIMES)
        {
            continue;
        }

        // Get the sector info from map
        Sector* sector = Map_GetSector(map, request.number);
        //Log_InfoF("Draw sector %d\n", request.number);

        const s32 ceilingY = sector->ceilingy;
        const s32 floorY = sector->floory;

        // Render all walls of the current sector
        // Discard those that do not face player
        for (s16 wi = 0; wi < sector->wallnum; wi++)
        {
            Wall* w = Map_GetWallInSectorPtr(map, sector, wi);
            Vector2 start = Vector2New(w->x, w->z);
            Wall* wend = Map_GetWallEnd(map, w);
            Vector2 end =  Vector2New(wend->x, wend->z);

            Vector2 startZ = Vector2Subtract(start, playerPos2);
            Vector2 endZ = Vector2Subtract(end, playerPos2);

            // The Y is how far ahead of player the point is
            // NOTE negative around player
            startZ = Vector2Rotate(startZ, -player->yawRad);
            endZ = Vector2Rotate(endZ, -player->yawRad);

            // Is the wall behind player?
            // Behind is positive Z
            if(startZ.y >= 0 && endZ.y >= 0)
            {
                // Wall is behind
                // Draw next wall
                continue;
            }
            // Clip to view frustum and check if
            // inside it
            bool startVisible = false;
            bool endVisible = false;
            if (startZ.y < 0)
            {
                startZ.x = ( startZ.x * settings->near)/-startZ.y;
                if (startZ.x <= request.limitRight)
                {
                    // start point is visible: on the left side of right frustum wall
                    startVisible = true;
                }

            }
            if (endZ.y < 0)
            {
                endZ.x = (endZ.x * settings->near)/-endZ.y;
                if (endZ.x >= request.limitLeft)
                {
                    // end point is visible
                    endVisible = true;
                }
            }

            // End of the wall is too much to left
            // or start of the wall is too much to right
            // or end is more left than start
            // This works because walls are always going clockwise around player
            if ( (endVisible || startVisible) == false)
            {
                // Neither point is visible
                continue;
            }

            // If the player sees the whole wall, but it faces away
            // OR player is very close to long wall so that start is more right
            // than end.
            if (Map_IsPointInsideWall(map, playerPos2, w) == false)
            {
                continue;

            }
            // Calculate new limits
            float newLimitLeft = maxF(request.limitLeft, startZ.x);
            float newLimitRight = minF(endZ.x, request.limitRight);


            if (newLimitLeft > newLimitRight)
            {
                // Special case where wall is long and other point is behind player
                // Line based renderer would clip the wall to player vision edge
                // This is only done if drawing player's sector
                if (request.number == player->sector)
                {
                    if (endVisible == false)
                    {
                        // Clip to right side of view
                        newLimitRight = right;
                    }
                    else if (startVisible == false)
                    {
                        newLimitLeft = left;
                    }
                }
                else
                {
                    continue;
                }
            }


            //if it was a portal Add neighbor to queue
            // if there is neighbor AND there is room in QUEUE
            OpenGLRender_DrawWall(map, w, floorY, ceilingY, settings);
            if (w->nextsector >= 0)
            {
                // When drawing walls seen from this portal,
                // limit the view cone to the wall start and end points

                // TODO Do we win anything with this complicated check?
                // Simpler just draw max times if requested
                // and draw all floors and ceilings after drawn sectors
                // are known

                // Check that there is space left to draw
                // TODO how much is one pixel? The difference must be at least that
                if (newLimitLeft < newLimitRight)
                {
                    if  ((head + MAX_PORTAL_QUEUE+1-tail)%MAX_PORTAL_QUEUE)
                    {
                        bool addRequest = true;//ShouldAddRequest(head, tail, w, newLimitLeft, newLimitRight);

                        if (addRequest == true)
                        {
                            // The w->nextsector needs to be drawn: for first time or again
                            (*head) = {w->nextsector, newLimitLeft, newLimitRight};
                            // Move head and loop around buffer
                            if ( (++head) == renderQueue + MAX_PORTAL_QUEUE)
                            {
                                head = renderQueue;
                                renderQueueInserts++;
                            }
                            //Log_InfoF("Sector request: S %d |%.2f , %.2f|\n", w->nextsector, newLimitLeft, newLimitRight);
                        }
                    }
                }
            }
        } // All walls of the sector have been drawn; head has moved forward

        // Mark the sector as drawn
        sectorDrawTimes[request.number] += 1;

    } while(head != tail); // Render until buffer is empty: if nothing was added, they are the same
}

void BuildRender_DrawSectorFloorsAndCeilings(Viewpoint* player, DukeMap* map, RenderSettingsOpenGL* settings)
{
    // Go through all sectors
    // Draw walls and ceilings of those
    // that had any walls drawn
    OpenGLRender_StartDrawingFloorsFromBuffer(map);
    for(int i = 0; i < map->sectorAmount; i++)
    {
        if (sectorDrawTimes[i] > 0)
        {
            // Get the sector info from map
            Sector* sector = Map_GetSector(map, i);
            //Log_InfoF("Draw sector %d\n", request.number);

            float ceilingY = sector->ceilingy;
            float floorY = sector->floory;
            // Draw the floor and ceiling with tesselation
            bool floor = true;
            do {
                // Draw only floors and ceilings the player can see
                if ((floor && player->position.y >= floorY) ||
                    (!floor && player->position.y <= ceilingY))
                {
                    OpenGLRender_DrawFloorOrCeiling(map, i, floor);
                }
                floor = !floor;
                // First round: floor is false
                // Second round: floor is true
            } while(floor == false);
        }
    }
}


void BuildRender_DrawSectorRequests(RenderSettingsOpenGL* settings3D)
{
    color32 blueColor = Debug_Blue;
    color32 yellowColor = Debug_Yellow;
    Texture* df = DefaultFont_GetDefaultFont();
    int H = mgdl_GetScreenHeight();
    int W = mgdl_GetScreenWidth();

    for (int i = 0; i < MAX_PORTAL_QUEUE; i++)
    {
        SectorRender* r = &renderQueue[i];
        if (r->number > -4096)
        {
            int number = r->number;
            if (i % 2 == 0) {
                OpenGLRender_SetColor(blueColor);
            }
            else
            {
                OpenGLRender_SetColor(yellowColor);
            }
            glBegin(GL_LINES);
            int lineleft = W/2 + (r->limitLeft/settings3D->near) * W/2;
            int lineright = W/2 + (r->limitRight/settings3D->near) * W/2;
            int lineY = 16 + (i * 18);
            OpenGLRender_Line2(lineleft, H, lineleft, 0);
            OpenGLRender_Line2(lineright, H, lineright, 0);
            OpenGLRender_Line2(lineleft, lineY, lineright, lineY);
            glEnd();

            Texture_DrawTextF(df, (i%2==0) ? blueColor : yellowColor, lineleft + (lineright-lineleft)/2, lineY, 16, "%d", number);
        }
        if (i>=renderQueueInserts)
        {
            break;
        }
    }
}


void BuildRender_DrawTopDown(Viewpoint* players, DukeMap* map, RenderSettingsOpenGL* settings3D, RenderSettings2D* settings2D)
{
    Texture* df = DefaultFont_GetDefaultFont();
    int H = mgdl_GetScreenHeight();
    int W = mgdl_GetScreenWidth();

    Vector2 firstPlayerPos2 = Vector2New(players[0].position.x, players[0].position.z);

    Vector2 collision_forward = Vector2New(0.0, WORLD_FORWARD.z * settings2D->collisionLength);

    collision_forward  = Vector2Rotate(collision_forward,  Deg2Rad(settings2D->collisionAngleDeg));
    Vector2 collisionEnd = Vector2Add(settings2D->collisionPoint, collision_forward);

    bool collisionMiss = true;
    Vector2 collisionOut;

        color32 whiteColor = Debug_White;
        color32 greenColor = Debug_Green;
        color32 portalColor = Debug_Red;
        color32 wallColor = Debug_Yellow;

    // The whole map zoom
    // Put the origo on the center of the screen
    glPushMatrix();

        glTranslatef(
            W/2 + settings2D->mapOffset.x,
            H/2 + settings2D->mapOffset.y,
            0.0f);
        glScalef(settings2D->mapZoom, settings2D->mapZoom, 1);

        // The walls zoom
        glPushMatrix();
            // Turn the world around player
            glScalef(settings3D->scale, settings3D->scale, 1);

            if (settings2D->rotateMap)
            {
                glRotatef(Rad2Deg( (-players[0].yawRad )), 0, 0, WORLD_FORWARD.z);
            }

            if (settings2D->centerMapToPlayer)
            {
                // Keep player at center of screen
                glTranslatef(-firstPlayerPos2.x, -firstPlayerPos2.y, 0);
            }

            glBegin(GL_LINES);

            // Draw Grid in grey under everything else
            glColor3f(0.2f, 0.2f, 0.2f);
            if (settings2D->gridSize > 0)
            {
                float antiscale = 1.0f / settings3D->scale;
                float gz = floorf(settings2D->gridSize) * antiscale;
                float dx = (-10 * gz);
                float dy = (-10 * gz);
                for(int x = 0; x < 20; x++)
                {
                    OpenGLRender_Line2(dx + gz * x, dy,
                                       dx + gz * x, dy + gz * 20);
                }
                for (int y = 0; y < 20; y++)
                {
                    OpenGLRender_Line2(dx, dy + gz * y,
                                        dx + gz * 20, dy + gz * y);

                }
            }
            glLineWidth(4.0f);

            // Draw origo
            OpenGLRender_SetColor(whiteColor);
            OpenGLRender_Line2(0, -10, 0 ,10);
            OpenGLRender_Line2(-10, 0, 10, 0);

            // Draw WORLD_FORWARD and WORLD_RIGHT
            int axisLength = 1024;
            OpenGLRender_SetColor(Debug_Red);
            OpenGLRender_Line2(0, 0, WORLD_RIGHT.x * axisLength, WORLD_RIGHT.z * axisLength);
            OpenGLRender_SetColor(Debug_Blue);
            OpenGLRender_Line2(0, 0, WORLD_FORWARD.x * axisLength, WORLD_FORWARD.z * axisLength);

            // DRAW WALLS
            ////////////////////////////
            settings2D->collisionInsideSector = -1;
            for(int si = 0; si < map->sectorAmount; si++)
            {
                if (settings2D->drawOneSector >=0 && settings2D->drawOneSector != si)
                { continue; }

                // Get the sector info from map
                Sector* sector = Map_GetSector(map, si);
                for (s16 wi = 0; wi < sector->wallnum; wi++)
                {
                    if (settings2D->drawOneWall >=0 && settings2D->drawOneWall != wi)
                    { continue; }

                    Wall* w = Map_GetWallInSector(map, si, wi);
                    Vector2 start = Vector2New(w->x, w->z);
                    Wall* wend = Map_GetWallEnd(map, w);
                    Vector2 end =  Vector2New(wend->x, wend->z);

                    if (settings2D->movePlayer == false)
                    {
                        if (Map_IsPointInsideSectorOG(map, settings2D->collisionPoint, si))
                        {
                            settings2D->collisionInsideSector = si;
                        }
                        if (Map_FindIntersectionWithWall(map,  settings2D->collisionPoint, collisionEnd, w, &collisionOut))
                        {
                            OpenGLRender_DrawDot(collisionOut, 48, Debug_White);
                        }
                        else
                        {
                            collisionMiss = true;
                        }
                        if (Map_IsPointInsideWall(map, settings2D->collisionPoint, w))
                        {
                            OpenGLRender_SetColor(Debug_Green);
                        }
                        else
                        {
                            OpenGLRender_SetColor(Debug_Black);
                        }
                    }
                    else
                    {
                        if (w->nextsector < 0)
                        {
                            OpenGLRender_SetColor(wallColor);
                        }
                        else
                        {
                            OpenGLRender_SetColor(portalColor);
                        }
                    }

                    OpenGLRender_Line2(start.x, start.y, end.x, end.y);
                    if (settings2D->drawNormals)
                    {
                        Vector2 m = Map_GetWallMiddle(map, w);
                        Vector2 N = Vector2Scale( Map_GetWallNormal(map, w), 32 );
                        OpenGLRender_Line2(m.x, m.y, m.x + N.x, m.y + N.y);
                    }
                }
            }
            glEnd(); // end walls

            // DRAW SPRITES
            // //////////////////////

            if (settings2D->drawSprites)
            {
                glBegin(GL_LINES);

                float spriteSize = 64;
                color32 spriteColor = Debug_Red;
                OpenGLRender_SetColor(spriteColor);
                Vector2 spriteForward = Vector2New(WORLD_FORWARD.x, WORLD_FORWARD.z);
                float spriteWidth = settings3D->spriteDefaultWidth/2;
                for(int spi =  0; spi < map->spriteAmount; spi++)
                {
                    MapSprite* sprite = &map->sprites[spi];
                    Vector2 spos2 = Vector2New(sprite->position.x, sprite->position.z);
                    SpriteAlignment al = Sprite_GetAlignment(sprite);

                    float angle = Math_DukeAngleToRad(sprite->ang);
                    if (al == Sprite_FACE)
                    {
                        angle = players[0].yawRad + Deg2Rad(180);
                    }
                    Vector2 spriteDir = Vector2Rotate(spriteForward, angle);

                    if (al == Sprite_FLOOR)
                    {
                        OpenGLRender_DrawDot(spos2, spriteSize, spriteColor);
                    }
                    else
                    {
                        OpenGLRender_DrawDot(spos2, spriteSize, spriteColor);
                        Vector2 spriteEnd = Vector2Add(spos2, Vector2Scale(spriteDir, spriteWidth));
                        OpenGLRender_Line2(spos2.x, spos2.y, spriteEnd.x, spriteEnd.y);
                    }
                }
                glEnd();
            }


            // DRAW WALL NUMBERS and SECTOR NUMBERS
            glPushMatrix();
                glScalef(1, -1, 1);

            int numberSize = 128;
            for(int si = 0; si < map->sectorAmount; si++)
            {
                if (settings2D->drawOneSector >=0 && settings2D->drawOneSector != si) { continue; }

                // Get the sector info from map
                Sector* sector = Map_GetSector(map, si);

                if (settings2D->drawSectorNumbers)
                {
                    int sx = sector->minXZPoint.x + sector->sizeXZ.x/2;
                    int sy = -(sector->minXZPoint.y + sector->sizeXZ.y/2);
                    int numbers = 1;
                    if (si >= 10)
                    {
                        numbers += 1;
                        if (si >= 100)
                        {
                            numbers += 1;
                        }
                    }
                    mgdl_DrawRectangle(sx, sy, numberSize * numbers, numberSize, Debug_Black);

                    if (sectorDrawTimes[si] > 0)
                    {
                        // Draw in green if rendered at least once
                        Texture_DrawTextF(df, greenColor, sx, sy, numberSize, "%d", si);
                    }
                    else
                    {
                        // Draw in white if not rendered
                        Texture_DrawTextF(df, whiteColor, sx, sy, numberSize, "%d", si);

                    }
                }

                if (settings2D->drawWallNumbers)
                {
                    // Render all walls of the current sector
                    // Discard those that do not face player
                    for (s16 wi = 0; wi < sector->wallnum; wi++)
                    {
                        if (settings2D->drawOneWall >=0 && settings2D->drawOneWall != wi) { continue; }

                        Wall* w = Map_GetWallInSector(map, si, wi);
                        Vector2 start = Vector2New(w->x, w->z);
                        Vector2 middle = Map_GetWallMiddle(map, w);

                        int mapWi =sector->wallptr+wi;

                        int tx = middle.x - numberSize/2;
                        int ty = -middle.y + numberSize/2;
                        int numbers = 1;
                        if (mapWi >= 10)
                        {
                            numbers += 1;
                            if (mapWi >= 100)
                            {
                                numbers += 1;
                            }
                        }
                        ty -= (numberSize * (numbers-1));

                        mgdl_DrawRectangle(tx, ty, numberSize*numbers, numberSize, Debug_Black);

                        if (w->nextsector < 0)
                        {
                            Texture_DrawTextF(df, wallColor, tx, ty, numberSize, "%d", mapWi);
                        }
                        else
                        {
                            Texture_DrawTextF(df, portalColor, tx, ty, numberSize, "%d", mapWi);
                        }
                        if (settings2D->drawOneWall == wi && settings2D->drawOneSector == si)
                        {

                            if (collisionMiss)
                            {
                                Texture_DrawTextF(df, whiteColor, start.x, start.y + 64+32, 32, "U: %.2f", collisionOut.y);
                                Texture_DrawTextF(df, whiteColor, start.x, start.y + 64, 32, "T: %.2f", collisionOut.x);
                            }
                            else
                            {
                                Texture_DrawTextF(df, whiteColor, start.x, start.y + 64+32, 32, "Y: %.2f", collisionOut.y);
                                Texture_DrawTextF(df, whiteColor, start.x, start.y + 64, 32, "X: %.2f", collisionOut.x);
                            }
                        }
                    }
                }
            }
            glPopMatrix(); // NUMBERS
        glPopMatrix(); // WALLS


        // DRAW PLAYERS
        // ////////////

    glPushMatrix();
        for (int pi = 0; pi < settings2D->drawPlayersAmount; pi++)
        {
            Viewpoint* player = &players[pi];
            Vector2 playerPos2 = Vector2New(player->position.x, player->position.z);

            glScalef(settings3D->scale, settings3D->scale, 1);
            glBegin(GL_LINES);

            Vector2 forward = Vector2New(WORLD_FORWARD.x, WORLD_FORWARD.z);
            if (settings2D->rotateMap == false)
            {
                forward = Vector2Rotate(forward, player->yawRad);
            }
            color32 pc =  Debug_Black;


            if (mgdl_GetElapsedFrames() % 30 == 0)
            {
                pc = Debug_White;
            }
            float dotSize = 512.0f;
            float playerArrowSize = 1024.0f;

            // PLAYER ARROW
            // //////////////////
            if (settings2D->movePlayer)
            {
                if (settings2D->centerMapToPlayer && pi == 0)
                {
                    playerPos2 = Vector2Zero();
                }
                OpenGLRender_DrawDot(playerPos2, dotSize,pc );
                OpenGLRender_SetColor(Debug_Red);
                forward = Vector2Scale(forward, playerArrowSize * 2);
                Vector2 end = Vector2Add(playerPos2, forward);

                glVertex2i(playerPos2.x,playerPos2.y);
                glVertex2f(end.x, end.y);

                Vector2 sideLeft = Vector2Rotate(forward,  M_PI * 3.0f/4.0f);
                Vector2 sideRight = Vector2Rotate(forward, -M_PI * 3.0f/4.0f);
                sideLeft = Vector2Add(sideLeft, end);
                sideRight = Vector2Add(sideRight, end);

                glVertex2f(end.x, end.y);
                glVertex2f(sideLeft.x, sideLeft.y);

                glVertex2f(end.x, end.y);
                glVertex2f(sideRight.x, sideRight.y);
            }
            else
            {
                OpenGLRender_DrawDot(playerPos2, dotSize,pc );
                OpenGLRender_SetColor(Debug_Black);

                glVertex2i(playerPos2.x,playerPos2.y);
                glVertex2f(collisionEnd.x, collisionEnd.y);
            }
        }

        glEnd();
        glPopMatrix(); // Player
    glPopMatrix(); // Whole map view
    glLineWidth(1.0f);
}



