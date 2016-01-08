#include "../osUtil.h"
#include <windows.h>
#include <stdio.h>
#include <assert.h>

namespace OS
{
	void sleep(u32 sleepDeltaMS)
	{
		Sleep(sleepDeltaMS);
	}
}