#pragma once
#include "types.h"

namespace iniReader
{
	typedef bool (*ReadIniCallback)(const char* key, const char* value);

	// liberal will allow for blank values.
	bool readIni(const char* filename, ReadIniCallback callback, bool liberal = false);

	bool readCmdLine( const char *cmdline, ReadIniCallback callback );
	
	////////////////////
	//helper functions//
	////////////////////

	//remove preceding and trailing spaces.
	//removes outter quotes - [  "This is a "string" in quotes"    ] becomes [This is a "string" in quotes]
	void fixupString(char* value);

	//converts all slashes to forward slashes and optionally adds a slash as the last character if one isn't already there
	//or removes the last character if it is a slash.
	void fixupSlashes(char* value, bool addSlashAtEnd, bool removeSlashFromEnd);
};