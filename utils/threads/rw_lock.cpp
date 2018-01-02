/*
 Imagine
 Copyright 2014 Peter Pearson.

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

#include "rw_lock.h"

namespace Imagine
{

RWLock::RWLock() : m_created(true)
{
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init(&attr);
	pthread_rwlock_init(&m_lock, &attr);
}

RWLock::~RWLock()
{
	if (m_created)
	{
		pthread_rwlock_destroy(&m_lock);
	}
}

void RWLock::readLock()
{
	pthread_rwlock_rdlock(&m_lock);
}

void RWLock::writeLock()
{
	pthread_rwlock_wrlock(&m_lock);
}

bool RWLock::tryReadLock()
{
	// return true if we succeed
	return (pthread_rwlock_tryrdlock(&m_lock) == 0);
}

bool RWLock::tryWriteLock()
{
	// return true if we succeed
	return (pthread_rwlock_trywrlock(&m_lock) == 0);
}

void RWLock::unlock()
{
	pthread_rwlock_unlock(&m_lock);
}

} // namespace Imagine
