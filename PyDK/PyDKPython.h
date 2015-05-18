#pragma once
#include <DK/DK.h>
#include "PyDKApi.h"
#include "PyDKInterpreter.h"

///////////////////////////////////////////////////////////////////////////////
// PyDKPython
// Python object. only one instance can be exist per process.
// You have to provide python-stand library path dir with arguments when
// create object.
// All sub-interpreters shares this object.

class PYDK_API PyDKPython : public PyDKInterpreter
{
public:
	static DKFoundation::DKObject<PyDKPython> Create(const DKFoundation::DKString::StringArray& paths);
	static PyDKPython* SharedInstance(void);

	DKFoundation::DKObject<PyDKInterpreter> NewInterpreter(void);

	~PyDKPython(void);

protected:
	virtual bool BindContext(void* interp) override;
	virtual void UnbindContext(void) override;

private:
	PyDKPython(void* threadState);
	void* threadState;
	PyDKPython(const PyDKPython&);
	PyDKPython& operator = (const PyDKPython&);
};
