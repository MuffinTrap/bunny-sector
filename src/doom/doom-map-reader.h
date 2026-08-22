#pragma once

#include <mgdl.h>
#include "doom-map.h"

enum UMDF_TOKEN
{

};

#ifdef __cplusplus
extern "C" {
#endif

	/**
	* @brief Loads a map from file
	* @param mapfilename Name of the map file
	* @param dukesPerUnit Adjusts dimensions when loading. All dimensions are divided by this. 1024 dukes is about 1 meter.
	* @returns Loaded map or nullptr if loading failed
	*/
    DoomMap* Doom_ReadMapFromFile(const char* mapfilename, int dukesPerUnit);

#ifdef __cplusplus
}
#endif
    DoomMap* Doom_ReadMapFromFile(const zstr& mapfilename, int dukesPerUnit);


