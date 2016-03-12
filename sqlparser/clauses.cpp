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

// Various SQL clauses

#include <stdio.h>
#include <string.h>
#include "sqlparser.h"

// DB2 ALTER COLUMN clause in ALTER TABLE
bool SqlParser::AlterColumnClause(Token *table, Token *table_name, Token *alter)
{
	if(alter == NULL)
		return false;

	Token *column = GetNextWordToken("COLUMN", L"COLUMN", 6);

	if(column == NULL)
		return false;

	Token *name = GetNextIdentToken();

	// Option
	Token *next = GetNextToken();

	if(next == NULL)
		return false;

	// RESTART WITH for identity column
	if(next->Compare("RESTART", L"RESTART", 7) == true)
	{
		Token *with = GetNextWordToken("WITH", L"WITH", 4);
		Token *value = GetNextNumberToken();

		// Change to ALTER SEQUENCE for PostgreSQL and Greenplum
		if(_target == SQL_POSTGRESQL || _target == SQL_GREENPLUM)
		{
			Token *seq_name = AppendIdentifier(table_name, "_seq", L"_seq", 4);
		
			Token::Change(table, "SEQUENCE", L"SEQUENCE", 8);
			Token::Change(table_name, seq_name);

			// Remove ALTER COLUMN name
			Token::Remove(alter, name);

			delete seq_name;
		}
	}

	return true;
}
