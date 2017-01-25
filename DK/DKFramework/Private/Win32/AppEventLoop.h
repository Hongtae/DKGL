//
//  File: AppEventLoop.h
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
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

				DKObject<PendingState> Post(const DKOperation* operation, double delay) override;
				DKObject<PendingState> Post(const DKOperation* operation, const DKDateTime& runAfter) override;

			private:
				DKApplication* appInstance;
				DWORD threadId;
				bool running;
			};
		}
	}
}
#endif // _WIN32
