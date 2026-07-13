
#include "opengl-render.h"
#include <mgdl.h>
#include <mgdl/mgdl-memory.h>
#include "dukemap.h"
#include "build-render.h"
#include "dukemath.h"
#include "tesselator.h"
#include "obj-export.h"
#include "../tinyxml2/tinyxml2.h"


// Used when drawing grass materials
static ShellGrass* m_grass = nullptr;

// Used to set normals when drawing floors and ceilings
static GLfloat floorNormal[3];
static GLfloat ceilingNormal[3];

// Store wall vertices of each wall to buffer
// This buffer needs to hold all the walls
static GLfloat* wallBuffer = nullptr; // All vertices of all walls: 3 position
static const u16 WALL_BUFFER_VERTEX_SIZE = 3; ///< How many floats per vertex
static u16 wallBufferSizeVertices = 0;
static u32 wallBufferVertexIndex = 0;

static GLushort* wallIndexBuffer = nullptr; // All indices of all walls
static u32 wallIndexBufferSize = 0;
static u32 wallIndexBufferIndex = 0;


// Arrays for storing sprite pointers and matching picnums to Sprites
#define RENDERER_MATERIAL_ARRAY_SIZE 128
#define RENDERER_PICNUM_TO_MATERIAL_ARRAY_SIZE 2048
static sizetype nextFreeMaterialSlot = 0;
static MapMaterial** materialPtrArray = nullptr;
static u16* picnumToMaterialArray = nullptr;

// Default texture
// Animation variables for animating sprites
int animationFrame = 0;
float animationRate = 0.05f;
float animationTimer = 0;

// Buffer for drawing vertices of the walls and sprites
// 3 position + 2 texture coordinates
#define FULL_VERTEX_SIZE_FLOATS (3 + 2)
#define VERTEX_BUFFER_SIZE_VERTICES 16 // This will always contain a quad
#define VERTEX_BUFFER_SIZE_BYTES (FULL_VERTEX_SIZE_FLOATS * sizeof(float) * VERTEX_BUFFER_SIZE_VERTICES)
static GLfloat* vertexBuffer = nullptr;
#define VERTEX_INDEX_BUFFER_SIZE_INDICES 6
static int vertexBufferIndexVertices = 0;
static GLushort vertexIndexBuffer[VERTEX_INDEX_BUFFER_SIZE_INDICES];

// What uv limits are active
static RectF polygonUVLimits;
static RectF zeroOffset;

// What OpenGL settings are active

static float activeScale = 1.0f;

/**
 * @brief Sets OpenGL to draw from vertexBuffer
 */
static void ActivateVertexBuffer()
{
    glVertexPointer(3, GL_FLOAT, sizeof(float) * FULL_VERTEX_SIZE_FLOATS, &vertexBuffer[0]);
    glTexCoordPointer(2, GL_FLOAT, sizeof(float) * FULL_VERTEX_SIZE_FLOATS, &vertexBuffer[3]);
}


/**
 * @brief Sets OpenGL to draw from floorbuffer at given index
 */
static void ActivateFloorBuffer(u32 index, const GLfloat* floorBuffer)
{
    glVertexPointer(3, GL_FLOAT, sizeof(float) * 5, &floorBuffer[index]);
    glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 5, &floorBuffer[index + 3]);
}

MapMaterial* GetMaterialForPicnum(s16 picnum)
{
    if (picnum >= 0 && picnum < RENDERER_PICNUM_TO_MATERIAL_ARRAY_SIZE)
    {
        u16 materialIndex = picnumToMaterialArray[picnum];
        if (materialIndex >= 0 && materialIndex <= RENDERER_MATERIAL_ARRAY_SIZE)
        {
            MapMaterial* material = materialPtrArray[materialIndex];
            return material;
        }
    }
    return nullptr;
}

void OpenGLRender_SetShellGrass(ShellGrass* grass)
{
    m_grass = grass;
}

void OpenGLRender_SetColor(color32 oc)
{
    mgdl_glColor32(oc);
}

void OpenGLRender_Line2(int x1, int z1, int x2, int z2)
{
    glVertex2i(x1, z1);
    glVertex2i(x2, z2);
}

void OpenGLRender_Line3(Vector3 start, Vector3 end)
{
	glVertex3f(start.x, start.y, start.z);
	glVertex3f(end.x, end.y, end.z);
}

/**
 * @brief Changes Duke shade value to grayscale
 * brightness. Used to color the vertices
 * @param brightnessOffset Positive values are darker. 32 is black. -1 is brighter but smaller values have no meaning
 */
static float BrightnessOffsetToColor(s8 brightnessOffset)
{
    static const float brightnessStep = 1.0f/32.0f;
    return clampF( (1.0f - (brightnessOffset * brightnessStep)), 0.0f, 1.0f);
}

/**
 * @brief Sets up opengl state to draw a polygon from vertex buffer
 */
static void BeginVertexBufferPolygon(const Vector3 normal, const float brightness)
{
    glNormal3f(normal.x, normal.y, normal.z);
    glColor3f(brightness, brightness, brightness);
    vertexBufferIndexVertices = 0;
}
static void EndVertexBufferPolygon()
{
    // Flush all written vertices
    mgdl_CacheFlushRange(vertexBuffer, VERTEX_BUFFER_SIZE_VERTICES * FULL_VERTEX_SIZE_FLOATS * sizeof(float));
    glDrawElements(GL_TRIANGLES, VERTEX_INDEX_BUFFER_SIZE_INDICES, GL_UNSIGNED_SHORT, vertexIndexBuffer);
    //glDrawArrays(GL_TRIANGLES, 0, vertexBufferIndexVertices);
}
static void DrawBufferWithMaterial(MapMaterial* material, Vector3 normal, BufferDrawFunction drawFunction)
{
    Material_Apply(material->mgdlMaterial);
    switch(material->type)
    {
        case Material_Texture:
            drawFunction();
            break;
        case Material_Grass:
        {
            color32 materialColor = Color_Create4f(
                material->mgdlMaterial->diffuseColor[0],
                material->mgdlMaterial->diffuseColor[1],
                material->mgdlMaterial->diffuseColor[2],
                material->mgdlMaterial->diffuseColor[3]);
            DrawGrassOnPolygonBuffer(m_grass,
                                     materialColor, normal,
                                     m_grass->height, 0.1f,
                                     drawFunction, activeScale);
        }
            break;
        case Material_Function:
            // TODO

            break;
        case Material_SpriteModel:
            // Not done here
            break;
        case Material_SpriteAnimated:
            // Get UVs of frame

            break;
    }

}
static void DrawVertexBufferWithMaterial(MapMaterial* material, Vector3 normal)
{
    DrawBufferWithMaterial(material, normal, EndVertexBufferPolygon);
}

static Tesselator_BufferIndices bufferedFloorIndices;
static MapFloorVertexData* bufferedFloorData;
static void DrawFloorBuffer()
{
    glDrawElements(GL_TRIANGLES,
                    bufferedFloorIndices.indexCount,
                    GL_UNSIGNED_SHORT,
                    &bufferedFloorData->floorIndexBuffer[bufferedFloorIndices.indexIndex]);
}

static void DrawFloorBufferWithMaterial(MapMaterial* material, Vector3 normal, MapFloorVertexData* floorData, Tesselator_BufferIndices indices)
{
    bufferedFloorData = floorData;
    bufferedFloorIndices = indices;
    DrawBufferWithMaterial(material, normal, DrawFloorBuffer);
}


/**
 * @brief Stores a vertex to vertexbuffer
 */
static void BufferVertex(const float x, const float y, const float z, const float u, const float v)
{
    GLfloat* vertex = &vertexBuffer[vertexBufferIndexVertices * FULL_VERTEX_SIZE_FLOATS];
    vertex[0] = x;
    vertex[1] = y;
    vertex[2] = z;

    vertex[3] = u;
    vertex[4] = v;

    vertexBufferIndexVertices += 1;

    // if buffer becomes full, render the contents and start from beginning
    if (vertexBufferIndexVertices >= VERTEX_BUFFER_SIZE_VERTICES)
    {
        // Flush all written vertices
        EndVertexBufferPolygon();
        vertexBufferIndexVertices = 0;
    }
}

void DrawMeshOnSprite(MapMaterial* material, Vector3 position, float angleYRadians)
{
    Mesh* mesh = material->data.meshPtr;
    Material_Apply(material->mgdlMaterial);
    glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(Rad2Deg(angleYRadians), WORLD_UP.x, WORLD_UP.y, WORLD_UP.z);
        float antiScale = 1.0f/activeScale;
        glScalef(antiScale, antiScale, antiScale);
        glPushMatrix();
            float ms = material->parameter.meshScale;
            glScalef(ms, ms, ms);
            Mesh_DrawArrays(mesh);
        glPopMatrix();
    glPopMatrix();

}

// Public functions
// //////////////////////////////////////////////

void OpenGLRender_StartDrawingPolygons(float scaleXYZ)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    mgdl_glSetAlphaTest(true);
    glScalef(scaleXYZ, scaleXYZ, scaleXYZ);
    activeScale = scaleXYZ;
}

void OpenGLRender_EndDrawingPolygons()
{
    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glColor3f(1.0f, 1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    mgdl_glSetAlphaTest(false);

}

/**
 * @brief Calculate the uv coordinates of a floor or ceiling vertex in a sector
 */
static Vector2 CalculateFloorOrCeilingUV(const Sector* sector, float x, float z)
{
    float xrange = sector->sizeXZ.x;
    float zrange = sector->sizeXZ.y;
    float xdiff = x - sector->minXZPoint.x;
    float zdiff = z - sector->minXZPoint.y;
    float tx = xdiff/xrange * sector->maxTexCoord.x;
    float tz = zdiff/zrange * sector->maxTexCoord.y;
    return Vector2New(tx, tz);
}

void SetWrap(GLuint textureName)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glDisable(GL_TEXTURE_2D);
}

void OpenGLRender_Init()
{
    // TODO This needs to depend on material somehow
    zeroOffset.x = 0.0f;
    zeroOffset.y = 0.0f;
    zeroOffset.w = 1.0f;
    zeroOffset.h = 1.0f;

    floorNormal[0] = 0;
    floorNormal[1] = 1;
    floorNormal[2] = 0;
    ceilingNormal[0] = 0;
    ceilingNormal[1] = -1;
    ceilingNormal[2] = 0;


    if (vertexBuffer == nullptr)
    {
        vertexBuffer = (GLfloat*)mgdl_AllocateGraphicsMemory(VERTEX_BUFFER_SIZE_BYTES);
    }
    vertexIndexBuffer[0] = 0;
    vertexIndexBuffer[1] = 1;
    vertexIndexBuffer[2] = 2;
    vertexIndexBuffer[3] = 2;
    vertexIndexBuffer[4] = 3;
    vertexIndexBuffer[5] = 0;

    if (picnumToMaterialArray == nullptr)
    {
        picnumToMaterialArray = (u16*)mgdl_AllocateGeneralMemory(RENDERER_PICNUM_TO_MATERIAL_ARRAY_SIZE * sizeof(u16));
        for (int i = 0; i < RENDERER_PICNUM_TO_MATERIAL_ARRAY_SIZE; i++)
        {
            picnumToMaterialArray[i] = 0;
        }
    }
    if (materialPtrArray == nullptr)
    {
        nextFreeMaterialSlot = 0;
        materialPtrArray = (MapMaterial**)mgdl_AllocateGeneralMemory(RENDERER_MATERIAL_ARRAY_SIZE * sizeof(MapMaterial*));
        for (int i = 0; i < RENDERER_MATERIAL_ARRAY_SIZE; i++)
        {
            materialPtrArray[i] = nullptr;
        }
    }
}

/*
XML structure
<materials>
	<folder>
	<material>
		Mandatory:
		<picnum>
		One of:
			<texture> & <mipmaps>
			<material type>
			<function id>
			
		Optional:
		<color>
		<self luminance>

*/

void OpenGLRender_ReadMaterialsXML(const char* materialsfile)
{
	if (mgdl_DoesFileExist(materialsfile) == false)
	{
		return;
	}
	tinyxml2::XMLDocument materials;
	tinyxml2::XMLError loadresult = materials.LoadFile(materialsfile);
	if (loadresult != tinyxml2::XML_SUCCESS)
	{
		Log_Error("Failed to load xml file\n");
	}
	
	// First child is <materials>
	tinyxml2::XMLElement* folderElement = materials.FirstChildElement()->FirstChildElement("folder");
	if (folderElement)
	{
		Log_InfoF("Textures are in folder: %s\n", folderElement->GetText());
	}
	else
	{
		Log_Error("Did not find <folder> from xml\n");
	}
	
	int materialindex = 0;
	tinyxml2::XMLElement* materialElement = materials.FirstChildElement()->FirstChildElement("material");
	if (materialElement == nullptr)
	{
		Log_Error("Did not find any <material> from xml\n");
	}
	while(materialElement)
	{
		int picnum;
		tinyxml2::XMLElement* picnumElement = materialElement->FirstChildElement("picnum");
		if (picnumElement)
		{
			picnumElement->QueryIntText(&picnum);
			Log_InfoF("material %d has picnum %d\n", materialindex, picnum);	
		}
	
		tinyxml2::XMLElement* textureElement = materialElement->FirstChildElement("texture");
		if (textureElement)
		{
			Log_InfoF("material %d has texture \"%s\"\n", materialindex, textureElement->GetText());
			
			// TODO Check for mipmaps
		
		
			mgdl_BufferPrintf("%s/%s", folderElement->GetText(), textureElement->GetText());
			TextureHandle textureH = AssetManager_LoadTexture(mgdl_GetPrintfBuffer(), false);
			if (Handle_IsValid(textureH))
			{
				Texture* texture = AssetManager_GetTexture(textureH);
				OpenGLRender_RegisterTexture(picnum, texture);
			}
		}
		// TODO Check for material type
		
		// TODO check for function id
		
		// TODO check others
		
		
		
		materialElement = materialElement->NextSiblingElement("material");
		materialindex += 1;
	}
}

void OpenGLRender_Deinit()
{
    for (int i = 0; i < RENDERER_MATERIAL_ARRAY_SIZE; i++)
    {
        if (i < nextFreeMaterialSlot)
        {
            if (materialPtrArray[i] != nullptr)
            {
                mgdl_FreeGraphicsMemory(materialPtrArray[i]);
            }
        }
    }
    mgdl_FreeGeneralMemory(materialPtrArray);
    mgdl_FreeGeneralMemory(picnumToMaterialArray);
}


bool OpenGLRender_RegisterTexture(s16 picnum, Texture* texture)
{
    if (nextFreeMaterialSlot < RENDERER_MATERIAL_ARRAY_SIZE)
    {
        Material* addMaterial = Material_Load("", texture, Diffuse);
        addMaterial->diffuseColor[0] = 1.0f;
        addMaterial->diffuseColor[1] = 1.0f;
        addMaterial->diffuseColor[2] = 1.0f;
        addMaterial->diffuseColor[3] = 1.0f;
        bool ok = OpenGLRender_RegisterMaterial(picnum, addMaterial, Material_Texture);
        if (ok == false)
        {
            Material_Free(addMaterial);
        }
        return ok;
    }
    else
    {
        return false;
    }
}

// TODO Grass and Function typea
bool OpenGLRender_RegisterMaterial(s16 picnum, Material* material, MapMaterialType materialType)
{
    if (nextFreeMaterialSlot < RENDERER_MATERIAL_ARRAY_SIZE)
    {
        MapMaterial* mapMaterial = (MapMaterial*)mgdl_AllocateGraphicsMemory(sizeof(MapMaterial));
        mapMaterial->mgdlMaterial = material;
        mapMaterial->type = materialType;
        return OpenGLRender_RegisterMapMaterial(picnum, mapMaterial);
    }
    return false;
}

bool OpenGLRender_RegisterMapMaterial(s16 picnum, MapMaterial* material)
{
    if (nextFreeMaterialSlot < RENDERER_MATERIAL_ARRAY_SIZE)
    {
        materialPtrArray[nextFreeMaterialSlot] = material;
        picnumToMaterialArray[picnum] = nextFreeMaterialSlot;
        nextFreeMaterialSlot += 1;
        SetWrap(material->mgdlMaterial->texture->textureId);
        return true;
    }
    return false;

}

void DrawQuad(Vector2 start, Vector2 end, const Vector2 normalXZ, float floorY, float ceilingY, s16 picnum, s8 brightnessOffset, RenderSettingsOpenGL* settings3D)
{
    // Keep texture aspect 1:1 unless told otherwise
    float width = Vector2Length( Vector2Subtract(end, start)) * settings3D->scale;
    float height = (ceilingY - floorY) * settings3D->scale;

    float aspect = width/height;
    float tex_x1 = 0.0f;
    float tex_x2 = aspect * height;
    float tex_bottom = 0.0f;
    float tex_top = 1.0 * height;
    Vector3 normal = Vector3New(normalXZ.x, 0.0f, normalXZ.y);

    MapMaterial* material = GetMaterialForPicnum(picnum);
    ActivateVertexBuffer();

    BeginVertexBufferPolygon(normal, BrightnessOffsetToColor(brightnessOffset));

        // Build Triangles for wall
        BufferVertex(start.x, floorY, start.y, tex_x1, tex_bottom); // 0
        BufferVertex(end.x, floorY, end.y, tex_x2, tex_bottom);
        BufferVertex(end.x, ceilingY, end.y, tex_x2, tex_top);
        BufferVertex(start.x, ceilingY, start.y, tex_x1, tex_top);

    DrawVertexBufferWithMaterial(material, normal);

}

void OpenGLRender_DrawWall(DukeMap* map, Wall* w, float floorY, float ceilingY, RenderSettingsOpenGL* settings)
{
    Vector2 start = Vector2New(w->x, w->z);
    Wall* wend = Map_GetWallEnd(map, w);
    Vector2 end =  Vector2New(wend->x, wend->z);
    Vector2 normalXZ = Map_GetWallNormal(map, w);
    if (w->nextsector >= 0)
    {
        // Create wall that goes down or up to adjacent sector: Note! both sectors dont need to do this. Only lower one
        Sector* neighbor = Map_GetSector(map, w->nextsector);
        int n_floorY = neighbor->floory;
        int n_ceilingY = neighbor->ceilingy;

        // if this floor height is less than adjacent: Greate wall in between: goes up
        if (floorY < n_floorY)
        {
            DrawQuad(start, end, normalXZ, floorY, n_floorY, w->picnum, w->shade, settings);
        }

        // Ceiling:
        // If this ceiling is higher than adjacent: Greate wall in between: goes down
        if (ceilingY > n_ceilingY)
        {
            Wall* otherWall = Map_GetWall(map, w->nextwall);
            DrawQuad(start, end, normalXZ, n_ceilingY, ceilingY, otherWall->picnum, w->shade, settings);
        }
    }
    else
    {
        // TODO Masked walls
        // Draw the wall
        DrawQuad(start, end, normalXZ, floorY, ceilingY, w->picnum, w->shade, settings);
    }
}


void OpenGLRender_DrawFloorOrCeiling(DukeMap* map, s16 sectorIndex, bool floor)
{
    Sector* sector = Map_GetSector(map, sectorIndex);
    float ceilingY = sector->ceilingy;
    float floorY = sector->floory;

    glPushMatrix();
    MapMaterial* material = nullptr;
    Vector3 normal;

    // Set translation offset, normal and color for the whole polygon
    if (floor)
    {
        glTranslatef(0.0f, floorY, 0.0f);
        float color = BrightnessOffsetToColor(sector->floorshade);
        material = GetMaterialForPicnum(sector->floorpicnum);
        glNormal3f(floorNormal[0], floorNormal[1], floorNormal[2]);
        normal = Vector3New(floorNormal[0], floorNormal[1], floorNormal[2]);
        glColor3f(color, color, color);
    }
    else
    {
        glTranslatef(0.0f, ceilingY, 0.0f);
        float color = BrightnessOffsetToColor(sector->ceilingshade);
        material = GetMaterialForPicnum(sector->ceilingpicnum);
        glNormal3f(ceilingNormal[0], ceilingNormal[1], ceilingNormal[2]);
        normal = Vector3New(ceilingNormal[0], ceilingNormal[1], ceilingNormal[2]);
        glColor3f(color, color, color);
    }

    if (!floor)
    {
        // Cull the ceiling faces the other way around
        glCullFace(GL_FRONT);
    }
        MapFloorVertexData* floorData = &map->floorVertexData;
        Tesselator_BufferIndices indices = floorData->floorStartIndices[sectorIndex];

        //Log_InfoF("Draw %d count vertices\n", count);
        // Set element pointers to floor buffer
        ActivateFloorBuffer(0, floorData->floorBuffer);
        DrawFloorBufferWithMaterial(material, normal, floorData, indices);

        // Reset face culling
        if (!floor)
        {
            glCullFace(GL_BACK);
        }
    glPopMatrix();
}

void OpenGLRender_DrawSprite(Vector3 position, float width, float height, float spriteAngle, float playerAngle, SpriteAlignment alignment, SpritePivot pivot, s16 picnum, s8 brightnessOffset)
{

    static const float pushOut = 8.0f; // In Duke units: 1024 is one meter
	if (alignment == Sprite_FACE)
	{
		spriteAngle = playerAngle + Deg2Rad(180);
	}

	Vector3 spriteForward = Vec3XYZRotateY(WORLD_FORWARD, spriteAngle+M_PI_2);
	Vector3 spriteRight = Vec3XYZRotateY(spriteForward, -M_PI_2);

    // Sprite right is on the left side when looking
    // from the player
	Vector3 toRight = Vector3Scale(spriteRight, width/2);

    // These are from player's point of view
    Vector3 bottomRight, bottomLeft, topLeft, topRight;
	if (alignment == Sprite_FLOOR)
    {
		// Raise up to avoid Z fighting
		position.y += pushOut;

		// Calculate four carpet corners
		bottomLeft = Vector3Add(position, Vector3Add( Vector3Scale(spriteRight, -width/2), Vector3Scale(spriteForward, -width/2)));
		bottomRight = Vector3Add(position, Vector3Add( Vector3Scale(spriteRight, width/2), Vector3Scale(spriteForward, -width/2)));
		topLeft = Vector3Add(position, Vector3Add( Vector3Scale(spriteRight, -width/2), Vector3Scale(spriteForward, width/2)));
		topRight = Vector3Add(position, Vector3Add( Vector3Scale(spriteRight, width/2), Vector3Scale(spriteForward, width/2)));
        // Change forward to up
        spriteForward = WORLD_UP;
    }
    else
    {
        if (alignment == Sprite_WALL)
        {
            // Push out of wall to avoid Z fight
            position = Vector3Add(position, Vector3Scale(spriteForward, pushOut));
        }
        if (pivot == Sprite_PivotCenter)
        {
            bottomRight = Vector3Subtract(position, toRight);
            bottomRight = Vector3Add(bottomRight, Vector3Scale(WORLD_UP, -height / 2));
            bottomLeft = Vector3Add(position, toRight);
            bottomLeft = Vector3Add(bottomLeft, Vector3Scale(WORLD_UP, -height / 2));
            topLeft = Vector3Add(bottomLeft, Vector3Scale(WORLD_UP, height));
            topRight = Vector3Add(bottomRight, Vector3Scale(WORLD_UP, height));
        }
        else
        {
            bottomRight = Vector3Subtract(position, toRight);
            bottomLeft = Vector3Add(position, toRight);
            topLeft = Vector3Add(bottomLeft, Vector3Scale(WORLD_UP, height));
            topRight = Vector3Add(bottomRight, Vector3Scale(WORLD_UP, height));
        }
    }

    MapMaterial* material = GetMaterialForPicnum(picnum);
    if (material->type == Material_SpriteModel)
    {
        DrawMeshOnSprite(material, position, spriteAngle);

    }
    else
    {

        /* NOTE THIS BROKE FOR SOME REASON LOL
        BeginVertexBufferPolygon(spriteForward, BrightnessOffsetToColor(brightnessOffset));

        float U = width;
        float V = height;
        BufferVertex(bottomLeft.x, bottomLeft.y, bottomLeft.z, 0.0f, 0.0f);
        BufferVertex(bottomRight.x, bottomRight.y, bottomRight.z, U, 0.0f);
        BufferVertex(topRight.x, topRight.y, topRight.z, U, V);
        BufferVertex(topLeft.x, topLeft.y, topLeft.z, 0.0f, V);

        ActivateVertexBuffer();
        DrawVertexBufferWithMaterial(material, spriteForward);
        */
        glPushMatrix();
            glTranslatef(position.x, position.y, position.z);
            glRotatef(spriteAngle + M_PI_2, 0.0f,1.0f, 0.0f);

        Texture_Draw(material->mgdlMaterial->texture, width, LJustify, Centered);
        glPopMatrix();
    }
}

void OpenGLRender_AnimateSprites()
{
    animationTimer += mgdl_GetDeltaTime();
    if (animationTimer > animationRate)
    {
        animationFrame++;
        animationTimer = 0.0f;
    }
}

void OpenGLRender_DrawDot(Vector2 point, float size, color32 color)
{
    OpenGLRender_SetColor(color);

    glVertex2i(point.x,point.y - size);
    glVertex2i(point.x + size,point.y);

    glVertex2i(point.x + size,point.y);
    glVertex2i(point.x ,point.y + size);

    glVertex2i(point.x ,point.y + size);
    glVertex2i(point.x - size,point.y);

    glVertex2i(point.x - size,point.y);
    glVertex2i(point.x,point.y - size);
}

static void StartCountingFloorBufferSize(DukeMap* map)
{
    MapFloorVertexData* floorData = &map->floorVertexData;
    const s16 sectorAmount = map->sectorAmount;
    const s16 wallAmount = map->wallAmount;
    // Reserve all memory
    if (floorData->floorStartIndices == nullptr)
    {
        floorData->floorStartIndices = (Tesselator_BufferIndices*)mgdl_AllocateGraphicsMemory(sectorAmount * sizeof(Tesselator_BufferIndices));
    }

    if (floorData->floorBuffer == nullptr)
    {
        floorData->floorBuffer = (GLfloat*)mgdl_AllocateGraphicsMemory(wallAmount * floorData->FLOOR_BUFFER_VERTEX_SIZE * sizeof(GLfloat));
    }

    static const int wallsToIndicesMultiplier = 3;
    if (floorData->floorIndexBuffer == nullptr)
    {
        // DANGER Try to allocate enough : multiply wall amount by some number
        floorData->floorIndexBuffer = (GLushort*)mgdl_AllocateGraphicsMemory(wallAmount * wallsToIndicesMultiplier * sizeof(GLushort));
    }
    floorData->floorIndexBufferSize = wallAmount * wallsToIndicesMultiplier;
    floorData->floorBufferSizeVertices = wallAmount;

    // Start tesselator and send buffer adresses

    Tesselator_Init();
    Tesselator_SetBuffers(floorData->floorBuffer, floorData->floorBufferSizeVertices, floorData->floorIndexBuffer, floorData->floorIndexBufferSize);
}

static void TesselateFloor(DukeMap* map, u16 sectorIndex)
{
    Sector* sector = Map_GetSector(map, sectorIndex);
    // Set translation offset, normal and color for the whole polygon
    RectF uvOffset = zeroOffset;
    Tesselator_BufferIndices indicesBefore;
    indicesBefore = Tesselator_BeginPolygon(floorNormal, uvOffset);

        Tesselator_BeginContour();
        // This is where the current contour started
        int contourStartPoint = sector->wallptr + sector->wallnum -1;

        Wall* startWall = Map_GetWall(map, contourStartPoint);
        int contourEndPoint = startWall->point2;
        /* Because Mapster saves points in clockwise order, but we render
        in counter-clockwise, we need to save the point2 of this vertex
        that is the last point of this contour

        Square sector:
        0 > 1 > 2 > 3 > 0

        Square sector with a square island:
        0 > 1 > 2 > 3 > 0   : Outer wall
        4 > 5 > 6 > 7 > 4   : Island

        Our rendering order is
        7, 6, 5, 4, 3, 2, 1, 0

        When starting from point 7, the value of point2 is 4
        Store that to contourEndPoint
        When we come to point 4, we know that the contour is complete
        and a new one should begin.
        If the first point of new contour is > sector's wallptr there is still more
        islands or the outside wall.
        If the contourEndPoint is greater than sector's wallptr, we know that the sector is complete and
        this was the last contour.
        */
        //Log_InfoF("Tesselating sector %d start %d/%d\n", sector->lotag, startingWall, sector->wallnum);

        // Keep track of global wall index
        int pointIndex = contourStartPoint;
        GLfloat vertex[3];
        Vector2 calculatedUV;
        GLfloat uv[2];
        for (s16 wi = sector->wallnum-1; wi >= 0; wi--)
        {
            Wall* w = Map_GetWallInSectorPtr(map, sector, wi);

            vertex[0] = w->x;
            vertex[1] = 0.0f;
            vertex[2] = w->z;
            calculatedUV = CalculateFloorOrCeilingUV(sector, w->x, w->z);
            uv[0] = calculatedUV.x;
            uv[1] = calculatedUV.y;

            Tesselator_AddVertexToPoly(vertex, uv);

            if (pointIndex == contourEndPoint)
            {
                Tesselator_EndContour();
                if (wi > 0 && contourEndPoint > sector->wallptr)
                {
                    Tesselator_BeginContour();
                    Wall* nextWall = Map_GetWallInSectorPtr(map, sector, (wi - 1));
                    contourEndPoint = nextWall->point2;
                }

            }
            pointIndex--;
        }
        Tesselator_BufferIndices indicesAfter = Tesselator_EndPolygon();

    MapFloorVertexData* floorData = &map->floorVertexData;
    floorData->floorStartIndices[sectorIndex].indexIndex = indicesBefore.indexIndex;
    u16 count = (indicesAfter.indexIndex - indicesBefore.indexIndex);
    floorData->floorStartIndices[sectorIndex].indexCount = count;
    //Log_InfoF("Sector %d: before %d After %d Count: %d\n", sectorIndex, indicesBefore.indexIndex, indicesAfter.indexIndex, count);
    // Set indices in our buffers
    floorData->floorStartIndices[sectorIndex].vertexIndex = indicesBefore.vertexIndex;
    u16 vertexCount = (indicesAfter.vertexIndex - indicesBefore.vertexIndex);
    floorData->floorStartIndices[sectorIndex].vertexCount = vertexCount;
}

static void StopCountingFloorBufferSize(DukeMap* map)
{
    MapFloorVertexData* floorData = &map->floorVertexData;
    Tesselator_BufferIndices lastIndices = floorData->floorStartIndices[map->sectorAmount-1];
    // Allocate the needed amount of memory
    u16 lastIndex = lastIndices.indexIndex + lastIndices.indexCount;
    if (lastIndex < floorData->floorIndexBufferSize)
    {
        floorData->floorIndexBuffer = (GLushort*)realloc(floorData->floorIndexBuffer, lastIndex * sizeof(GLushort));
        floorData->floorIndexBufferSize = lastIndex;
    }
    //Log_InfoF("Tesselator created %d indices in total\n", floorIndexBufferSize);
    Tesselator_Deinit();

    /*
    Log_Info("Floor Vertex buffer:\n");
    for (u32 v = 0; v < floorBufferSizeVertices; v++)
    {
        int i = v * FLOOR_BUFFER_VERTEX_SIZE;
        Log_InfoF("V %d: (%.1f, %.1f, %.1f)\n", v, floorBuffer[i+0], floorBuffer[i+1], floorBuffer[i+2]);
    }
    Log_Info("Floor Index buffer to triangles:\n");
    for (u32 v = 0; v < floorIndexBufferSize; v += 3)
    {
        Log_InfoF("F %d: (%d, %d, %d)\n", v/3, floorIndexBuffer[v+0], floorIndexBuffer[v+1], floorIndexBuffer[v+2]);
        Log_InfoF("       %d, %d, %d )\n",v+0, v+1, v+2);
    }
    */
}

void OpenGLRender_CreateFloorBuffers(DukeMap* map)
{

    StartCountingFloorBufferSize(map);
    for (int si = 0; si < map->sectorAmount; si++)
    {
        TesselateFloor(map, si);
    }
    StopCountingFloorBufferSize(map);
}

void OpenGLRender_StartDrawingFloorsFromBuffer(DukeMap* map)
{
    const MapFloorVertexData* floorData = &map->floorVertexData;
    mgdl_CacheFlushRange(floorData->floorBuffer, floorData->floorBufferSizeVertices * floorData->FLOOR_BUFFER_VERTEX_SIZE * sizeof(GLfloat));
    mgdl_CacheFlushRange(floorData->floorIndexBuffer, floorData->floorIndexBufferSize * sizeof(GLushort));
}

// //////////////////////////////
// OBJ EXPORT FUNCTIONS NOTE TODO DANGER TEST WARNING BUG
// //////////////////////////////

void OpenGLRender_StartObjExport(DukeMap* map, const char* filename, RenderSettingsOpenGL* settings)
{
    ObjExport_Start(filename, zstr_cstr(&map->mapfile), map->sectorAmount, settings->scale);
}
void OpenGLRender_StartFillingWallBuffer(DukeMap* map)
{
    u32 drawnWalls = 0;
    for (int si = 0; si < map->sectorAmount; si++)
    {
        Sector* sector = Map_GetSector(map, si);
        for(int wi = 0; wi < sector->wallnum; wi++)
        {
            Wall* w = Map_GetWallInSectorPtr(map, sector, wi);
            if (w->nextsector >= 0)
            {
                // Create wall that goes down or up to adjacent sector: Note! both sectors dont need to do this. Only lower one
                Sector* neighbor = Map_GetSector(map, w->nextsector);
                int n_floorY = neighbor->floory;
                int n_ceilingY = neighbor->ceilingy;

                // if this floor height is less than adjacent: Greate wall in between: goes up
                if (sector->floory < n_floorY)
                {
                    drawnWalls += 1;
                }

                // Ceiling:
                // If this ceiling is higher than adjacent: Greate wall in between: goes down
                if (sector->ceilingy > n_ceilingY)
                {
                    drawnWalls += 1;
                }
            }
            else
            {
                drawnWalls += 1;
            }
        }
    }
    // These are not drawn, just written to obj and newer on Wii
    // Each wall has at least 4 vertices,
    // some have 12 : floor bit, wall, ceiling bit
    wallBufferSizeVertices = drawnWalls * 4;
    wallBuffer = (GLfloat*)mgdl_AllocateGraphicsMemory(wallBufferSizeVertices * WALL_BUFFER_VERTEX_SIZE * sizeof(GLfloat));

    wallIndexBufferSize = drawnWalls * 6;
    wallIndexBuffer = (GLushort*)mgdl_AllocateGraphicsMemory(wallIndexBufferSize * sizeof(GLushort));

    wallIndexBufferIndex = 0;
    wallBufferVertexIndex = 0;
}

void BufferWallVertex(float x, float y, float z)
{
    int v = wallBufferVertexIndex * WALL_BUFFER_VERTEX_SIZE;
    wallBuffer[v + 0] = x;
    wallBuffer[v + 1] = y;
    wallBuffer[v + 2] = z;
    wallBufferVertexIndex += 1;

}
void BufferQuad(Vector2 start, Vector2 end, float floorY, float ceilingY, u16 quad)
{

    BufferWallVertex(start.x, floorY, start.y);
    BufferWallVertex(end.x, floorY, end.y);
    BufferWallVertex(end.x, ceilingY, end.y);
    BufferWallVertex(start.x, ceilingY, start.y);

    wallIndexBuffer[wallIndexBufferIndex+ 0] = quad * 4 + 0;
    wallIndexBuffer[wallIndexBufferIndex+ 1] = quad * 4 + 1;
    wallIndexBuffer[wallIndexBufferIndex+ 2] = quad * 4 + 2;

    wallIndexBuffer[wallIndexBufferIndex+ 3] = quad * 4 + 2;
    wallIndexBuffer[wallIndexBufferIndex+ 4] = quad * 4 + 3;
    wallIndexBuffer[wallIndexBufferIndex+ 5] = quad * 4 + 0;
    wallIndexBufferIndex += 6;
}
void OpenGLRender_BufferWalls(DukeMap* map)
{
    u16 quadCounter = 0;
    for (int si = 0; si < map->sectorAmount; si++)
    {
        Sector* sector = Map_GetSector(map, si);
        for(int wi = 0; wi < sector->wallnum; wi++)
        {
            Wall* w = Map_GetWallInSectorPtr(map, sector, wi);
            Vector2 start = Vector2New(w->x, w->z);
            Wall* wend = Map_GetWallEnd(map, w);
            Vector2 end =  Vector2New(wend->x, wend->z);
            if (w->nextsector >= 0)
            {
                // Create wall that goes down or up to adjacent sector: Note! both sectors dont need to do this. Only lower one
                Sector* neighbor = Map_GetSector(map, w->nextsector);
                int n_floorY = neighbor->floory;
                int n_ceilingY = neighbor->ceilingy;

                // if this floor height is less than adjacent: Greate wall in between: goes up
                if (sector->floory < n_floorY)
                {
                    BufferQuad(start, end, sector->floory, n_floorY, quadCounter);
                    quadCounter += 1;
                }

                // Ceiling:
                // If this ceiling is higher than adjacent: Greate wall in between: goes down
                if (sector->ceilingy > n_ceilingY)
                {
                    BufferQuad(start, end, n_ceilingY, sector->ceilingy, quadCounter);
                    quadCounter += 1;
                }
            }
            else
            {
                // TODO Masked walls
                // Draw the wall
                BufferQuad(start, end, sector->floory, sector->ceilingy, quadCounter);
                quadCounter += 1;
            }
        }
    }
}

void OpenGLRender_WriteToObj(DukeMap* map, const char* filename, RenderSettingsOpenGL* settings)
{
    OpenGLRender_StartObjExport(map, filename, settings);
    OpenGLRender_StartFillingWallBuffer(map);
    OpenGLRender_BufferWalls(map);

    const MapFloorVertexData* floorData = &map->floorVertexData;

    // VERTICES
    // This will be buffer 0
    u16 wallBufferIndex = 0;
    s32 noYOffset = 0;
    ObjExport_WriteVertices(wallBuffer, wallBufferSizeVertices, WALL_BUFFER_VERTEX_SIZE,
                            noYOffset,
                            wallBufferIndex, mgdl_BufferPrintf("%s", "Wall buffer"));

    // Buffers 1 2, 3 4, 5 6 : Two buffers per sector. First is floor, second is ceiling
    // Write all floors in one big buffer
    u16 firstFloorBuffer = 1;
    u16 firstCeilingBuffer = 2;
    for (int si = 0; si < map->sectorAmount; si++)
    {
        Sector* sector = Map_GetSector(map, si);
        Tesselator_BufferIndices indices = floorData->floorStartIndices[si];

        ObjExport_WriteVertices(&floorData->floorBuffer[indices.vertexIndex * floorData->FLOOR_BUFFER_VERTEX_SIZE], indices.vertexCount, floorData->FLOOR_BUFFER_VERTEX_SIZE,
                                sector->floory, firstFloorBuffer,
                                mgdl_BufferPrintf("%s", "Floor buffer"));
    }
    for (int si = 0; si < map->sectorAmount; si++)
    {
        Sector* sector = Map_GetSector(map, si);
        Tesselator_BufferIndices indices = floorData->floorStartIndices[si];

        ObjExport_WriteVertices(&floorData->floorBuffer[indices.vertexIndex * floorData->FLOOR_BUFFER_VERTEX_SIZE], indices.vertexCount, floorData->FLOOR_BUFFER_VERTEX_SIZE,
                                sector->ceilingy, firstCeilingBuffer,
                                mgdl_BufferPrintf("%s", "Ceiling buffer"));
    }

//        ObjExport_WriteVertices(&floorBuffer[indices.vertexIndex * FLOOR_BUFFER_VERTEX_SIZE], indices.vertexCount, FLOOR_BUFFER_VERTEX_SIZE, sector->ceilingy, mgdl_BufferPrintf("Sector %d ceiling", si));
    // FACES
    u16 wallVertexBuffer = 0;
    ObjExport_WriteFaces(wallIndexBuffer, wallIndexBufferSize, wallVertexBuffer, Wind_CCW, mgdl_BufferPrintf("%s", "Wall faces"));
    for (int si = 0; si < map->sectorAmount; si++)
    {
        Tesselator_BufferIndices indices = floorData->floorStartIndices[si];
        // These faces refer to earlier written floor and ceiling buffers
        ObjExport_WriteFaces(&floorData->floorIndexBuffer[indices.indexIndex], indices.indexCount,
                             firstFloorBuffer, Wind_CCW,
                             mgdl_BufferPrintf("Sector %d floor : refers to vb %d", si, firstFloorBuffer));
    }

    for (int si = 0; si < map->sectorAmount; si++)
    {
        Tesselator_BufferIndices indices = floorData->floorStartIndices[si];
        ObjExport_WriteFaces(&floorData->floorIndexBuffer[indices.indexIndex], indices.indexCount,
                             firstCeilingBuffer, Wind_CW,
                             mgdl_BufferPrintf("Sector %d ceiling : refers to vb %d", si, firstCeilingBuffer));
    }

    ObjExport_Stop();
}


