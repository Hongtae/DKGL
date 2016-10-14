#pragma once
#include <DK/DK.h>
#include "PyDKApi.h"

///////////////////////////////////////////////////////////////////////////////
// PyDKInterpreter : Python sub interpreter

class PYDK_API PyDKInterpreter
{
public:
	typedef DKFunctionSignature<void(const DKString&)> OutputCallback;
	typedef DKFunctionSignature<bool(const DKString&, DKString&)> InputCallback;

	virtual ~PyDKInterpreter(void);

	void PrintStdout(const DKString& str);
	void PrintStderr(const DKString& str);

	void SetInputCallback(InputCallback* inp);
	void SetOutputCallback(OutputCallback* out);
	void SetErrorCallback(OutputCallback* err);

	bool RunFile(const DKString& file);
	bool RunString(const DKString& str);
	bool RunString(const char* str);	// utf8 string

	bool RunInteractiveSingleCommand(const DKString& name);
	bool RunInteractiveLoop(const DKString& name);

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
	DKMutex interactiveLock;

	DKObject<InputCallback> input;
	DKObject<OutputCallback> output;
	DKObject<OutputCallback> error;

	void CallbackStdout(const char*);
	void CallbackStderr(const char*);
	bool CallbackRequestInput(const char*, DKString&);
	void FlushStdout(void);
	void FlushStderr(void);

	PyDKInterpreter(const PyDKInterpreter&);
	PyDKInterpreter& operator = (const PyDKInterpreter&);
};
