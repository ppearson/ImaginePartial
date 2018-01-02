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

#include "mutex.h"

namespace Imagine
{

Mutex::Mutex() : m_created(true)
{
#ifdef _MSC_VER
	m_mutex = CreateMutex(NULL, FALSE, NULL);
	
	if (!m_mutex)
		m_created = false;
#else
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(&m_mutex, &attr);
#endif
}

Mutex::~Mutex()
{
	if (m_created)
	{
#ifdef _MSC_VER
		WaitForSingleObject(m_mutex, INFINITE);
		CloseHandle(m_mutex);
#else
		pthread_mutex_lock(&m_mutex);
		pthread_mutex_unlock(&m_mutex);
		pthread_mutex_destroy(&m_mutex);
#endif
	}
}

void Mutex::lock()
{
#ifdef _MSC_VER
	WaitForSingleObject(m_mutex, INFINITE);
#else
	pthread_mutex_lock(&m_mutex);
#endif
}

void Mutex::unlock()
{
#ifdef _MSC_VER
	ReleaseMutex(m_mutex);
#else
	pthread_mutex_unlock(&m_mutex);
#endif
}

} // namespace Imagine
