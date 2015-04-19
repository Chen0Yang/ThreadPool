

#ifndef _CONDITION_H
#define _CONDITION_H

#include "Platform.h"
#include <assert.h>

static int threadid_count = 0;

class ThreadController
{
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	int thread_id;
	pthread_t handle;
public:
	void Setup(pthread_t h)
	{
		handle = h;
		assert((pthread_mutex_init(&mutex,NULL))==0);
		assert((pthread_cond_init(&cond,NULL))==0);
		thread_id = ++threadid_count;
	}
	~ThreadController()
	{
		assert((pthread_cond_destroy(&cond))==0);
		assert((pthread_mutex_destroy(&mutex))==0);
	}
	void Suspend()
	{
		assert((pthread_cond_wait(&cond, &mutex))==0);
	}
	void Resume()
	{
		assert((pthread_cond_signal(&cond))==0);
	}
	void Join()
	{
		assert((pthread_join(handle,NULL))==0);
	}
	inline uint32 GetId() { return (uint32)thread_id; }
};

#endif

	
