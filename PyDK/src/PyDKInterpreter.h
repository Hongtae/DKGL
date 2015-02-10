#pragma once
#include <DK/DK.h>
#include "PyDKApi.h"

///////////////////////////////////////////////////////////////////////////////
// PyDKInterpreter : 파이썬 인터프리터

class PYDK_API PyDKInterpreter
{
public:
	typedef DKFoundation::DKFunctionSignature<void(const DKFoundation::DKString&)> OutputCallback;
	typedef DKFoundation::DKFunctionSignature<bool(const DKFoundation::DKString&, DKFoundation::DKString&)> InputCallback;

	virtual ~PyDKInterpreter(void);

	void PrintStdout(const DKFoundation::DKString& str);
	void PrintStderr(const DKFoundation::DKString& str);

	void SetInputCallback(InputCallback* inp);
	void SetOutputCallback(OutputCallback* out);
	void SetErrorCallback(OutputCallback* err);

	bool RunFile(const DKFoundation::DKString& file);
	bool RunString(const DKFoundation::DKString& str);
	bool RunString(const char* str);	// utf8 string

	bool RunInteractiveSingleCommand(const DKFoundation::DKString& name);
	bool RunInteractiveLoop(const DKFoundation::DKString& name);

	static PyDKInterpreter* CurrentInterpreter(void);

	class Dispatcher;

protected:
	virtual bool BindContext(void* interp) = 0;
	virtual void UnbindContext(void) = 0;
	virtual bool Init(void);
	virtual void Terminate(void);

	PyDKInterpreter(void* interpreterState);

private:
	void* interpreterState;
	bool systemExitFlagged;
	DKFoundation::DKMutex interactiveLock;

	DKFoundation::DKObject<InputCallback> input;
	DKFoundation::DKObject<OutputCallback> output;
	DKFoundation::DKObject<OutputCallback> error;

	void CallbackStdout(const char*);
	void CallbackStderr(const char*);
	bool CallbackRequestInput(const char*, DKFoundation::DKString&);
	void FlushStdout(void);
	void FlushStderr(void);

	PyDKInterpreter(const PyDKInterpreter&);
	PyDKInterpreter& operator = (const PyDKInterpreter&);
};
