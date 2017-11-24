//
//  File: DKDataStream.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKStream.h"
#include "DKData.h"
#include "DKObject.h"

namespace DKFoundation
{
	/// @brief Stream object for data (DKData)
	class DKGL_API DKDataStream : public DKStream
	{
	public:
		DKDataStream(void);
		DKDataStream(DKData*);
		DKDataStream(DKData&);
		~DKDataStream(void);

		Position SetCurrentPosition(Position p) override;
		Position CurrentPosition(void) const override;
		Position RemainLength(void) const override;
		Position TotalLength(void) const override;

		size_t Read(void* p, size_t s) override;
		size_t Write(const void* p, size_t s) override;

		bool IsReadable(void) const override { return true; }
		bool IsSeekable(void) const override { return true; }
		bool IsWritable(void) const override { return false; }

		virtual DKData* Data(void);
		virtual const DKData* Data(void) const;

	private:
		DKObject<DKData> data;
		size_t offset;
		DKDataStream(const DKDataStream& ds) = delete;
		DKDataStream& operator = (const DKDataStream& ds) = delete;
	};
}
