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
	* @param dukesPerUnit Adjusts dimensions when loading. All dimensions are divided by this. 1024 dukes is about 1 meter.
	* @returns Loaded map or nullptr if loading failed
	*/
    DukeMap* ReadMapFromFile(const char* mapfilename, int dukesPerUnit);
#ifdef __cplusplus
}
    DukeMap* ReadMapFromFile(const zstr& mapfilename, int dukesPerUnit);
#endif
