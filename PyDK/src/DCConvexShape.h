#pragma once
#include "DCCollisionShape.h"

struct DCConvexShape
{
	DCCollisionShape base;
	DKConvexShape* shape;
};
