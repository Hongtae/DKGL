//
//  File: AppEventLoop.h
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <Windows.h>
#include "../../Interface/DKApplicationInterface.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Win32
		{
			class AppEventLoop : public DKEventLoop
			{
			public:
				AppEventLoop(DKApplication* app);
				~AppEventLoop(void);

				bool Run(void) override;
				void Stop(void) override;

			private:
				DKApplication* appInstance;
				DWORD threadId;
				bool running;
			};
		}
	}
}
#endif // _WIN32
