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

		Position SetPos(Position p);
		Position GetPos(void) const;
		Position RemainLength(void) const;
		Position TotalLength(void) const;

		size_t Read(void* p, size_t s);
		size_t Write(const void* p, size_t s);

		bool IsReadable(void) const {return true;}
		bool IsSeekable(void) const {return true;}
		bool IsWritable(void) const {return false;}

		virtual DKData* Data(void);
		virtual const DKData* Data(void) const;
	private:
		size_t offset;
		DKObject<DKData> data;
		DKDataStream(const DKDataStream& ds) = delete;
		DKDataStream& operator = (const DKDataStream& ds) = delete;
	};
}
