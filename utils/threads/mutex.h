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

#ifndef MUTEX_H
#define MUTEX_H

#ifndef _MSC_VER
#include <pthread.h>
#else
#include <windows.h>
#endif

class Mutex
{
public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();

protected:
#ifdef _MSC_VER
	HANDLE m_mutex;
#else
	pthread_mutex_t m_mutex;
#endif
	bool m_created;

};

class Guard
{
public:
	Guard()
	{
		m_mutex.lock();
	}
	
	~Guard()
	{
		m_mutex.unlock();
	}
	
protected:
	Mutex m_mutex;
};

#endif
