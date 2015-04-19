
#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include "Guard.h"
#include "Condition.h"
#include "ThreadBase.h"
#include <iostream>
#include <set>

struct  Thread
{
	ThreadBase * ExecutionTask;
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
		assert(_numThreads==(m_freeThreads.size()+m_activeThreads.size()));
		if(m_freeThreads.size()){
		ThreadSet::iterator itr = m_freeThreads.begin();
		while(itr!=m_freeThreads.end())
		{
			m_freeThreads.erase(itr);
			itr=m_freeThreads.begin();
		}
	}
	if(m_activeThreads.size()){
		ThreadSet::iterator itr = m_activeThreads.begin();
		while(itr!=m_activeThreads.end())
		{
			m_activeThreads.erase(itr);
			itr=m_activeThreads.begin();
		}
	}
	}

	void Startup();
	void RunTask(ThreadBase * ExecutionTask);
	void Stop();

private:

	Thread * StartThread(ThreadBase * ExecutionTask);
	void KillFreeThreads(uint32 count);
	
	int _numThreads;
	Mutex _mutex;

    ThreadSet m_activeThreads;
	ThreadSet m_freeThreads;
};

#endif
