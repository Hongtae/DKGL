#include <Python.h>
#include "PyDKPython.h"
#include "DCObject.h"


using namespace DKGL;
using namespace DKGL;

// type size check.
static_assert(SIZEOF_LONG == sizeof(long), "size mismatch");
static_assert(SIZEOF_SIZE_T == sizeof(size_t), "size mismatch");
static_assert(SIZEOF_TIME_T == sizeof(time_t), "size mismatch");
static_assert(SIZEOF_VOID_P == sizeof(void*), "size mismatch");
static_assert(SIZEOF_DOUBLE == sizeof(double), "size mismatch");
static_assert(SIZEOF_FLOAT == sizeof(float), "size mismatch");
static_assert(SIZEOF_FPOS_T == sizeof(fpos_t), "size mismatch");
static_assert(SIZEOF_INT == sizeof(int), "size mismatch");
static_assert(SIZEOF_LONG_LONG == sizeof(long long), "size mismatch");
static_assert(SIZEOF_OFF_T == sizeof(off_t), "size mismatch");
static_assert(SIZEOF_SHORT == sizeof(short), "size mismatch");
static_assert(SIZEOF_WCHAR_T == sizeof(wchar_t), "size mismatch");

#ifndef _MSC_VER
#include <pthread.h>
//static_assert(SIZEOF__BOOL == sizeof(_Bool), "size mismatch");
static_assert(SIZEOF_LONG_DOUBLE == sizeof(long double), "size mismatch");
static_assert(SIZEOF_UINTPTR_T == sizeof(uintptr_t), "size mismatch");
static_assert(SIZEOF_PID_T == sizeof(pid_t), "size mismatch");
static_assert(SIZEOF_PTHREAD_T == sizeof(pthread_t), "size mismatch");
#endif


PyMODINIT_FUNC PyInit__dk_core(void)
{
	auto version = [](PyObject*, PyObject*)->PyObject*
	{
		const char* str = DKVersion();
		return PyUnicode_FromString(str);
	};
	auto copyright = [](PyObject*, PyObject*)->PyObject*
	{
		const char* str = DKCopyright();
		return PyUnicode_FromString(str);
	};
	auto build = [](PyObject*, PyObject*)->PyObject*
	{
		if (IsDebugBuild())
			return PyUnicode_FromString("Debug");
		return PyUnicode_FromString("Release");
	};
	auto isDebugBuild = [](PyObject*, PyObject*)->PyObject*
	{
		return PyBool_FromLong(IsDebugBuild());
	};
	auto platform = [](PyObject*, PyObject*)->PyObject*
	{
#if   defined(DKGL_WIN32)
		return PyUnicode_FromString("windows");
#elif defined(DKGL_APPLE_OSX)
		return PyUnicode_FromString("osx");
#elif defined(DKGL_APPLE_IOS)
		return PyUnicode_FromString("ios");
#elif defined(DKGL_LINUX)
	#ifdef __ANDROID__
		return PyUnicode_FromString("android");
	#else
		return PyUnicode_FromString("linux");
	#endif
#endif
		return PyUnicode_FromString("unknown");
	};
	auto random = [](PyObject*, PyObject*)->PyObject*
	{
		return PyLong_FromLong(DKRandom());
	};
	auto temporaryDirectory = [](PyObject*, PyObject*)->PyObject*
	{
		DKString str = DKTemporaryDirectory();
		return PyUnicode_FromWideChar(str, -1);
	};
	auto processArguments = [](PyObject*, PyObject*)->PyObject*
	{
		DKArray<DKString> args = DKProcessArguments();
		PyObject* tuple = PyTuple_New(args.Count());
		for (Py_ssize_t i = 0; i < args.Count(); ++i)
		{
			PyObject* obj = PyUnicode_FromWideChar(args.Value(i), -1);
			PyTuple_SET_ITEM(tuple, i, obj);
		}
		return tuple;
	};
	auto processEnvironments = [](PyObject*, PyObject*)->PyObject*
	{
		DKMap<DKString, DKString> envs = DKProcessEnvironments();
		PyObject* dict = PyDict_New();
		envs.EnumerateForward([&](DKMap<DKString, DKString>::Pair& pair)
		{
			PyObject* key = PyUnicode_FromWideChar(pair.key, -1);
			PyObject* val = PyUnicode_FromWideChar(pair.value, -1);
			
			PyDict_SetItem(dict, key, val);
			
			Py_DECREF(key);
			Py_DECREF(val);
		});
		return dict;
	};
	auto uuidgen = [](PyObject*, PyObject*)->PyObject*
	{
		return PyUnicode_FromWideChar(DKUUID::Create().String(), -1);
	};
	auto logger = [](PyObject*, PyObject* args)->PyObject*
	{
		const char* str;
		if (!PyArg_ParseTuple(args, "s", &str))
			return NULL;

		DKLog("%s", str);
		Py_RETURN_NONE;
	};
	auto cleanup = [](PyObject*, PyObject*)->PyObject*
	{
		DKAllocatorChain::Cleanup();
		Py_RETURN_NONE;
	};
	auto objectCount = [](PyObject*, PyObject*)->PyObject*
	{
		return PyLong_FromSize_t(DCObjectCount());
	};
	auto threadSleep = [](PyObject*, PyObject* args)->PyObject*
	{
		double t = 0.0;
		if (!PyArg_ParseTuple(args, "d", &t))
			return NULL;

		Py_BEGIN_ALLOW_THREADS
		DKThread::Sleep(t);
		Py_END_ALLOW_THREADS
		Py_RETURN_NONE;
	};
	auto threadYield = [](PyObject*, PyObject*)->PyObject*
	{
		Py_BEGIN_ALLOW_THREADS
		DKThread::Yield();
		Py_END_ALLOW_THREADS
		Py_RETURN_NONE;
	};
	auto setDefaultClass = [](PyObject*, PyObject* args)->PyObject*
	{
		PyTypeObject* baseType = NULL;
		PyTypeObject* defaultType = NULL;
		if (!PyArg_ParseTuple(args, "O|O", &baseType, &defaultType))
			return NULL;

		if (!PyType_Check(baseType))
		{
			PyErr_Format(PyExc_TypeError, "first argument must a type object, not %.200s", Py_TYPE(baseType)->tp_name);
			return NULL;
		}
		if (defaultType)
		{
			if (!PyType_Check(defaultType))
			{
				PyErr_Format(PyExc_TypeError, "second argument must a type object, not %.200s", Py_TYPE(defaultType)->tp_name);
				return NULL;
			}
			if (!PyType_IsSubtype(defaultType, baseType))
			{
				PyErr_Format(PyExc_TypeError, "%.200s is not a subtype of %.200s", defaultType->tp_name, baseType->tp_name);
				return NULL;
			}
		}
		if (DCObjectSetDefaultClass(baseType, defaultType))
		{
			Py_RETURN_NONE;
		}
		PyErr_Format(PyExc_TypeError, "Cannot set default type. base type %.200s may not exist.", baseType->tp_name);
		return NULL;
	};
	auto defaultClass = [](PyObject*, PyObject* args)->PyObject*
	{
		PyTypeObject* baseType = NULL;
		if (!PyArg_ParseTuple(args, "O", &baseType))
			return NULL;

		if (!PyType_Check(baseType))
		{
			PyErr_Format(PyExc_TypeError, "argument must a type object, not %.200s", Py_TYPE(baseType)->tp_name);
			return NULL;
		}
		PyTypeObject* defaultType = DCObjectDefaultClass(baseType);
		if (defaultType)
		{
			Py_INCREF(defaultType);
			return (PyObject*)defaultType;
		}
		PyErr_Format(PyExc_TypeError, "Cannot set default type. base type %.200s may not exist.", baseType->tp_name);
		return NULL;
	};
	static PyMethodDef methods[] = {
		{ "version", version, METH_NOARGS },
		{ "copyright", copyright, METH_NOARGS },
		{ "build", build, METH_NOARGS },
		{ "isDebugBuild", isDebugBuild, METH_NOARGS },
		{ "platform", platform, METH_NOARGS },
		{ "random", random, METH_NOARGS },
		{ "temporaryDirectory", temporaryDirectory, METH_NOARGS },
		{ "processArguments", processArguments, METH_NOARGS },
		{ "processEnvironments", processEnvironments, METH_NOARGS },
		{ "uuidgen", uuidgen, METH_NOARGS },
		{ "cleanup", cleanup, METH_NOARGS },
		{ "objectCount", objectCount, METH_NOARGS },
		{ "threadSleep", threadSleep, METH_VARARGS },
		{ "threadYield", threadYield, METH_VARARGS },
		{ "log", logger, METH_VARARGS },
		{ "setDefaultClass", setDefaultClass, METH_VARARGS },
		{ "defaultClass", defaultClass, METH_VARARGS },
		{ 0, 0, 0, 0 }
	};
	static PyModuleDef igpy_module = {
		PyModuleDef_HEAD_INIT,
		PYDK_MODULE_NAME,
		PYDK_MODULE_DESC,
		-1,
		methods,
	};

	struct CoreTypes
	{
		const char* name;
		PyTypeObject* (*func)(void);
	};
#define CORE_TYPE_FUNC(x) { #x, PYDK_CORE_TYPE(x) }
	std::initializer_list<CoreTypes> types = {
		CORE_TYPE_FUNC(Data),
		CORE_TYPE_FUNC(Timer),
		CORE_TYPE_FUNC(Stream),
		CORE_TYPE_FUNC(ZipArchiver),
		CORE_TYPE_FUNC(ZipUnarchiver),
		CORE_TYPE_FUNC(RunLoopTimer),
		CORE_TYPE_FUNC(RunLoopOperationResult),
		CORE_TYPE_FUNC(Point),
		CORE_TYPE_FUNC(Size),
		CORE_TYPE_FUNC(Rect),
		CORE_TYPE_FUNC(Spline),
		CORE_TYPE_FUNC(Vector2),
		CORE_TYPE_FUNC(Vector3),
		CORE_TYPE_FUNC(Vector4),
		CORE_TYPE_FUNC(Quaternion),
		CORE_TYPE_FUNC(Matrix2),
		CORE_TYPE_FUNC(Matrix3),
		CORE_TYPE_FUNC(Matrix4),
		CORE_TYPE_FUNC(LinearTransform2),
		CORE_TYPE_FUNC(LinearTransform3),
		CORE_TYPE_FUNC(AffineTransform2),
		CORE_TYPE_FUNC(AffineTransform3),
		CORE_TYPE_FUNC(TransformUnit),
		CORE_TYPE_FUNC(USTransform),
		CORE_TYPE_FUNC(NSTransform),
		CORE_TYPE_FUNC(Color),
		CORE_TYPE_FUNC(App),
		CORE_TYPE_FUNC(Window),
		CORE_TYPE_FUNC(Screen),
		CORE_TYPE_FUNC(Frame),
		CORE_TYPE_FUNC(OpenGLContext),
		CORE_TYPE_FUNC(Renderer),
		CORE_TYPE_FUNC(BlendState),
		CORE_TYPE_FUNC(Font),
		CORE_TYPE_FUNC(Camera),
		CORE_TYPE_FUNC(RenderTarget),
		CORE_TYPE_FUNC(TextureSampler),
		CORE_TYPE_FUNC(Resource),
		CORE_TYPE_FUNC(Animation),
		CORE_TYPE_FUNC(GeometryBuffer),
		CORE_TYPE_FUNC(VertexBuffer),
		CORE_TYPE_FUNC(IndexBuffer),
		CORE_TYPE_FUNC(Material),
		CORE_TYPE_FUNC(Model),
		CORE_TYPE_FUNC(ActionController),
		CORE_TYPE_FUNC(CollisionObject),
		CORE_TYPE_FUNC(RigidBody),
		CORE_TYPE_FUNC(SoftBody),
		CORE_TYPE_FUNC(Constraint),
		CORE_TYPE_FUNC(ConeTwistConstraint),
		CORE_TYPE_FUNC(FixedConstraint),
		CORE_TYPE_FUNC(GearConstraint),
		CORE_TYPE_FUNC(Generic6DofConstraint),
		CORE_TYPE_FUNC(Generic6DofSpringConstraint),
		CORE_TYPE_FUNC(HingeConstraint),
		CORE_TYPE_FUNC(Point2PointConstraint),
		CORE_TYPE_FUNC(SliderConstraint),
		CORE_TYPE_FUNC(Mesh),
		CORE_TYPE_FUNC(StaticMesh),
		CORE_TYPE_FUNC(SkinMesh),
		CORE_TYPE_FUNC(Shader),
		CORE_TYPE_FUNC(ShaderProgram),
		CORE_TYPE_FUNC(Texture),
		CORE_TYPE_FUNC(Texture2D),
		CORE_TYPE_FUNC(Texture3D),
		CORE_TYPE_FUNC(TextureCube),
		CORE_TYPE_FUNC(CollisionShape),
		CORE_TYPE_FUNC(CompoundShape),
		CORE_TYPE_FUNC(ConcaveShape),
		CORE_TYPE_FUNC(StaticPlaneShape),
		CORE_TYPE_FUNC(StaticTriangleMeshShape),
		CORE_TYPE_FUNC(ConvexShape),
		CORE_TYPE_FUNC(CapsuleShape),
		CORE_TYPE_FUNC(ConeShape),
		CORE_TYPE_FUNC(CylinderShape),
		CORE_TYPE_FUNC(MultiSphereShape),
		CORE_TYPE_FUNC(PolyhedralConvexShape),
		CORE_TYPE_FUNC(BoxShape),
		CORE_TYPE_FUNC(ConvexHullShape),
		CORE_TYPE_FUNC(SphereShape),
		CORE_TYPE_FUNC(ResourceLoader),
		CORE_TYPE_FUNC(World),
		CORE_TYPE_FUNC(DynamicsWorld),
		CORE_TYPE_FUNC(AnimatedTransform),
		CORE_TYPE_FUNC(AnimationController),
		CORE_TYPE_FUNC(AudioListener),
		CORE_TYPE_FUNC(AudioPlayer),
		CORE_TYPE_FUNC(AudioSource)
	};
	for (auto& t : types)
	{
		PyTypeObject* type = t.func();
		if (PyType_Ready(type) < 0)
			return NULL;
	}

	PyObject* module = PyModule_Create(&igpy_module);
	if (module == NULL)
		return NULL;

	for (auto& t : types)
	{
		PyTypeObject* type = t.func();
		Py_INCREF(type);
		PyModule_AddObject(module, t.name, (PyObject*)type);
	}

	auto typesIterBegin = types.begin();
	auto typesIterEnd = types.end();
	DCObjectInitDefaultClasses(types.size(), DKFunction([&typesIterBegin, &typesIterEnd]()->PyTypeObject*
	{
		PyTypeObject* tp = NULL;
		if (typesIterBegin != typesIterEnd)
		{
			tp = typesIterBegin->func();
			typesIterBegin++;
		}
		return tp;
	})->Invocation());

	PyEval_InitThreads();

	return module;
}
