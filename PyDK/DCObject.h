#pragma once

/*****************************
*                            *
*   Encoding must be UTF-8   *
*                            *
*****************************/


#include <Python.h>
#include <DK/DK.h>


#ifdef __clang__
/* 
Compiler warning generated from lots of files, since the fourth parameter type
of PyArg_ParseTupleAndKeywords() is 'char**' ('kwlist').
We will ignore warning until the parameter type changes to 'const char**'.
*/
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
#endif

using namespace DKGL;
using namespace DKGL;

PyObject* DCObjectFromAddress(const void* addr);
void DCObjectSetAddress(const void* addr, PyObject* obj);
size_t DCObjectCount(void);


PyObject* DCObjectCreateDefaultClass(PyTypeObject*, PyObject*, PyObject*);
bool DCObjectSetDefaultClass(PyTypeObject*, PyTypeObject*);
PyTypeObject* DCObjectDefaultClass(PyTypeObject*);
size_t DCObjectInitDefaultClasses(size_t, DKInvocation<PyTypeObject*>*);


PyObject* DCObjectMethodNone(PyObject*, PyObject*);
PyObject* DCObjectMethodTrue(PyObject*, PyObject*);
PyObject* DCObjectMethodFalse(PyObject*, PyObject*);
PyObject* DCObjectMethodZero(PyObject*, PyObject*);


#define PYDK_MODULE_NAME		"_dk_core"
#define PYDK_MODULE_DESC		"DK core"

/*
C++ class name = DKClassName
Python class name = DCClassName
*/
#define PYDK_CORE_TYPE(x)				DC ## x ## TypeObject
#define PYDK_CORE_TRANS_F(x)			DC ## x ## FromObject
#define PYDK_CORE_TRANS_T(x)			DC ## x ## ToObject
#define PYDK_CORE_CONVERTER(x)			DC ## x ## Converter
#define PYDK_NATIVE_CLASS(x)				DK ## x
/*
EXTERN_DC_OBJECT_TYPE(ClassName)
PyTypeObject* DCClassName(void)
define function that creates type object.
*/
#define EXTERN_DC_OBJECT_TYPE(x)		PyTypeObject* PYDK_CORE_TYPE(x)(void)
/*
EXTERN_DC_OBJECT_FROM(ClassName)
PyObject* DCClassNameFromObject(DKClassName)
define function that converts DKClassName(C++) to PyObject*
*/
#define EXTERN_DC_OBJECT_FROM(x)		PyObject* PYDK_CORE_TRANS_F(x)( PYDK_NATIVE_CLASS(x) *)
/*
EXTERN_DC_OBJECT_TO(ClassName)
DKClassName* DCClassNameToObject(PyObject*)
define function that converts PyObject* to DKClassName(C++)
*/
#define EXTERN_DC_OBJECT_TO(x)			PYDK_NATIVE_CLASS(x)* PYDK_CORE_TRANS_T(x)(PyObject*)
/*
EXTERN_DC_OBJECT_CONVERT(ClassName)
int DCClassNameConverter(PyObject* DKClassName*)
define function that converts PyObject* to DKClassName.
this function will works even DKClassName is informal type of Core-Type.
(ie, convert tuple to DKVector3)
only few core-types support this.
it is useful to 'PyArg_ParseTuple', 'PyArg_ParseTupleAndKeywoard'
*/
#define EXTERN_DC_OBJECT_CONVERTER(x)	int PYDK_CORE_CONVERTER(x)(PyObject*, PYDK_NATIVE_CLASS(x)*)



/*
EXTERN_DC_RUNTIME_TYPE
a class contains DKObject<Type> internally.
EXTERN_DC_OBJECT_CONVERT macro cannot be used.
*/
#define EXTERN_DC_RUNTIME_TYPE(X) \
	EXTERN_DC_OBJECT_TYPE(X); \
	EXTERN_DC_OBJECT_FROM(X); \
	EXTERN_DC_OBJECT_TO(X)

/*
EXTERN_DC_FUNDAMENTAL_TYPE
a class contains value internally. (not DKObject)
EXTERN_DC_OBJECT_CONVERTER macro can be used.
*/

#define EXTERN_DC_FUNDAMENTAL_TYPE(X) \
	EXTERN_DC_RUNTIME_TYPE(X); \
	EXTERN_DC_OBJECT_CONVERTER(X)


/*
App (DCApp) is unique class name. Not interchangeable with DKApplication.
define Type-Object.
*/
EXTERN_DC_OBJECT_TYPE(App);

/*
Basic data types.
EXTERN_DC_OBJECT_CONVERTER macro can be used.
*/
EXTERN_DC_FUNDAMENTAL_TYPE(Point);
EXTERN_DC_FUNDAMENTAL_TYPE(Size);
EXTERN_DC_FUNDAMENTAL_TYPE(Rect);
EXTERN_DC_FUNDAMENTAL_TYPE(Spline);
EXTERN_DC_FUNDAMENTAL_TYPE(Vector2);
EXTERN_DC_FUNDAMENTAL_TYPE(Vector3);
EXTERN_DC_FUNDAMENTAL_TYPE(Vector4);
EXTERN_DC_FUNDAMENTAL_TYPE(Quaternion);
EXTERN_DC_FUNDAMENTAL_TYPE(Matrix2);
EXTERN_DC_FUNDAMENTAL_TYPE(Matrix3);
EXTERN_DC_FUNDAMENTAL_TYPE(Matrix4);
EXTERN_DC_FUNDAMENTAL_TYPE(LinearTransform2);
EXTERN_DC_FUNDAMENTAL_TYPE(LinearTransform3);
EXTERN_DC_FUNDAMENTAL_TYPE(AffineTransform2);
EXTERN_DC_FUNDAMENTAL_TYPE(AffineTransform3);
EXTERN_DC_FUNDAMENTAL_TYPE(TransformUnit);
EXTERN_DC_FUNDAMENTAL_TYPE(USTransform);
EXTERN_DC_FUNDAMENTAL_TYPE(NSTransform);
EXTERN_DC_FUNDAMENTAL_TYPE(Color);

/*
classes contain not interchangeable pointers internally.
*/
EXTERN_DC_RUNTIME_TYPE(Data);
EXTERN_DC_RUNTIME_TYPE(Timer);
EXTERN_DC_RUNTIME_TYPE(Stream);
EXTERN_DC_RUNTIME_TYPE(ZipArchiver);
EXTERN_DC_RUNTIME_TYPE(ZipUnarchiver);
EXTERN_DC_RUNTIME_TYPE(RunLoopTimer);
EXTERN_DC_RUNTIME_TYPE(RunLoopOperationResult);

EXTERN_DC_RUNTIME_TYPE(Window);
EXTERN_DC_RUNTIME_TYPE(Screen);
EXTERN_DC_RUNTIME_TYPE(Frame);
EXTERN_DC_RUNTIME_TYPE(Renderer);
EXTERN_DC_RUNTIME_TYPE(OpenGLContext);
EXTERN_DC_RUNTIME_TYPE(BlendState);
EXTERN_DC_RUNTIME_TYPE(Font);
EXTERN_DC_RUNTIME_TYPE(Camera);
EXTERN_DC_RUNTIME_TYPE(RenderTarget);

EXTERN_DC_RUNTIME_TYPE(Resource);
EXTERN_DC_RUNTIME_TYPE(Animation);
EXTERN_DC_RUNTIME_TYPE(GeometryBuffer);
EXTERN_DC_RUNTIME_TYPE(VertexBuffer);
EXTERN_DC_RUNTIME_TYPE(IndexBuffer);

EXTERN_DC_RUNTIME_TYPE(Material);
EXTERN_DC_RUNTIME_TYPE(Model);
EXTERN_DC_RUNTIME_TYPE(ActionController);
EXTERN_DC_RUNTIME_TYPE(CollisionObject);
EXTERN_DC_RUNTIME_TYPE(RigidBody);
EXTERN_DC_RUNTIME_TYPE(SoftBody);

EXTERN_DC_RUNTIME_TYPE(Constraint);
EXTERN_DC_RUNTIME_TYPE(ConeTwistConstraint);
EXTERN_DC_RUNTIME_TYPE(FixedConstraint);
EXTERN_DC_RUNTIME_TYPE(GearConstraint);
EXTERN_DC_RUNTIME_TYPE(Generic6DofConstraint);
EXTERN_DC_RUNTIME_TYPE(Generic6DofSpringConstraint);
EXTERN_DC_RUNTIME_TYPE(HingeConstraint);
EXTERN_DC_RUNTIME_TYPE(Point2PointConstraint);
EXTERN_DC_RUNTIME_TYPE(SliderConstraint);

EXTERN_DC_RUNTIME_TYPE(Mesh);
EXTERN_DC_RUNTIME_TYPE(StaticMesh);
EXTERN_DC_RUNTIME_TYPE(SkinMesh);

EXTERN_DC_RUNTIME_TYPE(Shader);
EXTERN_DC_RUNTIME_TYPE(ShaderProgram);

EXTERN_DC_RUNTIME_TYPE(Texture);
EXTERN_DC_RUNTIME_TYPE(Texture2D);
EXTERN_DC_RUNTIME_TYPE(Texture3D);
EXTERN_DC_RUNTIME_TYPE(TextureCube);
EXTERN_DC_RUNTIME_TYPE(TextureSampler);

EXTERN_DC_RUNTIME_TYPE(CollisionShape);
EXTERN_DC_RUNTIME_TYPE(CompoundShape);
EXTERN_DC_RUNTIME_TYPE(ConcaveShape);
EXTERN_DC_RUNTIME_TYPE(StaticPlaneShape);
EXTERN_DC_RUNTIME_TYPE(StaticTriangleMeshShape);
EXTERN_DC_RUNTIME_TYPE(ConvexShape);
EXTERN_DC_RUNTIME_TYPE(CapsuleShape);
EXTERN_DC_RUNTIME_TYPE(ConeShape);
EXTERN_DC_RUNTIME_TYPE(CylinderShape);
EXTERN_DC_RUNTIME_TYPE(MultiSphereShape);
EXTERN_DC_RUNTIME_TYPE(PolyhedralConvexShape);
EXTERN_DC_RUNTIME_TYPE(BoxShape);
EXTERN_DC_RUNTIME_TYPE(ConvexHullShape);
EXTERN_DC_RUNTIME_TYPE(SphereShape);

EXTERN_DC_RUNTIME_TYPE(ResourceLoader);
EXTERN_DC_RUNTIME_TYPE(World);
EXTERN_DC_RUNTIME_TYPE(DynamicsWorld);

EXTERN_DC_RUNTIME_TYPE(AnimatedTransform);
EXTERN_DC_RUNTIME_TYPE(AnimationController);

EXTERN_DC_RUNTIME_TYPE(AudioListener);
EXTERN_DC_RUNTIME_TYPE(AudioPlayer);
EXTERN_DC_RUNTIME_TYPE(AudioSource);

/*
DCOBJECT_DYANMIC_CAST_CONVERT
macro to be used to converts C++ object to PyObject*
It can also be used to verify inheritance type with it's parent class.
*/

#define DCOBJECT_DYANMIC_CAST_CONVERT(TYPE, VALUE) \
	if (dynamic_cast<PYDK_NATIVE_CLASS(TYPE)*>(VALUE)) \
	return PYDK_CORE_TRANS_F(TYPE)(static_cast<PYDK_NATIVE_CLASS(TYPE)*>(VALUE))

/*
DCOBJECT_VALIDATE
macro for validate object within Python methods. It does null-check simply.
*/
#define DCOBJECT_VALIDATE(OBJ, RET)	\
	if (OBJ == NULL) { \
	PyErr_SetString(PyExc_RuntimeError, "Object not initialized."); \
	return RET; }

/*
DCOBJECT_ATTRIBUTE_NOT_DELETABLE
macro for PyTypeObject's tp_getset property to be non-deletable.
*/
#define DCOBJECT_ATTRIBUTE_NOT_DELETABLE(VALUE)	\
	if (VALUE == NULL) { \
	PyErr_SetString(PyExc_TypeError, "Cannot delete attribute"); \
	return -1; }

/*
DCObjectCallPyCallable
function that makes call given Python method ('callable') from C++.
Warning: GIL status will not be tested.
*/
template <typename T> bool DCObjectCallPyCallable(T&& callable)
{
	bool result = false;
	if (PyErr_Occurred() == NULL)
	{
		callable();

		if (PyErr_Occurred())
		{
			int exitCode = -1;
			if (PyErr_ExceptionMatches(PyExc_SystemExit))
			{
				//PyErr_Clear();
				exitCode = 0;
			}
			else
			{
				PyObject* exc = NULL;
				PyObject* val = NULL;
				PyObject* tb = NULL;
				PyErr_Fetch(&exc, &val, &tb);
				PyErr_NormalizeException(&exc, &val, &tb);
				PyException_SetTraceback(val, tb);
				PySys_WriteStderr("\nUnhandled python exception occurred.\n");
				PyErr_Display(exc, val, tb);
				PySys_WriteStderr("\n");
				PyErr_Clear();
				PyErr_Restore(exc, val, tb);
				//PyErr_Print();
			}

			DKRunLoop* runLoop = DKRunLoop::CurrentRunLoop();
			if (runLoop)
			{
				DKObject<DKOperation> op = DKFunction(runLoop, &DKRunLoop::Stop)->Invocation().SafeCast<DKOperation>();
				runLoop->PostOperation(op);
			}

			DKApplication* app = DKApplication::Instance();
			if (app)
			{
				if (exitCode)
					DKLog("Unhandled python exception occurred. Terminating app.\n");
				else
					DKLog("[SystemExit] Terminating app.\n");
				app->Terminate(exitCode);
			}
		}
		else
		{
			result = true;
		}
	}
	return result;
}

/*
DCObjectCallPyCallableGIL
function that makes call given Python method ('callable') from C++.
this function will recover GIL.
*/
template <typename T> bool DCObjectCallPyCallableGIL(T&& callable)
{
	DKASSERT_DESC_DEBUG(Py_IsInitialized(), "Python not initialized");

	PyGILState_STATE st = PyGILState_Ensure();
	bool b = DCObjectCallPyCallable(std::forward<T>(callable));
	PyGILState_Release(st);

	return b;
}

/*
DCBufferRelease
an object that releases it's buffer object when it being destroyed.
*/
struct DCBufferRelease
{
	Py_buffer* buffer;
	DCBufferRelease(Py_buffer* pb) : buffer(pb)
	{
		DKASSERT_DEBUG(buffer);
	}
	~DCBufferRelease(void)
	{
		PyBuffer_Release(buffer);
	}
};
/*
DCObjectRelease
an object that releases PyObject when it being destroyed.
*/
struct DCObjectRelease
{
	PyObject* object;
	DCObjectRelease(PyObject* obj) : object(obj) {}
	~DCObjectRelease(void)
	{
		Py_XDECREF(object);
	}
};
