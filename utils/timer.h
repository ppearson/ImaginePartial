/*
 Imagine
 Copyright 2011-2019 Peter Pearson.

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

#ifndef TIMER_H
#define TIMER_H

#define USE_CHRONO 1

#if USE_CHRONO
#include <chrono>
#else
#include <sys/time.h>
#endif

#include <string>
#include <stdio.h>

#include "logger.h"

namespace Imagine
{

class Timer
{
public:
	Timer(const std::string& name, bool enabled = true) : m_name(name), m_pLogger(nullptr), m_enabled(enabled)
	{
		if (enabled)
		{
#if USE_CHRONO
			m_startTimePoint = std::chrono::steady_clock::now();
#else
			gettimeofday(&m_startTime, nullptr);
#endif
		}
	}

	Timer(const std::string& name, Logger& logger, bool enabled = true) : m_name(name), m_pLogger(&logger), m_enabled(enabled)
	{
#if USE_CHRONO
		m_startTimePoint = std::chrono::steady_clock::now();
#else
		gettimeofday(&m_startTime, nullptr);
#endif
	}

	~Timer()
	{
		if (!m_enabled)
			return;

#if USE_CHRONO
		std::chrono::steady_clock::time_point endTimePoint = std::chrono::steady_clock::now();
		std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double> >(endTimePoint - m_startTimePoint);
		
		double seconds = timeSpan.count();
#else
		timeval endTime;
		gettimeofday(&endTime, nullptr);

		double seconds = (endTime.tv_sec - m_startTime.tv_sec);
		seconds += (endTime.tv_usec - m_startTime.tv_usec) / 1000000.0;
#endif

		if (seconds > 60.0)
		{
			unsigned int minutes = (unsigned int)(seconds / 60.0);
			seconds -= double(minutes * 60);
			if (m_pLogger)
			{
				m_pLogger->info("%s: %02d:%05.2f mins.", m_name.c_str(), minutes, seconds);
			}
			else
			{
				fprintf(stderr, "%s: %02d:%05.2f mins.\n", m_name.c_str(), minutes, seconds);
			}
		}
		else
		{
			if (m_pLogger)
			{
				m_pLogger->info("%s: %0.5f secs.", m_name.c_str(), seconds);
			}
			else
			{
				fprintf(stderr, "%s: %0.5f secs.\n", m_name.c_str(), seconds);
			}
		}
	}
protected:
	std::string		m_name;
	Logger*			m_pLogger;
	bool			m_enabled;
	
#if USE_CHRONO
	std::chrono::steady_clock::time_point	m_startTimePoint;
#else
	timeval						m_startTime;
#endif
};


} // namespace Imagine

#endif
