#pragma once
#include <Python.h>
#include <DK/DK.h>

using namespace DKFoundation;
using namespace DKFramework;

struct DCAnimatedTransform
{
	PyObject_HEAD
	DKObject<DKAnimatedTransform> anim;
	bool localObject;
};
