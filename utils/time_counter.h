/*
 Imagine
 Copyright 2015 Peter Pearson.

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

#ifndef TIME_COUNTER_H
#define TIME_COUNTER_H

#include <sys/time.h>

namespace Imagine
{

// TODO: it should be possible to remove the non-subclassed (i.e. ones with no vfunctions) with no penalty,
//       as the compiler should be able to optimise out the virtual function - need to test this though

class TimerCounter
{
public:
	TimerCounter(bool startTimer = false) : m_count(0)
	{
		if (startTimer)
		{
			start();
		}
	}

	~TimerCounter()
	{
	}

	void start()
	{
		gettimeofday(&m_startTime, NULL);
	}

	void stop()
	{
		timeval endTime;
		gettimeofday(&endTime, NULL);

		double mseconds = (endTime.tv_sec - m_startTime.tv_sec) * 1000000;
		mseconds += (endTime.tv_usec - m_startTime.tv_usec);

		m_count += mseconds;
	}

	void reset()
	{
		m_count = 0;
	}

	uint64_t stopReset()
	{
		stop();
		uint64_t localCount = m_count;

		reset();

		return localCount;
	}

	uint64_t getTimeCount() const
	{
		return m_count;
	}

protected:
	timeval			m_startTime;

	uint64_t		m_count;
};

class TimerCounterNull
{
public:
	TimerCounterNull(bool startTimer = false)
	{
	}

	~TimerCounterNull()
	{
	}

	void start()
	{
	}

	void stop()
	{
	}

	void reset()
	{
	}

	uint64_t stopReset()
	{
		return 0;
	}

	uint64_t getTimeCount() const
	{
		return 0;
	}

protected:

};

// derived classes for extra dynamic (on or off) timing - we'll still be paying the price of
// an empty virtual function call for the case when profiling is off, but I think this is better
// than spreading templates everywhere

class ThreadTimeCounter
{
public:
	ThreadTimeCounter()
	{
	}

	virtual ~ThreadTimeCounter()
	{
	}

	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void reset() = 0;

	virtual uint64_t stopReset() = 0;
	virtual uint64_t getTimeCount() const = 0;
};

class ThreadTimeCounterReal : public ThreadTimeCounter
{
public:
	ThreadTimeCounterReal() : m_count(0)
	{
	}

	virtual void start()
	{
		gettimeofday(&m_startTime, NULL);
	}

	virtual void stop()
	{
		timeval endTime;
		gettimeofday(&endTime, NULL);

		double mseconds = (endTime.tv_sec - m_startTime.tv_sec) * 1000000;
		mseconds += (endTime.tv_usec - m_startTime.tv_usec);

		m_count += mseconds;
	}

	virtual void reset()
	{
		m_count = 0;
	}

	virtual uint64_t stopReset()
	{
		stop();
		uint64_t localCount = m_count;

		reset();

		return localCount;
	}

	virtual uint64_t getTimeCount() const
	{
		return m_count;
	}

protected:
	timeval			m_startTime;

	uint64_t		m_count;
};

class ThreadTimeCounterNull : public ThreadTimeCounter
{
public:
	ThreadTimeCounterNull()
	{
	}

	virtual void start()
	{

	}

	virtual void stop()
	{

	}

	virtual void reset()
	{

	}

	virtual uint64_t stopReset()
	{
		return 0;
	}

	virtual uint64_t getTimeCount() const
	{
		return 0;
	}
};

} // namespace Imagine

#endif // TIME_COUNTER_H
