#pragma once
#include "doom-map.h"

/**
 * @brief These functions draw the doom map
 */
void InitDoomRender(float verticalFovDegrees);
void RenderDoomMap(DoomMap* map);
