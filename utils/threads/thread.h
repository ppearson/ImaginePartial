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

#ifndef THREAD_H
#define THREAD_H

#ifndef _MSC_VER
#include <pthread.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "mutex.h"

namespace Imagine
{

class Thread
{
public:
	enum ThreadPriority
	{
		ePriorityHigh,
		ePriorityNormal,
		ePriorityLow
	};

	Thread(ThreadPriority priority = ePriorityNormal);
	virtual ~Thread();

	static void sleep(unsigned int seconds);
	static void pause(unsigned int delay);
	static void yield();

	bool start();
	void stop(bool kill = false);
	void waitForCompletion();

	void setAutodestruct(bool autoDestruct) { m_autoDestruct = autoDestruct; }
	bool shouldAutodestruct() const { return m_autoDestruct; }

	static void setCurrentThreadAffinity(int affinity);

	void setAffinity(int affinity) { m_affinity = affinity; }
	int getAffinity() const { return m_affinity; }

	virtual void run() = 0;

protected:
#ifdef _MSC_VER
	static unsigned long __stdcall threadProc(void* ptr);
#else
	friend void *threadProc(void* ptr);
#endif

	void setRunning(bool running);
	void setFinished(bool finished);

protected:
#ifdef _MSC_VER
	HANDLE		m_thread;
#else
	pthread_t	m_thread;
	pthread_attr_t  m_attr;
#endif

	volatile bool	m_isRunning;
	volatile bool	m_isFinished;
	bool			m_autoDestruct;
	Mutex			m_mutex;
	ThreadPriority	m_priority;
	int				m_affinity;

private:
	Thread(const Thread& rhs);
	Thread& operator=(const Thread& rhs);
};

} // namespace Imagine

#endif
