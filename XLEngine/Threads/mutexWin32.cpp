#include "mutex.h"

void Mutex::operator=(Mutex &M) {}

Mutex::Mutex( const Mutex &M ) {}

Mutex::Mutex()
{ InitializeCriticalSection(&C); }

Mutex::~Mutex()
{ DeleteCriticalSection(&C); }

int Mutex::Lock() const
{ EnterCriticalSection(&C); return 0; }

int Mutex::Unlock() const
{ LeaveCriticalSection(&C); return 0; }
