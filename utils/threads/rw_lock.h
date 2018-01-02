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

#ifndef RW_LOCK_H
#define RW_LOCK_H

#ifndef _MSC_VER
#include <pthread.h>
#else
#include <windows.h>
#endif

namespace Imagine
{

class RWLock
{
public:
	RWLock();
	~RWLock();

	void readLock();
	void writeLock();

	bool tryReadLock();
	bool tryWriteLock();

	void unlock();

protected:
#ifdef _MSC_VER

#else
	pthread_rwlock_t m_lock;
#endif
	bool m_created;
};

} // namespace Imagine

#endif // RW_LOCK_H
