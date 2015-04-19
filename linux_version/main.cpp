#include "ThreadPool.h"

class mytask1:public ThreadBase{
public:
	mytask1(){}
	~mytask1(){}
	bool run(){
		std::cout<<"my task 1 run\n"<<std::endl;
		return 1;
	}
};

class mytask2:public ThreadBase{
public:
	mytask2(){}
	~mytask2(){}
	bool run(){
		std::cout<<"my task 2 run\n"<<std::endl;
		return 1;
	}
};

int main(){
	CThreadPool *cpool=new CThreadPool();
	ThreadBase *t1=new mytask1();
	ThreadBase *t2=new mytask2();
	cpool->Startup();
	cpool->RunTask(t1);
	cpool->RunTask(t2);
	cpool->Stop();
	delete cpool;

	return 0;
}