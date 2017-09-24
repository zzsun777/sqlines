
// AppLog - Application logger class
// Copyright (c) 2012 SQLines. All rights reserved

#ifndef sqlines_applog_h
#define sqlines_applog_h

#if defined(WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <stdarg.h>
#include <string>
#include "os.h"

// Default log and trace file names
#define APPLOG_DEFAULT_FILE			"applog.log"
#define APPTRACE_DEFAULT_FILE		"applog.trc"

typedef void (*AppLogConsoleFunc)(void *object, const char *format, va_list args);

class AppLog
{
	// Log and trace file names
	std::string _filename;
	std::string _filename_trc;

	bool first_write;
	bool first_write_trc;

	bool _use_stderr;

	// A function to redirect console output
	AppLogConsoleFunc _console;
	void *_console_object;

#if defined(WIN32) || defined(_WIN64)
	CRITICAL_SECTION _log_critical_section;
#else
	pthread_mutex_t _log_critical_section;
#endif

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
	void LogFile(const char *data, int len);

	// Write to log file
    void LogFileVaList(const char *format, va_list args);
	// Write to trace file
    void TraceFileVaList(const char *format, va_list args);

	// Set log and trace file names
	void SetLogfile(const char *name, const char *default_dir); 
	void SetTracefile(const char *name, const char *default_dir); 

	void SetUseStdErr(bool val) { _use_stderr = val; }

	void Reset() { first_write = true; }

	// Set console output function
	void SetConsoleFunc(void *object, AppLogConsoleFunc console) { _console_object = object; _console = console; }
};

#endif // sqlines_applog_h