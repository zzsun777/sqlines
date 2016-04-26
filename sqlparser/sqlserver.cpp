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

// SQL Server conversions

#include <stdio.h>
#include <string.h>
#include "sqlparser.h"

// Convert row-level trigger using inserted and deleted system tables
void SqlParser::SqlServerConvertRowLevelTrigger(Token *table, Token *when, Token *insert, Token *update, 
					Token *delete_, Token *end)
{
	if(table == NULL || when == NULL)
		return;

	TokenStr old_cols;
	TokenStr old_vars;

	Token *decl_end = _spl_last_declare;

	// Skip statement delimiter
	if(decl_end != NULL && Token::Compare(decl_end->next, ';', L';') == true)
		decl_end = decl_end->next;
	
	int num = 0;
	size_t old_prefix_len = 4;

	// Skip old. in column name references (old already prefixed with @ in target)
	if(_spl_old_correlation_name != NULL)
		old_prefix_len = _spl_old_correlation_name->len + 1 + 1;

	// Generate declaration for referenced OLD columns
	for(ListwItem *i = _spl_tr_old_columns.GetFirst(); i != NULL; i = i->next)
	{
		Token *var = (Token*)i->value;

		if(num == 0)
			Append(decl_end, "\n", L"\n", 1);
		else
		{
			old_cols.Append(", ", L", ", 2);
			old_vars.Append(", ", L", ", 2);
		}

		Append(decl_end, "\nDECLARE ", L"\nDECLARE ", 9, _declare_format);
		AppendCopy(decl_end, var);
		Append(decl_end, " ", L" ", 1);

		// Define the data type of the variable
		TokenStr type;
		GuessType(var, type);
		AppendNoFormat(decl_end, &type);

		Append(decl_end, ";", L";", 1);

		old_cols.Append(var, old_prefix_len, var->len - old_prefix_len + 1);
		old_vars.Append(var);

		num++;
	}
	
	// Declare deleted cursor
	if(delete_ != NULL)
	{
		Append(decl_end, "\nDECLARE ", L"\nDECLARE ", 9, _declare_format); 
		AppendNoFormat(decl_end, "deleted_cur ", L"deleted_cur ", 12); 
		Append(decl_end, "CURSOR FOR SELECT ", L"CURSOR FOR SELECT ", 18, _declare_format);
		AppendNoFormat(decl_end, &old_cols);
		Append(decl_end, " FROM ", L" FROM ", 6, _declare_format);
		AppendNoFormat(decl_end, "deleted;", L"deleted;", 8);
	}

	if(delete_ != NULL)
		Append(decl_end, "\n\nOPEN deleted_cur;", L"\n\nOPEN deleted_cur;", 19); 

	Append(decl_end, "\nWHILE 1=0 BEGIN", L"\nWHILE 1=0 BEGIN", 16); 

	if(delete_ != NULL)
	{
		Append(decl_end, "\nFETCH deleted_cur INTO ", L"\nFETCH deleted_cur INTO ", 24);
		AppendNoFormat(decl_end, &old_vars);
		Append(decl_end, ";", L";", 1);
	}

	Append(decl_end, "\nIF @@FETCH_STATUS <> 0 BREAK;", L"\nIF @@FETCH_STATUS <> 0 BREAK;", 30);

	// Terminate the cursor loop
	Token *end_loop = Prepend(end, "END\n", L"END\n", 4);

	if(delete_ != NULL)
	{
		Append(end_loop, "CLOSE deleted_cur;", L"CLOSE deleted_cur;", 18);
		Append(end_loop, "\nDEALLOCATE deleted_cur;", L"\nDEALLOCATE deleted_cur;", 24);
	}

	// Perform DELETE in INSTEAD OF trigger
	if(delete_ != NULL && insert == NULL && update == NULL)
	{
		Append(end_loop, "\nDELETE FROM ", L"\nDELETE FROM ", 13);
		AppendCopy(end_loop, table);
		Append(end_loop, " WHERE (", L" WHERE (", 8); 
		AppendNoFormat(end_loop, &old_cols);
		Append(end_loop, ") IN (SELECT ", L") IN (SELECT ", 13);
		AppendNoFormat(end_loop, &old_cols);
		Append(end_loop, " FROM deleted);\n", L" FROM deleted);\n", 16);
	}
}

// SQL Server requires an alias for subquery
void SqlParser::SqlServerAppendSubqueryAlias(Token *append, int *appended_subquery_aliases)
{
	if(append == NULL)
		return;

	int alias_num = 0;

	// Define the current counter within SELECT
	if(appended_subquery_aliases != NULL)
		alias_num = *appended_subquery_aliases;

	TokenStr alias(" s", L" s", 2);
	
	if(alias_num != 0)
		alias.Append(alias_num);

	AppendNoFormat(append, &alias);

	alias_num++;

	if(appended_subquery_aliases != NULL)
		*appended_subquery_aliases = alias_num;
}

// Any Transact-SQL statement can be followed by ; and GO, any combination of them or nothing at all
void SqlParser::SqlServerDelimiter()
{
	if(Source(SQL_SQL_SERVER, SQL_SYBASE) == false || Target(SQL_SQL_SERVER, SQL_SYBASE) == true)
		return;

	Token *last = GetLastToken();

	// Optional ; can go before GO
	Token *semi = GetNextCharToken(';', L';');

	// Optional GO to terminate the batch
	Token *go = GetNextWordToken("GO", L"GO", 2);

	// No ; delimiter
	if(semi == NULL)
		AppendNoFormat(last, ";", L";", 1);

	Token::Remove(go);
}

// Add statement delimiter if not set when source is SQL Server, Sybase ASE
void SqlParser::SqlServerAddStmtDelimiter(bool force)
{
	if(Source(SQL_SQL_SERVER, SQL_SYBASE) == false)
		return;

	// Add the delimiter in statements in procedural block only
	if(_spl_scope == 0 && force == false)
		return;

	// Check if the delimiter already set
	Token *semi = GetNextCharToken(';', L';');

	if(semi != NULL)
	{
		PushBack(semi);
		return;
	}

	// Do not set if comma followed
	Token *comma = GetNextCharToken(',', L',');

	if(comma != NULL)
	{
		PushBack(comma);
		return;
	}

	AppendNoFormat(GetLastToken(), ";", L";", 1);
}

// Handle GO after a statement in SQL Server
bool SqlParser::SqlServerGoDelimiter(bool just_remove)
{
	if(Source(SQL_SQL_SERVER, SQL_SYBASE) == false || Target(SQL_SQL_SERVER, SQL_SYBASE) == true)
		return false;

	Token *stmt_end = GetLastToken();

	// Optional ; can go before GO
	Token *semi = GetNextCharToken(';', L';');

	Token *go = GetNextWordToken("GO", L"GO", 2);

	if(go == NULL)
	{
		// ; is processed at statement parser
		if(semi != NULL)
			PushBack(semi);

		return false;
	}

	// The delimiter ; does not already exist, add it otherwise just remove GO
	if(semi == NULL && just_remove == false)
		AppendNoFormat(stmt_end, ";", L";", 1);

	Token::Change(go, " ", L" ", 1);

	return true;
}

// SQL Server SET options, SET ANSI_NULLS ON i.e
bool SqlParser::ParseSqlServerSetOptions(Token *set)
{
	if(set == NULL)
		return false;

	bool exists = false;

	bool comment = false;
	bool remove = false;

	Token *option = GetNextToken();

	if(option == NULL)
		return false;

	// SET ANSI_NULLS ON | OFF
	if(option->Compare("ANSI_NULLS", L"ANSI_NULLS", 10) == true)
	{
		/*Token *value */ (void) GetNextToken();

		if(_target != SQL_SQL_SERVER)
			comment = true;

		exists = true;
	}
	else
	// SET ANSI_PADDING ON | OFF
	if(option->Compare("ANSI_PADDING", L"ANSI_PADDING", 12) == true)
	{
		/*Token *value */ (void) GetNextToken();

		if(_target != SQL_SQL_SERVER)
			comment = true;

		exists = true;
	}
	else
	// SET NOCOUNT ON | OFF
	if(option->Compare("NOCOUNT", L"NOCOUNT", 7) == true)
	{
		/*Token *value */ (void) GetNextToken();

		if(_target != SQL_SQL_SERVER)
			remove = true;

		exists = true;
	}
	else
	// SET QUOTED_IDENTIFIER ON | OFF
	if(option->Compare("QUOTED_IDENTIFIER", L"QUOTED_IDENTIFIER", 17) == true)
	{
		/*Token *value */ (void) GetNextToken();

		if(_target != SQL_SQL_SERVER)
			comment = true;

		exists = true;
	}
	// Not a SET option
	else
		PushBack(option);

	if(exists == true)
	{
		Token *last = Nvl(GetNextCharToken(';', L';'), GetLastToken());

		if(comment == true)
			Comment(set, last);
		else
		if(remove == true)
			Token::Remove(set, last);

		// Remove following GO if any
		SqlServerGoDelimiter(true);
	}

	return exists;
}

// SQL Server index options WITH (PAD_INDEX = OFF, ...)
bool SqlParser::ParseSqlServerIndexOptions(Token *token)
{
	if(token == NULL)
		return false;

	bool exists = false;

	// Start with WITH keyword
	if(token->Compare("WITH", L"WITH", 4) == false)
		return false;

	// ( can be omitted if FILLFACTOR is only specified
	Token *open = GetNext('(', L'(');

	while(true)
	{
		bool remove = false;

		Token *option = GetNextToken();

		if(option == NULL)
			break;

		// = between option name and value
		/*Token *equal */ (void) GetNext('=', L'=');

		Token *value = NULL;

		// ALLOW_PAGE_LOCKS = ON | OFF
		if(option->Compare("ALLOW_PAGE_LOCKS", L"ALLOW_PAGE_LOCKS", 16) == true)
		{
			value = GetNextToken();

			if(_target != SQL_SQL_SERVER)
				remove = true;

			exists = true;
		}
		else
		// ALLOW_ROW_LOCKS = ON | OFF
		if(option->Compare("ALLOW_ROW_LOCKS", L"ALLOW_ROW_LOCKS", 15) == true)
		{
			value = GetNextToken();

			if(_target != SQL_SQL_SERVER)
				remove = true;

			exists = true;
		}
		else
		// FILLFACTOR = num
		if(option->Compare("FILLFACTOR", L"FILLFACTOR", 10) == true)
		{
			value = GetNextToken();

			if(_target != SQL_SQL_SERVER)
				remove = true;

			exists = true;
		}
		else
		// IGNORE_DUP_KEY = ON | OFF
		if(option->Compare("IGNORE_DUP_KEY", L"IGNORE_DUP_KEY", 14) == true)
		{
			value = GetNextToken();

			if(_target != SQL_SQL_SERVER)
				remove = true;

			exists = true;
		}
		else
		// PAD_INDEX = ON | OFF
		if(option->Compare("PAD_INDEX", L"PAD_INDEX", 9) == true)
		{
			value = GetNextToken();

			if(_target != SQL_SQL_SERVER)
				remove = true;

			exists = true;
		}
		else
		// STATISTICS_NORECOMPUTE = ON | OFF
		if(option->Compare("STATISTICS_NORECOMPUTE", L"STATISTICS_NORECOMPUTE", 22) == true)
		{
			value = GetNextToken();

			if(_target != SQL_SQL_SERVER)
				remove = true;

			exists = true;
		}
		else
		{
			PushBack(option);
			break;
		}

		Token *comma = GetNext(',', L',');

		// Remove the option
		if(remove == true)
		{
			Token::Remove(option, value);
			Token::Remove(comma);
		}

		if(comma == NULL)
			break;
	}

	if(exists == false)
	{
		// PostgreSQL also has WITH ( ... option, so return open token
		PushBack(open);

		return false;
	}

	Token *close = GetNext(open, ')', L')');

	// Remove WITH ()
	if(_target != SQL_SQL_SERVER)
	{
		Token::Remove(token);
		Token::Remove(open);
		Token::Remove(close);
	}

	// Storage clause can follow 
	ParseSqlServerStorageClause();

	return exists;
}