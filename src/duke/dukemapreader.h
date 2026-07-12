#pragma once
#include "dukemap.h"
#include <mgdl.h>

// Reads duke nukem maps made with mapster32
#ifdef __cplusplus
extern "C" {
#endif
    DukeMap* ReadMapFromFile(const char* mapfilename);
#ifdef __cplusplus
}
    DukeMap* ReadMapFromFile(const zstr& mapfilename);
#endif
