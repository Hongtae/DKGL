#pragma once
#include <Python.h>
#include <DK/DK.h>

using namespace DKGL;
using namespace DKGL;

struct DCWorld
{
	PyObject_HEAD
	DKObject<DKWorld> scene;
	PyObject* nodes;
	void UpdateNodes(void);
};
