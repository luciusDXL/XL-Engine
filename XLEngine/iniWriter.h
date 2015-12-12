#pragma once

namespace iniWriter
{
	bool open(const char* filename);
	void close();

	void writeStrNoQuotes(const char* key, const char* value);
	void write(const char* key, const char* value);
	void write(const char* key,  s8 value);
	void write(const char* key,  u8 value);
	void write(const char* key, s16 value);
	void write(const char* key, u16 value);
	void write(const char* key, s32 value);
	void write(const char* key, u32 value);
	void write(const char* key, f32 value);
	void write(const char* key, f64 value);
	void write(const char* key, bool value);

	void comment(const char* comment);
	void newLine();
};