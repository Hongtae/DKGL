//
//  File: DKData.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2022 Hongtae Kim. All rights reserved.
//

#include "DKData.h"
#include "DKFile.h"
#include "DKFunction.h"

using namespace DKFoundation;


DKObject<DKData> DKData::StaticData(void* p, size_t len, bool readonly, DKOperation* cleanup)
{
	struct ReadOnlyData : public DKData
	{
		const void* p;
		size_t len;
		DKObject<DKOperation> cleanup;

		~ReadOnlyData()
		{
			if (cleanup)
				cleanup->Perform();
		}
		size_t Length() const override { return len; }
		bool IsReadable() const override { return true; }
		bool IsWritable() const override { return false; }
		bool IsExcutable() const override { return false; }
		bool IsTransient() const override { return len > 0 && cleanup == nullptr; }

		const void* Contents() const override { return p; }
	};
	struct WritableData : public DKData
	{
		void* p;
		size_t len;
		DKObject<DKOperation> cleanup;

		~WritableData()
		{
			if (cleanup)
				cleanup->Perform();
		}

		size_t Length() const override { return len; }
		bool IsReadable() const override { return true; }
		bool IsWritable() const override { return true; }
		bool IsExcutable() const override { return false; }
		bool IsTransient() const override { return len > 0 && cleanup == nullptr; }

		const void* Contents() const override { return p; }
		void* MutableContents() override { return p; }
	};

	if (p && len > 0)
	{
		DKObject<DKData> output = NULL;
		if (readonly)
		{
			DKObject<ReadOnlyData> data = DKObject<ReadOnlyData>::New();
			data->p = p;
			data->len = len;
			data->cleanup = cleanup;
			output = data.SafeCast<DKData>();
		}
		else
		{
			DKObject<WritableData> data = DKObject<WritableData>::New();
			data->p = p;
			data->len = len;
			data->cleanup = cleanup;
			output = data.SafeCast<DKData>();
		}
		return output;
	}
	else if (readonly)
	{
		DKObject<ReadOnlyData> data = DKObject<ReadOnlyData>::New();
		data->p = NULL;
		data->len = 0;
		data->cleanup = cleanup;
		return data.SafeCast<DKData>();
	}
	return NULL;
}

DKObject<DKData> DKData::StaticData(const void* p, size_t len, DKOperation* cleanup)
{
	return StaticData(const_cast<void*>(p), len, true, cleanup);
}

DKData::DKData()
{
}

DKData::~DKData()
{
}

bool DKData::WriteToFile(const DKString& file, bool overwrite) const
{
	if (this->Length() == 0)
		return false;

	DKFile::FileInfo info;
	if (DKFile::GetInfo(file, info) && !overwrite)
		return false;		// file is exists already.

	DKObject<DKFile> f = DKFile::Create(file, DKFile::ModeOpenNew, DKFile::ModeShareAll);
	if (f)
	{
		size_t written = f->Write(this);
		if (written < this->Length())
		{
			//f = NULL;
			//DKFile::Delete(file);
			return false;
		}
		return true;
	}
	return false;
}

bool DKData::WriteToStream(DKStream* stream) const
{
	if (stream && stream->IsWritable())
	{
		const void* p = this->Contents();
		size_t len = this->Length();
		if (len > 0)
			stream->Write(p, len);
		return true;
	}
	return false;
}

DKObject<DKData> DKData::ImmutableData() const
{
	if (IsReadable())
	{
		if (IsWritable() || IsTransient())
		{
			struct ReadOnlyData : public DKData
			{
				void* p;
				size_t len;
				ReadOnlyData() : p(NULL), len(0) {}
				~ReadOnlyData()
				{
					if (p)
						DKFree(p);
				}
				size_t Length() const override { return len; }
				bool IsReadable() const override { return true; }
				bool IsWritable() const override { return false; }
				bool IsExcutable() const override { return false; }
				bool IsTransient() const override { return false; }

				const void* Contents() const override { return p; }
			};
			DKObject<ReadOnlyData> target = DKOBJECT_NEW ReadOnlyData();
			target->len = this->Length();
			if (target->len > 0)
			{
				target->p = DKMalloc(target->len);
				memcpy(target->p, this->Contents(), target->len);
			}
			return target.SafeCast<DKData>();
		}
		else
			return (DKData*)this;
	}
	return nullptr;
}
