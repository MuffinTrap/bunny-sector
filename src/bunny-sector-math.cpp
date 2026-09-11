#include "bunny-sector-math.h"
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


Vector2 Vector2Project(Vector2 move, Vector2 wall)
{
	return Vector2Scale(wall, Vector2DotProduct(move, wall)/Vector2DotProduct(wall, wall));
}

/*
 * Duke angles are [0 , 2047]
 * Rotating the world forward should give
 * DA       Direction
 * 0     : ( 1, 0)
 * 512   : ( 0, 1)
 * 1024  : ( -1, 0)
 * 1536  : (0, -1)
 *
 * World forward is : 1, 0
 * Positive angles rotate clockwise
 * 0 dukes = 0 degrees
 * 512 dukes = 90 degrees
 *
*/
float Math_DukeAngleToRad(s16 angleInt)
{
	// Dukes turn clockwise
	// Radians turn counter-clockwise

	// How many radians to turn
    float ratio = (float)angleInt / (float)2048;
	float radians = ratio * (M_PI * 2.0f);
    return radians;
}

Vector3 ScaleVector3ToOpenGL(Vector3 position, RenderSettingsOpenGL* settings3D)
{
	return Vector3Scale(position, settings3D->scale);
}
Vector2 ScaleVector2ToOpenGL(Vector2 position, RenderSettingsOpenGL* settings3D)
{
	return Vector2Scale(position, settings3D->scale);
}
