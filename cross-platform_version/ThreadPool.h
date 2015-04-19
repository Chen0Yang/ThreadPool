
#include "Guard.h"
#include "Condition.h"
#include "ThreadBase.h"
#include <assert.h>
#include <iostream>
#include <set>

#include "Platform.h"

#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#ifdef WIN32

class  ThreadController
{
public:
	HANDLE hThread;
	uint32 thread_id;

	void Setup(HANDLE h)
	{
		hThread = h;
	}
	void Suspend()
	{
		// We can't be suspended by someone else. That is a big-no-no and will lead to crashes.
		assert(GetCurrentThreadId() == thread_id);
		SuspendThread(hThread);
	}
	void Resume()
	{
		// This SHOULD be called by someone else.
		assert(GetCurrentThreadId() != thread_id);
		if(!ResumeThread(hThread))
		{
			DWORD le = GetLastError();
			printf("lasterror: %u\n", le);
		}
	}
	void Join()
	{
		WaitForSingleObject(hThread, INFINITE);
	}
	uint32 GetId() { return thread_id; }	
	void Delet(){
		CloseHandle(hThread);
	}
};

#else
int GenerateThreadId();
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
		pthread_mutex_init(&mutex,NULL);
		pthread_cond_init(&cond,NULL);
		thread_id = GenerateThreadId();
	}
	~ThreadController()
	{
	}
	void Suspend()
	{
		pthread_cond_wait(&cond, &mutex);
	}
	void Resume()
	{
		pthread_cond_signal(&cond);
	}
	void Join()
	{
		pthread_join(handle,NULL);
	}
	void Delet(){
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);
	}
	inline uint32 GetId() { return (uint32)thread_id; }
};

#endif

struct  Thread
{
	ThreadBase * ExecutionTarget;
	ThreadController ControlInterface;
	Mutex SetupMutex;
};

class  CThreadPool
{
public:
	typedef std::set<Thread*> ThreadSet;
	
	CThreadPool(int numThreads=10):_numThreads(numThreads),_mutex(){
	}

	~CThreadPool(){
		if(m_freeThreads.size()){
		ThreadSet::iterator itr = m_freeThreads.begin();
		while(itr!=m_freeThreads.end())
		{
			(*itr)->ControlInterface.Delet();
			m_freeThreads.erase(itr);
			itr=m_freeThreads.begin();
		}
	}
	if(m_activeThreads.size()){
		ThreadSet::iterator itr = m_activeThreads.begin();
		while(itr!=m_activeThreads.end())
		{
			(*itr)->ControlInterface.Delet();
			m_activeThreads.erase(itr);
			itr=m_activeThreads.begin();
		}
	}

	}

	void Startup();

	void ExecuteTask(ThreadBase * ExecutionTarget);

	void Shutdown();

private:

	Thread * StartThread(ThreadBase * ExecutionTarget);
	void KillFreeThreads(uint32 count);
	
	int _numThreads;
	Mutex _mutex;

    ThreadSet m_activeThreads;
	ThreadSet m_freeThreads;

};

#endif
