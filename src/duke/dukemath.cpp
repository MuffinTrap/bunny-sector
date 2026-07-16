#include "dukemath.h"
#include <mgdl/mgdl-types.h>

#include "build-render.h"


// RIGHT HANDED COORDINATE SYSTEM
const Vector3 WORLD_RIGHT = 	Vector3New(1, 0, 0);
const Vector3 WORLD_UP = 		Vector3New(0,1,0);
const Vector3 WORLD_FORWARD = 	Vector3New(0, 0, -1);

const Vector2 FLOOR_FORWARD = Vector2New(1.0f, 0.0f);
const Vector2 FLOOR_RIGHT = 	Vector2New(0, 1);

Vector2 Vec2XZCrossWithY(Vector2 a)
{
	float ax = a.x;
	float ay = 0.0f;
	float az = a.y;

	float bx = 0.0f;
	float by = 1.0f;
	float bz = 0.0f;


	float x = ay*bz - az*by;
	// // float y = az*bx - ax*bz;
	float z = ax*by - ay*bx;
	return Vector2New( x, z);
}


Vector2 Vec2Project(Vector2 move, Vector2 wall)
{
	return Vector2Scale(wall, Vector2DotProduct(move, wall)/Vector2DotProduct(wall, wall));
}

Vector2 Vec2XZRotateY(Vector2 p, float angle)
{
	float sin_a = sin(angle);
	float cos_a = cos(angle);
	float xt = p.x * cos_a + p.y*sin_a;
	float zt = p.x *-sin_a + p.y*cos_a;
    return Vector2New(xt, zt);
}
Vector3 Vec3XYZRotateY(Vector3 p, float angle)
{

	float sin_a = sin(angle);
	float cos_a = cos(angle);
	float xt = p.x * cos_a + p.z*sin_a;
	float yt = p.y;
	float zt = p.x *-sin_a + p.z*cos_a;
    return Vector3New(xt, yt, zt);
}

/*
 * Duke angles are [0 , 2047]
 * Rotating the world forward should give
 * DA       Direction
 * 0     : ( 0, 0, 1)
 * 512   : ( 1, 0, 0)
 * 1024  : ( 0, 0,-1)
 * 1536  : (-1, 0, 0)
 *
 * World forward is : 0, 0, -1
 * Rotating that by 0 radians gives the same, so we need to subtract PI from
 * result
 * Positive angles rotate counter-clockwise
 * 0 dukes = 0 degrees
 * 0 degrees = (0, 0, -1)
 * 0 degrees -90 degrees = (1, 0, 0)
 *
*/
float Math_DukeAngleToRad(s16 angleInt)
{
	// Dukes turn clockwise
	// Radians turn counter-clockwise

	// How many radians to turn
    float ratio = (float)angleInt / (float)2048;
	float radians = ratio * M_PI * 2.0f;
    return radians;
}

Vector3 Vec3DukePosToOpenGL(Vector3 dukepos, RenderSettingsOpenGL* settings3D)
{
	return Vector3Scale(dukepos, settings3D->scale);
}
Vector2 Vec2DukePosToOpenGL(Vector2 dukepos, RenderSettingsOpenGL* settings3D)
{
	return Vector2Scale(dukepos, settings3D->scale);
}
