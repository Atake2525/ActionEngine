#include "Vector3.h"

#pragma once

struct OBB
{
	Vector3 center;
	Vector3 orientations[3];
	Vector3 size;
};
