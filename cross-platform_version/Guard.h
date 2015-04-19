
#ifndef _GUARD_H
#define _GUARD_H

#include "Mutex.h"
#include <iostream>

class  Guard
{
public:
	Guard(Mutex& mutex) : _mutex(mutex)
	{
		_mutex.lock();
	}
	~Guard()
	{
		_mutex.unlock();
	}
private:
	Mutex& _mutex;
};

#endif

