//
//  File: DKAnimationController.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKAnimationController.h"
#include "DKAffineTransform3.h"

using namespace DKFramework;

DKAnimationController::DKAnimationController()
	: lastUpdatedTick(0)
	, frame(0)
	, speed(1.0)
{
}

DKAnimationController::~DKAnimationController()
{
}

void DKAnimationController::Update(double timeDelta, DKTimeTick tick)
{
	if (tick == this->lastUpdatedTick)
		return;

	this->lastUpdatedTick = tick;

	if (this->IsPlaying())
	{
		double delta = timeDelta * speed;
		if (delta != 0)
		{
			this->frame += delta;
			this->UpdateFrame(this->frame);
		}
	}
}

void DKAnimationController::SetFrame(float frame)
{
	this->frame = frame;
	this->UpdateFrame(this->frame);
}

void DKAnimationController::SetSpeed(float speed)
{
	this->speed = speed;
}
