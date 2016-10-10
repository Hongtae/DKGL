#pragma once
#include <Python.h>
#include <DK/DK.h>

using namespace DKGL;
using namespace DKGL;

struct DCAnimatedTransform
{
	PyObject_HEAD
	DKObject<DKAnimatedTransform> anim;
	bool localObject;
};
