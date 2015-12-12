#pragma once
#include "types.h"

#define XL_LOGGING_ENABLED   1
#define XL_DEBUG_OUT_ENABLED 1

enum LogMessageType
{
	LOG_ERROR = 0,
	LOG_WARNING,
	LOG_MESSAGE,
	LOG_COUNT
};

#if XL_LOGGING_ENABLED || XL_DEBUG_OUT_ENABLED
	#define LOG Log::write
#else
	#define LOG
#endif

namespace Log
{
	bool open(const char* filename);
	void close();

	void write(LogMessageType type, const char* msg, ...);
};