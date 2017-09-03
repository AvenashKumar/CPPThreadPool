#include "cThreadPool.h"

// the constructor just launches some amount of workers
cThreadPool::cThreadPool(const int p_ciThreads)
	: m_bIsStopAllWorkers(false)
{
	for (int i = 0; i < p_ciThreads; ++i)
	{
		m_vtWorkers.emplace_back(thread(cThreadTaskExecutor(this)));
	}
}

cThreadPool::cThreadPool(const cThreadPool&) :
m_vtWorkers(),
m_qTasks(),
m_QueueMutex(),
m_ConditionVariable(),
m_bIsStopAllWorkers(false)
{
};

// the destructor joins all threads
cThreadPool::~cThreadPool()
{
	{
		unique_lock<mutex> lock(m_QueueMutex);
		m_bIsStopAllWorkers = true;
	}
	m_ConditionVariable.notify_all();

	// join them
	for (int i = 0; i< (int)m_vtWorkers.size(); ++i)
		m_vtWorkers[i].join();
}
bool cBoolCondition::operator()()
{
	return (m_pPool->m_bIsStopAllWorkers || !m_pPool->m_qTasks.empty());
}

void cThreadTaskExecutor::operator()()
{
	/*Don't delete me!*/
	cThreadPool* ptrPool = (cThreadPool*)this->m_ptrPool;
	for (;;)
	{
		function<void()> task;

		{
			cBoolCondition returnBool(ptrPool);

			/*Since we are updating condition variable and add tasks in queue (Both are shared) that's way we need this lock*/
			unique_lock<mutex> lock(ptrPool->m_QueueMutex);

			/*Update condition variable, It is manadatory to update condition variable When m_bIsStopAllWorkers is true, so we can notify all the workers to stop execution*/
			ptrPool->m_ConditionVariable.wait(lock, returnBool);

			/*No need to fill queue again if all workers wants to rest and after completing there previous tasks.*/
			if (ptrPool->m_bIsStopAllWorkers && ptrPool->m_qTasks.empty())
				return;

			/*Get ip front job/task(FIFO)*/
			task = move(ptrPool->m_qTasks.front());

			/*Perform pop operation on queue after getting head task/job*/
			ptrPool->m_qTasks.pop();
		}

		/*Execute task*/
		task();
	}

}
