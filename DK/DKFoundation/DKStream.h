//
//  File: DKStream.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	/// a simple stream interface.
	class DKStream
	{
	public:
		using Position = uint64_t;
		enum : Position { PositionError = ~Position(0) };

		DKStream() {}
		virtual ~DKStream() {}

		virtual Position SetCurrentPosition(Position p) = 0;
		virtual Position CurrentPosition() const = 0;
		virtual Position RemainLength() const = 0;
		virtual Position TotalLength() const = 0;

		virtual size_t Read(void* p, size_t s) = 0;
		virtual size_t Write(const void* p, size_t s) = 0;
		
		virtual bool IsReadable() const = 0;
		virtual bool IsWritable() const = 0;
		virtual bool IsSeekable() const = 0;
	};
}
