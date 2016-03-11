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

#ifndef sqlines_os_h
#define sqlines_os_h

class Os
{
public:
	// Get procedure address
	static void* GetProcAddress(int module, const char *name);

	// Get current time in milliseconds
	static int GetTickCount();

	// Get error message of the last system error
	static void GetLastErrorText(const char *prefix, char *output, int len);
};

#endif // sqlines_os_h