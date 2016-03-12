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

// COBOL language parser

#include <stdio.h>
#include <string.h>
#include "sqlparser.h"
#include "cobol.h"

// Constructor
Cobol::Cobol()
{
	_sqlparser = NULL;

	_source = 0;
	_target = 0;
	_target_app = 0;

	ClearScope();
}

// Parse a COBOL statement
bool Cobol::ParseStatement(Token *token)
{
	if(token == NULL)
		return false;

	bool exists = false;

	// DATA division
	if(token->Compare("DATA", L"DATA", 4) == true)
		exists = ParseDataDivision(token);
	else
	// ENVIRONMENT division
	if(token->Compare("ENVIRONMENT", L"ENVIRONMENT", 11) == true)
		exists = ParseEnvironmentDivision(token);
	else
	// IDENTIFICATION division
	if(token->Compare("IDENTIFICATION", L"IDENTIFICATION", 14) == true)
		exists = ParseIdentificationDivision(token);
	else
	// WORKING-STORAGE section
	if(token->Compare("WORKING-STORAGE", L"WORKING-STORAGE", 15) == true)
		exists = ParseWorkingStorageSection(token);

	return exists;
}

// DATA DIVISION
bool Cobol::ParseDataDivision(Token *data)
{
	if(data == NULL)
		return false;

	// DIVISION keyword
	Token *division = GetNextWordToken("DIVISION", L"DIVISION", 8);

	Token *dot = GetNextCharToken('.', L'.');

	// Comment for PL/SQL
	if(_target_app == APP_PLSQL)
		Prepend(data, "-- ", L"-- ", 3);

	return true;
}

// ENVIRONMENT DIVISION
bool Cobol::ParseEnvironmentDivision(Token *environment)
{
	if(environment == NULL)
		return false;

	// DIVISION keyword
	Token *division = GetNextWordToken("DIVISION", L"DIVISION", 8);

	Token *dot = GetNextCharToken('.', L'.');

	// Comment for PL/SQL
	if(_target_app == APP_PLSQL)
		Prepend(environment, "-- ", L"-- ", 3);

	return true;
}

// IDENTIFICATION DIVISION
bool Cobol::ParseIdentificationDivision(Token *identification)
{
	if(identification == NULL)
		return false;

	// DIVISION keyword
	Token *division = GetNextWordToken("DIVISION", L"DIVISION", 8);

	Token *dot = GetNextCharToken('.', L'.');

	// Comment for PL/SQL
	if(_target_app == APP_PLSQL)
		Prepend(identification, "-- ", L"-- ", 3);

	// Parse IDENTIFICATION division paragraphs
	while(true)
	{
		Token *next = GetNextToken();

		if(next == NULL)
			break;

		Token *dot = GetNextCharToken('.', L'.');

		// PROGRAM_ID. name
		if(next->Compare("PROGRAM-ID", L"PROGRAM-ID", 10) == true)
		{
			// Program name
			Token *name = GetNextToken();

			Token *dot2 = GetNextCharToken('.', L'.');
			
			if(_target_app == APP_PLSQL)
			{
				Prepend(next, "CREATE OR REPLACE PACKAGE ", L"CREATE OR REPLACE PACKAGE ", 26);
				PrependCopy(next, name);
				Prepend(next, " IS\n", L" IS\n", 4);
				Prepend(next, "-- ", L"-- ", 3);				
			}

			continue;
		}
		else
		// AUTHOR. text
		if(next->Compare("AUTHOR", L"AUTHOR", 6) == true)
		{
			Token *text = GetNextUntilNewlineToken();
			
			if(_target_app == APP_PLSQL)
				Prepend(next, "-- ", L"-- ", 3);				

			continue;
		}
		else
		// DATE-WRITTEN. date.
		if(next->Compare("DATE-WRITTEN", L"DATE-WRITTEN", 12) == true)
		{
			Token *date = GetNextUntilNewlineToken();
			
			if(_target_app == APP_PLSQL)
				Prepend(next, "-- ", L"-- ", 3);				

			continue;
		}
		else
		// DATE-COMPILED. date.
		if(next->Compare("DATE-COMPILED", L"DATE-COMPILED", 13) == true)
		{
			Token *date = GetNextUntilNewlineToken();
			
			if(_target_app == APP_PLSQL)
				Prepend(next, "-- ", L"-- ", 3);				

			continue;
		}
		
		// Not a division paragraph
		PushBack(next);
		break;
	}

	return true;
}

// WORKING-STORAGE SECTION - program static data
bool Cobol::ParseWorkingStorageSection(Token *working_storage)
{
	if(working_storage == NULL)
		return false;

	// SECTION keyword
	Token *section = GetNextWordToken("SECTION", L"SECTION", 7);

	Token *dot = GetNextCharToken('.', L'.');

	// Comment for PL/SQL
	if(_target_app == APP_PLSQL)
		Prepend(working_storage, "-- ", L"-- ", 3);

	ParseDataSection();

	return true;
}

// Parse data section that can contain variable definitions, COPY and INCLUDE commands
bool Cobol::ParseDataSection()
{
	bool exists = false;

	while(true)
	{
		Token *next = GetNextToken();

		if(next == NULL)
			break;

		// Check for a number starting variable or record declaration
		if(next->IsNumeric() == true)
		{
			PushBack(next);
			ParseVariableSection();

			exists = true;
			continue;
		}
		else
		// COPY command
		if(next->Compare("COPY", L"COPY", 4) == true)
		{
			ParseCopyStatement(next);

			exists = true;
			continue;
		}

		// Not a data section command
		PushBack(next);
		break;
	}

	return exists;
}

// Parse variable definitions
bool Cobol::ParseVariableSection(Token *parent_record)
{
	bool exists = false;

	// Level number for the first item
	Token *entry_num = NULL;

	int e_num = 0;

	// Period after the last parsed item
	Token *var_dot = NULL;

	while(true)
	{
		Token *next = GetNextToken();

		if(next == NULL)
			break;

		// Check for a number representing item hierarchy 01, 05, 10 etc.
		if(next->IsNumeric() == true)
		{
			// Check that a higher level is not got
			if(entry_num != NULL)
			{
				int c_num = next->GetInt();

				// If within a record, change period for the previous item to comma for Oracle; it will be removed for last item later
				if(parent_record != NULL && _target_app == APP_PLSQL)
					Token::Change(var_dot, ",", L",", 1);

				// 01 is higher level than 05 i.e.
				if(c_num < e_num)
				{
					PushBack(next);
					break;
				}
			}
			else
			{
				entry_num = next;
				e_num = entry_num->GetInt();
			}

			// Identifier must be next
			Token *var = GetNextToken();

			if(var == NULL)
				break;

			// If period follows the name it means record definition, otherwise variable definition
			Token *dot = GetNextCharToken('.', L'.');

			// Parse the record
			if(dot != NULL)
			{
				if(parent_record == NULL)
					_current_top_record = next;

				ParseRecordDeclaration(parent_record, next, var, dot);
			}
			else
			{
				ParseVariableDeclaration(next, var);

				// Each variable declaration ends with period
				var_dot = GetNextCharToken('.', L'.');
			}
		}
		else
		{
			PushBack(next);
			break;
		}
	}

	return exists;
}

// Record declaration
bool Cobol::ParseRecordDeclaration(Token *parent_record, Token *num, Token *name, Token *dot)
{
	if(name == NULL)
		return false;

	ParseVariableSection(name);

	// Last token should point to period (end of last variable of this record)
	Token *end_dot = GetLastToken();

	// Use TYPE rec IS RECORD in Oracle
	if(_target_app == APP_PLSQL)
	{
		if(parent_record == NULL)
			Token::Change(end_dot, ";", L";", 1);

		Prepend(name, "TYPE ", L"TYPE ", 5);
		Append(name, " IS RECORD (", L" IS RECORD (", 12);
		Token *last = Prepend(end_dot, ")", L")", 1);

		Token::Remove(num);
		Token::Remove(dot);

		// If it is not top level record, move TYPE definition before top level recod
		if(parent_record != NULL)
		{
			Token *last = Prepend(end_dot, ";", L";", 1);

			PrependCopy(_current_top_record, num, last, false);
			Prepend(_current_top_record, "\n\n", L"\n\n", 2);

			// Specify a record variable inside the record instead full nested record definition
			PrependCopy(end_dot, name);
			Prepend(end_dot, " ", L" ", 1);
			PrependCopy(end_dot, name);
			//Append(last, ";", L";", 1);

			Token::Remove(num, last);
		}
	}

	return false;
}

// Parse single variable declaration, not a record
bool Cobol::ParseVariableDeclaration(Token *num, Token *name)
{
	if(name == NULL)
		return false;

	// Variable options
	while(true)
	{
		Token *next = GetNextToken();

		if(next == NULL)
			break;

		// Exit when the options are ended with period
		if(next->Compare('.', L'.') == true)
		{
			PushBack(next);
			break;
		}

		// PIC
		if(next->Compare("PIC", L"PIC", 3) == true)
			ParsePicFormat(next);
	}

	if(_target_app == APP_PLSQL)
		Token::Remove(num);

	return true;
}

// Parse PIC format for variable
bool Cobol::ParsePicFormat(Token *pic)
{
	if(pic == NULL)
		return false;

	// Multiple format items can be specified
	while(true)
	{
		Token *next = GetNextToken();

		if(next == NULL)
			break;

		// Exit when period is faced
		if(next->Compare('.', L'.') == true)
		{
			PushBack(next);
			break;
		}

		// X(n)
		if(next->Compare("X", L"X", 1) == true)
		{
			Token *open = GetNextCharToken('(', L'(');
			Token *num = GetNextToken(open);
			Token *close = GetNextCharToken(')', L')');

			// VARCHAR2 in Oracle
			if(_target_app == APP_PLSQL)
			{
				Token::Change(next, "VARCHAR2", L"VARCHAR2", 8);
				Token::Remove(pic);
			}
		}
		else
		// XXXXX...
		if(next->Compare("X", L"X", 1, 1) == true && next->len > 1)
		{
			TokenStr len("VARCHAR2(", L"VARCHAR2(", 9);
			len.Append(next->len);
			len.Append(")", L")", 1);

			// VARCHAR2(len) in Oracle
			if(_target_app == APP_PLSQL)
			{
				Token::ChangeNoFormat(next, len);
				Token::Remove(pic);
			}
		}
		else
		// 9(n)
		if(next->Compare("9", L"9", 1) == true)
		{
			Token *open = GetNextCharToken('(', L'(');
			Token *num = GetNextToken(open);
			Token *close = GetNextCharToken(')', L')');

			// NUMBER in Oracle
			if(_target_app == APP_PLSQL)
			{
				Token::Change(next, "NUMBER", L"NUMBER", 6);
				Token::Remove(pic);
			}
		}
	}

	return true;
}

// COPY statement
bool Cobol::ParseCopyStatement(Token *copy)
{
	if(copy == NULL)
		return false;

	// Copy book
	Token *name = GetNextToken();

	Token *dot = GetNextCharToken('.', L'.');

	// Comment for PL/SQL
	if(_target_app == APP_PLSQL)
		Prepend(copy, "-- ", L"-- ", 3);

	return true;
}

// Clear the program scope
void Cobol::ClearScope()
{
	_div_scope = 0;
	_current_top_record = NULL;
}

// SqlParser helpers
Token* Cobol::GetNextToken()
{
	if(_sqlparser != NULL) 
		return _sqlparser->GetNextToken(); 
	else 
		return NULL; 
}

Token* Cobol::GetNextToken(Token *prev)
{
	if(_sqlparser != NULL) 
		return _sqlparser->GetNextToken(prev); 
	else 
		return NULL; 
}

Token* Cobol::GetNextCharToken(const char ch, const wchar_t wch)
{ 
	if(_sqlparser != NULL) 
		return _sqlparser->GetNextCharToken(ch, wch); 
	else 
		return NULL; 
}

Token* Cobol::GetNextWordToken(const char *str, const wchar_t *wstr, int len) 
{ 
	if(_sqlparser != NULL) 
		return _sqlparser->GetNextWordToken(str, wstr, len); 
	else 
		return NULL; 
}

Token* Cobol::GetNextUntilNewlineToken()
{ 
	if(_sqlparser != NULL) 
		return _sqlparser->GetNextUntilNewlineToken(); 
	else 
		return NULL; 
}
void Cobol::PushBack(Token *token)
{
	if(_sqlparser != NULL) 
		return _sqlparser->PushBack(token);
}

Token* Cobol::GetLastToken()
{
	if(_sqlparser != NULL) 
		return _sqlparser->GetLastToken();
	else
		return NULL;
}

Token* Cobol::Append(Token *token, const char *str, const wchar_t *wstr, int len, Token *format)
{
	if(_sqlparser != NULL) 
		return _sqlparser->Append(token, str, wstr, len, format);
	else
		return NULL;
}

Token* Cobol::AppendCopy(Token *token, Token *append)
{
	if(_sqlparser != NULL) 
		return _sqlparser->AppendCopy(token, append);

	return NULL;
}

Token* Cobol::Prepend(Token *token, const char *str, const wchar_t *wstr, int len, Token *format)
{
	if(_sqlparser != NULL) 
		return _sqlparser->Prepend(token, str, wstr, len, format);
	else
		return NULL;
}

Token* Cobol::PrependCopy(Token *token, Token *prepend)
{
	if(_sqlparser != NULL) 
		return _sqlparser->PrependCopy(token, prepend);
	else
		return NULL;
}

Token* Cobol::PrependCopy(Token *token, Token *first, Token *last, bool prepend_removed)
{
	if(_sqlparser != NULL) 
		return _sqlparser->PrependCopy(token, first, last, prepend_removed);
	else
		return NULL;
}
