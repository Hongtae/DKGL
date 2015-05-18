#pragma once
#include "DCModel.h"

struct DCCollisionObject
{
	DCModel base;
	DKCollisionObject* object;
	PyObject* collisionShape;
	void UpdateCollisionShape(void);
};
