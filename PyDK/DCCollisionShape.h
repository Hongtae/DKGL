#pragma once
#include <Python.h>
#include <DK/DK.h>

using namespace DKGL;
using namespace DKGL;

struct DCCollisionShape
{
	PyObject_HEAD
	DKObject<DKCollisionShape> shape;
};
