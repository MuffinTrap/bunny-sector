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
	* @returns Loaded map or nullptr if loading failed
	*/
    DoomMap* Doom_ReadMapFromFile(const char* mapfilename);

#ifdef __cplusplus
}
#endif
    DoomMap* Doom_ReadMapFromFile(const zstr& mapfilename);


