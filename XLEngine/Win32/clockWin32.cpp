#include "../clock.h"
#include <windows.h>
#include <stdio.h>
#include <assert.h>

#define SEC_TO_uS 1000000.0

f32 Clock::s_deltaTime;
f32 Clock::s_realDeltaTime;
s32 Clock::s_deltaTicks;

static LARGE_INTEGER s_timerFreq;
static u64 s_startTick[16] = {0};

u64 getCurTickCnt();

bool Clock::init()
{
	BOOL ret = QueryPerformanceFrequency(&s_timerFreq);
	return (ret) ? true : false;
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

	f32 fTimeDelta = (f32)( (f64)(End - s_startTick[timerID]) / (f64)(s_timerFreq.QuadPart) );
	if ( fTimeDelta > fMax ) 
	{ 
		fTimeDelta = fMax; 
	}

	return fTimeDelta;
}

u64 Clock::getDeltaTime_uS(int timerID/*=0*/)
{
	u64 End = getCurTickCnt();
	f64 quadPart_uS = (f64)(s_timerFreq.QuadPart) / SEC_TO_uS;
	return (u64)( (f64)(End - s_startTick[timerID]) / quadPart_uS );
}

u64 Clock::getTime_uS()
{
	u64 value = getCurTickCnt();
	f64 quadPart_uS = (f64)(s_timerFreq.QuadPart) / SEC_TO_uS;
	return (u64)( (f64)value / quadPart_uS );
}

u64 getCurTickCnt()
{
	LARGE_INTEGER lcurtick;
	QueryPerformanceCounter(&lcurtick);

	return lcurtick.QuadPart;
}
