
#include "ThreadPool.h"

void CThreadPool::Startup()
{
	for(int i=0; i < _numThreads; ++i)
		m_freeThreads.insert(StartThread(NULL));
}

void CThreadPool::RunTask(ThreadBase * ExecutionTask)
{
	Thread * t;
	if(_numThreads==0){
		ExecutionTask->run();
	}
	if(m_freeThreads.size())
	{
		t = *m_freeThreads.begin();
		m_freeThreads.erase(m_freeThreads.begin());

		t->ExecutionTask = ExecutionTask;
		t->ControlInterface.Resume();//make thread start working
	}
	else
	{
		t = StartThread(ExecutionTask);//no idle thread, then start a new thread. it will work without resume
		++_numThreads;
	}

	m_activeThreads.insert(t);
}

void CThreadPool::Stop()
{
	size_t tcount = m_activeThreads.size() + m_freeThreads.size();	
	KillFreeThreads((uint32)m_freeThreads.size());

	Guard lock(_mutex);
	
	ThreadSet::iterator itr; 
	while((itr= m_activeThreads.begin())!=m_activeThreads.end())
	{
		if((*itr)->ExecutionTask){
			(*itr)->ControlInterface.Join();//wait all active threads finishing working 
		}		
		m_freeThreads.insert(*itr);//insert the idle thread into m_freeThreads set
		m_activeThreads.erase(m_activeThreads.begin());
	}
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
		t->ExecutionTask = NULL; 
		t->ControlInterface.Resume();
	}
}

static void * thread_proc(void * param)
{
	Thread * t = (Thread*)param;

	Guard lock(t->SetupMutex);
	for(;;)
	{
		if(t->ExecutionTask != NULL)
		{
			if(t->ExecutionTask->run())
				delete t->ExecutionTask;

			t->ExecutionTask = NULL;
			break;
		}
		else
		{
			t->ControlInterface.Suspend();//no task to run, then suspend
		}
	}
//	pthread_exit(0);
}

Thread * CThreadPool::StartThread(ThreadBase * ExecutionTask)
{
	pthread_t target;
	Thread * t = new Thread;
	t->ExecutionTask = ExecutionTask;

	Guard lock(_mutex);
	pthread_create(&target, NULL, &thread_proc, (void*)t);
	t->ControlInterface.Setup(target);
	return t;
}

