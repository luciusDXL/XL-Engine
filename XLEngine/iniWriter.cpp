#pragma once
#include "iniReader.h"
#include "filestream.h"

namespace iniWriter
{
	FileStream s_stream;
	char s_workBuffer[1024];

	bool open(const char* filename)
	{
		return s_stream.open(filename, FileStream::MODE_WRITE);
	}

	void close()
	{
		s_stream.close();
	}

	void comment(const char* comment)
	{
		sprintf(s_workBuffer, "#%s\r\n", comment);

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void newLine()
	{
		strcpy(s_workBuffer, "\r\n");

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void write(const char* key, const char* value)
	{
		sprintf(s_workBuffer, "%s=\"%s\"\r\n", key, value);

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void writeStrNoQuotes(const char* key, const char* value)
	{
		sprintf(s_workBuffer, "%s=%s\r\n", key, value);

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void write(const char* key,  s8 value)
	{
		sprintf(s_workBuffer, "%s=%d\r\n", key, value);

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void write(const char* key,  u8 value)
	{
		sprintf(s_workBuffer, "%s=%u\r\n", key, value);

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void write(const char* key, s16 value)
	{
		sprintf(s_workBuffer, "%s=%d\r\n", key, value);

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void write(const char* key, u16 value)
	{
		sprintf(s_workBuffer, "%s=%u\r\n", key, value);

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void write(const char* key, s32 value)
	{
		sprintf(s_workBuffer, "%s=%d\r\n", key, value);

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void write(const char* key, u32 value)
	{
		sprintf(s_workBuffer, "%s=%u\r\n", key, value);

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void write(const char* key, f32 value)
	{
		sprintf(s_workBuffer, "%s=%f\r\n", key, value);

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void write(const char* key, f64 value)
	{
		sprintf(s_workBuffer, "%s=%f\r\n", key, value);

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}

	void write(const char* key, bool value)
	{
		sprintf(s_workBuffer, "%s=%s\r\n", key, value ? "true" : "false");

		size_t len = strlen(s_workBuffer);
		s_stream.write(s_workBuffer, (u32)len);
	}
}
