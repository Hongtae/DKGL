//
//  File: DKScreen.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKScreen.h"

namespace DKFramework
{
	namespace Private
	{
		class RenderEventLoop : public DKEventLoop
		{
		};
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKScreen::DKScreen(DKWindow*, DKFrame*)
{
	eventLoop = DKOBJECT_NEW RenderEventLoop();
}

DKScreen::~DKScreen(void)
{
	if (thread && thread->IsAlive())
	{
		eventLoop->Stop();
		thread->WaitTerminate();
	}
}

void DKScreen::Start(void)
{

}

void DKScreen::Pause(void)
{
}

void DKScreen::Resume(void)
{
}

void DKScreen::Stop(void)
{
}

void DKScreen::DrawFrame(void)
{
}

void DKScreen::EventLoopIdle(DKScreen*, DKEventLoop*)
{
}
