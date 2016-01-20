#pragma once
#include <Windows.h>
#include "../thread.h"

class ThreadWin32 : public Thread
{
public:
	ThreadWin32(const char* name, ThreadFunc func, void* userData);
	virtual ~ThreadWin32();

	virtual bool run();
	virtual void pause();
	virtual void resume();

protected:
	HANDLE m_win32Handle;
};
