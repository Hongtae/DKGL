//
//  File: DKAnimationController.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKTransform.h"

namespace DKFramework
{
	/// @brief Animated-Transform interface.
	///
	/// Encapsulate any kind of animation that can produce DKTransformUnit
	class DKGL_API DKAnimatedTransform
	{
	public:
		typedef DKString NodeId;
		virtual ~DKAnimatedTransform() {}
		virtual void Update(double timeDelta, DKTimeTick tick) {}
		virtual bool GetTransform(const NodeId& key, DKTransformUnit& out) = 0;
	};

	/// @brief Animation control interface.
	///
	/// Sublcass to customize your action behaviors how object being animated.
	/// Used for DKModel node animation.
	class DKGL_API DKAnimationController : public DKAnimatedTransform
	{
	public:
		DKAnimationController();
		virtual ~DKAnimationController();

		void Update(double timeDelta, DKTimeTick tick);

		virtual bool GetTransform(const NodeId& key, DKTransformUnit& out) = 0;

		virtual bool IsPlaying() const = 0;
		virtual float Duration() const = 0;

		virtual void Play() = 0;
		virtual void Stop() = 0;

		void SetFrame(float frame);	 ///< set animation frame (in time unit)
		void SetSpeed(float speed);  ///< animation speed
		float Frame() const				{return frame;}
		float Speed() const				{return speed;}

	protected:
		virtual void UpdateFrame(float frame) = 0;

		float			frame;
		float			speed;

		DKTimeTick lastUpdatedTick;
	};
}
