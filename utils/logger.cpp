/*
 Imagine
 Copyright 2016-2018 Peter Pearson.

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

#include "logger.h"

#include <ctime>
#include <cstdarg>
#include <unistd.h>

namespace Imagine
{

#define kCodeReset			"\033[0m"
#define kCodeColourBlack	"\033[30m"
#define kCodeColourRed		"\033[31m"
#define kCodeColourGreen	"\033[32m"
#define kCodeColourOrange	"\033[33m"
#define kCodeColourBlue		"\033[34m"
#define kCodeColourMagenta	"\033[35m"
#define kCodeColourCyan		"\033[36m"

static const char* kLogLevelFullNames[] = { "Debug", "Info", "Warning", "Notice", "Error", "Critical", "Off", 0 };
static const char* kLogLevelColourCodes[] = { kCodeColourBlack, kCodeColourGreen, kCodeColourOrange,
											  kCodeColourCyan, kCodeColourRed, kCodeColourRed, kCodeColourBlack, 0 };

Logger::Logger() : m_pFileHandle(nullptr), m_pSecondaryStdErr(nullptr),
					m_initialised(false), m_logLevel(eLevelOff),
					m_timeStampType(eTimeStampNone), m_colouredOutput(false)
{

}

Logger::~Logger()
{
	if (m_logOutputType == eLogFile && m_pFileHandle)
	{
		fclose(m_pFileHandle);
		m_pFileHandle = nullptr;
	}

	// Note: we don't need to worry about m_pSecondaryStdErr...
}

bool Logger::initialiseFileLogger(const std::string& logPath, LogLevel level, LogTimeStampType timeStampType)
{
	m_logOutputType = eLogFile;
	m_logFilePath = logPath;

	m_logLevel = level;
	m_timeStampType = timeStampType;

	m_pFileHandle = fopen(m_logFilePath.c_str(), "w+");

	m_initialised = (m_pFileHandle != nullptr);

	// this is a bit silly, but if we weren't initialised, set the mode to stderr mode, so at least the user gets
	// something...
	if (!m_initialised)
	{
		m_logOutputType = eLogStdErr;
		critical("Can't create/open log file: %s", m_logFilePath.c_str());
	}

	return m_initialised;
}

bool Logger::initialiseConsoleLogger(LogOutputDestination logDest, LogLevel level, bool coloured)
{
	m_logOutputType = logDest;

	m_logLevel = level;
	m_timeStampType = eTimeStampNone;

	if (m_logOutputType == eLogStdOut)
	{
		m_pFileHandle = stdout;
	}
	else if (m_logOutputType == eLogStdErr)
	{
		m_pFileHandle = stderr;
	}
	else if (m_logOutputType == eLogStdOutOrStdErr)
	{
		// the main one for warnings will point to stdout
		m_pFileHandle = stdout;

		// and we'll initialise the secondary one to point to stderr
		m_pSecondaryStdErr = stderr;
	}

	// Note: this isatty() check fails within Katana, so we need to force it on...
	m_colouredOutput = coloured && isatty(fileno(m_pFileHandle));

	m_initialised = true;
	return true;
}

void Logger::setColouredOutput(bool colouredOutput, bool force)
{
	if (colouredOutput)
	{
		if (!force)
		{
			colouredOutput = isatty(fileno(m_pFileHandle));
		}
	}

	m_colouredOutput = colouredOutput;
}

// TODO: with large amounts of logging, it might be beneficial performance-wise to check the log level
//       short-circuit before doing the varargs stuff...
void Logger::debug(const char* format, ...)
{
	va_list argPtr;
	va_start(argPtr, format);
	outputLogItem(eLevelDebug, format, argPtr);
	va_end(argPtr);
}

void Logger::info(const char* format, ...)
{
	va_list argPtr;
	va_start(argPtr, format);
	outputLogItem(eLevelInfo, format, argPtr);
	va_end(argPtr);
}

void Logger::warning(const char* format, ...)
{
	va_list argPtr;
	va_start(argPtr, format);
	outputLogItem(eLevelWarning, format, argPtr);
	va_end(argPtr);
}

void Logger::notice(const char* format, ...)
{
	va_list argPtr;
	va_start(argPtr, format);
	outputLogItem(eLevelNotice, format, argPtr);
	va_end(argPtr);
}

void Logger::error(const char* format, ...)
{
	va_list argPtr;
	va_start(argPtr, format);
	outputLogItem(eLevelError, format, argPtr);
	va_end(argPtr);
}

void Logger::critical(const char* format, ...)
{
	va_list argPtr;
	va_start(argPtr, format);
	outputLogItem(eLevelCritical, format, argPtr);
	va_end(argPtr);
}

void Logger::outputLogItem(LogLevel itemLevel, const char* format, va_list args)
{
	if (itemLevel < m_logLevel)
		return;

	char szLogMessage[4096]; // needs to be more than long enough for full production Katana location names in the strings...

	vsprintf(szLogMessage, format, args);

	if (m_logOutputType != eLogFile)
	{
		FILE* pFileTarget = m_pFileHandle;

		// if we're configured to be split based on the log level, check if we need to change to stderr...
		if (m_logOutputType == eLogStdOutOrStdErr && (itemLevel >= eLevelError && itemLevel < eLevelOff))
		{
			pFileTarget = m_pSecondaryStdErr;
		}

		// to console
		if (m_colouredOutput)
		{
			// TODO: first two could be combined...
			fprintf(pFileTarget, "%s[%s] %s%s\n", kLogLevelColourCodes[itemLevel], kLogLevelFullNames[itemLevel], szLogMessage, kCodeReset);
		}
		else
		{
			fprintf(pFileTarget, "[%s] %s\n", kLogLevelFullNames[itemLevel], szLogMessage);
		}
	}
	else
	{
		if (m_timeStampType == eTimeStampNone)
		{
			fprintf(m_pFileHandle, "[%s] %s\n", kLogLevelFullNames[itemLevel], szLogMessage);
		}
		else if (m_timeStampType == eTimeStampTime || m_timeStampType == eTimeStampTimeAndDate)
		{
			time_t time1;
			time(&time1);
			struct tm* pTimeinfo;
			pTimeinfo = localtime(&time1);

			char szTime[64];
			const char* timeFormat = (m_timeStampType == eTimeStampTime) ? "%H:%M:%S" : "%F %H:%M:%S";
			strftime(szTime, 64, timeFormat, pTimeinfo);
			fprintf(m_pFileHandle, "%s [%s] %s\n", szTime, kLogLevelFullNames[itemLevel], szLogMessage);
		}

		if (m_logOutputType == eLogFile)
		{
			fflush(m_pFileHandle);
		}
	}
}

} // namespace Imagine
