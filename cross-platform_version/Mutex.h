
#ifndef _MUTEX_H
#define _MUTEX_H

#include "Platform.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/syscall.h>
#endif

#define uint32 unsigned int
class  Mutex
{
public:
	friend class Condition;

	#ifdef WIN32

/* Windows Critical Section Implementation */
	Mutex() { 
		InitializeCriticalSection(&cs); 
	}
	~Mutex() { 
		DeleteCriticalSection(&cs); 
	}

	#else
	Mutex():holder(0){
		pthread_mutex_init(&mutex, NULL);
	}
	~Mutex() { 
		pthread_mutex_destroy(&mutex); 
	}
	
	pid_t gettid(){
		return static_cast<pid_t>(::syscall(SYS_gettid));
	}

#endif

	 void lock(){
#ifndef WIN32
		pthread_mutex_lock(&mutex);
		holder=gettid();
#else
		EnterCriticalSection(&cs);
#endif
	}
	
	void unlock(){
#ifndef WIN32
		pthread_mutex_unlock(&mutex);
		holder=0;
#else
		LeaveCriticalSection(&cs);
#endif
	}
	bool AttemptLock()
	{
#ifndef WIN32
		return (pthread_mutex_trylock(&mutex) == 0);
#else
		return (TryEnterCriticalSection(&cs) == TRUE ? true : false);
#endif
	}
protected:
#ifdef WIN32
	
	CRITICAL_SECTION cs;
#else
	
	pthread_mutex_t mutex;
    	pid_t holder;
#endif
};

#endif


