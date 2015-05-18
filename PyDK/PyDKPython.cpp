#include <Python.h>
#include "PyDKPython.h"
#include "DCObject.h"

using namespace DKFoundation;
using namespace DKFramework;


// posix 에서는 _sysconfigdata 모듈이 필요함. (빌드정보)
// build_time_vars 라는 dictionary 가 있어야 함.
static PyObject* Init_dummy_sysconfigdata(void)
{
	static PyModuleDef module =
	{
		PyModuleDef_HEAD_INIT,
		"_sysconfigdata",
		"_sysconfigdata",
		-1,
		NULL,
	};
	PyObject* m = PyModule_Create(&module);

	PyObject* dict = Py_BuildValue("{s:s, s:s}",
		"SYSTEM", "PyDK",
		"VERSION", "1.0");

	PyModule_AddObject(m, "build_time_vars", dict);
	return m;
}

PyMODINIT_FUNC PyInit__dk_core(void);

static _inittab pydkExtensions[] =
{
	{ "_sysconfigdata", Init_dummy_sysconfigdata },
	{ PYDK_MODULE_NAME, PyInit__dk_core },
	{ 0, 0 } // Sentinel
};

static wchar_t* programName = NULL;
static PyDKPython* pythonInstance = NULL;
static DKSpinLock pythonInstanceLock;
static DKStack<PyGILState_STATE> gilStateStack;


// 파이썬 초기화 설정 플래그 (Embedded python)
#ifndef Py_ENABLE_SHARED
extern "C" int Py_DebugFlag;				/* Needed by parser.c */
extern "C" int Py_VerboseFlag;				/* Needed by import.c */
extern "C" int Py_QuietFlag;				/* Needed by sysmodule.c */
extern "C" int Py_InteractiveFlag;			/* Needed by Py_FdIsInteractive() below */
extern "C" int Py_InspectFlag;				/* Needed to determine whether to exit at SystemExit */
extern "C" int Py_NoSiteFlag;				/* Suppress 'import site' */
extern "C" int Py_BytesWarningFlag;			/* Warn on str(bytes) and str(buffer) */
extern "C" int Py_DontWriteBytecodeFlag;	/* Suppress writing bytecode files (*.py[co]) */
extern "C" int Py_UseClassExceptionsFlag;	/* Needed by bltinmodule.c: deprecated */
extern "C" int Py_FrozenFlag;				/* Needed by getpath.c */
extern "C" int Py_IgnoreEnvironmentFlag;	/* e.g. PYTHONPATH, PYTHONHOME */
extern "C" int Py_NoUserSiteDirectory;		/* for -s and site.py */
extern "C" int Py_UnbufferedStdioFlag;		/* Unbuffered binary std{in,out,err} */
extern "C" int Py_HashRandomizationFlag;	/* for -R and PYTHONHASHSEED */

// file-system-encoding
extern const char* Py_FileSystemDefaultEncoding;
extern int Py_HasFileSystemDefaultEncoding;
#endif


#ifdef HAVE_MBCS
#pragma message("WARNING: Python fs-encoding might be MBCS!")
// 파일을 실행할때 파일명을 올바르게 출력하려면 utf-8 이어야 한다.
// mbcs 를 utf-8 로 변경하는 방법
//  1. HAVE_MBCS 제거 (unicodeobject.h)
//  2. Py_FileSystemDefaultEncoding 값 변경 (mbcs -> utf-8, bltinmodule.c)
//  3. PyUnicode_EncodeFSDefault 수정 (mbcs -> utf-8, unicodeobject.c)
//  4. PyUnicode_DecodeFSDefault 수정 (mbcs -> utf-8, unicodeobject.c)
#endif

PyDKPython::PyDKPython(void* ts)
: threadState(ts)
, PyDKInterpreter(static_cast<PyThreadState*>(ts)->interp)
{
	this->Init();
}

PyDKPython::~PyDKPython(void)
{
	DKCriticalSection<DKSpinLock> guard(pythonInstanceLock);
	//this->Terminate();
	pythonInstance = NULL;

	DKLog("Shutting down python (Py_Finalize)\n");
	PyEval_RestoreThread((PyThreadState*)this->threadState);
	Py_Finalize();
}

DKObject<PyDKPython> PyDKPython::Create(const DKString::StringArray& paths)
{
	DKCriticalSection<DKSpinLock> guard(pythonInstanceLock);

#ifdef __ANDROID__
	if (Py_FileSystemDefaultEncoding == NULL && Py_HasFileSystemDefaultEncoding == 0)
	{
		Py_FileSystemDefaultEncoding = "utf-8";
		Py_HasFileSystemDefaultEncoding = 1;
	}
#endif

	DKString pathStr = L"";
	if (paths.Count() > 0)
		pathStr = paths.Value(0);
	for (int i = 1; i < paths.Count(); ++i)
	{
#ifdef _WIN32
		pathStr.Append(L";");
#else
		pathStr.Append(L":");
#endif
		pathStr.Append(paths.Value(i));
	}

#ifndef Py_ENABLE_SHARED
	Py_NoSiteFlag = 1;
	Py_NoUserSiteDirectory = 1;
	Py_IgnoreEnvironmentFlag = 1;
	Py_InspectFlag = 1;
#ifdef DKLIB_DEBUG_ENABLED
	//Py_VerboseFlag = 1;
#endif
#endif
	//PyDKIgnoreStdio = 1;

	//Py_SetProgramName(L"PyDK");
	//Py_SetPythonHome(L"");
	if (pathStr.Length() > 0)
		Py_SetPath(pathStr);

	PyImport_ExtendInittab(pydkExtensions);

#ifdef _WIN32
	// win32 에서 콘솔이 아닌경우 stdio 초기화 하다가 에러가 난다.
	auto ioFiles = { stdin, stdout, stderr };
	for (FILE* f : ioFiles)
	{
		int fd = fileno(f);
		struct stat buf;
		if (fstat(fd, &buf) < 0 && errno == EBADF)
			fclose(f);
	}
#endif

	Py_Initialize();
	//Py_InitializeEx(1);

	// stdin, stdout, stderr 리다이렉팅
	PySys_SetObject("__stdin__", Py_None);
	PySys_SetObject("__stdout__", Py_None);
	//PySys_SetObject("__stderr__", Py_None);

	PyEval_InitThreads();

	DKLog("PyDK Python initialized.\n");
	//DKLog("PyDK name: %ls\n", (const wchar_t*)app);
	DKLog("PyDK Python path: %ls\n", (const wchar_t*)pathStr);

	PyThreadState* ts = PyEval_SaveThread();
	DKObject<PyDKPython> python = DKOBJECT_NEW PyDKPython(ts);
	pythonInstance = python;
	return python;
}

PyDKPython* PyDKPython::SharedInstance(void)
{
	DKCriticalSection<DKSpinLock> guard(pythonInstanceLock);
	return pythonInstance;
}

bool PyDKPython::BindContext(void* interp)
{
	PyGILState_STATE st = PyGILState_Ensure();
	gilStateStack.Push(st);
	return true;
}

void PyDKPython::UnbindContext(void)
{
	PyGILState_STATE st;
	gilStateStack.Pop(st);
	PyGILState_Release(st);
}

DKObject<PyDKInterpreter> PyDKPython::NewInterpreter(void)
{
	class SubInterpreter : public PyDKInterpreter
	{
	public:
		SubInterpreter(PyInterpreterState* st, PyDKPython* py)
			: PyDKInterpreter(st), python(py)
		{
		}
		~SubInterpreter(void)
		{
			this->Terminate();

			PyGILState_STATE st = PyGILState_Ensure();
			DKCriticalSection<DKSpinLock> gaurd(lock);

			threadStateMap.EnumerateForward([](ThreadStateMap::Pair& pair)
			{
				PyThreadState* ts = pair.value;
				PyThreadState_Clear(ts);
				PyThreadState_Delete(ts);
			});
			threadStateMap.Clear();
			PyGILState_Release(st);
		}
		bool BindContext(void* itp) override
		{
			DKCriticalSection<DKSpinLock> gaurd(lock);
			PyInterpreterState* interp = static_cast<PyInterpreterState*>(itp);

			DKThread::ThreadId threadId = DKThread::CurrentThreadId();
			auto p = threadStateMap.Find(threadId);
			if (p)
			{
				PyEval_RestoreThread(p->value);
			}
			else
			{
				PyThreadState* ts = PyThreadState_New(interp);
				PyEval_RestoreThread(ts);
				threadStateMap.Insert(threadId, ts);
			}
			return true;
		}
		void UnbindContext(void) override
		{
			DKCriticalSection<DKSpinLock> gaurd(lock);
			auto p = threadStateMap.Find(DKThread::CurrentThreadId());
			PyThreadState* st = PyEval_SaveThread();
			DKASSERT_DEBUG(p->value == st);
		}
		bool Init(void) override
		{
			return PyDKInterpreter::Init();
		}

		DKSpinLock lock;
		typedef DKMap<DKThread::ThreadId, PyThreadState*> ThreadStateMap;
		ThreadStateMap threadStateMap;
		DKObject<PyDKPython> python;		// hold ref-count
	};

	PyEval_RestoreThread(static_cast<PyThreadState*>(threadState));
	PyThreadState* ts = Py_NewInterpreter();
	PyInterpreterState* interp = ts->interp;
	PyEval_ReleaseThread(ts);

	DKObject<SubInterpreter> obj = DKOBJECT_NEW SubInterpreter(interp, this);

	PyEval_RestoreThread(static_cast<PyThreadState*>(threadState));
	PyThreadState_Clear(ts);
	PyThreadState_Delete(ts);
	PyEval_ReleaseThread(static_cast<PyThreadState*>(threadState));

	if (obj->Init())
		return obj.SafeCast<PyDKInterpreter>();
	return NULL;
}
