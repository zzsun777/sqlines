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

// Copyright (c) 2013 SQLines. All rights reserved.

#include <stdio.h>
#include <string.h>
#include "sqlparser.h"

// Teradata CREATE TABLE options before column definition
bool SqlParser::ParseTeradataTableOptions()
{
	bool exists = false;

	// , goes after table name before even first option
	while(true)
	{
		Token *comma = GetNextCharToken(',', L',');

		if(comma == NULL)
			break;

		// Most options can start with NO keyword to disable option
		Token *no = GetNextWordToken("NO", L"NO", 2);

		Token *next = GetNextToken();

		if(next == NULL)
		{
			if(no != NULL)
				PushBack(no);

			break;
		}

		// [NO] FALLBACK to store a row copy
		if(next->Compare("FALLBACK", L"FALLBACK", 8) == true)
		{
			if(_target != SQL_TERADATA)
				Token::Remove(comma, next);

			exists = true;
			continue;
		}
		else
		// [NO] BEFORE JOURNAL to store before image
		if(next->Compare("BEFORE", L"BEFORE", 6) == true)
		{
			Token *journal = GetNextWordToken("JOURNAL", L"JOURNAL", 7);

			if(_target != SQL_TERADATA && journal != NULL)
				Token::Remove(comma, journal);

			exists = true;
			continue;
		}
		else
		// [NO] AFTER JOURNAL to store after image
		if(next->Compare("AFTER", L"AFTER", 5) == true)
		{
			Token *journal = GetNextWordToken("JOURNAL", L"JOURNAL", 7);

			if(_target != SQL_TERADATA && journal != NULL)
				Token::Remove(comma, journal);

			exists = true;
			continue;
		}

		if(no != NULL)
			PushBack(no);

		break;
	}

	return exists;
}

// PRIMARY INDEX hash partitioning clause
bool SqlParser::ParseTeradataPrimaryIndex(Token *unique, Token *primary, int obj_scope, Token *last_colname,
											Token *last_colend)
{
	if(primary == NULL)
		return false;

	Token *index = GetNextWordToken("INDEX", L"INDEX", 5);

	if(index == NULL)
		return false;

	Token *open = GetNextCharToken('(', L'(');

	// List of partitioning columns
	while(true)
	{
		Token *column = GetNextToken();

		if(column == NULL)
			break;

		Token *comma = GetNextCharToken(',', L',');

		if(comma == NULL)
			break;
	}

	Token *close = GetNextCharToken(open, ')', L')');

	// Add UNIQUE constraint for other databases
	if(unique != NULL && _target != SQL_TERADATA)
	{
		AppendNoFormat(last_colend, ",", L",", 1);
		Append(last_colend, "\n", L"\n", 1, last_colname);
		
		AppendCopy(last_colend, unique);
		AppendNoFormat(last_colend, " ", L" ", 1);
		AppendCopy(last_colend, open, close);

		Token::Remove(unique, false);
	}

	// PARTITION BY HASH in Oracle
	if(_target == SQL_ORACLE)
	{
		// In Oracle, temporary tables cannot be partitioned 
		if(obj_scope != SQL_SCOPE_TEMP_TABLE)
		{
			Token::Change(primary, "PARTITION", L"PARTITION", 9);
			Token::Change(index, "BY HASH", L"BY HASH", 7);
		}
		else
			Token::Remove(primary, close);
	}

	return true;
}

// Teradata HELP statement
bool SqlParser::ParseTeradataHelpStatement(Token *help)
{
	if(help == NULL)
		return false;

	// HELP STATISTICS
	Token *statistics = GetNext("STATISTICS", L"STATISTICS", 10);

	if(statistics != NULL)
		return ParseTeradataHelpStatistics(help, statistics);	

	return false;
}

// Teradata HELP STATISTICS statement
bool SqlParser::ParseTeradataHelpStatistics(Token *help, Token *statistics)
{
	if(help == NULL || statistics == NULL)
		return false;

	// Table name
	/*Token *table_name */ (void) GetNextIdentToken(SQL_IDENT_OBJECT);

	// Comment for other databases
	if(_target != SQL_TERADATA)
		Comment(help, Nvl(GetNext(';', L';'), GetLastToken()));

	return true;
}

// Teradata comparison operators NE, EQ, LT, LE, GT, GE
bool SqlParser::ParseTeradataComparisonOperator(Token *op)
{
	if(op == NULL || _source != SQL_TERADATA)
		return false;

	bool exists = false;

	// Equal
	if(op->Compare("EQ", L"EQ", 2) == true)
	{
		if(_target != SQL_TERADATA)
			Token::Change(op, "=", L"=", 1);

		exists = true;
	}
	else
	// Less than or equal
	if(op->Compare("LE", L"LE", 2) == true)
	{
		if(_target != SQL_TERADATA)
			Token::Change(op, "<=", L"<=", 2);

		exists = true;
	}
	else
	// Less than 
	if(op->Compare("LT", L"LT", 2) == true)
	{
		if(_target != SQL_TERADATA)
			Token::Change(op, "<", L"<", 1);

		exists = true;
	}
	else
	// Not equal
	if(op->Compare("NE", L"NE", 2) == true)
	{
		if(_target != SQL_TERADATA)
			Token::Change(op, "<>", L"<>", 2);

		exists = true;
	}
	else
	// Greater than or equal
	if(op->Compare("GE", L"GE", 2) == true)
	{
		if(_target != SQL_TERADATA)
			Token::Change(op, ">=", L">=", 2);

		exists = true;
	}
	else
	// Greater than 
	if(op->Compare("GT", L"GT", 2) == true)
	{
		if(_target != SQL_TERADATA)
			Token::Change(op, ">", L">", 1);

		exists = true;
	}

	return exists;
}

// COMPRESS values clause in Teradata
bool SqlParser::ParseTeradataCompressClauseDefaultExpression(Token *compress)
{
	if(compress == NULL)
		return false;

	Token *open = GetNext('(', L'(');

	// List of values to compress
	while(true)
	{
		Token *value = GetNext();

		if(value == NULL)
			break;

		Token *comma = GetNext(',', L',');

		if(comma == NULL)
			break;
	}

	Token *close = GetNext(open, ')', L')');

	// Remove the clause for other databases
	if(_target != SQL_TERADATA)
		Token::Remove(compress, close);

	return true;
}