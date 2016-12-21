//
//  File: DKStream.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"

namespace DKFoundation
{
	/// a simple stream interface.
	class DKStream
	{
	public:
		using Position = uint64_t;
		enum : Position { PositionError = (Position)-1 };

		DKStream(void) {}
		virtual ~DKStream(void) {}

		virtual Position SetPos(Position p) = 0;
		virtual Position GetPos(void) const = 0;
		virtual Position RemainLength(void) const = 0;
		virtual Position TotalLength(void) const = 0;

		virtual size_t Read(void* p, size_t s) = 0;
		virtual size_t Write(const void* p, size_t s) = 0;
		
		virtual bool IsReadable(void) const = 0;
		virtual bool IsWritable(void) const = 0;
		virtual bool IsSeekable(void) const = 0;
	};
}
