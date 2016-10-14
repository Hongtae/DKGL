#pragma once
#include <Python.h>
#include <DK/DK.h>

using namespace DKGL;
using namespace DKGL;

struct DCScene
{
	PyObject_HEAD
	DKObject<DKScene> scene;
	PyObject* nodes;
	void UpdateNodes(void);
};
