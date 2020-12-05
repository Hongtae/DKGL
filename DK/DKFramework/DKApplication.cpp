//
//  File: DKApplication.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2019 Hongtae Kim. All rights reserved.
//

#include "DKApplication.h"
#include "Interface/DKApplicationInterface.h"


namespace DKFoundation::Private
{
    void PerformOperationWithErrorHandler(const DKOperation*, size_t);
}

namespace DKFramework::Private
{
    using namespace DKFoundation::Private;
    static DKCondition appCond;
    static DKApplication* application = NULL;
    static bool disableLogger = false;
}
using namespace DKFramework;

DKApplication::DKApplication(int argc, char* argv[])
	: exitCode(0)
{
	DKCriticalSection<DKCondition> guard(Private::appCond);
	while (Private::application != NULL)
	{
		DKLog("DKApplication: Waiting for previous application instance terminated.\n");
		Private::appCond.Wait();
	}

	impl = DKApplicationInterface::CreateInterface(this, argc, argv);
	Private::disableLogger = false;
	DKPropertySet::SystemConfig().LookUpValueForKeyPath("DisableApplicationLogger",
														DKFunction([&](const DKVariant& var)
	{
		if (var.ValueType() == DKVariant::TypeInteger)
		{
			Private::disableLogger = var.Integer() != 0;
			return true;
		}
		return false;
	}));
	if (!Private::disableLogger)
		impl->DefaultLogger()->Bind();

	Private::application = this;
}

DKApplication::DKApplication() : DKApplication(0, 0)
{
}

DKApplication::~DKApplication()
{
	if (!Private::disableLogger)
		impl->DefaultLogger()->Unbind();

	DKCriticalSection<DKCondition> guard(Private::appCond);

	delete impl;

	Private::application = NULL;
	Private::appCond.Broadcast();
}

int DKApplication::Run()
{
	// Only one thread can have running instance!
	DKCriticalSection<DKMutex> section(mutex);

	struct MainLoopRunner : public DKOperation
	{
		DKApplication::EventLoop* mainLoop;
		void Perform() const override
		{
			mainLoop->Run();
		}
	};
	MainLoopRunner runner;
	runner.mainLoop = impl->MainLoop();
	Private::PerformOperationWithErrorHandler(&runner, DKERROR_DEFAULT_CALLSTACK_TRACE_DEPTH);

	return exitCode;
}

DKApplication* DKApplication::Instance()
{
	DKCriticalSection<DKCondition> guard(Private::appCond);
	return Private::application;
}

DKApplication::EventLoop* DKApplication::MainLoop()
{
	return impl->MainLoop();
}

void DKApplication::Initialize()
{
	DKDateTime current = DKDateTime::Now();
	DKLog("DKApplication prepare to launch at %ls. (MainThread:0x%x)\n",
		  (const wchar_t*)current.Format(DKDateTime::DateFormatWithWeekdayShort, DKDateTime::TimeFormat24HourWithMicrosecondTimezone), DKThread::CurrentThreadId());

	initializedAt = current;
	
	OnInitialize();
	
	current = DKDateTime::Now();
	
	DKLog("DKApplication initialized at %ls. (MainThread:0x%x)\n",
		  (const wchar_t*)current.Format(DKDateTime::DateFormatWithWeekdayShort, DKDateTime::TimeFormat24HourWithMicrosecondTimezone), DKThread::CurrentThreadId());
}

void DKApplication::Finalize()
{
	OnTerminate();
	
	DKDateTime current = DKDateTime::Now();
	DKLog("DKApplication terminated at %04d-%02d-%02d %02d:%02d:%02d.%06d.(MainThread:0x%x, exitCode:%d)\n",
		current.Year(), current.Month(), current.Day(),
		current.Hour(), current.Minute(), current.Second(), current.Microsecond(),
		DKThread::CurrentThreadId(), exitCode);
	double elapsed = current.Interval(initializedAt);
	DKLog("DKApplication runs %f seconds.\n", elapsed);
}

void DKApplication::Terminate(int exitCode)
{
	impl->MainLoop()->Submit(DKFunction([=]()
	{
		this->exitCode = exitCode;
	})->Invocation());
	impl->MainLoop()->Stop();
}

DKString DKApplication::DefaultPath(SystemPath sp)
{
	return impl->DefaultPath(sp);
}

DKString DKApplication::ProcessInfoString(ProcessInfo pi)
{
	return impl->ProcessInfoString(pi);
}

void DKApplication::OnHidden()
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}

void DKApplication::OnRestore()
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}

void DKApplication::OnActivated()
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}

void DKApplication::OnDeactivated()
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}

void DKApplication::OnInitialize()
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}

void DKApplication::OnTerminate()
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}

DKObject<DKData> DKApplication::LoadResource(const DKString& res, DKAllocator& alloc)
{
	return impl->LoadResource(res, alloc);
}

DKObject<DKData> DKApplication::LoadStaticResource(const DKString& res)
{
	return impl->LoadStaticResource(res);
}

DKRect DKApplication::DisplayBounds(int displayId) const
{
	return impl->DisplayBounds(displayId);
}

DKRect DKApplication::ScreenContentBounds(int displayId) const
{
	return impl->ScreenContentBounds(displayId);
}
