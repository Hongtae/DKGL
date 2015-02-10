#pragma once
#include <DK/DK.h>
#include "PyDKApi.h"
#include "PyDKInterpreter.h"

///////////////////////////////////////////////////////////////////////////////
// PyDKApplication
// Python app class
// 

namespace DKFramework
{
	class PYDK_API PyDKApplication : public DKApplication
	{
	public:
		PyDKApplication(void);
		~PyDKApplication(void);

	protected:
		void OnHidden(void) override;
		void OnRestore(void) override;
		void OnActivated(void) override;
		void OnDeactivated(void) override;
		void OnInitialize(void) override;
		void OnTerminate(void) override;

		void CancelScriptBinding(void);
	};
}
