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

#include <stdio.h>
#include <algorithm>
#include "stats.h"

// Constructor/destructor
Stats::Stats() {}

Stats::~Stats() {}

// Collect statistics for the specified item
void Stats::Add(std::map<std::string, int> &map, Token *token, bool case_insense)
{
    if(token == NULL || token->str == NULL || token->len == 0)
        return;

    Add(map, std::string(token->str, token->len), case_insense);
}

void Stats::Add(std::map<std::string, int> &map, TokenStr *token, bool case_insense)
{
    if(token == NULL)
        return;

    Add(map, token->str, case_insense);
}

void Stats::Add(std::map<std::string, int> &map, Token *start, Token *end, bool case_insense )
{
    if(start == NULL || start->str == NULL || start->len == 0)
        return;

    Add(map, std::string(start->str, start->len + start->remain_size - end->remain_size), case_insense);
}

void Stats::Add(std::map<std::string, int> &map, std::string value, bool case_insense)
{
    if(case_insense)
        std::transform(value.begin(), value.end(), value.begin(), ::toupper);

    // Check if this value already used
	std::map<std::string, int>::iterator i = map.find(value);

    // Increment the counter or insert new key-value pair
	if(i != map.end())
    	(*i).second++;
    else 
	    map[value] = 1;    
}
