#include "../osUtil.h"
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

namespace OS
{
	void sleep(u32 sleepDeltaMS)
	{
		u32 sleepDeltaUS = sleepDeltaMS ? sleepDeltaMS * 1000 : 1;
		usleep( sleepDeltaUS );
	}
}
