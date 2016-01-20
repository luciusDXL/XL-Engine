#pragma once
#include <Windows.h>
#include "../mutex.h"

class MutexWin32 : public Mutex
{
public:
	MutexWin32();
	virtual ~MutexWin32();

	virtual s32 lock() const;
	virtual s32 unlock() const;

private:
	mutable CRITICAL_SECTION C;
};
