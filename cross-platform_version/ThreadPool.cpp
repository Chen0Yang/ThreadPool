
#include "Platform.h"
#include "ThreadPool.h"

#ifdef WIN32
	#include <process.h>
#else	
	volatile int threadid_count = 0;
	int GenerateThreadId()
	{
		int i = ++threadid_count;
		return i;
	}
#endif

void CThreadPool::ExecuteTask(ThreadBase * ExecutionTarget)
{
	Thread * t;
	// grab one from the pool, if we have any.
	if(_numThreads==0){
		ExecutionTarget->run();
	}
	if(m_freeThreads.size())
	{
		t = *m_freeThreads.begin();
		m_freeThreads.erase(m_freeThreads.begin());

		// execute the task on this thread.
		t->ExecutionTarget = ExecutionTarget;

		// resume the thread, and it should start working.
		t->ControlInterface.Resume();
	}
	else
	{
		// creating a new thread means it heads straight to its task.
		// no need to resume it :)
		t = StartThread(ExecutionTarget);
		++_numThreads;
	}

	m_activeThreads.insert(t);
}

void CThreadPool::Startup()
{
	for(int i=0; i < _numThreads; ++i)
		m_freeThreads.insert(StartThread(NULL));
}

void CThreadPool::KillFreeThreads(uint32 count)
{
	Thread * t;
	ThreadSet::iterator itr;
	uint32 i;
	Guard lock(_mutex);
	for(i = 0, itr = m_freeThreads.begin(); i < count && itr != m_freeThreads.end(); ++i, ++itr)
	{
		t = *itr;
		t->ExecutionTarget = NULL; 
		t->ControlInterface.Resume();
	}
}

void CThreadPool::Shutdown()
{
	size_t tcount = m_activeThreads.size() + m_freeThreads.size();		// exit all
	KillFreeThreads((uint32)m_freeThreads.size());

	Guard lock(_mutex);
	
	ThreadSet::iterator itr; 
	while((itr= m_activeThreads.begin())!=m_activeThreads.end())
	{
		if((*itr)->ExecutionTarget){
			(*itr)->ControlInterface.Join();
		}		
		m_freeThreads.insert(*itr);
		m_activeThreads.erase(m_activeThreads.begin());
	}
}

#ifdef WIN32

static unsigned long WINAPI thread_proc(void* param)
{
	Thread * t = (Thread*)param;
	uint32 tid = t->ControlInterface.GetId();
	bool ht = (t->ExecutionTarget != NULL);

	Guard lock(t->SetupMutex);

	for(;;)
	{
		if(t->ExecutionTarget != NULL)
		{
			if(t->ExecutionTarget->run())
				delete t->ExecutionTarget;

			t->ExecutionTarget = NULL;
			break;
		}
		else
		{
			if(ht)
			t->ControlInterface.Suspend();
		}
	}

	ExitThread(0);
}

Thread * CThreadPool::StartThread(ThreadBase * ExecutionTarget)
{
	HANDLE h;
	Thread * t = new Thread;
	
	t->ExecutionTarget = ExecutionTarget;
	Guard lock(t->SetupMutex);
	h = CreateThread(NULL, 0, &thread_proc, (LPVOID)t, CREATE_SUSPENDED, (LPDWORD)&t->ControlInterface.thread_id);
	t->ControlInterface.Setup(h);

	return t;
}

#else

static void * thread_proc(void * param)
{
	Thread * t = (Thread*)param;

	Guard lock(t->SetupMutex);
	for(;;)
	{
		if(t->ExecutionTarget != NULL)
		{
			if(t->ExecutionTarget->run())
				delete t->ExecutionTarget;

			t->ExecutionTarget = NULL;
			break;
		}
		else
		{
			// enter "suspended" state. when we return, the threadpool will either tell us to fuk off, or to execute a new task.
			t->ControlInterface.Suspend();
			// after resuming, this is where we will end up. start the loop again, check for tasks, then go back to the threadpool.
		}
	}
//	pthread_exit(0);
}

Thread * CThreadPool::StartThread(ThreadBase * ExecutionTarget)
{
	pthread_t target;
	Thread * t = new Thread;
	t->ExecutionTarget = ExecutionTarget;

	// lock the main mutex, to make sure id generation doesn't get messed up
	Guard lock(_mutex);
	pthread_create(&target, NULL, &thread_proc, (void*)t);
	t->ControlInterface.Setup(target);
	return t;
}

#endif
