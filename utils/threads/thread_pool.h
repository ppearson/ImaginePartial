/*
 Imagine
 Copyright 2011-2012 Peter Pearson.

 Licensed under the Apache License, Version 2.0 (the "License");
 You may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ---------
*/

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <deque>
#include <vector>
#include <bitset>

#include "thread.h"
#include "mutex.h"
#include "event.h"

namespace Imagine
{

#define kMAX_THREADS 256 // used for controller bitset...
#define kTASK_BUNDLE_SIZE 4

class ThreadPool;

class ThreadPoolTask
{
public:
	ThreadPoolTask();
	virtual ~ThreadPoolTask() { }

	void setThreadPool(ThreadPool* pThreadPool);

	bool isActive() const;

protected:
	ThreadPool*	m_pThreadPool;
};

class TaskBundle
{
public:
	TaskBundle();
	virtual ~TaskBundle() { }

	void clear();

	unsigned char		m_size;
	ThreadPoolTask*		m_pTasks[kTASK_BUNDLE_SIZE];
};

class RequeuedTasks
{
public:
	RequeuedTasks()
	{
	}

	std::deque<ThreadPoolTask*> m_pTasks;
	Mutex	m_lock;
};

// assumes events accurately portray whether there are threads available or not
class ThreadController
{
public:
	ThreadController(unsigned int threads = 2);
	~ThreadController() { }

	int getThread();
	// assumes there are threads ready...
	int getThreadNoLock();
	void freeThread(unsigned int thread);
	bool isActive(unsigned int thread);
	
	unsigned int threadsActive() const;
	unsigned int threadsAvailable() const;

protected:
	Mutex						m_lock;
	unsigned int				m_numberOfThreads;
	std::bitset<kMAX_THREADS>	m_bsThreadsAvailable;
	Event						m_threadAvailable;

	unsigned int				m_activeThreads;
};

class ThreadPoolThread : public Thread
{
public:
	ThreadPoolThread(ThreadPool* pThreadPool, unsigned int threadID, Thread::ThreadPriority priority);
	virtual ~ThreadPoolThread();

	virtual void run();

	TaskBundle* createTaskBundle();

	void deleteTasksInBundle();

	void setTaskBundleSize(unsigned int size) { m_bundleSize = size; }
	void setTask(ThreadPoolTask* pTask) { m_pTask = pTask; }

protected:
	void runSingleTask();
	void runTaskBundle();

protected:
	ThreadPoolTask*	m_pTask;
	TaskBundle*		m_pTaskBundle;
	RequeuedTasks	m_requeueTasks;
	unsigned int	m_bundleIndex;
	unsigned int	m_bundleSize;
	ThreadPool*		m_pThreadPool;
	unsigned int	m_threadID;

private:
	ThreadPoolThread(const ThreadPoolThread& vc);

	ThreadPoolThread& operator=(const ThreadPoolThread& vc);
};

class ThreadPool
{
public:
	ThreadPool(unsigned int threads = 2, bool useBundles = false);
	virtual ~ThreadPool();

	friend class ThreadPoolThread;

	void terminate();

	bool isActive() const;
	bool wasCancelled() const;

	//! called after each task is done
	virtual void taskDone() { }

protected:

	enum PoolExecutionTypeFlags
	{
		POOL_WAIT_FOR_COMPLETION		= 1 << 0, // this can't be used with POOL_ASYNC_COMPLETION_EVENT
		POOL_ASYNC_COMPLETION_EVENT		= 1 << 1, // this can't be used with POOL_WAIT_FOR_COMPLETION
		POOL_WAIT_FOR_NEW_TASKS			= 1 << 2,
		POOL_FORCE_ALL_THREADS			= 1 << 3, // will create the full number of threads, even if there aren't enough tasks
		POOL_ALLOW_START_EMPTY			= 1 << 4
	};

	// these destroy any existing threads and create new ones...
	void startPool(unsigned int flags);

	// called from async thread to trigger a finish
	void externalFinished();

	void processTask(ThreadPoolTask* pTask, unsigned int threadID);

	virtual bool doTask(ThreadPoolTask* pTask, unsigned int threadID) = 0;

	void addTask(ThreadPoolTask* pTask);
	void requeueTask(ThreadPoolTask* pTask, unsigned int threadID);

	void addTaskNoLock(ThreadPoolTask* pTask);

	void addRequeuedTasks(RequeuedTasks& rqt);

	void deleteThreads();
	void deleteThreadsAndRequeuedTasks();

	void deleteTask(ThreadPoolTask* pTask, bool lockAndRemoveFromQueue);

	ThreadPoolTask* getNextTask();
	unsigned int getNextTaskBundle(TaskBundle* pBundle);

	void freeThread(unsigned int threadID);

	void clearTasks();

	// these assume locking is done elsewhere
	ThreadPoolTask* getNextTaskInternal();
	// assumes the TaskBundle is blank
	unsigned int getNextTaskBundleInternal(TaskBundle* pBundle);

private:
	class AsyncFinishEventWaitThread : public Thread
	{
	public:
		AsyncFinishEventWaitThread(ThreadPool* pThreadPool) : m_pThreadPool(pThreadPool)
		{

		}

		virtual ~AsyncFinishEventWaitThread()
		{
		}

		virtual void run();


	protected:
		ThreadPool*		m_pThreadPool;
	};


protected:
	ThreadController	m_controller;

	std::vector<ThreadPoolThread*>	m_aThreads;
	std::vector<RequeuedTasks*>		m_aRequeuedTasks;

	std::deque<ThreadPoolTask*>	m_aTasks;

	Mutex				m_lock;
	Mutex				m_requeueLock;
	
	AsyncFinishEventWaitThread*	m_pAsyncFinishThread;

	Event				m_finishedEvent;


// configuration stuff
	unsigned int		m_numberOfThreads;

	//! Note: if this is set on Linux, any other threads started by a task started by a thread pool
	//!       will share (inherit) the same affinity and will be locked to that processor, effectively throttling the execution
	//!       of all further spawned threads to a single core, so be careful when using this... However, on NUMA systems,
	//!       it can make a noticable difference to efficiency when enabled and memory used by each thread is allocated (first touch)
	//!       appropriately...
	bool				m_setAffinity;

	bool				m_lowPriorityThreads;

	bool				m_useBundles;
	unsigned int		m_threadBundleSizeThreshold1; // overall size to use the thread size (* numThreads)
	unsigned int		m_threadBundleSizeThreshold2; // half above

	unsigned int		m_startedThreads;
	volatile bool		m_isActive;
	bool				m_wasCancelled;

	unsigned int		m_originalNumberOfTasks;
	float				m_fOriginalNumberOfTasks;
	float				m_invOriginalNumTasks;
};

} // namespace Imagine

#endif
