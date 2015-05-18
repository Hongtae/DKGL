#include <Python.h>
#include <errcode.h>
#include "PyDKInterpreter.h"

using namespace DKFoundation;
using namespace DKFramework;


#define PYDK_DISPATCH_FAILED_ERROR		DKERROR_THROW("Interpreter dispatch failed!")

class PyDKInterpreter::Dispatcher
{
	DKMap<PyInterpreterState*, PyDKInterpreter*> interpreterMap;

public:
	bool RegisterInterpreter(PyInterpreterState* st, PyDKInterpreter* intp)
	{
		return interpreterMap.Insert(st, intp);
	}
	void UnregisterInterpreter(PyInterpreterState* st)
	{
		interpreterMap.Remove(st);
	}
	PyDKInterpreter* FindInterpreter(PyInterpreterState* st)
	{
		auto pair = interpreterMap.Find(st);
		if (pair)
			return pair->value;
		return NULL;
	}
	PyDKInterpreter* CurrentInterpreter(void)
	{
		return FindInterpreter(PyThreadState_GET()->interp);
	}
	char* RequestStdin(const char* prompt)
	{
		char* p = NULL;
		PyDKInterpreter* interp = CurrentInterpreter();
		if (interp)
		{
			DKString userInput;
			if (interp->CallbackRequestInput(prompt, userInput))
			{
				DKStringU8 inputU8(userInput);
				size_t len = inputU8.Bytes();
				p = (char*)PyMem_MALLOC(len + 1);
				memcpy(p, (const char*)inputU8, len);
				p[len] = 0;
			}
		}
		if (p == NULL)
		{
			// EOF
			p = (char*)PyMem_MALLOC(2);
			p[0] = 0;
			p[1] = 0;
		}
		return p;
	}
	bool PrintStdout(const char* mesg)
	{
		PyDKInterpreter* interp = CurrentInterpreter();
		if (interp)
		{
			interp->CallbackStdout(mesg);
			return true;
		}
		return false;
	}
	bool PrintStderr(const char* mesg)
	{
		PyDKInterpreter* interp = CurrentInterpreter();
		if (interp)
		{
			interp->CallbackStderr(mesg);
			return true;
		}
		return false;
	}
	bool FlushStdout(void)
	{
		PyDKInterpreter* interp = CurrentInterpreter();
		if (interp)
		{
			interp->FlushStdout();
			return true;
		}
		return false;
	}
	bool FlushStderr(void)
	{
		PyDKInterpreter* interp = CurrentInterpreter();
		if (interp)
		{
			interp->FlushStderr();
			return true;
		}
		return false;
	}
	PyModuleDef* StdinModule(void)
	{
		auto readline = [](PyObject* self, PyObject* args)->PyObject*
		{
			char* inp = Instance().RequestStdin("");
			if (inp)
			{
				PyObject* result = PyUnicode_FromString(inp);
				PyMem_FREE(inp);
				return result;
			}
			Py_RETURN_NONE;
		};
		static PyMethodDef methods[] =
		{
			{ "readline", readline, METH_VARARGS, "read single line from stdin" },
			{ 0, 0, 0, 0 } // sentinel
		};
		static PyModuleDef module =
		{
			PyModuleDef_HEAD_INIT,
			"igpython_stdin",
			"forwarded stdin for GUI console",
			-1,
			methods,
		};
		return &module;
	}
	PyModuleDef* StdoutModule(void)
	{
		auto write = [](PyObject* self, PyObject* args)->PyObject*
		{
			const char* str;
			if (!PyArg_ParseTuple(args, "s", &str))
				return NULL;

			if (!Instance().PrintStdout(str))
			{
				PYDK_DISPATCH_FAILED_ERROR;
			}
			Py_RETURN_NONE;
		};
		auto flush = [](PyObject* self, PyObject* args)->PyObject*
		{
			if (!Instance().FlushStdout())
			{
				PYDK_DISPATCH_FAILED_ERROR;
			}
			Py_RETURN_NONE;
		};
		static PyMethodDef methods[] =
		{
			{ "write", write, METH_VARARGS, "write text to stdout" },
			{ "flush", flush, METH_VARARGS, "flush stdout" },
			{ 0, 0, 0, 0 } // sentinel
		};
		static PyModuleDef module =
		{
			PyModuleDef_HEAD_INIT,
			"igpython_stdout",
			"forwarded stdout for GUI console",
			-1,
			methods,
		};
		return &module;
	}
	PyModuleDef* StderrModule(void)
	{
		auto write = [](PyObject* self, PyObject* args)->PyObject*
		{
			const char* str;
			if (!PyArg_ParseTuple(args, "s", &str))
				return NULL;

			if (!Instance().PrintStderr(str))
			{
				PYDK_DISPATCH_FAILED_ERROR;
			}
			Py_RETURN_NONE;
		};
		auto flush = [](PyObject* self, PyObject* args)->PyObject*
		{
			if (!Instance().FlushStderr())
			{
				PYDK_DISPATCH_FAILED_ERROR;
			}
			Py_RETURN_NONE;
		};
		static PyMethodDef methods[] =
		{
			{ "write", write, METH_VARARGS, "write text to stdout" },
			{ "flush", flush, METH_VARARGS, "flush stdout" },
			{ 0, 0, 0, 0 } // sentinel
		};
		static PyModuleDef module =
		{
			PyModuleDef_HEAD_INIT,
			"igpython_stderr",
			"forwarded stderr for GUI console",
			-1,
			methods,
		};
		return &module;
	}
	PyModuleDef* SysExceptHookModule(void)
	{
		auto excepthook = [](PyObject* self, PyObject* args)->PyObject*
		{
			PyObject *exc, *value, *tb;
			if (!PyArg_UnpackTuple(args, "excepthook", 3, 3, &exc, &value, &tb))
				return NULL;

			auto SetSystemExitFlagged = []()->bool
			{
				PyDKInterpreter* interp = Instance().CurrentInterpreter();
				if (interp)
				{
					interp->systemExitFlagged = true;
					return true;
				}
				return false;
			};
			if (PyErr_GivenExceptionMatches(exc, PyExc_SystemExit) && SetSystemExitFlagged())
			{
			}
			else
			{
				PyErr_Display(exc, value, tb);
			}
			Py_RETURN_NONE;
		};
		static PyMethodDef methods[] =
		{
			{
				"excepthook", excepthook, METH_VARARGS,
				"excepthook(exctype, value, traceback) -> None\n\n"
				"Handle an exception by displaying it with a traceback on sys.stderr.\n"
			},
			{ 0, 0, 0, 0 } // sentinel
		};
		static PyModuleDef module =
		{
			PyModuleDef_HEAD_INIT,
			"igpython_sys",
			"system utilities",
			-1,
			methods,
		};
		return &module;
	}
	static Dispatcher& Instance(void)
	{
		static Dispatcher d;
		return d;
	}
};

using Dispatcher = PyDKInterpreter::Dispatcher;
static Dispatcher& dispatcher = Dispatcher::Instance();

static DKMutex interactiveInterpreterLock;
static PyThreadState* currentInteractiveInterpreter = NULL;

static char* PyDKInterpreterReadline(FILE*, FILE*, const char* prompt)
{
	// interactiveInterpreterLock 락이 걸려있는 상태에서 호출된다.
	PyThreadState* interp = currentInteractiveInterpreter;
	currentInteractiveInterpreter = NULL;
	interactiveInterpreterLock.Unlock();

	char* str = NULL;
	if (interp)
	{
		PyEval_RestoreThread(interp);
		str = dispatcher.RequestStdin(prompt);
		PyEval_SaveThread();
	}
	else
	{
		// EOF
		str = (char*)PyMem_MALLOC(2);
		str[0] = 0;
		str[1] = 0;
	}

	interactiveInterpreterLock.Lock();
	currentInteractiveInterpreter = interp;
	return str;
}

PyDKInterpreter::PyDKInterpreter(void* interp)
: interpreterState(interp), systemExitFlagged(false)
{
	DKASSERT_DEBUG(interpreterState != NULL);
}

PyDKInterpreter::~PyDKInterpreter(void)
{
	if (Py_IsInitialized())
	{
		DKASSERT_DEBUG(interpreterState != NULL);

		PyInterpreterState* interp = static_cast<PyInterpreterState*>(this->interpreterState);
		DKASSERT_DEBUG(dispatcher.FindInterpreter(interp) == NULL);

		PyInterpreterState_Delete(interp);
	}
}

bool PyDKInterpreter::Init(void)
{
	DKASSERT_DEBUG(interpreterState != NULL);

	if (this->BindContext(this->interpreterState))
	{
		PyInterpreterState* interp = static_cast<PyInterpreterState*>(this->interpreterState);
		bool result = dispatcher.RegisterInterpreter(interp, this);
		if (result)
		{
			// stdio, sys 모듈 등록함.
			auto setSysObjectAttr = [](const char* attr, PyObject* obj)
			{
				if (obj)
				{
					PyObject_SetAttrString(obj, "encoding", PyUnicode_FromString("utf-8"));
					PySys_SetObject(attr, obj);
					Py_DECREF(obj);
				}
			};
			setSysObjectAttr("stdin", PyModule_Create(dispatcher.StdinModule()));
			setSysObjectAttr("stdout", PyModule_Create(dispatcher.StdoutModule()));
			setSysObjectAttr("stderr", PyModule_Create(dispatcher.StderrModule()));

			// sys.excepthook 리다이렉팅 (SystemExit 처리용)
			PyObject* exceptHook = PyModule_Create(dispatcher.SysExceptHookModule());
			if (exceptHook)
			{
				PyObject* hook = PyDict_GetItemString(PyModule_GetDict(exceptHook), "excepthook");
				if (hook)
				{
					PySys_SetObject("__excepthook__", hook);
					PySys_SetObject("excepthook", hook);
				}
				Py_DECREF(exceptHook);
			}

			// 프롬프트 설정
			if (PySys_GetObject("ps1") == NULL)
			{
				PyObject* ps1 = PyUnicode_FromString(">>> ");
				PySys_SetObject("ps1", ps1);
				Py_XDECREF(ps1);
			}
			if (PySys_GetObject("ps2") == NULL)
			{
				PyObject* ps2 = PyUnicode_FromString("... ");
				PySys_SetObject("ps2", ps2);
				Py_XDECREF(ps2);
			}
			DKLog("PyDKInterpreter(0x%x) initialized.\n", this);
		}
		this->UnbindContext();
		return result;
	}
	return false;
}

void PyDKInterpreter::Terminate(void)
{
	DKASSERT_DEBUG(interpreterState != NULL);

	if (this->BindContext(this->interpreterState))
	{
		//int r = PyRun_SimpleString("import gc\ngc.collect()\n");
		PyInterpreterState* interp = static_cast<PyInterpreterState*>(this->interpreterState);
		PyInterpreterState_Clear(interp);

		dispatcher.UnregisterInterpreter(interp);

		DKLog("PyDKInterpreter(0x%x) terminated.\n", this);

		this->UnbindContext();
	}
}

PyDKInterpreter* PyDKInterpreter::CurrentInterpreter(void)
{
	return dispatcher.CurrentInterpreter();
}

void PyDKInterpreter::PrintStdout(const DKFoundation::DKString& str)
{
	if (this->BindContext(this->interpreterState))
	{
		PySys_WriteStdout("%s", (const char*)DKStringU8(str));
		this->UnbindContext();
	}
}

void PyDKInterpreter::PrintStderr(const DKFoundation::DKString& str)
{
	if (this->BindContext(this->interpreterState))
	{
		PySys_WriteStderr("%s", (const char*)DKStringU8(str));
		this->UnbindContext();
	}
}

void PyDKInterpreter::SetInputCallback(InputCallback* inp)
{
	this->input = inp;
}

void PyDKInterpreter::SetOutputCallback(OutputCallback* out)
{
	this->output = out;
}

void PyDKInterpreter::SetErrorCallback(OutputCallback* err)
{
	this->error = err;
}

void PyDKInterpreter::CallbackStdout(const char* str)
{
	if (output)
	{
		PyThreadState* ts = PyEval_SaveThread();
		output->Invoke(str);
		PyEval_RestoreThread(ts);
	}
	else
		DKLog("%s", str);
}

void PyDKInterpreter::CallbackStderr(const char* str)
{
	if (error)
	{
		PyThreadState* ts = PyEval_SaveThread();
		error->Invoke(str);
		PyEval_RestoreThread(ts);
	}
	else
		DKLog("%s", str);
}

void PyDKInterpreter::FlushStdout(void)
{
	//DKLog("PyDKInterpreter[0x%x] flush-stdout\n", DKThread::CurrentThreadId());
}

void PyDKInterpreter::FlushStderr(void)
{
	//DKLog("PyDKInterpreter[0x%x] flush-stderr\n", DKThread::CurrentThreadId());
}

bool PyDKInterpreter::CallbackRequestInput(const char* prompt, DKString& userInput)
{
	bool succeed = false;
	if (input)
	{
		PyThreadState* ts = PyEval_SaveThread();
		succeed = this->input->Invoke(prompt, userInput);
		PyEval_RestoreThread(ts);
	}
	return succeed;
}

bool PyDKInterpreter::RunFile(const DKString& file)
{
	bool result = false;
	FILE* fp = NULL;
	DKStringU8 fileU8(file);
#ifdef _WIN32
	fp = _wfopen((const wchar_t*)file, L"rb");
#else
	fp = fopen((const char*)fileU8, "rb");
#endif
	if (fp)
	{
		if (this->BindContext(this->interpreterState))
		{
			this->systemExitFlagged = false;

			const char* filename = fileU8;
#ifdef HAVE_MBCS
			size_t s = wcstombs(0, (const wchar_t*)file, 0xffff);
			DKArray<char> buff;
			if (s > 0 && s != size_t(-1))
			{
				buff.Resize(s + 2);
				char* fileMBCS = buff;
				size_t k = wcstombs(fileMBCS, (const wchar_t*)file, s + 2);
				fileMBCS[k] = 0;
				filename = fileMBCS;
			}
			else
			{
				DKLog("WARNING: filename cannot convert to mbcs!\n");
				filename = "<unknown_file>";
			}
#endif
			int ret = PyRun_SimpleFileExFlags(fp, filename, 0, NULL);
			this->UnbindContext();

			result = ret == 0;
		}
		fclose(fp);
	}
	return result;
}

bool PyDKInterpreter::RunString(const DKString& str)
{
	return RunString(DKStringU8(str));
}

bool PyDKInterpreter::RunString(const char* str)
{
	bool result = false;
	if (str && str[0])
	{
		if (this->BindContext(this->interpreterState))
		{
			this->systemExitFlagged = false;
			result = PyRun_SimpleString((const char*)str) == 0;
			this->UnbindContext();
		}
	}
	return result;
}

bool PyDKInterpreter::RunInteractiveSingleCommand(const DKString& name)
{
	bool result = false;
	if (input)
	{
		DKASSERT_DEBUG(this->interpreterState);

		DKCriticalSection<DKMutex> guard(interactiveLock);
		if (this->BindContext(this->interpreterState))
		{
			PyCompilerFlags cf;
			cf.cf_flags = PyCF_SOURCE_IS_UTF8;

			if (true)
			{
				DKCriticalSection<DKMutex> guard(interactiveInterpreterLock);

				auto oldReadline = PyOS_ReadlineFunctionPointer;
				PyOS_ReadlineFunctionPointer = PyDKInterpreterReadline;
				auto oldInterp = currentInteractiveInterpreter;
				currentInteractiveInterpreter = PyThreadState_GET();

				this->systemExitFlagged = false;
				result = PyRun_InteractiveOneFlags(stdin, DKStringU8(name), &cf) == 0;

				PyOS_ReadlineFunctionPointer = oldReadline;
				currentInteractiveInterpreter = oldInterp;
			}

			this->UnbindContext();
		}
	}
	return result;
}

bool PyDKInterpreter::RunInteractiveLoop(const DKString& name)
{
	// 2014-4-16
	// PyRun_InteractiveOneFlags 을 사용할때, FILE* fp 를 파일을 넣어도
	// tokenizer.c 에서 stdin, stdout 으로 바꿔버린다. (PyOS_Readline 함수)
	//

	bool result = false;
	if (input)
	{
		DKASSERT_DEBUG(this->interpreterState);

		DKCriticalSection<DKMutex> guard(interactiveLock);
		if (this->BindContext(this->interpreterState))
		{
			PyCompilerFlags cf;
			cf.cf_flags = PyCF_SOURCE_IS_UTF8;

			DKLog("PyDKInterpreter(0x%x) entering interactive loop.\n", this);

			DKStringU8 filename(name);
			enum class LoopState
			{
				Running = 0,
				TerminatedWithEOF,
				TerminatedOutOfMemory,
				TerminatedBySystemExit,
			};
			LoopState loop = LoopState::Running;
			while (loop == LoopState::Running)
			{
				int ret = E_OK;
				if (true)
				{
					DKCriticalSection<DKMutex> guard(interactiveInterpreterLock);

					auto oldReadline = PyOS_ReadlineFunctionPointer;
					PyOS_ReadlineFunctionPointer = PyDKInterpreterReadline;
					auto oldInterp = currentInteractiveInterpreter;
					currentInteractiveInterpreter = PyThreadState_GET();

					this->systemExitFlagged = false;
					ret = PyRun_InteractiveOneFlags(stdin, filename, &cf);

					PyOS_ReadlineFunctionPointer = oldReadline;
					currentInteractiveInterpreter = oldInterp;
				}

#ifdef Py_REF_DEBUG
				DKLog("PythonRef:%lu\n", (unsigned long)_Py_GetRefTotal());
#endif
				switch (ret)
				{
				case E_EOF:
					loop = LoopState::TerminatedWithEOF;
					break;
				case E_NOMEM:
					loop = LoopState::TerminatedOutOfMemory;
					break;
				default:
					if (this->systemExitFlagged)
						loop = LoopState::TerminatedBySystemExit;
					break;
				}
			}
			const char* reason = "";
			switch (loop)
			{
			case LoopState::TerminatedWithEOF:
				reason = "EOF";
				break;
			case LoopState::TerminatedOutOfMemory:
				reason = "Out of Memory";
				break;
			case LoopState::TerminatedBySystemExit:
				reason = "SystemExit";
				break;
			default:
				reason = "Unknown";
				break;
			}

			result = true;

			DKLog("PyDKInterpreter(0x%x) leaving interactive loop:%s.\n", this, reason);
			this->UnbindContext();
		}
	}
	return result;
}
