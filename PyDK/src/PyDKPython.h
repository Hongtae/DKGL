#pragma once
#include <DK/DK.h>
#include "PyDKApi.h"
#include "PyDKInterpreter.h"

///////////////////////////////////////////////////////////////////////////////
// PyDK
// 파이썬 코어. 프로세스당 단 하나의 파이썬 객체가 있어야 한다.
// 모듈 초기화 때문에 path 경로를 매개변수로 받는다.
// 모든 인터프리터(PyDKInterpreter) 객체들이 이 객체(PyDK)를 공유한다.

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
