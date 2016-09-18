/*
 Imagine
 Copyright 2011-2015 Peter Pearson.

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

#include "thread.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef _MSC_VER

#elif __APPLE__
#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#include <mach/mach_init.h>
#else
	// linux

#endif

#include "utils/maths/maths.h"

namespace Imagine
{

Thread::Thread(ThreadPriority priority) : m_thread(0), m_isRunning(false), m_autoDestruct(false), m_priority(priority),
	m_affinity(-1)
{
}

Thread::~Thread()
{
	stop();
}

void Thread::sleep(unsigned int seconds)
{
#ifndef _MSC_VER
	::sleep(seconds);
#else
	::Sleep(seconds * 1000);
#endif
}

void Thread::pause(unsigned int delay)
{
	for (unsigned int i = 0; i < delay; i++)
	{
		__asm__ __volatile("pause;");
	}
}

void Thread::yield()
{
	sched_yield();
}

#ifdef _MSC_VER
unsigned long __stdcall Thread::threadProc(void* ptr)
#else
void* threadProc(void* ptr)
#endif
{
	Thread* pThread = (Thread*)(ptr);

	int affinity = pThread->getAffinity();
	if (affinity >= 0)
	{
		Thread::setCurrentThreadAffinity(affinity);
	}

	pThread->run();

	pThread->setRunning(false);

	// this doesn't always seem to work safely... - sometimes Threadpools are still waiting for completion and crash
	if (pThread->shouldAutodestruct())
	{
//		fprintf(stderr, "Autodestructing thread...\n");
		delete pThread;
	}

#ifdef _MSC_VER
	return 0;
#else
	pthread_exit(ptr);
#endif
}

// TODO: if this is called twice in a row, the handle to the first thread will be lost, but
// the first thread will still run to completion - maybe return a pointer to thread??
bool Thread::start()
{
	setRunning(true);

#ifdef _MSC_VER
	unsigned long threadID = 0;

	if (m_thread)
		CloseHandle(m_thread);

	m_thread = CreateThread(0, 0, threadProc, this, 0, &threadID);
#else

	int ret = pthread_attr_init(&m_attr);
//	pthread_attr_setdetachstate(&m_attr, PTHREAD_CREATE_JOINABLE);

	if (m_priority != ePriorityNormal)
	{
		ret = pthread_create(&m_thread, NULL, threadProc, (void*)this);

		if (ret)
		{
			setRunning(false);
//			fprintf(stderr, "Couldn't create thread.\n");
			return false;
		}

		struct sched_param schedParam;
		int policy;
		ret = pthread_getschedparam(m_thread, &policy, &schedParam);
		if (ret)
		{
			// failed to get params
//			fprintf(stderr, "Error getting thread params: %s\n", strerror(ret));

			// just return true - the thread was created, but there's no point setting the priority
			// as we don't know what the acceptable priority bounds are
			return true;
		}

		int minPriority = sched_get_priority_min(policy);
		int maxPriority = sched_get_priority_max(policy);

		if (minPriority != maxPriority)
		{
			// according to POSIX, the final priority must be within the above (inclusive) bounds for the process

			float ratio = 0.5f;

			// bizarrely, with these ratios the wrong way around it seems to work in linux - at the very least,
			// with low priorty setting a higher ratio, and thus a higher priority int, the effect is very close to running with nice -5 which
			// is the aim.
			// But something's not right here, as higher values *should* mean higher priority
			switch (m_priority)
			{
				default:
				case ePriorityNormal:
					ratio = 0.5f;
					break;
				case ePriorityHigh:
					ratio = 0.35f;
					break;
				case ePriorityLow:
					ratio = 0.65f;
					break;
			}

			float finalPriority = linearInterpolate((float)minPriority, (float)maxPriority, ratio);
			schedParam.sched_priority = (int)finalPriority;

			ret = pthread_setschedparam(m_thread, policy, &schedParam);
			if (ret != 0)
			{
				fprintf(stderr, "Could not set thread priority: %s\n", strerror(ret));
			}
		}

		// thread was created okay, we *might* have set the priority correctly...
	}
	else
	{
		ret = pthread_create(&m_thread, NULL, threadProc, (void*)this);
//		ret = pthread_create(&m_thread, &m_attr, threadProc, (void*)this);

		if (ret)
		{
			setRunning(false);
//			fprintf(stderr, "Couldn't create thread.\n");
			return false;
		}
	}

#endif

	return true;
}

// TODO: maybe add support for killing the thread while it's running?
void Thread::stop(bool kill)
{
	setRunning(false);

	if (m_thread)
	{
#ifdef _MSC_VER
		CloseHandle(m_thread);

		if (kill)
		{
			unsigned long exitCode = 0;
			GetExitCodeThread(m_thread, &exitCode);

			if (exitCode == STILL_ACTIVE)
			{
				TerminateThread(m_thread, -1);
			}
		}
#else
		if (kill)
			pthread_cancel(m_thread);
#endif
	}
}

void Thread::waitForCompletion()
{
	if (!m_isRunning)
	{
//		fprintf(stderr, "Thread isn't running...\n");
		return;
	}

#ifdef _MSC_VER
	WaitForSingleObject(m_thread, INFINITE);
#else
 //   pthread_attr_destroy(&m_attr);
	int error = pthread_join(m_thread, NULL);
//    assert(error == 0);

	/*
	 * return errors:
	 * 3: no such process or thread...
	 * 22: nothing to join - something else has done a join on that thread
	 *
	*/

#endif
}

//! Depending on the platform, does different things...
void Thread::setCurrentThreadAffinity(int affinity)
{
	if (affinity < 0)
		return;

#ifdef _MSC_VER

#elif __APPLE__
	thread_affinity_policy ap;
	ap.affinity_tag = affinity;
	// On OS X, this doesn't actually guarentee that it will be forced to a particular thread/core, as OS X only uses it as
	// a hint
	if (thread_policy_set(mach_thread_self(), THREAD_AFFINITY_POLICY, (integer_t*)&ap, THREAD_AFFINITY_POLICY_COUNT) != KERN_SUCCESS)
	{
		fprintf(stderr, "Can't set affinity\n");
	}
#else
	// Linux
	//! Linux will obey this, and any threads created by a thread which has affinity set will inherit the affinity,
	//! which effectively makes both threads share the same core, and can throttle concurrency, depending on usage
	cpu_set_t cset;
	CPU_ZERO(&cset);
	CPU_SET(affinity, &cset);
	if (pthread_setaffinity_np(pthread_self(), sizeof(cset), &cset) != 0)
	{
		fprintf(stderr, "Can't set affinity\n");
	}
#endif
}

void Thread::setRunning(bool running)
{
	m_mutex.lock();
	m_isRunning = running;
	m_mutex.unlock();
}

} // namespace Imagine
