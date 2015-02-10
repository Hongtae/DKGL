//
//  File: DKAnimationController.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKTransform.h"

////////////////////////////////////////////////////////////////////////////////
// DKAnimationController
// Animation control interface, abstract class.
// sublcass to customize your action behaviors how object being animated.
// Used for DKModel node animation.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKAnimatedTransform
	{
	public:
		typedef DKFoundation::DKString NodeId;
		virtual ~DKAnimatedTransform(void) {}
		virtual void Update(double timeDelta, DKFoundation::DKTimeTick tick) {}
		virtual bool GetTransform(const NodeId& key, DKTransformUnit& out) = 0;
	};

	class DKLIB_API DKAnimationController : public DKAnimatedTransform
	{
	public:
		DKAnimationController(void);
		virtual ~DKAnimationController(void);

		void Update(double timeDelta, DKFoundation::DKTimeTick tick);

		virtual bool GetTransform(const NodeId& key, DKTransformUnit& out) = 0;

		virtual bool IsPlaying(void) const = 0;
		virtual float Duration(void) const = 0;

		virtual void Play(void) = 0;
		virtual void Stop(void) = 0;

		void SetFrame(float frame);	 // set animation frame (in time unit)
		void SetSpeed(float speed);  // animation speed
		float Frame(void) const				{return frame;}
		float Speed(void) const				{return speed;}

	protected:
		virtual void UpdateFrame(float frame) = 0;

		float			frame;
		float			speed;

		DKFoundation::DKTimeTick lastUpdatedTick;
	};
}
