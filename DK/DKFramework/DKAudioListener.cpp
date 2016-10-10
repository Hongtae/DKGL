//
//  File: DKAudioListener.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenAL.h"
#include "DKAudioListener.h"
#include "DKOpenALContext.h"

using namespace DKGL;
using namespace DKGL;

DKAudioListener::DKAudioListener(void)
	: context(DKOpenALContext::SharedInstance())
	, position(0,0,0)
	, velocity(1,1,1)
	, forward(0,0,-1)
	, up(0,1,0)
	, gain(1.0)
{
	alGetListenerf(AL_GAIN, &gain);
	alGetListener3f(AL_POSITION, &position.x, &position.y, &position.z);
	alGetListener3f(AL_VELOCITY, &velocity.x, &velocity.y, &velocity.z);
	
	float v[6] = {forward.x, forward.y, forward.z, up.x, up.y, up.z};
	alGetListenerfv(AL_ORIENTATION, v);
	forward = DKVector3(v[0], v[1], v[2]);
	up = DKVector3(v[3], v[4], v[5]);
}

DKAudioListener::~DKAudioListener(void)
{
}

void DKAudioListener::SetGain(float f)
{
	gain = Max(f, 0.0);
	alListenerf(AL_GAIN, gain);
}

float DKAudioListener::Gain(void) const
{
	return gain;
}

void DKAudioListener::SetPosition(const DKVector3& v)
{
	position = v;
	alListener3f(AL_POSITION, position.x, position.y, position.z);
}

const DKVector3& DKAudioListener::Position(void) const
{
	return position;
}

void DKAudioListener::SetOrientation(const DKVector3& f, const DKVector3& u)
{
	forward = DKVector3(f).Normalize();
	up = DKVector3(u).Normalize();
	
	float v[6] = {forward.x, forward.y, forward.z, up.x, up.y, up.z};
	alListenerfv(AL_ORIENTATION, v);
}

void DKAudioListener::SetOrientation(const DKMatrix3& m)
{
	SetOrientation(DKVector3(m._31, m._32, m._33), DKVector3(m._21, m._22, m._23));
}

const DKVector3& DKAudioListener::Forward(void) const
{
	return forward;
}

const DKVector3& DKAudioListener::Up(void) const
{
	return up;
}

void DKAudioListener::SetVelocity(const DKVector3& v)
{
	velocity = v;
	alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

const DKVector3& DKAudioListener::Velocity(void) const
{
	return velocity;
}
