#include "bunny-sector-items.h"
#include "gameplay/actor.h"
#include "tinyxml2/tinyxml2.h"
#include "bunny-sector-materials.h"
#include "bunny-sector-config.h"
#include "bunny-sector_main.h"
#include "render/opengl-render.h"

#define NAME type_item_map
#define KEY_TY DOOM_EDITOR_NUMBER
#define VAL_TY int // Index to Actor array
#define HASH_FN vt_hash_integer
#define CMPR_FN vt_cmpr_integer
#include "verstable/verstable.h"

// This maps what editor number is at what index of ItemPrefabs
static type_item_map ItemConfigs;

BunnySector::ItemManager::ItemManager()
{
	ItemPrefabs = (Actor*)mgdl_AllocateGeneralMemory(sizeof(Actor) * PrefabAmount);
	type_item_map_init(&ItemConfigs);
	nextPrefabIndex = 0;
	
}

BunnySector::ItemManager::~ItemManager()
{
	mgdl_FreeGeneralMemory(ItemPrefabs);
	type_item_map_clear(&ItemConfigs);
}

bool BunnySector::ItemManager::LoadItemToActor(Actor* actor, DOOM_EDITOR_NUMBER editorNumber)
{
	type_item_map_itr iterator = type_item_map_get(&ItemConfigs, editorNumber);
	if (type_item_map_is_end(iterator) == false)
	{
		int prefabIndex = iterator.data->val;
		if (prefabIndex >= 0 && prefabIndex < PrefabAmount)
		{
			BunnySector::MaterialManager* mm = BunnySector_GetMaterialManager();
			Actor* prefab = &ItemPrefabs[prefabIndex];
			prefab->texture = mm->LoadMaterialByName(DoomTypeToStringZstr(editorNumber));

			// Copy from prefab to actor

			actor->texture = prefab->texture;
			actor->size = prefab->size;
			actor->height = prefab->height;
			actor->renderHeight = 0;
			actor->renderHeight = 0; // NOTE Avoid carbage values
			actor->model = prefab->model;

			MapMaterial* material = OpenGLRender_GetMaterialForMaterialId(prefab->texture);
			if (Handle_IsValid(actor->model))
			{
				actor->renderWidth = prefab->renderWidth;
				actor->renderHeight = prefab->renderHeight;
			}
			else if (material != nullptr)
			{
				if (prefab->renderWidth == 0)
				{
					actor->renderWidth = material->mgdlMaterial->texture->width;
				}
				else
				{
					actor->renderWidth = prefab->renderWidth;
				}
				if (prefab->renderHeight == 0)
				{
					actor->renderHeight = material->mgdlMaterial->texture->height;
				}
				else
				{
					actor->renderHeight = prefab->renderHeight;
				}
			}

			return true;
		}
	}
	return false;
}



bool BunnySector::ItemManager::LoadXML(const char* filename)
{
	if (mgdl_DoesFileExist(filename) == false)
	{
		return false;
	}

	tinyxml2::XMLDocument itemsXml;
	tinyxml2::XMLError loadresult = itemsXml.LoadFile(filename);
	if (loadresult != tinyxml2::XML_SUCCESS)
	{
		Log_ErrorF("Failed to load items xml file %s\n", filename);
	}

	BunnySector::MaterialManager* materials = BunnySector_GetMaterialManager();


	int itemIndex = 0;
	tinyxml2::XMLElement* itemElement = itemsXml.FirstChildElement()->FirstChildElement("item");
	if (itemElement == nullptr)
	{
		Log_Error("Did not find any <item> from xml\n");
		return false;
	}

	// Go through all item siblings
	while(itemElement)
	{

		// NOTE all info loaded and item is valid. Load a new config
		Actor* prefab = &ItemPrefabs[nextPrefabIndex];

		prefab->Init();
		// Set all default values
		prefab->typeNumber = editorNumber_none;
		prefab->texture = 0;
		prefab->size = 1;
		prefab->height = 1;
		prefab->renderHeight = 0; // NOTE These are set from texture
		prefab->renderWidth = 0;

		tinyxml2::XMLElement* numberElement = itemElement->FirstChildElement("number");
		if (numberElement)
		{
			int typeout;
			numberElement->QueryIntText(&typeout);
			Log_InfoF("item %d has editor type number %d\n", itemIndex, typeout);
			prefab->typeNumber = (DOOM_EDITOR_NUMBER)typeout;
		}
		else
		{
			// is this a doom type name?
			bool found= false;

			tinyxml2::XMLElement* nameElement = itemElement->FirstChildElement("name");
			if (nameElement)
			{
				Log_InfoF("item %d has name %s\n", itemIndex, nameElement->GetText());
				prefab->typeNumber = NameToDoomEditorNumberChar(nameElement->GetText());
				found = (prefab->typeNumber != editorNumber_none);

				if (!found)
				{
					Log_ErrorF("Item name %s did not match any DOOM type name\n", nameElement->GetText());
					break;
				}
			}
			else
			{
				Log_Error("Item has no number or name\n");
				break;
			}
		}
		tinyxml2::XMLElement* customNameElement = itemElement->FirstChildElement("customname");
		if (customNameElement)
		{
			SetDoomTypeCustomName((DOOM_EDITOR_NUMBER)prefab->typeNumber, zstr_from(customNameElement->GetText()));
		}

		tinyxml2::XMLElement* textureElement = itemElement->FirstChildElement("texture");
		if (textureElement)
		{
			Log_InfoF("item %d has texture \"%s\n", itemIndex, textureElement->GetText());

			// Get the texture index from material manager
			int doomTextureNameIndex = materials->RecordMaterialName(DoomTypeToStringChar((DOOM_EDITOR_NUMBER)prefab->typeNumber));
			if (materials->ConnectTextureToMaterialIndex(doomTextureNameIndex, textureElement->GetText()))
			{
			 // OK
			}
		}
		else
		{
			Log_ErrorF("Item index %d has no texture specified\n", itemIndex);
		}

		tinyxml2::XMLElement* sizeElement = itemElement->FirstChildElement("size");
		tinyxml2::XMLElement* heightElement = itemElement->FirstChildElement("height");
		if (sizeElement && heightElement)
		{
			int sizeOut;
			int heightOut;

			sizeElement->QueryIntText(&sizeOut);
			heightElement->QueryIntText(&heightOut);
			Log_InfoF("item %d has size and height %d, %d\n", itemIndex, sizeOut, heightOut);
			prefab->size = sizeOut;
			prefab->height = heightOut;
		}

		XML_ReadChildInt(itemElement, "renderwidth", &prefab->renderWidth);
		XML_ReadChildInt(itemElement, "renderwidth", &prefab->renderHeight);


		tinyxml2::XMLElement* modelElement = itemElement->FirstChildElement("model");
		if (modelElement)
		{
			// This item is a model, try getting it from asset manager
			// TODO Missing a material is not an assert
			// or add mesh loading
			prefab->model = AssetManager_LoadModel(modelElement->GetText(), false);
		}

		type_item_map_insert(&ItemConfigs, (DOOM_EDITOR_NUMBER)prefab->typeNumber, nextPrefabIndex);
		nextPrefabIndex += 1;

		itemElement = itemElement->NextSiblingElement("item");
		itemIndex += 1;
	}

	return true;
}

