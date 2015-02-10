#include <Python.h>
#include <structmember.h>
#include "PyDKApplication.h"
#include "PyDKPython.h"
#include "PyDKInterpreter.h"
#include "DCObject.h"

using namespace DKFoundation;
using namespace DKFramework;

static DKCondition mainAppCond;
static PyObject* mainApp = NULL;
static bool appInstanceCreated = false;		// PyDKApplication 객체가 파이썬에서 생성되었는지 여부
static bool mainAppTerminated = false;		// PyDKApplication 이 종료될지 여부

static bool CallPyAppMethod(DKApplication* app, PyObject* obj, const char* method)
{
	DKASSERT_DEBUG(Py_IsInitialized());

	bool result = false;
	
	DCObjectCallPyCallableGIL([&]()
	{
		PyObject* tmp = PyObject_CallMethod(obj, (char*)method, 0);
		result = tmp != NULL;
		Py_XDECREF(tmp);
	});
	return result;
}

PyDKApplication::PyDKApplication(void)
{
}

PyDKApplication::~PyDKApplication(void)
{
}

void PyDKApplication::OnHidden(void)
{
	DKCriticalSection<DKCondition> guard(mainAppCond);
	if (mainApp)
		CallPyAppMethod(this, mainApp, "onHidden");
}

void PyDKApplication::OnRestore(void)
{
	DKCriticalSection<DKCondition> guard(mainAppCond);
	if (mainApp)
		CallPyAppMethod(this, mainApp, "onRestore");
}

void PyDKApplication::OnActivated(void)
{
	DKCriticalSection<DKCondition> guard(mainAppCond);
	if (mainApp)
		CallPyAppMethod(this, mainApp, "onActivated");
}

void PyDKApplication::OnDeactivated(void)
{
	DKCriticalSection<DKCondition> guard(mainAppCond);
	if (mainApp)
		CallPyAppMethod(this, mainApp, "onDeactivated");
}

void PyDKApplication::OnInitialize(void)
{
	DKCriticalSection<DKCondition> guard(mainAppCond);
	mainAppTerminated = false;

	while (mainAppTerminated == false)
	{
		if (mainApp)
		{
			// mainApp.onInit 호출함.
			CallPyAppMethod(this, mainApp, "onInit");
			return;
		}
		else
		{
			DKLog("Waiting for script binding...\n");
			mainAppCond.WaitTimeout(2.0);
		}
	}
	DKLog("Script binding cancelled.\n");
	this->Terminate(0);
}

void PyDKApplication::OnTerminate(void)
{
	DKCriticalSection<DKCondition> guard(mainAppCond);
	// 스크립트 루프 종료시킴.
	if (mainApp)
	{
		// mainApp.onExit 호출함.
		CallPyAppMethod(this, mainApp, "onExit");

		mainAppTerminated = true;
		mainAppCond.Broadcast();

		if (!appInstanceCreated)	// 스크립트에서 생성한 인스턴스가 아님. (스크립트만 종료)
		{
			// 스크립트 종료될때까지 기다림.
			while (mainApp)
			{
				DKLog("Waiting for script unbinding...\n");
				mainAppCond.WaitTimeout(2.0);
			}
		}
	}
}

void PyDKApplication::CancelScriptBinding(void)
{
	DKCriticalSection<DKCondition> guard(mainAppCond);
	if (mainApp == NULL)
	{
		mainAppTerminated = true;
		mainAppCond.Broadcast();
	}
}

struct DCApp
{
	PyObject_HEAD
	DKObject<PyDKApplication> app;
	PyObject* envPathDict;
	PyObject* appInfoDict;
	bool proxyInstance;
};
static DCApp* sharedInstance = NULL;

static PyObject* DCAppNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	if (sharedInstance)
	{
		PyErr_SetString(PyExc_RuntimeError, "DCApp instance already exist.");
		return NULL;
	}

	DCApp* self = (DCApp*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->app) DKObject<PyDKApplication>();
		self->envPathDict = NULL;
		self->appInfoDict = NULL;

		DKApplication* app = DKApplication::Instance();
		if (app)
		{
			self->app = dynamic_cast<PyDKApplication*>(app);
			DKCriticalSection<DKCondition> guard(mainAppCond);
			appInstanceCreated = false;
			self->proxyInstance = !appInstanceCreated;
		}
		else
		{
			self->app = DKObject<PyDKApplication>::New();
			DKCriticalSection<DKCondition> guard(mainAppCond);
			appInstanceCreated = true;
			self->proxyInstance = !appInstanceCreated;
		}

		if (self->app)
		{
			DKLog("DCApp(%p) allocated.\n", self);
			sharedInstance = self;
			return (PyObject*)self;
		}
		Py_DECREF(self);
		PyErr_SetString(PyExc_RuntimeError, "Cannot locate PyDKApplication instance.");
	}
	return NULL;
}

static int DCAppInit(DCApp *self, PyObject *args, PyObject *kwds)
{
	DCOBJECT_VALIDATE(self->app, -1);

	Py_CLEAR(self->envPathDict);
	Py_CLEAR(self->appInfoDict);

	self->envPathDict = PyDict_New();
	struct
	{
		DKApplication::SystemPath key;
		const char* name;
	} pathKeys[] = {
		{ DKApplication::SystemPathSystemRoot,		"SystemRoot" },
		{ DKApplication::SystemPathAppRoot,			"AppRoot" },
		{ DKApplication::SystemPathAppResource,		"AppResource" },
		{ DKApplication::SystemPathAppExecutable,	"AppExecutable" },
		{ DKApplication::SystemPathAppData,			"AppData" },
		{ DKApplication::SystemPathUserHome,		"UserHome" },
		{ DKApplication::SystemPathUserDocuments,	"UserDocuments" },
		{ DKApplication::SystemPathUserPreferences,	"UserPreferences" },
		{ DKApplication::SystemPathUserCache,		"UserCache" },
		{ DKApplication::SystemPathUserTemp,		"UserTemp" }
	};
	for (auto& k : pathKeys)
	{
		DKStringU8 str = DKStringU8(self->app->EnvironmentPath(k.key));
		if (str.Length() > 0)
		{
			PyObject* value = PyUnicode_FromString(str);
			if (value)
			{
				PyDict_SetItemString(self->envPathDict, k.name, value);
				Py_DECREF(value);
			}
		}
	}

	PyObject* hostName = PyUnicode_FromWideChar((const wchar_t*)self->app->HostName(), -1);
	PyObject* OSName = PyUnicode_FromWideChar((const wchar_t*)self->app->OSName(), -1);
	PyObject* userName = PyUnicode_FromWideChar((const wchar_t*)self->app->UserName(), -1);

	self->appInfoDict = PyDict_New();
	PyDict_SetItemString(self->appInfoDict, "HostName", hostName);
	PyDict_SetItemString(self->appInfoDict, "OSName", OSName);
	PyDict_SetItemString(self->appInfoDict, "UserName", userName);

	Py_DECREF(hostName);
	Py_DECREF(OSName);
	Py_DECREF(userName);

	DKLog("DCApp(%p) initialized.\n", self);
	return 0;
}

static int DCAppClear(DCApp* self)
{
	Py_CLEAR(self->envPathDict);
	Py_CLEAR(self->appInfoDict);
	return 0;
}

static int DCAppTraverse(DCApp* self, visitproc visit, void* arg)
{
	Py_VISIT(self->envPathDict);
	Py_VISIT(self->appInfoDict);
	return 0;
}

static void DCAppDealloc(DCApp* self)
{
	DCAppClear(self);

	if (sharedInstance == self)
		sharedInstance = NULL;
	self->app = NULL;
	self->app.~DKObject<PyDKApplication>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCAppRun(DCApp* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->app, NULL);

	DKLog("DCApp(%x) registering context.\n", self);
	mainAppCond.Lock();
	mainApp = (PyObject*)self;

	PyThreadState* ts = PyEval_SaveThread();

	mainAppCond.Broadcast();

	int ret = 0;

	if (!appInstanceCreated)
	{
		DKLog("DCApp(%x) attaching current loop.\n", self);
		while (mainAppTerminated == false)
			mainAppCond.Wait();
	}
	else
	{
		mainAppCond.Unlock();
		DKLog("DCApp(%x) entering main loop.\n", self);
		ret = self->app->Run();
		DKLog("DCApp(%x) leaving main loop with result:%d.\n", self, ret);
		mainAppCond.Lock();
	}

	PyEval_RestoreThread(ts);
	mainApp = NULL;
	mainAppCond.Broadcast();
	mainAppCond.Unlock();

	if (PyErr_Occurred())
	{
		return NULL;
	}
	return PyLong_FromLong(ret);
}

static PyObject* DCAppTerminate(DCApp* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->app, NULL);

	int exitCode = 0;

	if (!PyArg_ParseTuple(args, "i:terminate", &exitCode)) {
		return NULL;
	}

	self->app->Terminate(exitCode);
	Py_RETURN_NONE;
}

static PyObject* DCAppDisplayBounds(DCApp* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->app, NULL);
	int displayId = 0;
	if (!PyArg_ParseTuple(args, "i", &displayId))
		return NULL;

	DKRect rc = self->app->DisplayBounds(displayId);
	return DCRectFromObject(&rc);
}

static PyObject* DCAppScreenContentBounds(DCApp* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->app, NULL);
	int displayId = 0;
	if (!PyArg_ParseTuple(args, "i", &displayId))
		return NULL;

	DKRect rc = self->app->ScreenContentBounds(displayId);
	return DCRectFromObject(&rc);
}

static PyObject* DCAppIsProxyInstance(DCApp* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->app, NULL);
	return PyBool_FromLong(self->proxyInstance);
}

static PyMethodDef methods[] = {
	{ "run", (PyCFunction)&DCAppRun, METH_NOARGS, "Enter application main loop" },
	{ "terminate", (PyCFunction)&DCAppTerminate, METH_VARARGS, "Terminate application main loop" },
	{ "displayBounds", (PyCFunction)&DCAppDisplayBounds, METH_VARARGS},
	{ "screenContentBounds", (PyCFunction)&DCAppScreenContentBounds, METH_VARARGS},
	{ "isProxyInstance", (PyCFunction)&DCAppIsProxyInstance, METH_NOARGS },

	{ "onInit", &DCObjectMethodNone, METH_NOARGS },
	{ "onExit", &DCObjectMethodNone, METH_NOARGS },
	{ "onHidden", &DCObjectMethodNone, METH_NOARGS },
	{ "onRestore", &DCObjectMethodNone, METH_NOARGS },
	{ "onActivated", &DCObjectMethodNone, METH_NOARGS },
	{ "onDeactivated", &DCObjectMethodNone, METH_NOARGS },

	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyMemberDef members[] = {
	{ "envPaths", T_OBJECT_EX, offsetof(DCApp, envPathDict), 0, "environment paths" },
	{ "appInfo", T_OBJECT_EX, offsetof(DCApp, appInfoDict), 0, "application info" },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".App",					/* tp_name */
	sizeof(DCApp),								/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)&DCAppDealloc,					/* tp_dealloc */
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
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE|
	Py_TPFLAGS_HAVE_GC,							/* tp_flags */
	0,											/* tp_doc */
	(traverseproc)&DCAppTraverse,				/* tp_traverse */
	(inquiry)&DCAppClear,						/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	methods,									/* tp_methods */
	members,									/* tp_members */
	0,											/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)&DCAppInit,						/* tp_init */
	0,											/* tp_alloc */
	&DCAppNew,									/* tp_new */
};

PyTypeObject* DCAppTypeObject(void)
{
	return &objectType;
}

