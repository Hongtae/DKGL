#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCResource.h"
#include "DCObject.h"

struct DCAnimation
{
	DCResource base;
	DKAnimation* animation;
};

static PyObject* DCAnimationNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCAnimation* self = (DCAnimation*)DCResourceTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->animation = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCAnimationInit(DCAnimation *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKAnimation> ani = NULL;
	if (self->animation == NULL)
	{
		ani = DKOBJECT_NEW DKAnimation();
		self->animation = ani;
		DCObjectSetAddress(self->animation, (PyObject*)self);
	}

	self->base.resource = self->animation;
	return DCResourceTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCAnimationDealloc(DCAnimation* self)
{
	self->animation = NULL;
	DCResourceTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCAnimationAddSamplingNode(DCAnimation* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->animation, NULL);

	const char* name;
	Py_buffer data;
	if (!PyArg_ParseTuple(args, "sy*", &name, &data))
		return NULL;

	const DKTransformUnit* tu = reinterpret_cast<const DKTransformUnit*>(data.buf);
	size_t numTu = data.len / sizeof(DKTransformUnit);

	bool result = false;
	Py_BEGIN_ALLOW_THREADS
	result = self->animation->AddSamplingNode(name, tu, numTu);
	Py_END_ALLOW_THREADS

	PyBuffer_Release(&data);
	if (!result)
	{
		PyErr_SetString(PyExc_ValueError, "Invalid sampling node data");
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject* DCAnimationAddKeyframeNode(DCAnimation* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->animation, NULL);

	const char* name;
	Py_buffer scale, rotate, translate;
	if (!PyArg_ParseTuple(args, "sy*y*y*", &name, &scale, &rotate, &translate))
		return NULL;

	auto sk = reinterpret_cast<const DKAnimation::KeyframeNode::ScaleKey*>(scale.buf);
	size_t numSk = scale.len / sizeof(DKAnimation::KeyframeNode::ScaleKey);
	auto rk = reinterpret_cast<const DKAnimation::KeyframeNode::RotationKey*>(rotate.buf);
	size_t numRk = rotate.len / sizeof(DKAnimation::KeyframeNode::RotationKey);
	auto tk = reinterpret_cast<const DKAnimation::KeyframeNode::TranslationKey*>(translate.buf);
	size_t numTk = translate.len / sizeof(DKAnimation::KeyframeNode::TranslationKey);

	bool result = false;
	Py_BEGIN_ALLOW_THREADS
	result = self->animation->AddKeyframeNode(name, sk, numSk, rk, numRk, tk, numTk);
	Py_END_ALLOW_THREADS
	PyBuffer_Release(&scale);
	PyBuffer_Release(&rotate);
	PyBuffer_Release(&translate);

	if (!result)
	{
		PyErr_SetString(PyExc_ValueError, "Invalid keyframe node data");
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject* DCAnimationRemoveNode(DCAnimation* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->animation, NULL);

	const char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	self->animation->RemoveNode(name);
	Py_RETURN_NONE;
}

static PyObject* DCAnimationNodeCount(DCAnimation* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->animation, NULL);
	return PyLong_FromSize_t(self->animation->NodeCount());
}

static PyObject* DCAnimationGetTransform(DCAnimation* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->animation, NULL);
	const char* name;
	float t;
	if (!PyArg_ParseTuple(args, "sf", &name, &t))
		return NULL;

	DKTransformUnit transform;
	if (self->animation->GetNodeTransform(name, t, transform))
	{
		return DCTransformUnitFromObject(&transform);
	}
	PyErr_SetString(PyExc_ValueError, "node not exist");
	return NULL;
}

static PyObject* DCAnimationCreateLoopController(DCAnimation* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->animation, NULL);
	DKObject<DKAnimationController> con = self->animation->CreateLoopController();
	if (con)
	{
		return DCAnimationControllerFromObject(con);
	}
	PyErr_SetString(PyExc_RuntimeError, "failed to create loop controller");
	return NULL;
}

static PyObject* DCAnimationCreateSnapshot(DCAnimation* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->animation, NULL);
	float t;
	if (!PyArg_ParseTuple(args, "f", &t))
		return NULL;

	DKArray<DKAnimation::NodeSnapshot> nodes = self->animation->CreateSnapshot(t);

	PyObject* dict = PyDict_New();
	for (DKAnimation::NodeSnapshot& ns : nodes)
	{
		PyObject* key = PyUnicode_FromWideChar(ns.name, -1);
		PyObject* val = DCTransformUnitFromObject(&ns.transform);

		PyDict_SetItem(dict, key, val);

		Py_DECREF(key);
		Py_DECREF(val);
	}
	return dict;
}

static PyObject* DCAnimationHasNode(DCAnimation* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->animation, NULL);
	const char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	DKAnimation::NodeIndex idx = self->animation->IndexOfNode(name);
	if (idx == DKAnimation::invalidNodeIndex)
	{
		Py_RETURN_FALSE;
	}
	Py_RETURN_TRUE;
}

static PyObject* DCAnimationAllNodeNames(DCAnimation* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->animation, NULL);

	size_t count = self->animation->NodeCount();
	PyObject* tuple = PyTuple_New(count);

	for (size_t i = 0; i < count; ++i)
	{
		auto node = self->animation->NodeAtIndex(i);
		PyTuple_SET_ITEM(tuple, i, PyUnicode_FromWideChar(node->name, -1));
	}
	return tuple;
}

static PyMethodDef methods[] = {
	{ "addSamplingNode", (PyCFunction)&DCAnimationAddSamplingNode, METH_VARARGS },
	{ "addKeyframeNode", (PyCFunction)&DCAnimationAddKeyframeNode, METH_VARARGS },
	{ "removeNode", (PyCFunction)&DCAnimationRemoveNode, METH_VARARGS },
	{ "nodeCount", (PyCFunction)&DCAnimationNodeCount, METH_NOARGS },
	{ "getTransform", (PyCFunction)&DCAnimationGetTransform, METH_VARARGS },
	{ "createLoopController", (PyCFunction)&DCAnimationCreateLoopController, METH_NOARGS },
	{ "createSnapshot", (PyCFunction)&DCAnimationCreateSnapshot, METH_VARARGS },
	{ "hasNode", (PyCFunction)&DCAnimationHasNode, METH_VARARGS },
	{ "allNodeNames", (PyCFunction)&DCAnimationAllNodeNames, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCAnimationDuration(DCAnimation* self, void*)
{
	DCOBJECT_VALIDATE(self->animation, NULL);
	return PyFloat_FromDouble(self->animation->Duration());
}

static int DCAnimationSetDuration(DCAnimation* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->animation, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	double t = PyFloat_AsDouble(value);
	if (PyErr_Occurred() || t < 0.0000001)
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be positive float");
		return -1;
	}
	self->animation->SetDuration(t);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "duration", (getter)&DCAnimationDuration, (setter)&DCAnimationSetDuration, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Animation",				/* tp_name */
	sizeof(DCAnimation),						/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCAnimationDealloc,			/* tp_dealloc */
	0,											/* tp_print */
	0,											/* tp_getattr */
	0,											/* tp_setattr */
	0,											/* tp_reserved */
	0,											/* tp_repr */
	0,											/* tp_as_number */
	0,											/* tp_as_sequence */
	0,											/* tp_as_mapping */
	0,											/* tp_hash  */
	0,											/* tp_call */
	0,											/* tp_str */
	0,											/* tp_getattro */
	0,											/* tp_setattro */
	0,											/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
	0,											/* tp_doc */
	0,											/* tp_traverse */
	0,											/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	methods,									/* tp_methods */
	0,											/* tp_members */
	getsets,									/* tp_getset */
	DCResourceTypeObject(),						/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCAnimationInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCAnimationNew,								/* tp_new */
};

PyTypeObject* DCAnimationTypeObject(void)
{
	return &objectType;
}

PyObject* DCAnimationFromObject(DKAnimation* animation)
{
	if (animation)
	{
		DCAnimation* self = (DCAnimation*)DCObjectFromAddress(animation);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCAnimation*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->animation = animation;
				DCObjectSetAddress(self->animation, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKAnimation* DCAnimationToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCAnimation*)obj)->animation;
	}
	return NULL;
}
