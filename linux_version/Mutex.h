
#ifndef _MUTEX_H
#define _MUTEX_H

#include "Platform.h"
#include <unistd.h>
#include <sys/syscall.h>

#define uint32 unsigned int

class  Mutex
{
public:
	friend class Condition;
	Mutex():holder(0){
		pthread_mutex_init(&mutex, NULL);
	}
	~Mutex() { 
		pthread_mutex_destroy(&mutex); 
	}
	
	pid_t gettid(){
		return static_cast<pid_t>(::syscall(SYS_gettid));
	}

	 void lock(){
		pthread_mutex_lock(&mutex);
		holder=gettid();
	}
	
	void unlock(){
		pthread_mutex_unlock(&mutex);
		holder=0;
	}
	bool AttemptLock()
	{
		return (pthread_mutex_trylock(&mutex) == 0);
	}
protected:
	
	pthread_mutex_t mutex;
    pid_t holder;
};

#endif


