//
//  File: DKScreen.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKScreen.h"

using namespace DKFramework;
using namespace DKFramework::Private;

DKScreen::DKScreen(DKWindow*, DKFrame*)
{
	struct LocalEventLoop : public DKEventLoop
	{
		DKScreen* screen;
	};
	DKObject<LocalEventLoop> localEventLoop = DKOBJECT_NEW LocalEventLoop();
	localEventLoop->screen = this;

	eventLoop = localEventLoop.SafeCast<DKEventLoop>();
	graphicsDevice = DKGraphicsDevice::SharedInstance();
	audioDevice = DKAudioDevice::SharedInstance();
}

DKScreen::~DKScreen()
{
	if (thread && thread->IsAlive())
	{
		eventLoop->Stop();
		thread->WaitTerminate();
	}
}

void DKScreen::Start()
{

}

void DKScreen::Pause()
{
}

void DKScreen::Resume()
{
}

void DKScreen::Stop()
{
}

void DKScreen::DrawFrame()
{
}

void DKScreen::EventLoopIdle(DKScreen*, DKEventLoop*)
{
}
