#pragma once
#include "DCModel.h"

struct DCConstraint
{
	DCModel base;
	DKConstraint* constraint;
	PyObject* bodyA;
	PyObject* bodyB;
	void UpdateTargets(void);
};
