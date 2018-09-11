//
//  File: DKAudioListener.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKAudioDevice.h"
#include "DKVector3.h"
#include "DKQuaternion.h"
#include "DKMatrix3.h"
#include "DKMatrix4.h"

namespace DKFramework
{
	/// @brief Controls audio listener environment.
	///
	/// To get shared instance, call DKAudioListener::SharedInstance().
	/// You can create one or more instances, but they share same environment.
	class DKGL_API DKAudioListener : public DKSharedInstance<DKAudioListener>
	{
	public:
		~DKAudioListener();

		void SetGain(float f);
		float Gain() const;
		
		void SetPosition(const DKVector3& v);
		const DKVector3& Position() const;
		
		void SetOrientation(const DKVector3& forward, const DKVector3& up);
		void SetOrientation(const DKMatrix3& m);
		
		const DKVector3& Forward() const;
		const DKVector3& Up() const;
		
		void SetVelocity(const DKVector3& v);
		const DKVector3& Velocity() const;
				
	private:
		DKVector3 position;
		DKVector3 velocity;
		DKVector3 forward;
		DKVector3 up;
		float gain;
		
		friend class DKObject<DKAudioListener>;
		friend class DKSharedInstance<DKAudioListener>;		
		DKAudioListener();
		DKAudioListener(const DKAudioListener&) = delete;
		DKAudioListener& operator = (const DKAudioListener&) = delete;		

		DKObject<DKAudioDevice> context;
	};
}
