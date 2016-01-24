#include "../clock.h"
#include "../log.h"
#include <time.h>
#include <stdio.h>
#include <assert.h>

const u64 c_secToNs = 1000000000ULL;
const f64 c_nsToSec (1.0 / f64(c_secToNs));

f32 Clock::s_deltaTime;
f32 Clock::s_realDeltaTime;
s32 Clock::s_deltaTicks;

static timespec s_timerFreq;
static u64 s_startTick[16] = {0};

u64 getCurTickCnt();

bool Clock::init()
{
	return (clock_getres(CLOCK_MONOTONIC, &s_timerFreq) == 0);
}

void Clock::destroy()
{
	//nothing to do currently.
}

void Clock::startTimer(int timerID/*=0*/)
{
	assert( timerID < 16 );
	s_startTick[timerID] = getCurTickCnt();
}

float Clock::getDeltaTime(float fMax, int timerID/*=0*/)
{
	assert( timerID < 16 );
	u64 End = getCurTickCnt();

	f32 fTimeDelta = (f32)( (f64)(End - s_startTick[timerID]) * c_nsToSec );
	if ( fTimeDelta > fMax )
	{
		fTimeDelta = fMax;
	}

	return fTimeDelta;
}

u64 Clock::getDeltaTime_uS(int timerID/*=0*/)
{
	u64 End = getCurTickCnt();
	return (u64)( (f64)(End - s_startTick[timerID]) * c_nsToSec );
}

f64 Clock::getDeltaTime_f64(int timerID/*=0*/)
{
	u64 End = getCurTickCnt();
	return f64(End - s_startTick[timerID]) * c_nsToSec;
}

u64 Clock::getTime_uS()
{
	u64 value = getCurTickCnt();
	return (u64)( (f64)value * 1000.0 );
}

f64 Clock::getTime_uS_flt()
{
	u64 value = getCurTickCnt();
	return (f64)value * 1000.0;
}

u64 getCurTickCnt()
{
    timespec timingRes;
	s32 res = clock_gettime(CLOCK_MONOTONIC, &timingRes);
	if (res != 0)
	{
        LOG( LOG_ERROR, "clock_gettime() failed, result = %d.", res );
        return 0;
	}

	return u64(timingRes.tv_sec)*c_secToNs + u64(timingRes.tv_nsec);
}
