//
//  File: DKLogger.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#include "DKLog.h"
#include "DKLogger.h"
#include "DKSpinLock.h"

namespace DKFoundation
{
	namespace Private
	{
		static DKSpinLock& LoggerLock(void)
		{
			static DKSpinLock lock;
			return lock;
		}
		static DKArray<DKObject<DKLogger>>& LoggerArray(void)
		{
			static DKArray<DKObject<DKLogger>> loggers;
			return loggers;
		}
	}
}

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKLogger::DKLogger(void)
{
}

DKLogger::~DKLogger(void)
{
}

void DKLogger::Bind(void)
{
	DKCriticalSection<DKSpinLock> guard(LoggerLock());
	DKArray<DKObject<DKLogger>>& loggers = LoggerArray();
	for (size_t i = 0; i < loggers.Count(); ++i)
	{
		DKLogger* logger = loggers.Value(i);
		if (logger == this)
		{
			return;
		}
	}
	loggers.Add(this);
	this->OnBind();
}

void DKLogger::Unbind(void)
{
	DKCriticalSection<DKSpinLock> guard(LoggerLock());
	DKArray<DKObject<DKLogger>>& loggers = LoggerArray();
	for (size_t i = 0; i < loggers.Count(); ++i)
	{
		DKLogger* logger = loggers.Value(i);
		if (logger == this)
		{
			this->OnUnbind();
			loggers.Remove(i);
			return;
		}
	}
}

bool DKLogger::IsBound(void) const
{
	DKCriticalSection<DKSpinLock> guard(LoggerLock());
	DKArray<DKObject<DKLogger>>& loggers = LoggerArray();
	for (size_t i = 0; i < loggers.Count(); ++i)
	{
		DKLogger* logger = loggers.Value(i);
		if (logger == this)
		{
			return true;
		}
	}
	return false;
}

size_t DKLogger::Broadcast(Category c, const DKString& str)
{
	size_t num = 0;

	DKCriticalSection<DKSpinLock> guard(LoggerLock());
	DKArray<DKObject<DKLogger>>& loggers = LoggerArray();
	for (size_t i = 0; i < loggers.Count(); ++i)
	{
		DKLogger* logger = loggers.Value(i);
		logger->Log(c, str);
		num++;
	}
	return num;
}

DKObject<DKLogger> DKLogger::CreateSimpleLogger(void (*fn)(Category, const DKString&))
{
	struct Logger : public DKLogger
	{
		void (*fn)(Category, const DKString&);
		void Log(Category c, const DKString& str) override
		{
			fn(c, str);
		}
	};
	DKObject<Logger> logger = DKOBJECT_NEW Logger();
	logger->fn = fn;
	return logger.SafeCast<DKLogger>();
}
