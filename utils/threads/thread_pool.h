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
#include <bitset>

#include "thread.h"
#include "mutex.h"
#include "event.h"

namespace Imagine
{

#define MAX_THREADS 128
#define TASK_BUNDLE_SIZE 4

class ThreadPool;

class Task
{
public:
	Task();
	virtual ~Task() { }

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
	Task*				m_pTasks[TASK_BUNDLE_SIZE];
};

class RequeuedTasks
{
public:
	RequeuedTasks()
	{
	}

	std::deque<Task*> m_pTasks;
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
	std::bitset<MAX_THREADS>	m_bsThreadsAvailable;
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
	void setTask(Task* pTask) { m_pTask = pTask; }

protected:
	void runSingleTask();
	void runTaskBundle();

protected:
	Task*			m_pTask;
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
	void startPoolAndWaitForCompletion();
	void startPool();
	
	void processTask(Task* pTask, unsigned int threadID);

	virtual bool doTask(Task* pTask, unsigned int threadID) = 0;

	void addTask(Task* pTask);
	void requeueTask(Task* pTask, unsigned int threadID);

	void addTaskNoLock(Task* pTask);

	void addRequeuedTasks(RequeuedTasks& rqt);

	void deleteThreads();
	void deleteThreadsAndRequeuedTasks();

	void deleteTask(Task* pTask, bool lockAndRemoveFromQueue);

	Task* getNextTask();
	unsigned int getNextTaskBundle(TaskBundle* pBundle);

	void freeThread(unsigned int threadID);
	
	void clearTasks();

	// these assume locking is done elsewhere
	Task* getNextTaskInternal();
	// assumes the TaskBundle is blank
	unsigned int getNextTaskBundleInternal(TaskBundle* pBundle);

protected:
	unsigned int		m_numberOfThreads;
	ThreadPoolThread*	m_pThreads[MAX_THREADS];
	RequeuedTasks*		m_pRequeuedTasks[MAX_THREADS];
	ThreadController	m_controller;
	std::deque<Task*>	m_aTasks;
	Mutex				m_lock;
	Mutex				m_requeueLock;

	//! Note: if this is set on Linux, any other threads started by a task started by a thread pool
	//!       will share the same affinity and will be locked to that processor, effectively throttling the execution
	//!       of all further spawned threads to a single core...
	bool				m_setAffinity;

	bool				m_lowPriorityThreads;

	bool				m_useBundles;
	unsigned int		m_threadBundleSizeThreshold1; // overall size to use the thread size (* numThreads)
	unsigned int		m_threadBundleSizeThreshold2; // half above

	volatile bool		m_isActive;
	bool				m_wasCancelled;

	unsigned int		m_originalNumberOfTasks;
	float				m_fOriginalNumberOfTasks;
	float				m_invOriginalNumTasks;
};

} // namespace Imagine

#endif
