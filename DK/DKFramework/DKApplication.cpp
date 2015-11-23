//
//  File: DKApplication.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKApplication.h"
#include "Interface/DKApplicationInterface.h"

namespace DKFoundation
{
	namespace Private
	{
		void WaitTerminateAllRunLoops(void);
		void PerformOperationWithErrorHandler(const DKOperation*, size_t);
	}
}

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		static DKCondition appCond;
		static DKApplication *application = NULL;
	}
}

using namespace DKFramework;


DKApplication::DKApplication(int argc, char* argv[])
	: args(argv, argc)
{
	DKCriticalSection<DKCondition> guard(Private::appCond);
	while (Private::application != NULL)
	{
		DKLog("DKApplication: Waiting for previous application instance terminated.\n");
		Private::appCond.Wait();
	}

	impl = DKApplicationInterface::CreateInterface(this);
	DKLoggerCompareAndReplace(NULL, &impl->DefaultLogger());

	Private::application = this;
}

DKApplication::DKApplication(void) : DKApplication(0, 0)
{
}

DKApplication::~DKApplication(void)
{
	DKLoggerCompareAndReplace(&impl->DefaultLogger(), NULL);

	delete impl;

	DKCriticalSection<DKCondition> guard(Private::appCond);
	Private::application = NULL;
	Private::appCond.Broadcast();
}

void DKApplication::SetArgs(int argc, char* argv[])
{
	args.Clear();
	args.Add(argv, argc);
}

int DKApplication::Run(void)
{
	DKCriticalSection<DKMutex> section(mutex);

	struct ContextRun : public DKOperation
	{
		ContextRun(DKApplication& a) : app(a) {}
		DKApplication& app;
		mutable int result;
		void Perform(void) const override
		{
			result = app.impl->Run(app.args);
		}
	};
	ContextRun run(*this);
	DKFoundation::Private::PerformOperationWithErrorHandler( &run, DKERROR_DEFAULT_CALLSTACK_TRACE_DEPTH);
	return run.result;
}

DKApplication* DKApplication::Instance(void)
{
	//DKCriticalSection<DKCondition> guard(Private::appCond);
	return Private::application;
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
	
	DKLog("Waiting for runloop being terminated...\n");
	DKFoundation::Private::WaitTerminateAllRunLoops();

	DKDateTime current = DKDateTime::Now();
	DKLog("DKApplication terminated at %04d-%02d-%02d %02d:%02d:%02d.%06d.(MainThread:0x%x)\n",
		  current.Year(), current.Month(), current.Day(), current.Hour(), current.Minute(), current.Second(), current.Microsecond(),
		  DKThread::CurrentThreadId());
	double elapsed = current.Interval(initializedAt);
	DKLog("DKApplication runs %f seconds.\n", elapsed);
}

void DKApplication::Terminate(int exitCode)
{
	impl->Terminate(exitCode);
}

DKString DKApplication::EnvironmentPath(SystemPath env)
{
	return impl->EnvironmentPath(env);
}

DKString DKApplication::ModulePath(void)
{
	return impl->ModulePath();
}

void DKApplication::OnHidden(void)
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}

void DKApplication::OnRestore(void)
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}

void DKApplication::OnActivated(void)
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}

void DKApplication::OnDeactivated(void)
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}

void DKApplication::OnInitialize(void)
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}

void DKApplication::OnTerminate(void)
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

DKString DKApplication::HostName(void) const
{
	return impl->HostName();
}

DKString DKApplication::OSName(void) const
{
	return impl->OSName();
}

DKString DKApplication::UserName(void) const
{
	return impl->UserName();
}
