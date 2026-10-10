#pragma once
#include "bunny-sector-types.h"

namespace BunnySector
{

class MaterialManager
{

	// Maps doom texture name to unique index
	zstr* DoomTextureNames;
	int lastDoomTextureNameIndex;

	// maps that unique index to MapMaterial
	MapMaterial* mapMaterials;
	int lastMaterialIndex;

	zstr assetFolderName;
	public:
		MaterialManager();
		bool ReadXML(const char* materialsfile);
		/**
		 * @brief Records a Doom material name or editornumber name to array
		 * @param name Name of the material.
		 * @returns Index of the name in materials array or -1 on error
		 */
		int RecordMaterialName(const char* name);
		/**
		 * @brief Connect a previously loaded name index to texture.
		 * @note The texture is not loaded yet, but it is recorded to material
		 * @param doomTextureNameIndex Index to materials array. Get it from RecordMaterialName
		 * @param texture Filename of the texture
		 * @returns True if index was valid and material was modified
		 */
		bool ConnectTextureToMaterialIndex(int doomTextureNameIndex, const char* texture);

		MaterialId LoadMaterialByName(zstr* name);
};


};
