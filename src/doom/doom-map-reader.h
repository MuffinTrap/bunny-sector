#pragma once

#include <mgdl.h>
class BunnySector_Map;

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
    BunnySector_Map* Doom_ReadMapFromFile(const char* mapfilename);

#ifdef __cplusplus
}
#endif
    BunnySector_Map* Doom_ReadMapFromFile(const zstr& mapfilename);


