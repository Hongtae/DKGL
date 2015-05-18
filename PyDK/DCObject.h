#pragma once

/*****************************
*                            *
*   Encoding must be UTF-8   *
*                            *
*****************************/


#include <Python.h>
#include <DK/DK.h>


#ifdef __clang__
/* PyArg_ParseTupleAndKeywords() 의 네번째 파라메터 (kwlist)가 char** 이기 때문에 많은 파일에서 경고가 뜬다. */
/* API 가 const char** 로 바뀌기 전까지는 경고 무시함 */
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
#endif

using namespace DKFoundation;
using namespace DKFramework;

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
C++ 클래스명 DKClassName
Python 클래스 명 DCClassName
*/
#define PYDK_CORE_TYPE(x)				DC ## x ## TypeObject
#define PYDK_CORE_TRANS_F(x)			DC ## x ## FromObject
#define PYDK_CORE_TRANS_T(x)			DC ## x ## ToObject
#define PYDK_CORE_CONVERTER(x)			DC ## x ## Converter
#define PYDK_NATIVE_CLASS(x)				DK ## x
/*
EXTERN_DC_OBJECT_TYPE(ClassName)
PyTypeObject* DCClassName(void) 타입 함수 선언
*/
#define EXTERN_DC_OBJECT_TYPE(x)		PyTypeObject* PYDK_CORE_TYPE(x)(void)
/*
EXTERN_DC_OBJECT_FROM(ClassName)
PyObject* DCClassNameFromObject(DKClassName) 함수 선언
DKClassName(C++) 을 PyObject* 로 변환해주는 함수 선언함
*/
#define EXTERN_DC_OBJECT_FROM(x)		PyObject* PYDK_CORE_TRANS_F(x)( PYDK_NATIVE_CLASS(x) *)
/*
EXTERN_DC_OBJECT_TO(ClassName)
DKClassName* DCClassNameToObject(PyObject*) 함수 선언
PyObject* 를 DKClassName(C++) 로 변환하는 함수 선언함
*/
#define EXTERN_DC_OBJECT_TO(x)			PYDK_NATIVE_CLASS(x)* PYDK_CORE_TRANS_T(x)(PyObject*)
/*
EXTERN_DC_OBJECT_CONVERT(ClassName)
int DCClassNameConverter(PyObject* DKClassName*) 함수 선언
PyObject* 로부터 DKClassName(C++) 로 변환하는 함수 선언함.
Python 객체(PyObject*) 가 C++ 객체 (DKClassName)의 형식이 아니라도 변환 할 수 있다.
기본 데이터 타입에만 사용가능!
PyArg_ParseTuple, PyArg_ParseTupleAndKeywoard 함수 등에서 사용
*/
#define EXTERN_DC_OBJECT_CONVERTER(x)	int PYDK_CORE_CONVERTER(x)(PyObject*, PYDK_NATIVE_CLASS(x)*)



/*
EXTERN_DC_RUNTIME_TYPE
내부적으로 DKObject<Type> 형식의 포인터 객체를 사용하는 클래스
EXTERN_DC_OBJECT_CONVERTER 매크로를 사용할 수 없다.
*/
#define EXTERN_DC_RUNTIME_TYPE(X) \
	EXTERN_DC_OBJECT_TYPE(X); \
	EXTERN_DC_OBJECT_FROM(X); \
	EXTERN_DC_OBJECT_TO(X)

/*
EXTERN_DC_FUNDAMENTAL_TYPE
내부적으로 값을 직접 가지고 있는 객체
EXTERN_DC_OBJECT_CONVERTER 사용 가능
*/

#define EXTERN_DC_FUNDAMENTAL_TYPE(X) \
	EXTERN_DC_RUNTIME_TYPE(X); \
	EXTERN_DC_OBJECT_CONVERTER(X)


/*
App (DCApp) 은 고유 클래스임. DKApplication 과 변환되지 않는다.
타입 객체로만 선언함.
*/
EXTERN_DC_OBJECT_TYPE(App);

/*
기본 데이터 타입,
EXTERN_DC_OBJECT_CONVERTER 사용 가능함
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
내부적으로 변환할 수 없는 포인터를 가지고 있는 클래스들
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
EXTERN_DC_RUNTIME_TYPE(Light);
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
EXTERN_DC_RUNTIME_TYPE(Scene);
EXTERN_DC_RUNTIME_TYPE(DynamicsScene);

EXTERN_DC_RUNTIME_TYPE(AnimatedTransform);
EXTERN_DC_RUNTIME_TYPE(AnimationController);

EXTERN_DC_RUNTIME_TYPE(AudioListener);
EXTERN_DC_RUNTIME_TYPE(AudioPlayer);
EXTERN_DC_RUNTIME_TYPE(AudioSource);

/*
DCOBJECT_DYANMIC_CAST_CONVERT
C++ 객체를 PyObject* 로 변환할 때
부모 클래스에서 상속받은 타입을 확인하기 위해서 사용함.
*/

#define DCOBJECT_DYANMIC_CAST_CONVERT(TYPE, VALUE) \
	if (dynamic_cast<PYDK_NATIVE_CLASS(TYPE)*>(VALUE)) \
	return PYDK_CORE_TRANS_F(TYPE)(static_cast<PYDK_NATIVE_CLASS(TYPE)*>(VALUE))

/*
DCOBJECT_VALIDATE
Python 메서드 내에서 객체 유효성을 확인하는 매크로
*/
#define DCOBJECT_VALIDATE(OBJ, RET)	\
	if (OBJ == NULL) { \
	PyErr_SetString(PyExc_RuntimeError, "Object not initialized."); \
	return RET; }

/*
DCOBJECT_ATTRIBUTE_NOT_DELETABLE
Python tp_getset 에 정의된 setter 함수에서 값을 지울수 없도록 하는 매크로
*/
#define DCOBJECT_ATTRIBUTE_NOT_DELETABLE(VALUE)	\
	if (VALUE == NULL) { \
	PyErr_SetString(PyExc_TypeError, "Cannot delete attribute"); \
	return -1; }

/*
DCObjectCallPyCallable
C++ 에서 Python 메서드 호출하는 함수 (주의: GIL 여부 확인하지 않음)
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
				DKObject<DKOperation> op = DKFunction(runLoop, &DKRunLoop::Terminate)->Invocation(false).SafeCast<DKOperation>();
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
C++ 에서 Python 메서드 호출해주는 함수. GIL 락을 잡는다.
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
DCBufferRelease 버퍼 자동으로 릴리즈 해주는 객체
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
DCObjectRelease PyObject* 자동으로 릴리즈 해주는 객체
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
