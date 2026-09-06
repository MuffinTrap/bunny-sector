#pragma once
#include <mgdl.h>
class BunnySector_Map;

// Reads duke nukem maps made with mapster32
#ifdef __cplusplus
extern "C" {
#endif
	/**
	* @brief Loads a map from file
	* @param mapfilename Name of the map file
	* @returns Loaded map or nullptr if loading failed
	*/
    BunnySector_Map* Duke_ReadMapFromFile(const char* mapfilename);
#ifdef __cplusplus
}
    BunnySector_Map* Duke_ReadMapFromFile(const zstr& mapfilename);
#endif
