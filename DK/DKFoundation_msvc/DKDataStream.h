//
//  File: DKDataStream.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"
#include "DKStream.h"
#include "DKData.h"
#include "DKObject.h"

////////////////////////////////////////////////////////////////////////////////
// DKDataStream
// using DKData as a stream (DKStream)
// provide stream interface.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKDataStream : public DKStream
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

		virtual DKData* DataSource(void);
		virtual const DKData* DataSource(void) const;
	private:
		size_t offset;
		DKObject<DKData> data;
		DKDataStream(const DKDataStream& ds);
		DKDataStream& operator = (const DKDataStream& ds);
	};
}
