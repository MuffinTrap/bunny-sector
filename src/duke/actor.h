#pragma once
#include <mgdl.h>

struct DukeMap;
struct RenderSettings2D;
struct RenderSettingsOpenGL;
/**
 * @brief This is the player data.
 * @details It starts with the info loaded from the map
 * and then comes game specific stuff
 */
struct Player
{
    int playerNumber;

    s16 sectorNumber;

    // Position in duke units
    Vector3 position; /**< Where player is */
    Vector3 velocity; /**< Where player is trying to go */
    Vector3 prevPosition; /**< Place to store position before moving */
    // Direction as a normal Vector
    Vector3 lookDirection;

    float yawRad; // Turning
    float pitchRad; // looking up and down

    float turnSpeedDegrees;
    // These are in dukes
    float moveSpeed;
    float verticalSpeed;
    float fallingSpeed;
    float standingHeight; ///< How much above ground when standing
    float kneelingHeight; ///< How much above ground when kneeling/crouching

    float radius;

    // Shooting
    float shootTimer;
    float shootRate;
    bool shotThisFrame;
    Vector3 shotOrigin;
    Vector3 shotDirection;
};
typedef struct Player Player;

const float CONTROLLER_DEADZONE = 0.4f;

void Player_UpdateMove(Player* player, WiiController* controller, RenderSettings2D* settings2D, RenderSettingsOpenGL* settingsGL, DukeMap* map, int amountPlayers);
bool IsPointInsideRect(RectF rect, Vector2 point);
