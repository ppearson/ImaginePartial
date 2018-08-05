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

#include "system.h"

#ifndef _MSC_VER
#if __APPLE__
#include <mach/host_info.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#else
// linux / unix
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif
#else
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils/bitset.h"

namespace Imagine
{

System::System()
{
}

unsigned int System::getNumberOfThreads()
{
#ifdef _MSC_VER
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return (unsigned int)sysinfo.dwNumberOfProcessors;
#elif __APPLE__
	int mib[2];
	mib[0] = CTL_HW;
	size_t size = 2;

	if (sysctlnametomib("hw.logicalcpu", mib, &size) == -1)
		return 1;

	unsigned int coreCount = 0;
	size = sizeof(coreCount);
	if (sysctl(mib, 2, &coreCount, &size, NULL, 0) == -1)
		return 1;

	return coreCount;
#else
	// linux
	return (unsigned int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

unsigned int System::getNumberOfCores()
{
#ifdef _MSC_VER
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return (unsigned int)sysinfo.dwNumberOfProcessors;
#elif __APPLE__
	int mib[2];
	mib[0] = CTL_HW;
	size_t size = 2;

	if (sysctlnametomib("hw.physicalcpu", mib, &size) == -1)
		return 1;

	unsigned int coreCount = 0;
	size = sizeof(coreCount);
	if (sysctl(mib, 2, &coreCount, &size, NULL, 0) == -1)
		return 1;

	return coreCount;
#else
	// linux
	System::CPUInfo info;
	if (!getLinuxCPUInfo(info))
	{
		return 1;
	}

	return info.numCores;
#endif
}


System::CPUInfo System::getCPUInfo()
{
	System::CPUInfo info;
#if __linux__
	getLinuxCPUInfo(info);
#else
	info.numSockets = 1;
#endif

	return info;
}

size_t System::getTotalMemory()
{
	size_t total = 0;
#if __APPLE__
	int64_t memory = 0;
	size_t size = sizeof(memory);
	int mib[2];
	mib[0] = CTL_HW;
	mib[1] = HW_MEMSIZE;
	if (sysctl(mib, 2, &memory, &size, NULL, 0) != -1)
	{
		total = memory;
	}
#else
	size_t pageSize = (size_t)sysconf(_SC_PAGESIZE);
	size_t totalPages = (size_t)sysconf(_SC_PHYS_PAGES);

	total = pageSize * totalPages;
#endif
	return total;
}

size_t System::getAvailableMemory()
{
	size_t available = 0;
#if __APPLE__
	// get page size first
	int64_t pageSize = 0;
	size_t size = sizeof(pageSize);
	int mib[2];
	mib[0] = CTL_HW;
	mib[1] = HW_PAGESIZE;
	if (sysctl(mib, 2, &pageSize, &size, NULL, 0) == -1)
	{
		return 0;
	}

	kern_return_t kret;
	vm_statistics_data_t stats;
	unsigned int numBytes = HOST_VM_INFO_COUNT;
	kret = host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&stats, &numBytes);
	if (kret == KERN_SUCCESS)
	{
		available = stats.free_count * pageSize;
	}
#else
	size_t pageSize = (size_t)sysconf(_SC_PAGESIZE);
	size_t availablePages = (size_t)sysconf(_SC_AVPHYS_PAGES);

	available = pageSize * availablePages;
#endif
	return available;
}

size_t System::getProcessCurrentMemUsage()
{
	size_t memSize = 0;

#if __APPLE__
	struct mach_task_basic_info info;
	mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
	if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &infoCount) == KERN_SUCCESS)
	{
		memSize = (size_t)info.resident_size;
	}
#else
	FILE* pFile = fopen("/proc/self/statm", "r");
	if (!pFile)
		return false;

	unsigned long rssSize = 0;
	// ignore first item up to whitespace
	fscanf(pFile, "%*s%ld", &rssSize);

	size_t pageSize = (size_t)sysconf(_SC_PAGESIZE);
	memSize = rssSize * pageSize;

	fclose(pFile);
#endif

	return memSize;
}

System::ProcessMemInfo System::getProcessMemInfo()
{
	System::ProcessMemInfo info;

	// getrusage() isn't that useful, as doesn't have current mem info, and on Linux
	// it's close to useless...
	
	// TODO: we could get this from below at the same time now...
	info.currentRSS = getProcessCurrentMemUsage();

#if __linux__
	// on Linux, getrusage() is completely unreliable (ru_maxrss is almost always wrong), so
	// again we have to use /proc/self/ stuff to get the correct answer

	 FILE* pFile = fopen("/proc/self/status", "r");	
	 if (!pFile)
		 return info;
	 
	 char szTemp[1024];
	 // not sure we need this, but...
	 memset(szTemp, 0, 1024);
	 
	 while (fscanf(pFile, " %1023s", szTemp))
	 {
		 if (strcmp(szTemp, "VmHWM:") == 0)
		 {
			 unsigned int value = 0;
			 fscanf(pFile, " %u", &value);
			 
			 info.maxRSS = (size_t)value;
			 break;
		 }
	 }
	 
	 fclose(pFile);
	 
	 // on Linux, these values are in KB, so...
	 info.maxRSS *= 1024;
#else
	// it seems to be reliable on OS X though...
	struct rusage rusage;
	getrusage(RUSAGE_SELF, &rusage); // returns 0 if okay, but...
	
	info.maxRSS = rusage.ru_maxrss;
#endif

	return info;
}

float System::getLoadAverage()
{
	return 1.0f;
}

bool System::setProcessPriority(ProcessPriority priority)
{
#ifdef _MSC_VER


#else
	int nice = 0;
	switch (priority)
	{
		case ePriorityHigh:
		case ePriorityNormal:
			nice = 0;
			break;
		case ePriorityLow:
			nice = -5;
			break;
	}

	setpriority(PRIO_PROCESS, 0, nice);
#endif

	return true;
}

#if __linux__
bool System::getLinuxCPUInfo(CPUInfo& info)
{
	FILE* pFile = fopen("/proc/cpuinfo", "r");
	if (!pFile)
		return false;

	char szTemp[64]; // we don't care about flags, so don't need all of it

	bool newPhysicalCore = false;

	// because the IDs are staggered for HT, we can't check against last...
	Bitset physicalIDsSeen;
	physicalIDsSeen.initialise(128);

	while (fgets(szTemp, 64, pFile) != NULL)
	{
		unsigned int physicalID = 0;
		if (getLinuxInfoToken(szTemp, "physical id", physicalID))
		{
			if (!physicalIDsSeen.isSet(physicalID))
			{
				// we haven't seen this one before, so increase the socket count
				info.numSockets += 1;

				physicalIDsSeen.setBit(physicalID);

				newPhysicalCore = true;
			}
			else
			{
				newPhysicalCore = false;
			}

			continue;
		}

		if (newPhysicalCore)
		{
			// as we're in the listing for a physical id (socket),
			// we haven't seen before, we can use these counts...
			unsigned int cpuCores = 0;
			if (getLinuxInfoToken(szTemp, "cpu cores", cpuCores))
			{
				// as we're in the listing for the next physical id (socket),
				// increment the cpu core count
				info.numCores += cpuCores;
			}

			unsigned int siblings = 0;
			if (getLinuxInfoToken(szTemp, "siblings", siblings))
			{
				info.numThreads += siblings;
			}
		}

		if (feof(pFile))
			break;
	}

	fclose(pFile);

	return true;
}

bool System::getLinuxInfoToken(const char* cpuInfoLine, const char* token, unsigned int& value)
{
	if (memcmp(cpuInfoLine, token, strlen(token)) != 0)
		return false;

	const char* pSep = strstr(cpuInfoLine, ": ");
	if (pSep)
	{
		unsigned int tokenValue = atol(pSep + 2);
		value = tokenValue;
		return true;
	}

	return false;
}

#endif

} // namespace Imagine
