#pragma once
#include "DCCollisionShape.h"

struct DCConcaveShape
{
	DCCollisionShape base;
	DKConcaveShape* shape;
};
