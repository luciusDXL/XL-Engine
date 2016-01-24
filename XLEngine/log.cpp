#include "log.h"
#include "filestream.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN 1
	#include <Windows.h>
	#define debugMessage OutputDebugStringA
#else
	#define debugMessage printf
#endif

namespace Log
{
	const char* c_logMsgType[]=
	{
		"[Error]",
		"[Warning]",
		"",
	};

	static FileStream s_logFile;
	static char s_tmpBuffer[4096];
	static char s_printString[4096];

	bool open(const char* filename)
	{
	#if XL_LOGGING_ENABLED
		return s_logFile.open(filename, FileStream::MODE_WRITE);
	#else
		return true;
	#endif
	}

	void close()
	{
	#if XL_LOGGING_ENABLED
		s_logFile.close();
	#endif
	}

	void write(LogMessageType type, const char* msg, ...)
	{
	#if XL_LOGGING_ENABLED || XL_DEBUG_OUT_ENABLED
		assert(type < LOG_COUNT);

		//first build up the string.
		va_list arg;
		va_start(arg, msg);
		vsprintf(s_tmpBuffer, msg, arg);
		va_end(arg);
	#endif

	#if XL_LOGGING_ENABLED
		sprintf(s_printString, "%s%s\r\n", c_logMsgType[type], s_tmpBuffer);
		const size_t len = strlen(s_printString);

		s_logFile.write(s_printString, (u32)len);
		s_logFile.flush();	//flush to disk in case of a crash.
	#endif

	#if XL_DEBUG_OUT_ENABLED
		sprintf(s_printString, "%s%s\n", c_logMsgType[type], s_tmpBuffer);
		debugMessage(s_printString);
	#endif
	}
};
