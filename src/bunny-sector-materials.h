#pragma once
#include "bunny-sector-types.h"

namespace BunnySector
{

class Materials
{

	// Maps doom texture name to unique index
	zstr* DoomTextureNames;
	int lastDoomTextureNameIndex;

	// maps that unique index to MapMaterial
	MapMaterial* mapMaterials;
	int lastMaterialIndex;

	zstr assetFolderName;
	public:
		Materials();
		bool ReadXML(const char* materialsfile);
		MaterialId LoadMaterialByName(zstr* name);
		MaterialId LoadMaterialByPicnum(s16 picnum);
};


};
