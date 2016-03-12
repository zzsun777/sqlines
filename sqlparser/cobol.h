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

// COBOL Class

#ifndef sqlines_cobol_h
#define sqlines_cobol_h

class SqlParser;
class Token;

class Cobol
{
	SqlParser *_sqlparser;

	// Source and target SQL dialects
	short _source;
	short _target;

	// Target application type
	short _target_app;

	// Current top level record
	Token *_current_top_record;

public:
	Cobol();

	// Current division (identification i.e.)
	int _div_scope;

	bool ParseStatement(Token *token);

	bool ParseDataDivision(Token *data);
	bool ParseEnvironmentDivision(Token *environment);
	bool ParseIdentificationDivision(Token *identification);
	bool ParseWorkingStorageSection(Token *working_storage);

	// Parse variable definitions
	bool ParseDataSection();
	bool ParseVariableSection(Token *parent_record = NULL);
	bool ParseRecordDeclaration(Token *parent_record, Token *num, Token *name, Token *dot);
	bool ParseVariableDeclaration(Token *num, Token *name);
	bool ParsePicFormat(Token *pic);

	// Statements
	bool ParseCopyStatement(Token *copy);

	// Clear the program scope
	void ClearScope(); 

	void SetParser(SqlParser *parser) { _sqlparser = parser; }
	void SetTypes(short s, short t, short ta) { _source = s; _target = t; _target_app = ta; }

	// SqlParser helpers
	Token* GetNextToken();
	Token* GetNextToken(Token *prev);
	Token* GetNextCharToken(const char ch, const wchar_t wch);
	Token* GetNextWordToken(const char *str, const wchar_t *wstr, int len);
	Token* GetNextUntilNewlineToken();

	void PushBack(Token *token);
	Token *GetLastToken();

	Token* Append(Token *token, const char *str, const wchar_t *wstr, int len, Token *format = NULL);
	Token* AppendCopy(Token *token, Token *append);

	Token* Prepend(Token *token, const char *str, const wchar_t *wstr, int len, Token *format = NULL);
	Token* PrependCopy(Token *token, Token *prepend);
	Token* PrependCopy(Token *token, Token *first, Token *last, bool prepend_removed = true);
};

#endif // sqlines_cobol_h
