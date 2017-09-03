#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <stdexcept>
using namespace std;

/**
* This class is responsible for:
* 1. Adding worker threads.
* 2. Adding jobs.
* 3. Execute jobs based on number of worker threads.
*/
class cThreadPool
{
private:
	friend class cThreadTaskExecutor;
	friend class cBoolCondition;

	vector< thread > m_vtWorkers;					///< Responsible holding workers list.
	queue< function<void()> > m_qTasks;				///< Responsible for holding jobs.

	/* Synchronization attributes*/
	mutex m_QueueMutex;								///< Locking attribute.
	condition_variable m_ConditionVariable;			///< Block a thread, or multiple threads at the same time, until another thread both modifies a shared variable (the condition), and notify that condition all threads.
	bool m_bIsStopAllWorkers;						///< Either to stop workers? stop worker true means we are forcing Thread pool to stop execution.

private:
	/**Disable default constructor.
	*/
	cThreadPool();

public:

	/**Parameterize constructor
	* \param p_ciTotalWorkers: Total number of worker threads.
	*/
	cThreadPool(const int p_ciTotalWorkers);

	/**Copy constructor
	* \param p_crefPoolOfThreads: Pool of threads.
	*/
	cThreadPool(const cThreadPool& p_crefPoolOfThreads);

	/**Destructor.
	*/
	~cThreadPool();

	/**Enqueue jobs
	* \param p_Tasks: Job.
	*/
	template<class F>
	void enqueue(F p_Tasks);
};
// Add new work item to the pool
template<class F>
void cThreadPool::enqueue(F p_Tasks)
{
	{ // acquire lock
		unique_lock<mutex> lock(m_QueueMutex);

		// add the task
		m_qTasks.push(function<void()>(p_Tasks));
	} // release lock

	// wake up one thread
	m_ConditionVariable.notify_one();
}

/**
* This class is actually responsible for updating condition variable.
* Note: Purpose behind the creation of seperate class: Since we are updating a shared variable
that's way in C++ we have to 2 options either we create global static function, or create
class which override bracket operator.
*/
class cBoolCondition
{
private:
	cThreadPool* m_pPool;		///<Don't delete this pointer thread pool because caller function/class is the owner of this pointer.

	/**Disable default constructor.
	*/
	cBoolCondition();
public:
	/**Parameterize constructor
	* \param pPool: Pointer to cThreadPool.
	*/
	cBoolCondition(cThreadPool* pPool) :m_pPool(pPool){}

	/**Responsible for checking stop conditions and return the same
	* \return Stop/Run condition.
	*/
	bool operator()();
};

/**
* This class is actually responsible for the:.
* 1. Execution of jobs.
* Note: Purpose behind the creation of seperate class: Since we are updating a shared variable
that's way in C++ we have  2 options either we create global/ static function, or create
class which overrides bracket "()" operator.
*/
class cThreadTaskExecutor
{
private:
	cThreadPool* m_ptrPool;	///<Don't delete this pointer thread pool because caller function/class is the owner of this pointer.

private:
	/**Disable default constructor.
	*/
	cThreadTaskExecutor();
public:
	/**Parameterize constructor
	* \param pPool: Pointer to cThreadPool.
	*/
	cThreadTaskExecutor(cThreadPool* p_ptrPool) :m_ptrPool(p_ptrPool){}

	/**Execute job.
	*/
	void operator()();
};
