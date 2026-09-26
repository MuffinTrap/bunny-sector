#pragma once
#include <mgdl/mgdl-types.h>

extern const Vector3 WORLD_UP;
extern const Vector3 WORLD_RIGHT;
extern const Vector3 WORLD_FORWARD;

extern const Vector2 FLOOR_RIGHT;
extern const Vector2 FLOOR_FORWARD;

// Macros from bisqwit
#define map_min(a,b)             (((a) < (b)) ? (a) : (b)) // min: Choose smaller of two scalars.
#define map_max(a,b)             (((a) > (b)) ? (a) : (b)) // max: Choose greater of two scalars.
#define map_clamp(a, mi,ma)      map_min(map_max(a,mi),ma)         // clamp: Clamp value into set range.
#define vxs(x0,y0, x1,y1)    ((x0)*(y1) - (x1)*(y0))   // vxs: Vector cross product
// Overlap:  Determine whether the two number ranges overlap.
#define Overlap(a0,a1,b0,b1) (map_min(a0,a1) <= map_max(b0,b1) && map_min(b0,b1) <= map_max(a0,a1))
// IntersectBox: Determine whether two 2D-boxes intersect.
#define IntersectBox(x0,y0, x1,y1, x2,y2, x3,y3) (Overlap(x0,x1,x2,x3) && Overlap(y0,y1,y2,y3))
#define IntersectBoxV(v0, v1, v2, v3) (Overlap(v0.x,v1.x,v2.x,v3.x) && Overlap(v0.y,v1.y,v2.y,v3.y))

struct RenderSettingsOpenGL;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the cross product xy of a x WORLD_UP
 */
Vector2 Vec2XZCrossWithY(Vector2 a);

Vector2 Vector2Project(Vector2 move, Vector2 wall);

Vector2 Vec2XZRotateY(Vector2 p, float angle);
Vector3 Vec3XYZRotateY(Vector3 p, float angle);

/**
 * @brief Converts duke angle to radians
 * @param angleInt Angle between [0,2047]
 */
float Math_DukeAngleToRad(s16 angleInt);

Vector3 ScaleVector3ToOpenGL(Vector3 position, RenderSettingsOpenGL* settings3D);
Vector2 ScaleVector2ToOpenGL(Vector2 position, RenderSettingsOpenGL* settings3D);

bool IsPointInsideRect(RectF rect, Vector2 point);

#ifdef __cplusplus
}
#endif
