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

#include "thread_pool.h"

#include <algorithm>
#include <string.h> // for memset
#include <assert.h>

namespace Imagine
{

ThreadPoolTask::ThreadPoolTask() : m_pThreadPool(NULL)
{
}

void ThreadPoolTask::setThreadPool(ThreadPool* pThreadPool)
{
	m_pThreadPool = pThreadPool;
}

bool ThreadPoolTask::isActive() const
{
	return m_pThreadPool->isActive();
}

TaskBundle::TaskBundle() : m_size(0)
{
	memset(m_pTasks, 0, sizeof(ThreadPoolTask*) * TASK_BUNDLE_SIZE);
}

void TaskBundle::clear()
{
	memset(m_pTasks, 0, sizeof(ThreadPoolTask*) * TASK_BUNDLE_SIZE);
	m_size = 0;
}

ThreadController::ThreadController(unsigned int threads) : m_numberOfThreads(threads), m_activeThreads(0)
{
	// set the number of available threads to 1 (available) in the bitset - the remainder
	// are 0

	m_bsThreadsAvailable.reset();

	m_threadAvailable.reset();

	for (unsigned int i = 0; i < m_numberOfThreads; i++)
	{
		m_bsThreadsAvailable.set(i, true);
	}
}

void ThreadPool::terminate()
{
	if (!m_isActive)
	{
		return;
	}

	m_wasCancelled = true;
	m_isActive = false;

	for (unsigned int i = 0; i < m_numberOfThreads; i++)
	{
		if (m_controller.isActive(i))
		{
			Thread* pThread = m_pThreads[i];
			pThread->stop(false);
		}
	}

	// as Thread::waitForCompletion()'s pthread_join() doesn't seem to work properly when
	// there's already something joining on it, just delete all the tasks and wait for the threads
	// to kill themselves
	m_lock.lock();

	std::deque<ThreadPoolTask*>::iterator it = m_aTasks.begin();
	for (; it != m_aTasks.end(); ++it)
	{
		delete *it;
	}

	m_aTasks.clear();
	m_lock.unlock();
}

bool ThreadController::isActive(unsigned int thread)
{
	m_lock.lock();

	bool isAvailable = false;

	if (!m_bsThreadsAvailable.test(thread))
	{
		isAvailable = true;
	}

	m_lock.unlock();

	return isAvailable;
}

unsigned int ThreadController::threadsActive() const
{
	return m_activeThreads;
}

unsigned int ThreadController::threadsAvailable() const
{
	return m_numberOfThreads - m_activeThreads;
}

int ThreadController::getThread()
{
	int thread = -1;

	// first see if there's already a free one
	m_lock.lock();

	if (m_bsThreadsAvailable.any())
	{
		for (unsigned int i = 0; i < m_numberOfThreads; i++)
		{
			if (m_bsThreadsAvailable.test(i))
			{
				thread = i;
				m_bsThreadsAvailable.set(i, false);
				m_activeThreads++;
				break;
			}
		}

		m_lock.unlock();
	}
	else
	{
		m_lock.unlock();

		m_threadAvailable.wait();

		m_lock.lock();

		// find the thread that's available

		for (unsigned int i = 0; i < m_numberOfThreads; i++)
		{
			if (m_bsThreadsAvailable.test(i))
			{
				thread = i;
				m_bsThreadsAvailable.set(i, false);
				m_activeThreads++;
				break;
			}
		}

		m_threadAvailable.reset();

		m_lock.unlock();
	}

	return thread;
}

// this assumes there are threads available...
int ThreadController::getThreadNoLock()
{
	int thread = -1;

	if (m_bsThreadsAvailable.any())
	{
		for (unsigned int i = 0; i < m_numberOfThreads; i++)
		{
			if (m_bsThreadsAvailable.test(i))
			{
				thread = i;
				m_bsThreadsAvailable.set(i, false);
				m_activeThreads++;
				break;
			}
		}
	}

	return thread;
}

void ThreadController::freeThread(unsigned int thread)
{
	if (thread >= m_numberOfThreads)
		return;

	m_lock.lock();

	m_bsThreadsAvailable.set(thread, true);
	m_activeThreads--;

	m_lock.unlock();

	m_threadAvailable.broadcast();
}

ThreadPoolThread::ThreadPoolThread(ThreadPool* pThreadPool, unsigned int threadID, Thread::ThreadPriority priority) : Thread(priority),
	m_pTask(NULL), m_pTaskBundle(NULL),	m_bundleIndex(0), m_bundleSize(0), m_pThreadPool(pThreadPool), m_threadID(threadID)
{
}

ThreadPoolThread::~ThreadPoolThread()
{
	if (m_pTaskBundle)
	{
		delete m_pTaskBundle;
		m_pTaskBundle = NULL;
	}
}

void ThreadPoolThread::run()
{
	if (m_pTaskBundle)
		runTaskBundle();
	else
		runSingleTask();
}

void ThreadPoolThread::runSingleTask()
{
	assert(m_pThreadPool);

	// while we have a task...
	while (m_pTask)
	{
		// call ThreadPool subclass's doTask method
		if (m_pThreadPool->doTask(m_pTask, m_threadID))
		{
			// task is done, so remove it
			m_pThreadPool->deleteTask(m_pTask, false);
			m_pTask = NULL;
		}
		else
		{
			// re-add the task
			m_pThreadPool->addTask(m_pTask);
			m_pTask = NULL;
		}

		m_pThreadPool->taskDone();

		if (m_isRunning && m_pThreadPool)
		{
			// to save constantly creating and destroying loads of threads, try and reuse them if there's work to do...
			m_pTask = m_pThreadPool->getNextTask();
		}
	}

	// otherwise, free the thread
	m_pThreadPool->freeThread(m_threadID);
}

void ThreadPoolThread::runTaskBundle()
{
	assert(m_pThreadPool);

	ThreadPoolTask* pTask = NULL;
	while (m_isRunning)
	{
		// while we've got tasks in our local bundle...
		while (m_bundleSize > 0 && m_pThreadPool)
		{
			pTask = m_pTaskBundle->m_pTasks[m_bundleIndex];

			if (m_pThreadPool->doTask(pTask, m_threadID))
			{
				// task is done, so remove it
				m_pThreadPool->deleteTask(pTask, false);
				m_pTaskBundle->m_pTasks[m_bundleIndex] = NULL;
				m_pTask = NULL;
			}
			else
			{
				m_requeueTasks.m_pTasks.push_back(pTask);
			}

			m_pThreadPool->taskDone();
			m_bundleIndex++;
			m_bundleSize--;
		}

		if (m_isRunning)
		{
			// we've run out of tasks in our bundle, so see if there are any more available for us
			m_pTaskBundle->clear();

			m_bundleIndex = 0;

			m_pThreadPool->addRequeuedTasks(m_requeueTasks);
			m_bundleSize = m_pThreadPool->getNextTaskBundle(m_pTaskBundle);

			if (m_bundleSize == 0)
			{
				// there aren't any left to do...
				break;
			}
		}
	}

	// otherwise, free the thread
	m_pThreadPool->freeThread(m_threadID);
}

TaskBundle* ThreadPoolThread::createTaskBundle()
{
	m_pTaskBundle = new TaskBundle();
	return m_pTaskBundle;
}

void ThreadPoolThread::deleteTasksInBundle()
{
	if (!m_pTaskBundle)
		return;

	for (unsigned int i = 0; i < m_bundleSize; i++)
	{
		ThreadPoolTask* pTask = m_pTaskBundle->m_pTasks[i];

		if (pTask)
			delete pTask;
	}
}

ThreadPool::ThreadPool(unsigned int threads, bool useBundles) : m_numberOfThreads(threads), m_controller(threads),
	m_setAffinity(false), m_lowPriorityThreads(false), m_useBundles(useBundles), m_isActive(false),
	m_wasCancelled(false), m_originalNumberOfTasks(0)
{
	for (unsigned int i = 0; i < m_numberOfThreads; i++)
	{
		m_pThreads[i] = NULL;
		if (m_useBundles)
		{
			m_pRequeuedTasks[i] = new RequeuedTasks();
		}
		else
		{
			m_pRequeuedTasks[i] = NULL;
		}
	}

	m_threadBundleSizeThreshold1 = TASK_BUNDLE_SIZE * m_numberOfThreads;
	m_threadBundleSizeThreshold2 = m_threadBundleSizeThreshold1 / 2;
}

ThreadPool::~ThreadPool()
{
	deleteThreadsAndRequeuedTasks();
}

void ThreadPool::addTask(ThreadPoolTask* pTask)
{
	m_lock.lock();

	pTask->setThreadPool(this);

	m_aTasks.push_back(pTask);

	m_lock.unlock();
}

void ThreadPool::requeueTask(ThreadPoolTask* pTask, unsigned int threadID)
{
	RequeuedTasks* pRQT = m_pRequeuedTasks[threadID];

	pRQT->m_pTasks.push_back(pTask);
}

void ThreadPool::addTaskNoLock(ThreadPoolTask* pTask)
{
	pTask->setThreadPool(this);

	m_aTasks.push_back(pTask);
}

ThreadPoolTask* ThreadPool::getNextTask()
{
	ThreadPoolTask* pTask = NULL;

	m_lock.lock();

	if (!m_aTasks.empty())
	{
		pTask = m_aTasks.front();
		m_aTasks.pop_front();
	}

	m_lock.unlock();

	return pTask;
}

unsigned int ThreadPool::getNextTaskBundle(TaskBundle* pBundle)
{
	unsigned int bundleSize = 0;

	m_lock.lock();

	bundleSize = getNextTaskBundleInternal(pBundle);

	m_lock.unlock();

	return bundleSize;
}

void ThreadPool::startPoolAndWaitForCompletion()
{
	deleteThreads();

	int threadID = -1;

	m_originalNumberOfTasks = m_aTasks.size();
	m_fOriginalNumberOfTasks = (float)m_originalNumberOfTasks;
	m_invOriginalNumTasks = 1.0f / m_fOriginalNumberOfTasks;

	// don't bother creating more threads than there are tasks if that is the case...
	unsigned int threadsToStart = std::min(m_originalNumberOfTasks, m_numberOfThreads);

	unsigned int threadsCreated = 0;
	unsigned int threadsStarted = 0;

	m_wasCancelled = false;
	m_isActive = true;

	// TODO: there is a mis-match here between the thread objects created along with
	//       the tasks popped for them, and the actual number of threads that were started

	// initialise all potential threads to NULL
	memset(m_pThreads, 0, sizeof(ThreadPoolThread*) * MAX_THREADS);

	m_lock.lock();

	bool shouldCreateBundleThreads = m_useBundles && (m_originalNumberOfTasks > m_numberOfThreads * 2);

	Thread::ThreadPriority newThreadPriority = Thread::ePriorityNormal;

	if (m_lowPriorityThreads)
	{
		newThreadPriority = Thread::ePriorityLow;
	}

	for (unsigned int j = 0; j < threadsToStart; j++)
	{
		threadID = m_controller.getThreadNoLock();

		if (threadID != -1)
		{
			if (!m_aTasks.empty())
			{
				ThreadPoolThread* pThread = new ThreadPoolThread(this, threadID, newThreadPriority);
				if (pThread)
				{
					if (shouldCreateBundleThreads)
					{
						TaskBundle* pTB = pThread->createTaskBundle();
						unsigned int numTasks = getNextTaskBundleInternal(pTB);
						pThread->setTaskBundleSize(numTasks);
					}
					else
					{
						ThreadPoolTask* pTask = getNextTaskInternal();
						pThread->setTask(pTask);
					}

					m_pThreads[threadID] = pThread;

					if (m_setAffinity)
					{
						pThread->setAffinity(j);
					}

					threadsCreated ++;
				}
			}
		}
		else
		{
			// something weird happened

			Thread::sleep(1);
		}
	}

	m_lock.unlock();

	// start them - this is best done in its own loop
	for (unsigned int i = 0; i < threadsCreated; i++)
	{
		if (m_pThreads[i])
		{
			if (m_pThreads[i]->start())
			{
				threadsStarted++;
			}
			else
			{
				// indicate that it's not actually active
				m_controller.freeThread(i);
			}
		}
	}

	// now need to make sure any active threads have finished before we go out of scope
	for (unsigned int i = 0; i < threadsStarted; i++)
	{
		if (m_controller.isActive(i))
		{
			Thread* pThread = m_pThreads[i];
			pThread->waitForCompletion();
		}
	}

	m_lock.lock();

	std::deque<ThreadPoolTask*>::iterator it = m_aTasks.begin();
	for (; it != m_aTasks.end(); ++it)
	{
		delete *it;
	}

	m_aTasks.clear();

	m_lock.unlock();

	m_isActive = false;
}

void ThreadPool::startPool()
{
	deleteThreads();

	int threadID = -1;

	m_originalNumberOfTasks = m_aTasks.size();
	m_fOriginalNumberOfTasks = (float)m_originalNumberOfTasks;
	m_invOriginalNumTasks = 1.0f / m_fOriginalNumberOfTasks;

	// don't bother creating more threads than there are tasks if that is the case...
	unsigned int threadsToStart = std::min(m_originalNumberOfTasks, m_numberOfThreads);

	unsigned int threadsCreated = 0;
	unsigned int threadsStarted = 0;

	m_wasCancelled = false;
	m_isActive = true;

	// TODO: there is a mis-match here between the thread objects created along with
	//       the tasks popped for them, and the actual number of threads that were started

	// initialise all potential threads to NULL
	memset(m_pThreads, 0, sizeof(ThreadPoolThread*) * MAX_THREADS);

	m_lock.lock();

	bool shouldCreateBundleThreads = m_useBundles && (m_originalNumberOfTasks > m_numberOfThreads * 2);

	Thread::ThreadPriority newThreadPriority = Thread::ePriorityNormal;

	if (m_lowPriorityThreads)
	{
		newThreadPriority = Thread::ePriorityLow;
	}

	for (unsigned int j = 0; j < threadsToStart; j++)
	{
		threadID = m_controller.getThreadNoLock();

		if (threadID != -1)
		{
			if (!m_aTasks.empty())
			{
				ThreadPoolThread* pThread = new ThreadPoolThread(this, threadID, newThreadPriority);
				if (pThread)
				{
					if (shouldCreateBundleThreads)
					{
						TaskBundle* pTB = pThread->createTaskBundle();
						unsigned int numTasks = getNextTaskBundleInternal(pTB);
						pThread->setTaskBundleSize(numTasks);
					}
					else
					{
						ThreadPoolTask* pTask = getNextTaskInternal();
						pThread->setTask(pTask);
					}

					m_pThreads[threadID] = pThread;

					if (m_setAffinity)
					{
						pThread->setAffinity(j);
					}

					threadsCreated ++;
				}
			}
		}
		else
		{
			// something weird happened

			Thread::sleep(1);
		}
	}

	m_lock.unlock();

	// start them - this is best done in its own loop
	for (unsigned int i = 0; i < threadsCreated; i++)
	{
		if (m_pThreads[i])
		{
			if (m_pThreads[i]->start())
			{
				threadsStarted++;
			}
			else
			{
				// indicate that it's not actually active
				m_controller.freeThread(i);
			}
		}
	}
}

void ThreadPool::addRequeuedTasks(RequeuedTasks& rqt)
{
	if (rqt.m_pTasks.empty())
		return;

	m_lock.lock();

	std::deque<ThreadPoolTask*>::iterator it = rqt.m_pTasks.begin();
	for (; it != rqt.m_pTasks.end(); ++it)
	{
		ThreadPoolTask* pTask = *it;

		m_aTasks.push_back(pTask);
	}

	rqt.m_pTasks.clear();

	m_lock.unlock();
}

void ThreadPool::deleteThreads()
{
	for (unsigned int i = 0; i < m_numberOfThreads; i++)
	{
		Thread* pThread = m_pThreads[i];
		if (pThread)
			delete pThread;
	}
}

void ThreadPool::deleteThreadsAndRequeuedTasks()
{
	for (unsigned int i = 0; i < m_numberOfThreads; i++)
	{
		ThreadPoolThread* pThread = m_pThreads[i];
		if (pThread)
			delete pThread;

		RequeuedTasks* pRQT = m_pRequeuedTasks[i];
		if (pRQT)
			delete pRQT;
	}
}

void ThreadPool::deleteTask(ThreadPoolTask* pTask, bool lockAndRemoveFromQueue)
{
	if (lockAndRemoveFromQueue)
	{
		// safely delete the task, so that we don't try and delete it later
		// TODO: this is needed, but might be slow in some cases, although thanks to tasks being re-used for things like
		// rendering, it probably isn't *that* bad...
		m_lock.lock();

		std::deque<ThreadPoolTask*>::iterator itFind = std::find(m_aTasks.begin(), m_aTasks.end(), pTask);
		if (itFind != m_aTasks.end())
		{
			m_aTasks.erase(itFind);
		}

		m_lock.unlock();
	}

	delete pTask;
}

bool ThreadPool::isActive() const
{
	return m_isActive;
}

bool ThreadPool::wasCancelled() const
{
	return m_wasCancelled;
}

void ThreadPool::freeThread(unsigned int threadID)
{
	m_controller.freeThread(threadID);
}

void ThreadPool::clearTasks()
{

}



// assumes mutex is already held...
ThreadPoolTask* ThreadPool::getNextTaskInternal()
{
	ThreadPoolTask* pTask = m_aTasks.front();

	m_aTasks.pop_front();

	return pTask;
}

// assumes mutex is already held...
unsigned int ThreadPool::getNextTaskBundleInternal(TaskBundle* pBundle)
{
	// check we've got enough tasks to go around
	unsigned int tasksPending = m_aTasks.size();

	if (tasksPending >= m_threadBundleSizeThreshold1)
	{
		// TODO: spread out the tasks we take, so that we still get a linear progression of tiles...
		unsigned int tasksAdded = 0;
		for (unsigned int i = 0; i < TASK_BUNDLE_SIZE; i++)
		{
			ThreadPoolTask* pTask = m_aTasks.front();
			m_aTasks.pop_front();

			pBundle->m_pTasks[i] = pTask;
			tasksAdded ++;
		}

		pBundle->m_size = tasksAdded;

		return tasksAdded;
	}
	else if (tasksPending > m_threadBundleSizeThreshold2)
	{
		unsigned int tasksAdded = 0;
		for (unsigned int i = 0; i < TASK_BUNDLE_SIZE / 2; i++)
		{
			ThreadPoolTask* pTask = m_aTasks.front();
			m_aTasks.pop_front();

			pBundle->m_pTasks[i] = pTask;
			tasksAdded ++;
		}

		pBundle->m_size = tasksAdded;

		return tasksAdded;
	}
	else if (tasksPending > 0)
	{
		ThreadPoolTask* pTask = m_aTasks.front();
		m_aTasks.pop_front();

		pBundle->m_pTasks[0] = pTask;
		pBundle->m_size = 1;

		return 1;
	}
	else
	{
		return 0;
	}
}

} // namespace Imagine
