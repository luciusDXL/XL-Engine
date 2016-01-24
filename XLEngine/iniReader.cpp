#include "iniReader.h"
#include "filestream.h"

#include <string.h>

namespace iniReader
{
	bool getKey(char** fileData, char* keyName, const char* fileEnd);
	bool readValue(char** fileData, char* value, const char* fileEnd,bool liberal);

	////////////////////////////////////
	// API
	////////////////////////////////////

	bool readIni(const char* filename, ReadIniCallback callback, bool liberal)
	{
		if (!callback)
		{
			return false;
		}

		bool res = false;
		FileStream file;
		if ( file.open(filename, FileStream::MODE_READ) )
		{
			size_t size = file.getSize();
			char* fileData = new char[size+1];
			if (fileData)
			{
				file.read(fileData, (u32)size);
				res = true;
			}
			file.close();

			if (res)
			{
				const char* fileEnd = &fileData[size];
				char* readPos = fileData;

				//now read the data.
				while (1)
				{
					char keyName[512], keyValue[512];

					//we are done here.
					if (!getKey(&readPos, keyName, fileEnd))
					{
						break;
					}

					if (!readValue(&readPos, keyValue, fileEnd, liberal))
					{
						break;
					}

					//remove preceding or trailing spaces, remove exterior quotation marks.
					fixupString(keyName);
					fixupString(keyValue);

					res &= callback(keyName, keyValue);
				};
			}

			delete [] fileData;
		}

		return res;
	}

	void fixupSlashes(char* value, bool addSlashAtEnd, bool removeSlashFromEnd)
	{
		size_t len = strlen(value);
		for (size_t c=0; c<len; c++)
		{
			if (value[c] == '\\')
			{
				value[c] = '/';
			}
		}

		if (addSlashAtEnd)
		{
			size_t c = len - 1;
			if (value[c] != '/')
			{
				value[c+1] = '/';
				value[c+2] = 0;
			}
		}
		else if (removeSlashFromEnd)
		{
			size_t c = len - 1;
			if (value[c] == '/')
			{
				value[c] = 0;
			}
		}
	}

	void fixupString(char* value)
	{
		char copy[512];
		strcpy(copy, value);

		//remove preceding spaces.
		size_t len = strlen(value);
		s32 firstNonSpaceChar = -1;
		s32 lastNonSpaceChar = -1;
		for (size_t c=0; c<len; c++)
		{
			if ( value[c] > ' ' && firstNonSpaceChar < 0 )
			{
				firstNonSpaceChar = (s32)c;
				lastNonSpaceChar  = (s32)c;
			}
			else if (value[c] > ' ')
			{
				lastNonSpaceChar = (s32)c;
			}
		}

		if (firstNonSpaceChar < 0 || lastNonSpaceChar < 0 || firstNonSpaceChar == lastNonSpaceChar)
		{
			return;
		}

		if (value[ firstNonSpaceChar ] == '"')
		{
			firstNonSpaceChar++;
		}
		if (value[ lastNonSpaceChar ] == '"')
		{
			lastNonSpaceChar--;
		}
		if (lastNonSpaceChar <= firstNonSpaceChar)
		{
			return;
		}

		s32 c=firstNonSpaceChar;
		for (; c<=lastNonSpaceChar; c++)
		{
			value[c-firstNonSpaceChar] = copy[c];
		}
		value[c-firstNonSpaceChar] = 0;
	}

	bool readCmdLine(const char *cmdline, ReadIniCallback callback)
	{
		static char* empty = "";
		char* buf = new char[strlen(cmdline)+1];
		strcpy(buf,cmdline);

		char* ps = buf;
		while(*ps && *ps != ';')
		{
			ps++;
		}
		*ps = 0;
		ps = buf;

		bool res = true;
		while (*ps && res)
		{
			char* parg = NULL;
			while (*ps && (*ps == ' ' || *ps == '\t'))
			{
				ps++;
			}

			if (*ps == '"')
			{
				parg = ++ps;
				while(*ps && *ps != '"')
				{
					ps++;
				}
				if (*ps)
				{
					*ps++ = '\0';
				}
			}
			else if (*ps)
			{
				parg = ps;
				while(*ps && *ps != ' ' && *ps != '\t')
				{
					ps++;
				}
				if (*ps)
				{
					*ps++ = '\0';
				}
			}

			if (parg)
			{
				char* value = parg;
				for (; *value && *value != '='; value++) {;}

				if (*value == '=')
				{
					*value++ = '\0';
					fixupString(parg);
					fixupString(value);

				}
				else
				{
					fixupString(parg);
					value = empty;
				}
				res &= callback(parg, value);
			}
		}

		delete buf;
		return res;
	}


	////////////////////////////////////
	// Internal Functions.
	////////////////////////////////////

	bool getKey(char** fileData, char* keyName, const char* fileEnd)
	{
		u32 keyIndex = 0;
		keyName[0] = 0;

		bool inComment = false;
		char* curPos = *fileData;
		while (curPos < fileEnd)
		{
			if (*curPos == '#')
			{
				inComment = true;
			}
			else if (inComment)
			{
				//we stay in the comment until the end of the line.
				if (*curPos == '\r' || *curPos == '\n')
				{
					inComment = false;
				}
			}
			else if (*curPos != '=' && *curPos >= 32)
			{
				if (keyIndex != 0 || *curPos != ' ')
				{
					keyName[ keyIndex++ ] = *curPos;
				}
			}
			else if (*curPos == '=')
			{
				break;
			}

			curPos++;
		};

		keyName[keyIndex] = 0;
		*fileData = (curPos<fileEnd) ? curPos+1 : curPos;

		return (keyName[0] != 0);
	}

	bool readValue(char** fileData, char* value, const char* fileEnd, bool liberal)
	{
		u32 valueIndex = 0;
		value[0] = 0;

		bool inComment = false;
		char* curPos = *fileData;
		while (curPos < fileEnd)
		{
			if (*curPos == '#')
			{
				inComment = true;
			}
			else if (*curPos == '\r' || *curPos == '\n')
			{
				inComment = false;
				break;
			}
			else if (*curPos >= 32 && !inComment)
			{
				if (valueIndex != 0 || *curPos != ' ')
				{
					value[ valueIndex++ ] = *curPos;
				}
			}

			curPos++;
		};

		value[valueIndex] = 0;
		*fileData = (curPos<fileEnd) ? curPos+1 : curPos;

		return (liberal || value[0] != 0);
	}
}
