#pragma once
#ifdef _WIN32
#include <Windows.h>
#endif

class Mutex
{
public:

	Mutex();
	virtual ~Mutex();

	void operator=(Mutex &M);
	Mutex( const Mutex &M );

	int Lock() const;
	int Unlock() const;

private:
	mutable CRITICAL_SECTION C;
};
