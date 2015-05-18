#pragma once
#include "DCResource.h"

struct DCModel
{
	DCResource base;
	DKModel* model;
	PyObject* children;
	PyObject* animation;
	void UpdateChildren(void);
	void UpdateAnimation(void);
};
