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

#include "event.h"

#include "mutex.h"

#include <stdio.h>

namespace Imagine
{

Event::Event() : m_hasHappened(false)
{
#ifdef _MSC_VER
	m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	pthread_mutexattr_t mutexAttr;
	pthread_mutexattr_init(&mutexAttr);
	pthread_mutex_init(&m_lock, &mutexAttr);
	pthread_cond_init(&m_ready, NULL);
#endif
}

Event::~Event()
{
#ifdef _MSC_VER
	CloseHandle(m_event);
#else
	pthread_cond_destroy(&m_ready);
	pthread_mutex_destroy(&m_lock);
#endif
}

void Event::signal()
{
#ifdef _MSC_VER
	SetEvent(m_event);
#else
	pthread_mutex_lock(&m_lock);
	m_hasHappened = true;
	pthread_cond_signal(&m_ready);
	pthread_mutex_unlock(&m_lock);
#endif
}

void Event::broadcast()
{
#ifdef _MSC_VER
	SetEvent(m_event);
#else
	pthread_mutex_lock(&m_lock);
	m_hasHappened = true;
	pthread_cond_broadcast(&m_ready);
	pthread_mutex_unlock(&m_lock);
#endif
}

void Event::wait()
{
#ifdef _MSC_VER
	WaitForSingleObject(m_event, INFINITE);
#else	
	pthread_mutex_lock(&m_lock);
	while (!m_hasHappened)
	{
		pthread_cond_wait(&m_ready, &m_lock);
	}
	pthread_mutex_unlock(&m_lock);
#endif
}

void Event::wait(Mutex& mutex)
{
	while (!m_hasHappened)
	{
		pthread_cond_wait(&m_ready, &mutex.m_mutex);
	}
}

void Event::reset()
{
#ifndef _MSC_VER
	pthread_mutex_lock(&m_lock);
#endif
	
	m_hasHappened = false;
#ifndef _MSC_VER
	pthread_mutex_unlock(&m_lock);
#endif
}

} // namespace Imagine
