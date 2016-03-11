/** 
 * Copyright (c) 2016 SQLines
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef sqlines_applog_h
#define sqlines_applog_h

#include <stdarg.h>
#include <string>

// Default log file name
#define APPLOG_DEFAULT_FILE			"applog.log"

class AppLog
{
	// Log file name
	std::string _filename;
	// Trace mode 
	bool _trace;
public:
	AppLog();

	// Log message to console and log file
    void Log(const char *format, ...);
    // Log messages to file if trace mode is set
    void Trace(const char *format, ...);
    // Write to console only
    void LogConsole(const char *format, ...);
    // Write to log file only
    void LogFile(const char *format, ...);

	// Set log file name
	void SetLogfile(const char *name) { _filename = name; }
private:
    // Write to log file
    void LogFileVaList(const char *format, va_list args);	
};

#endif // sqlines_applog_h