#pragma once
#include <Python.h>
#include <DK/DK.h>

using namespace DKFoundation;
using namespace DKFramework;

struct DCScene
{
	PyObject_HEAD
	DKObject<DKScene> scene;
	PyObject* nodes;
	void UpdateNodes(void);
};
