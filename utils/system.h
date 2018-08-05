/*
 Imagine
 Copyright 2012 Peter Pearson.

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

#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>

namespace Imagine
{

class System
{
public:
	System();

	struct CPUInfo
	{
		CPUInfo() : numSockets(0), numCores(0), numThreads(0)
		{
		}

		unsigned int	numSockets;
		unsigned int	numCores;
		unsigned int	numThreads;
	};

	struct ProcessMemInfo
	{
		ProcessMemInfo() : currentRSS(0), maxRSS(0)
		{
		}

		// in bytes
		size_t		currentRSS;
		size_t		maxRSS;
	};

	static unsigned int getNumberOfThreads();
	static unsigned int getNumberOfCores();

	static System::CPUInfo getCPUInfo();

	static size_t getTotalMemory();
	static size_t getAvailableMemory();

	static size_t getProcessCurrentMemUsage();
	static ProcessMemInfo getProcessMemInfo();

	static float getLoadAverage();

	enum ProcessPriority
	{
		ePriorityHigh,
		ePriorityNormal,
		ePriorityLow
	};

	static bool setProcessPriority(ProcessPriority priority);


private:
	static bool getLinuxCPUInfo(CPUInfo& info);
	static bool getLinuxInfoToken(const char* cpuInfoLine, const char* token, unsigned int& value);
};

} // namespace Imagine

#endif // SYSTEM_H
