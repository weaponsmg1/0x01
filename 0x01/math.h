#pragma once
#include <cmath>

struct Vector3
{
	float x, y, z;
};

float gWorldToScreen[16];
int width = 0;
int height = 0;

bool WorldToScreen(Vector3 point, float* screen)
{
	float x = gWorldToScreen[0] * point.x + gWorldToScreen[4] * point.y + gWorldToScreen[8] * point.z + gWorldToScreen[12];
	float y = gWorldToScreen[1] * point.x + gWorldToScreen[5] * point.y + gWorldToScreen[9] * point.z + gWorldToScreen[13];
	float z = gWorldToScreen[3] * point.x + gWorldToScreen[7] * point.y + gWorldToScreen[11] * point.z + gWorldToScreen[15];

	if (z < 0.001f)
		return false;

	float inv_z = 1.0f / z;
	float screen_x = x * inv_z;
	float screen_y = y * inv_z;

	if (screen_x < -1.0f || screen_x > 1.0f || screen_y < -1.0f || screen_y > 1.0f)
		return false;

	screen[0] = (width / 2.0f) * (1.0f + screen_x);
	screen[1] = (height / 2.0f) * (1.0f - screen_y);  

	if (screen[0] < 0 || screen[0] > width || screen[1] < 0 || screen[1] > height)
		return false;

	return true;
}