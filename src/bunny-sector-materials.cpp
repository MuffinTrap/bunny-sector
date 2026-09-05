#include <stdio.h>
#include <mgdl.h>
#include "../tinyxml2/tinyxml2.h"
#include "duke/opengl-render.h"

#define NAME int_int_map
#define KEY_TY int
#define VAL_TY int
#define HASH_FN vt_hash_integer
#define CMPR_FN vt_cmpr_integer
#include "../verstable/verstable.h"

#include "bunny-sector-materials.h"

// TextureId to zstr array
static const int TEXTURE_NAME_AMOUNT = 64;

static int_int_map TextureIndexToMaterialId;
static bool mapinitdone = false;

BunnySector::Materials::Materials()
{
	if (mapinitdone == false)
	{
		int_int_map_init(&TextureIndexToMaterialId);
		mapinitdone = true;
	}

	// TODO int to s16 array for picnums?

	mapMaterials = (MapMaterial*)mgdl_AllocateGeneralMemory(TEXTURE_NAME_AMOUNT * sizeof(MapMaterial));
	lastMaterialIndex = 0;

	DoomTextureNames = (zstr*)mgdl_AllocateGeneralMemory(TEXTURE_NAME_AMOUNT * sizeof(zstr));
	lastDoomTextureNameIndex = 0;
	for (int i = 0; i < TEXTURE_NAME_AMOUNT; i++)
	{
		DoomTextureNames[i] = zstr_init();
	}
}

/*
s16 OpenGLRender_GetPicnumForName(zstr* textureName)
{
    printf("Get picnum for name %d\n", zstr_cstr(textureName));
    for (int i = 0; i < nextFreeMaterialSlot; i++)
    {
        if (zstr_eq(&TextureFileNames[i], textureName))
        {
            for (int pi = 0; pi < nextFreeMaterialSlot; pi++)
            {
                if (picnumToMaterialArray[pi] == i)
                {
                    return pi;
                }
            }
        }
    }
    return 0;
}
*/

bool BunnySector::Materials::ReadXML(const char* materialsfile)
{
/*
XML structure
<materials>
	<folder>
	<material>
		Either:
		<picnum> // Duke maps
		or
		<name> // Doom maps
		One of:
			<texture> & <mipmaps>
			<material type>
			<function id>

		Optional:
		<color>
		<self luminance>

*/
	if (mgdl_DoesFileExist(materialsfile) == false)
	{
		return false;
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
		assetFolderName = zstr_from(folderElement->GetText());
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


	// Go through all material siblings
	while(materialElement)
	{
		// Attempt to get either picnum or doom texture name for each material
		int picnum = -1;
		int doomTextureNameIndex = -1;

		tinyxml2::XMLElement* picnumElement = materialElement->FirstChildElement("picnum");
		if (picnumElement)
		{
			picnumElement->QueryIntText(&picnum);
			Log_InfoF("material %d has picnum %d\n", materialindex, picnum);
		}

		tinyxml2::XMLElement* nameElement = materialElement->FirstChildElement("name");
		if (nameElement)
		{
			Log_InfoF("material %d has name %s\n", materialindex, nameElement->GetText());
			zstr namez = zstr_from(nameElement->GetText());

			// Is the name already in array
			bool found= false;
			for (int i = 0; i < lastDoomTextureNameIndex; i++)
			{
				zstr* ati = &DoomTextureNames[i];
				if (zstr_eq(&namez, ati))
				{
					found = true;
					doomTextureNameIndex = i;
					break;
				}
			}
			if (!found)
			{
				doomTextureNameIndex = lastDoomTextureNameIndex;
				DoomTextureNames[lastDoomTextureNameIndex] = namez;
				lastDoomTextureNameIndex += 1;
			}
			else
			{
				zstr_free(&namez);
			}
		}

		tinyxml2::XMLElement* textureElement = materialElement->FirstChildElement("texture");
		if (textureElement)
		{
			Log_InfoF("material %d has texture \"%s\"\n", materialindex, textureElement->GetText());
			// TODO Check for mipmaps
			MapMaterial* mat = &mapMaterials[doomTextureNameIndex];

			// NOTE Texture is not loaded yet
			mat->mgdlMaterial = Material_Load(textureElement->GetText(), nullptr, MaterialType::Diffuse);

			// TODO can be some other type too
			mat->type = MapMaterialType::Material_Texture;
		}


		// TODO Check for material type

		// TODO check for function id

		// TODO check others

		materialElement = materialElement->NextSiblingElement("material");
		materialindex += 1;
	}

	return true;
}

MaterialId BunnySector::Materials::LoadMaterialByName(zstr* name)
{
	// Is this Doom Texture name in our array
	bool found= false;
	int doomTextureNameIndex = -1;
	for (int i = 0; i < lastDoomTextureNameIndex; i++)
	{
		zstr* ati = &DoomTextureNames[i];
		if (zstr_eq(name, ati))
		{
			found = true;
			doomTextureNameIndex = i;
			break;
		}
	}
	MapMaterial* material;
	if (found)
	{
		// Get Texture name
		material = &mapMaterials[doomTextureNameIndex];
		MaterialId existing = OpenGLRender_GetMapMaterialId(material);
		if (existing != INVALID_MATERIAL_ID)
		{
			return existing;
		}
		mgdl_BufferPrintf("%s/%s", zstr_cstr(&assetFolderName), zstr_cstr(&material->mgdlMaterial->name));
		TextureHandle textureH = AssetManager_LoadTexture(mgdl_GetPrintfBuffer(), true);
		if (Handle_IsValid(textureH))
		{
			mgdl_SetTextureFilterMin(textureH, TextureFilterModes::MipmapLinear);
			material->mgdlMaterial->texture = AssetManager_GetTexture(textureH);
			return OpenGLRender_RegisterMapMaterial(material);
		}
	}
	else
	{
		Log_ErrorF("Doom Material name '%s' has no material definition ", zstr_cstr(name));
	}

	return INVALID_MATERIAL_ID;
}



