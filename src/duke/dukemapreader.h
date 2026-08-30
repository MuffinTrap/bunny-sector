#pragma once
#include "dukemap.h"
#include <mgdl.h>

// Reads duke nukem maps made with mapster32
#ifdef __cplusplus
extern "C" {
#endif
	/**
	* @brief Loads a map from file
	* @param mapfilename Name of the map file
	* @returns Loaded map or nullptr if loading failed
	*/
    DukeMap* Duke_ReadMapFromFile(const char* mapfilename);
#ifdef __cplusplus
}
    DukeMap* Duke_ReadMapFromFile(const zstr& mapfilename);
#endif
