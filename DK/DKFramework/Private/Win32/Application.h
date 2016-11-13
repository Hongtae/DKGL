//
//  File: Application.h
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <Windows.h>
#include "../../Interface/DKApplicationInterface.h"
#include "AppLogger.h"
#include "AppEventLoop.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Win32
		{
			class Application : public DKApplicationInterface
			{
			public:
				Application(DKApplication* app);
				~Application(void);

				DKEventLoop* EventLoop(void) override;
				DKLogger* DefaultLogger(void) override;

				DKString DefaultPath(SystemPath) override;
				DKString ProcessInfoString(ProcessInfo) override;

				DKObject<DKData> LoadResource(const DKString& res, DKAllocator& alloc) override;		// read-writable
				DKObject<DKData> LoadStaticResource(const DKString& res) override;	// read-only

				DKRect DisplayBounds(int displayId) const override;
				DKRect ScreenContentBounds(int displayId) const override;

			private:
				AppEventLoop mainLoop;
			};
		}
	}
}
#endif // _WIN32
