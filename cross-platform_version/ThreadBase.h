
#ifndef _THREADING_BASE_H
#define _THREADING_BASE_H

#include "Platform.h"

class  ThreadBase
{
public:
	ThreadBase() {}
	virtual ~ThreadBase() {}
	virtual bool run() = 0;
#ifdef WIN32
	HANDLE THREAD_HANDLE;
#else
	pthread_t THREAD_HANDLE;
#endif
};

#endif

