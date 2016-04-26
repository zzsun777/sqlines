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

// OS Class - Operating system related functions

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <stdio.h>
#include "os.h"

// Get procedure address
void* Os::GetProcAddress(int module, const char *name)
{
#ifdef WIN32
	FARPROC proc = ::GetProcAddress((HMODULE)module, name);

	return proc;
#else
    return NULL;
#endif
}

// Get current time in milliseconds
int Os::GetTickCount()
{
#ifdef WIN32
	return ::GetTickCount();
#else
	struct timeval  tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000 + tv.tv_usec/1000;
#endif
}

// Get error message of the last system error
void Os::GetLastErrorText(const char *prefix, char *output, int len)
{
	if(output == NULL || len <= 0)
		return;

#ifdef WIN32
   	char error[1024];

    DWORD rc = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
						1033, (LPSTR)error, 1024, NULL);
 
	// No error found
	if(rc == 0)
	{
		*output = '\x0';
		return;
	}

	strcpy(output, prefix);
	strcat(output, error);

#endif
}