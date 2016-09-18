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

#ifndef TIMER_H
#define TIMER_H

#include <string>
#include <sys/time.h>
#include <stdio.h>

namespace Imagine
{

class Timer
{
public:
	Timer(const std::string &name, bool enabled = true) : m_enabled(enabled), m_name(name)
	{
		if (enabled)
			gettimeofday(&m_startTime, NULL);
	}

	~Timer()
	{
		if (!m_enabled)
			return;

		timeval endTime;
		gettimeofday(&endTime, NULL);

		double seconds = (endTime.tv_sec - m_startTime.tv_sec);
		seconds += (endTime.tv_usec - m_startTime.tv_usec) / 1000000.0;

		if (seconds > 60.0)
		{
			unsigned int minutes = (unsigned int)(seconds / 60.0);
			seconds -= double(minutes * 60);
			fprintf(stderr, "%s: %02d:%05.2f mins.\n", m_name.c_str(), minutes, seconds);
		}
		else
		{
			fprintf(stderr, "%s: %0.5f secs.\n", m_name.c_str(), seconds);
		}
	}
protected:
	bool			m_enabled;
	std::string		m_name;
	timeval			m_startTime;
};


} // namespace Imagine

#endif
