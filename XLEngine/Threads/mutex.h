#pragma once
#include "../types.h"

class Mutex
{
public:
	virtual ~Mutex() {};

	virtual s32 lock() const = 0;
	virtual s32 unlock() const = 0;

protected:
	Mutex() {};

public:
	//static factory function - creates the correct platform dependent version
	static Mutex* create();
};
