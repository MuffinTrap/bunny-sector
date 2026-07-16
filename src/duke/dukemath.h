#pragma once
#include <mgdl/mgdl-types.h>

extern const Vector3 WORLD_UP;
extern const Vector3 WORLD_RIGHT;
extern const Vector3 WORLD_FORWARD;

extern const Vector2 FLOOR_RIGHT;
extern const Vector2 FLOOR_FORWARD;

struct RenderSettingsOpenGL;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the cross product xy of a x WORLD_UP
 */
Vector2 Vec2XZCrossWithY(Vector2 a);

Vector2 Vec2Project(Vector2 move, Vector2 wall);

Vector2 Vec2XZRotateY(Vector2 p, float angle);
Vector3 Vec3XYZRotateY(Vector3 p, float angle);

/**
 * @brief Converts duke angle to radians
 * @param angleInt Angle between [0,2047]
 */
float Math_DukeAngleToRad(s16 angleInt);

Vector3 Vec3DukePosToOpenGL(Vector3 dukepos, RenderSettingsOpenGL* settings3D);
Vector2 Vec2DukePosToOpenGL(Vector2 dukepos, RenderSettingsOpenGL* settings3D);

#ifdef __cplusplus
}
#endif
