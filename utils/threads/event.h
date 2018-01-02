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

#ifndef EVENT_H
#define EVENT_H

#ifndef _MSC_VER
#include <pthread.h>
#else
#include <windows.h>
#endif

namespace Imagine
{

class Mutex;

class Event
{
public:
	Event();
	~Event();

	void signal();
	void broadcast();
	void wait();
	void wait(Mutex& mutex);
	void reset();

protected:
#ifdef _MSC_VER
	HANDLE m_event;
#else
	pthread_cond_t m_ready;
	pthread_mutex_t m_lock;
#endif
	
	volatile bool	m_hasHappened;

};

} // namespace Imagine

#endif
