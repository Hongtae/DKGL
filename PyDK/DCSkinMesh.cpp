#include <Python.h>
#include <structmember.h>
#include <DK/DK.h>
#include "DCStaticMesh.h"
#include "DCObject.h"

struct DCSkinMesh
{
	DCStaticMesh base;
	DKSkinMesh* mesh;
};

static PyObject* DCSkinMeshNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCSkinMesh* self = (DCSkinMesh*)DCStaticMeshTypeObject()->tp_new(type, args, kwds);
	if (self)
	{
		self->mesh = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCSkinMeshInit(DCSkinMesh *self, PyObject *args, PyObject *kwds)
{
	DKObject<DKSkinMesh> mesh = NULL;
	if (self->mesh == NULL)
	{
		mesh = DKOBJECT_NEW DKSkinMesh();
		self->mesh = mesh;
		DCObjectSetAddress(self->mesh, (PyObject*)self);
	}
	self->base.mesh = self->mesh;
	return DCStaticMeshTypeObject()->tp_init((PyObject*)self, args, kwds);
}

static void DCSkinMeshDealloc(DCSkinMesh* self)
{
	self->mesh = NULL;
	DCStaticMeshTypeObject()->tp_dealloc((PyObject*)self);
}

static PyObject* DCSkinMeshSetBones(DCSkinMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	PyObject* dict;
	if (!PyArg_ParseTuple(args, "O", &dict))
		return NULL;

	if (!PyMapping_Check(dict))
	{
		PyErr_SetString(PyExc_TypeError,"argument must be mapping object.");
		return NULL;
	}

	PyObject* itemsList = PyMapping_Items(dict);
	if (itemsList == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be mapping object.");
		return NULL;
	}
	DCObjectRelease itemsList_(itemsList);
	Py_ssize_t numItems = PyList_Size(itemsList);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be mapping object.");
		return NULL;
	}

	DKSkinMesh::BoneArray boneArray;
	boneArray.Reserve(numItems);

	for (Py_ssize_t i = 0; i < numItems; ++i)
	{
		PyObject* obj = PyList_GET_ITEM(itemsList, i);
		if (!PyTuple_Check(obj))
		{
			PyErr_BadInternalCall();
			return NULL;
		}
		const char* nodeId;
		DKMatrix4 nodeTM;
		if (!PyArg_ParseTuple(obj, "sO&", &nodeId, &DCMatrix4Converter, &nodeTM))
		{
			PyErr_Clear();
			PyErr_SetString(PyExc_TypeError, "each bone must have string key with Matrix4 value.");
			return NULL;
		}
		DKSkinMesh::Bone bone;
		bone.id = nodeId;
		bone.tm = nodeTM;

		boneArray.Add(bone);
	}

	self->mesh->SetBones(boneArray);
	Py_RETURN_NONE;
}

static PyObject* DCSkinMeshNumberOfBones(DCSkinMesh* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	return PyLong_FromSize_t(self->mesh->NumberOfBones());
}

static PyObject* DCSkinMeshBoneAtIndex(DCSkinMesh* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	unsigned int index;
	if (!PyArg_ParseTuple(args, "I", &index))
		return NULL;

	if (index >= self->mesh->NumberOfBones())
	{
		PyErr_SetString(PyExc_IndexError, "index is out of range.");
		return NULL;
	}
	DKSkinMesh::Bone& bone = self->mesh->BoneAtIndex(index);
	return Py_BuildValue("NN", PyUnicode_FromWideChar(bone.id, -1), DCMatrix4FromObject(&bone.tm));
}

static PyMethodDef methods[] = {
	{ "setBones", (PyCFunction)&DCSkinMeshSetBones, METH_VARARGS },
	{ "numberOfBones", (PyCFunction)&DCSkinMeshNumberOfBones, METH_NOARGS },
	{ "boneAtIndex", (PyCFunction)&DCSkinMeshBoneAtIndex, METH_VARARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCSkinMeshNodeResolved(DCSkinMesh* self, void*)
{
	DCOBJECT_VALIDATE(self->mesh, NULL);
	return PyBool_FromLong(self->mesh->NodeResolved());
}

static PyGetSetDef getsets[] = {
	{ "nodeResolved", (getter)&DCSkinMeshNodeResolved, 0, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".SkinMesh",				/* tp_name */
	sizeof(DCSkinMesh),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCSkinMeshDealloc,				/* tp_dealloc */
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
	0,											/* tp_as_mesh */
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
	DCStaticMeshTypeObject(),					/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCSkinMeshInit,					/* tp_init */
	0,											/* tp_alloc */
	&DCSkinMeshNew,								/* tp_new */
};

PyTypeObject* DCSkinMeshTypeObject(void)
{
	return &objectType;
}

PyObject* DCSkinMeshFromObject(DKSkinMesh* mesh)
{
	if (mesh)
	{
		DCSkinMesh* self = (DCSkinMesh*)DCObjectFromAddress(mesh);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCSkinMesh*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->mesh = mesh;
				DCObjectSetAddress(self->mesh, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKSkinMesh* DCSkinMeshToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCSkinMesh*)obj)->mesh;
	}
	return NULL;
}
