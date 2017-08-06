/*
 Imagine
 Copyright 2016-2017 Peter Pearson.

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

#ifndef LOGGER_H
#define LOGGER_H

#include <string>

#include <stdio.h>

namespace Imagine
{

class Logger
{
public:

	enum LogOutputDestination
	{
		eLogStdOut,
		eLogStdErr,
		eLogFile
//		eLogStdErrAndLogFile
	};

	enum LogLevel
	{
		eLevelDebug,
		eLevelInfo,
		eLevelWarning,
		eLevelNotice,
		eLevelError,
		eLevelCritical,
		eLevelOff
	};

	enum LogTimeStampType
	{
		eTimeStampNone,
		eTimeStampTime,
		eTimeStampTimeAndDate,
		eTimeStampElapsedTime
	};

	Logger();
	~Logger();

	bool initialiseFileLogger(const std::string& logPath, LogLevel level, LogTimeStampType timeStampType);
	bool initialiseConsoleLogger(LogOutputDestination logDest, LogLevel level, bool coloured);

	// explicitly sets coloured output, regardless of output destination. Passing force as true will
	// ignore isatty() check (for use in Katana currently)
	void setColouredOutput(bool colouredOutput, bool force);

	void debug(const char* format, ...);
	void info(const char* format, ...);
	void warning(const char* format, ...);
	void notice(const char* format, ...);
	void error(const char* format, ...);
	void critical(const char* format, ...);

protected:

	void outputLogItem(LogLevel itemLevel, const char* format, va_list args);

protected:
	LogOutputDestination	m_logOutputType;
	std::string				m_logFilePath; // will only be valid if LogOutputDestination == eLogFile

	// this will point to either stdout, stderr, or a FILE handle we own if m_logOutputType == eLogFile
	FILE*					m_pFileHandle;
	bool					m_initialised;

	LogLevel				m_logLevel;
	LogTimeStampType		m_timeStampType;

	bool					m_colouredOutput;

	// cached stuff for timestamps
};

} // namespace Imagine

#endif // LOGGER_H
