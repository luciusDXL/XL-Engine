#pragma once
#include "../types.h"
#include <string.h>

#ifdef _WIN32
    #define XL_THREADRET u32
#else
    #define XL_THREADRET void*
#endif

typedef XL_THREADRET (XL_STDCALL *ThreadFunc)(void*);

class Thread
{
public:
	virtual ~Thread() {};

	virtual bool run()    = 0;
	virtual void pause()  = 0;
	virtual void resume() = 0;

protected:
	Thread(const char* name, ThreadFunc func, void* userData)
	{
		m_func = func;
		m_userData = userData;
		strcpy(m_name, name);
	}

public:
	//static factory function - creates the correct platform dependent version
	static Thread* create(const char* name, ThreadFunc func, void* userData);

protected:
	ThreadFunc m_func;
	void* m_userData;
	char  m_name[256];
};
