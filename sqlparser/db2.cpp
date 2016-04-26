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

// DB2 clauses

#include <stdio.h>
#include <string.h>
#include "sqlparser.h"

// DB2 identity options in GENERATED ALWAYS or BY DEFAULT AS IDENTITY
bool SqlParser::ParseDb2GeneratedClause(Token *create, Token *table_name, Token *column, Token *generated,
									Token **id_col, Token **id_start, Token **id_inc, bool *id_default)
{
	if(generated == NULL)
		return false;

	Token *always = GetNextWordToken("ALWAYS", L"ALWAYS", 6);
	Token *by = NULL;
	Token *default_ = NULL;

	if(always == NULL)
	{
		by = GetNextWordToken("BY", L"BY", 2);
		default_ = GetNextWordToken("DEFAULT", L"DEFAULT", 7);

		if(id_default != NULL)
			*id_default = true;
	}
	else
	{
		if(id_default != NULL)
			*id_default = false;
	}
	
	Token *as = GetNextWordToken("AS", L"AS", 2);
	Token *identity = GetNextWordToken("IDENTITY", L"IDENTITY", 8);

	// Identity parameters are optional
	Token *open = GetNextCharToken('(', L'(');
	Token *close = NULL;

	Token *start_with = NULL;
	Token *increment_by = NULL;

	while(true)
	{
		bool exists = false;

		if(open == NULL)
			break;

		Token *option = GetNextToken();

		if(option == NULL)
			break;

		// START WITH
		if(option->Compare("START", L"START", 5) == true)
		{
			/*Token *with */ (void) GetNextWordToken("WITH", L"WITH", 4);
			start_with = GetNextNumberToken();

			exists = true;
			continue;
		}
		else
		// INCREMENT BY
		if(option->Compare("INCREMENT", L"INCREMENT", 9) == true)
		{
			/*Token *by */ (void) GetNextWordToken("BY", L"BY", 2);
			increment_by = GetNextNumberToken();

			exists = true;
			continue;
		}
		else
		// MINVALUE
		if(option->Compare("MINVALUE", L"MINVALUE", 8) == true)
		{
			/*Token *value */ (void) GetNextNumberToken();

			exists = true;
			continue;
		}
		else
		// MAXVALUE
		if(option->Compare("MAXVALUE", L"MAXVALUE", 8) == true)
		{
			/*Token *value */ (void) GetNextNumberToken();

			exists = true;
			continue;
		}
		else
		// NO CYCLE and NO ORDER
		if(option->Compare("NO", L"NO", 2) == true)
		{
			/*Token *attr */ (void) GetNextToken();

			exists = true;
			continue;
		}
		else
		// CACHE
		if(option->Compare("CACHE", L"CACHE", 5) == true)
		{
			/*Token *value */ (void) GetNextNumberToken();

			exists = true;
			continue;
		}
		else
		// CYCLE
		if(option->Compare("CYCLE", L"CYCLE", 5) == true)
		{
			exists = true;
			continue;
		}
		else
		// Looks like comma is optional
		if(option->Compare(',', L',') == true)
		{
			exists = true;
			continue;
		}

		PushBack(option);
		break;
	}

	if(open != NULL)
		close = GetNextCharToken(')', L')');
	
	// IDENTITY(start, inc) in SQL Server
	if(_target == SQL_SQL_SERVER)
	{
		Token::Change(generated, "IDENTITY(", L"IDENTITY(", 9);
		AppendCopy(generated, start_with);

		if(increment_by != NULL)
		{
			Append(generated, ", ", L", ", 2);
			AppendCopy(generated, increment_by);
		}

		Append(generated, ")", L")", 1);

		if(always != NULL)
			Token::Remove(always, close);
		else
			Token::Remove(by, close);
	}
	else
	// Use a sequence and DEFAULT nextval for PostgreSQL and Greenplum
	if(_target == SQL_POSTGRESQL || _target == SQL_GREENPLUM)
	{
		TokenStr seq_name(table_name);

		AppendIdentifier(seq_name, "_seq", L"_seq", 4);
		
		// Generate CREATE SEQUENCE before CREATE TABLE
		Prepend(create, "CREATE SEQUENCE ", L"CREATE SEQUENCE ", 16);
		PrependNoFormat(create, &seq_name);

		if(start_with != NULL)
		{
			Prepend(create, " START WITH ", L" START WITH ", 12);
			PrependCopy(create, start_with);
		}

		if(increment_by != NULL)
		{
			Prepend(create, " INCREMENT BY ", L" INCREMENT BY ", 14);
			PrependCopy(create, increment_by);
		}

		Prepend(create, ";\n\n", L";\n\n", 3);

		// Generate DEFAULT nextval('tablename_seq') clause
		Token::Change(generated, "DEFAULT ", L"DEFAULT ", 8);
		Append(generated, "nextval ('", L"nextval ('", 10);

		AppendNoFormat(generated, &seq_name);
		Append(generated, "')", L"')", 2);

		if(always != NULL)
			Token::Remove(always, close);
		else
			Token::Remove(by, close);
	}
	else
	// Remove for other databases
	if(_target != SQL_DB2)
	{
		Token::Remove(generated);

		Token::Remove(always);
		Token::Remove(by, default_);
		Token::Remove(as, identity);
		
		Token::Remove(open, close);
	}

	if(id_col != NULL)
		*id_col = column; 

	if(id_start != NULL)
		*id_start = start_with;
	
	if(id_inc != NULL)
		*id_inc = increment_by;

	return true;
}

// DB2 CREATE INDEX storage options
bool SqlParser::ParseDb2CreateIndexOptions()
{
	bool exists = false;

	while(true)
	{
		Token *next = GetNextToken();

		if(next == NULL)
			break;

		// DB2 z/OS USING STOGROUP
		if(next->Compare("USING", L"USING", 5) == true)
		{
			Token *stogroup = GetNextWordToken("STOGROUP", L"STOGROUP", 8);

			if(stogroup != NULL)
				ParseDb2StogroupClause(NULL, next, stogroup, SQL_SCOPE_INDEX);

			exists = true;
			continue;
		}
		else
		// DB2 CLUSTER option
		if(next->Compare("CLUSTER", L"CLUSTER", 7) == true)
		{
			if(_target != SQL_DB2)
				Token::Remove(next);

			exists = true;
			continue;
		}
		else
		// DB2 for z/OS NOT CLUSTER, NOT PADDED 
		if(next->Compare("NOT", L"NOT", 3) == true)
		{
			Token *cluster = GetNextWordToken("CLUSTER", L"CLUSTER", 7);
			Token *padded = NULL;

			if(cluster == NULL)
				padded = GetNextWordToken("PADDED", L"PADDED", 6);

			if(cluster != NULL || padded != NULL)
			{
				if(_target != SQL_DB2)
				{
					if(cluster != NULL)
						Token::Remove(next, cluster);
					else
					if(padded != NULL)
						Token::Remove(next, padded);
				}

				exists = true;
				continue;
			}
		}
		else
		// DB2 COMPRESS NO | YES option
		if(next->Compare("COMPRESS", L"COMPRESS", 8) == true)
		{
			Token *value = GetNextToken();

			if(_target != SQL_DB2)
				Token::Remove(next, value);

			exists = true;
			continue;
		}
		else
		// DB2 ALLOW REVERSE SCAN
		if(next->Compare("ALLOW", L"ALLOW", 5) == true)
		{
			/*Token *reverse */ (void) GetNextWordToken("REVERSE", L"REVERSE", 7);
			Token *scan = GetNextWordToken("SCAN", L"SCAN", 4);
			
			if(_target != SQL_DB2)
				Token::Remove(next, scan);

			exists = true;
			continue;
		}
		else
		// DB2 PCTFREE
		if(next->Compare("PCTFREE", L"PCTFREE", 7) == true)
		{
			Token *value = GetNextToken();

			int int_value = (value != NULL) ? value->GetInt() : 0;
			
			// WITH (FILLFACTOR = 100 - val) in Greenplum
			if(_target == SQL_GREENPLUM)
			{
				Token::Change(next, "WITH (FILLFACTOR =", L"WITH (FILLFACTOR =", 18);
				Token::Change(value, 100 - int_value);

				Append(value, ")", L")", 1);
			}
			else
			// Oracle supports PCTFREE
			if(Target(SQL_ORACLE, SQL_DB2) == false)
				Token::Remove(next, value);

			exists = true;
			continue;
		}
		else
		// DB2 MINPCTUSED
		if(next->Compare("MINPCTUSED", L"MINPCTUSED", 10) == true)
		{
			Token *value = GetNextToken();

			if(_target != SQL_DB2)
				Token::Remove(next, value);

			exists = true;
			continue;
		}
		else
		// DB2 z/OS FREEPAGE num
		if(next->Compare("FREEPAGE", L"FREEPAGE", 8) == true)
		{
			Token *value = GetNextToken();

			if(_target != SQL_DB2)
				Token::Remove(next, value);

			exists = true;
			continue;
		}
		else
		// DB2 z/OS BUFFERPOOL name
		if(next->Compare("BUFFERPOOL", L"BUFFERPOOL", 10) == true)
		{
			Token *name = GetNextToken();

			if(_target != SQL_DB2)
				Token::Remove(next, name);

			exists = true;
			continue;
		}
		else
		// DB2 SPECIFICATION ONLY (index will be commented)
		if(next->Compare("SPECIFICATION", L"SPECIFICATION", 13) == true)
		{
			/*Token *only */ (void) GetNextWordToken("ONLY", L"ONLY", 4);

			exists = true;
			continue;
		}
		else
		// DB2 z/OS GBPCACHE CHANGED | ALL | NONE  
		if(next->Compare("GBPCACHE", L"GBPCACHE", 8) == true)
		{
			Token *value = GetNextToken();

			if(value != NULL)
			{
				if(_target != SQL_DB2)
					Token::Remove(next, value);

				exists = true;
				continue;
			}
		}
		else
		// DB2 z/OS CLOSE NO | YES  
		if(next->Compare("CLOSE", L"CLOSE", 5) == true)
		{
			Token *value = GetNextToken();

			if(value != NULL)
			{
				if(_target != SQL_DB2)
					Token::Remove(next, value);

				exists = true;
				continue;
			}
		}
		else
		// DB2 z/OS COPY YES | NO  
		if(next->Compare("COPY", L"COPY", 4) == true)
		{
			Token *value = GetNextToken();

			if(value != NULL)
			{
				if(_target != SQL_DB2)
					Token::Remove(next, value);

				exists = true;
				continue;
			}
		}
		else
		// DB2 z/OS PIECESIZE num [K | M | G]
		if(next->Compare("PIECESIZE", L"PIECESIZE", 9) == true)
		{
			Token *num = GetNextToken();

			if(num != NULL)
			{
				// Size can be optionally followed by specifier K, M or G
				Token *spec = Db2SizeSpecifier();

				if(_target != SQL_DB2)
				{
					Token::Remove(next, num);
					Token::Remove(spec);
				}

				exists = true;
				continue;
			}
		}
		else
		// DB2 for z/OS PADDED option
		if(next->Compare("PADDED", L"PADDED", 6) == true)
		{
			if(_target != SQL_DB2)
				Token::Remove(next);

			exists = true;
			continue;
		}
		
		// Not an index clause
		PushBack(next);

		break;
	}

	return exists;
}

// DB2 CREATE TABLESPACE
bool SqlParser::ParseDb2CreateTablespace(Token *create, Token *tablespace)
{
	if(create == NULL || tablespace == NULL)
		return false;

	// Tablespace name
	Token *name = GetNextToken();

	if(name == NULL)
		return false;

	bool exists = false;

	while(true)
	{
		Token *next = GetNextToken();

		if(next == NULL)
			break;

		// IN database
		if(next->Compare("IN", L"IN", 2) == true)
		{
			Token *database = GetNextToken();

			if(database != NULL)
			{
				if(_target != SQL_DB2)
					Token::Remove(next, database);

				exists = true;
				continue;
			}
		}
		else
		// DB2 z/OS USING STOGROUP
		if(next->Compare("USING", L"USING", 5) == true)
		{
			Token *stogroup = GetNextWordToken("STOGROUP", L"STOGROUP", 8);

			if(stogroup != NULL)
				ParseDb2StogroupClause(name, next, stogroup, SQL_SCOPE_TABLESPACE);

			exists = true;
			continue;
		}
		else
		// DB2 COMPRESS NO | YES option
		if(next->Compare("COMPRESS", L"COMPRESS", 8) == true)
		{
			Token *value = GetNextToken();

			if(_target != SQL_DB2)
				Token::Remove(next, value);

			exists = true;
			continue;
		}
		else
		// DB2 PCTFREE
		if(next->Compare("PCTFREE", L"PCTFREE", 7) == true)
		{
			Token *value = GetNextToken();

			// Oracle does not support PCTFREE in CREATE TABLESPACE
			if(_target != SQL_DB2)
				Token::Remove(next, value);

			exists = true;
			continue;
		}
		else
		// DB2 z/OS FREEPAGE num
		if(next->Compare("FREEPAGE", L"FREEPAGE", 8) == true)
		{
			Token *value = GetNextToken();

			if(_target != SQL_DB2)
				Token::Remove(next, value);

			exists = true;
			continue;
		}
		else
		// DB2 z/OS BUFFERPOOL name
		if(next->Compare("BUFFERPOOL", L"BUFFERPOOL", 10) == true)
		{
			Token *name = GetNextToken();

			if(_target != SQL_DB2)
				Token::Remove(next, name);

			exists = true;
			continue;
		}
		else
		// DB2 z/OS GBPCACHE CHANGED | ALL | SYSTME | NONE  
		if(next->Compare("GBPCACHE", L"GBPCACHE", 8) == true)
		{
			Token *value = GetNextToken();

			if(value != NULL)
			{
				if(_target != SQL_DB2)
					Token::Remove(next, value);

				exists = true;
				continue;
			}
		}
		else
		// DB2 z/OS CLOSE NO | YES  
		if(next->Compare("CLOSE", L"CLOSE", 5) == true)
		{
			Token *value = GetNextToken();

			if(value != NULL)
			{
				if(_target != SQL_DB2)
					Token::Remove(next, value);

				exists = true;
				continue;
			}
		}
		else
		// DB2 z/OS COPY YES | NO  
		if(next->Compare("COPY", L"COPY", 4) == true)
		{
			Token *value = GetNextToken();

			if(value != NULL)
			{
				if(_target != SQL_DB2)
					Token::Remove(next, value);

				exists = true;
				continue;
			}
		}
		else
		// DB2 z/OS PIECESIZE num [K | M | G]
		if(next->Compare("PIECESIZE", L"PIECESIZE", 9) == true)
		{
			Token *num = GetNextToken();

			if(num != NULL)
			{
				// Size can be optionally followed by specifier K, M or G
				Token *spec = Db2SizeSpecifier();

				if(_target != SQL_DB2)
				{
					Token::Remove(next, num);
					Token::Remove(spec);
				}

				exists = true;
				continue;
			}
		}
		else
		// DB2 z/OS LOGGED option
		if(next->Compare("LOGGED", L"LOGGED", 6) == true)
		{
			if(_target == SQL_ORACLE)
				Token::Change(next, "LOGGING", L"LOGGING", 7);
			else
			if(_target != SQL_DB2)
				Token::Remove(next);

			exists = true;
			continue;
		}
		else
		// DB2 for z/OS NOT LOGGED
		if(next->Compare("NOT", L"NOT", 3) == true)
		{
			Token *logged = GetNextWordToken("LOGGED", L"LOGGED", 6);

			if(logged != NULL)
			{
				if(_target == SQL_ORACLE)
				{
					Token::Change(next, "NOLOGGING", L"NOLOGGING", 9);
					Token::Remove(logged);
				}
				else
				if(_target != SQL_DB2)
					Token::Remove(next, logged);

				exists = true;
				continue;
			}
		}
		else
		// DB2 z/OS TRACKMOD NO | YES  
		if(next->Compare("TRACKMOD", L"TRACKMOD", 8) == true)
		{
			Token *value = GetNextToken();

			if(value != NULL)
			{
				if(_target != SQL_DB2)
					Token::Remove(next, value);

				exists = true;
				continue;
			}
		}
		else
		// DB2 z/OS SEGSIZE num  
		if(next->Compare("SEGSIZE", L"SEGSIZE", 7) == true)
		{
			Token *num = GetNextToken();

			if(num != NULL)
			{
				if(_target != SQL_DB2)
					Token::Remove(next, num);

				exists = true;
				continue;
			}
		}
		else
		// DB2 z/OS LOCKSIZE ANY | TABLESPACE | TABLE | PAGE | ROW | LOB
		if(next->Compare("LOCKSIZE", L"LOCKSIZE", 8) == true)
		{
			Token *value = GetNextToken();

			if(value != NULL)
			{
				if(_target != SQL_DB2)
					Token::Remove(next, value);

				exists = true;
				continue;
			}
		}
		else
		// DB2 for z/OS LOCKMAX num | SYSTEM
		if(next->Compare("LOCKMAX", L"LOCKMAX", 7) == true)
		{
			Token *value = GetNextToken();

			if(value != NULL)
			{
				if(_target != SQL_DB2)
					Token::Remove(next, value);

				exists = true;
				continue;
			}
		}
		else
		// CCSID ASCII | UNICODE | EBCDIC
		if(next->Compare("CCSID", L"CCSID", 5) == true)
		{
			Token *encoding = GetNextToken();
			
			if(_target != SQL_DB2)
				Token::Remove(next, encoding);

			exists = true;
			continue;
		}
		else
		// DB2 for z/OS MAXROWS num 
		if(next->Compare("MAXROWS", L"MAXROWS", 7) == true)
		{
			Token *num = GetNextToken();
			
			if(_target != SQL_DB2)
				Token::Remove(next, num);

			exists = true;
			continue;
		}

		PushBack(next);
		break;
	}

	return exists;
}

// DB2 z/OS USING STOGROUP clause
bool SqlParser::ParseDb2StogroupClause(Token *objname, Token *using_, Token *stogroup, int scope)
{
	if(using_ == NULL || stogroup == NULL)
		return false;

	// STOGROUP name
	Token *name = GetNextToken();

	if(name == NULL)
		return false;

	bool exists = false;

	// STOGROUP options
	while(true)
	{
		Token *next = GetNextToken();

		if(next == NULL)
			break;

		// PRIQTY num
		if(next->Compare("PRIQTY", L"PRIQTY", 6) == true)
		{
			Token *num = GetNextToken();

			if(_target != SQL_DB2)
				Token::Remove(next, num);

			exists = true;
			continue;
		}
		else
		// SECQTY num
		if(next->Compare("SECQTY", L"SECQTY", 6) == true)
		{
			Token *num = GetNextToken();

			if(_target != SQL_DB2)
				Token::Remove(next, num);

			exists = true;
			continue;
		}
		else
		// ERASE NO | YES
		if(next->Compare("ERASE", L"ERASE", 5) == true)
		{
			Token *value = GetNextToken();

			if(_target != SQL_DB2)
				Token::Remove(next, value);

			exists = true;
			continue;
		}
	
		PushBack(next);
		break;
	}

	// Set DATAFILE clause in CREATE TABLESPACE for Oracle
	if(_target == SQL_ORACLE && scope == SQL_SCOPE_TABLESPACE)
	{
		Token::Change(using_, "DATAFILE '", L"DATAFILE '", 10);

		// Use tablespace name as multiple tablespaces can be created in a single storage group
		AppendCopy(using_, objname);
		Append(using_, ".dbf'", L".dbf'", 5);

		Token::Remove(stogroup);
		Token::Remove(name);
	}
	else
	if(_target != SQL_DB2)
		Token::Remove(using_, name);

	return exists;
}

// DB2 FETCH FIRST n ROWS ONLY clause in SELECT statement
bool SqlParser::ParseDb2FetchFirstRowOnly(Token *fetch, Token **rowlimit_soptions, int *rowlimit)
{
	if(fetch == NULL || Token::Compare(fetch, "FETCH", L"FETCH", 5) == false)
		return false;

	Token *first = GetNextWordToken("FIRST", L"FIRST", 5);

	if(first == NULL)
		return false;

	// Optional number of rows, meaning 1 row if skipped
	Token *num = GetNextToken();
	Token *rows = NULL;

	if(num == NULL)
		return false;

	// Check if num already points to ROW or ROWS
	if(num->Compare("ROW", L"ROW", 3) == true || num->Compare("ROWS", L"ROWS", 4) == true)
	{
		rows = num;
		num = NULL;
	}

	// ROWS or ROW
	if(rows == NULL)
		rows = GetNextToken();

	Token *only = GetNextWordToken("ONLY", L"ONLY", 4);

	if(only == NULL)
		return false;

	// Set rowlimit to output
	if(num == NULL && rowlimit != NULL)
		*rowlimit = 1;
	else
	if(num != NULL && rowlimit_soptions != NULL)
		*rowlimit_soptions = num;

	// ROWNUM is used in Oracle
	if(_target == SQL_ORACLE)
	{
		Token::Remove(fetch);
		Token::Remove(first, only);
	}

	return true;
}

// DB2 CREATE DATABASE statement
bool SqlParser::Db2CreateDatabase(Token *create, Token * /*database*/, Token * /*name*/)
{
	bool exists = false;

	// CREATE DATABASE options
	while(true)
	{
		Token *next = GetNextToken();

		if(next == NULL)
			break;

		// BUFFERPOOL name
		if(next->Compare("BUFFERPOOL", L"BUFFERPOOL", 10) == true)
		{
			Token *name = GetNextToken();

			if(Target(SQL_DB2, SQL_ORACLE) == false)
				Token::Remove(next, name);

			exists = true;
			continue;
		}
		else
		// INDEXBP name
		if(next->Compare("INDEXBP", L"INDEXBP", 7) == true)
		{
			Token *name = GetNextToken();

			if(Target(SQL_DB2, SQL_ORACLE) == false)
				Token::Remove(next, name);

			exists = true;
			continue;
		}
		else
		// STOGROUP name
		if(next->Compare("STOGROUP", L"STOGROUP", 8) == true)
		{
			Token *name = GetNextToken();

			if(Target(SQL_DB2, SQL_ORACLE) == false)
				Token::Remove(next, name);

			exists = true;
			continue;
		}
		else
		// CCSID ASCII | EBCDIC | UNICODE
		if(next->Compare("CCSID", L"CCSID", 5) == true)
		{
			Token *code = GetNextToken();

			if(Target(SQL_DB2, SQL_ORACLE) == false)
				Token::Remove(next, code);

			exists = true;
			continue;
		}
	
		PushBack(next);
		break;
	}

	// Comment the entire statement for Oracle
	if(_target == SQL_ORACLE)
	{
		Token *end = GetLastToken(GetNextCharToken(';', L';'));
		Comment(create, end);
	}

	return exists;
}

// In DB2 a number can be followed by size specifier K, M or G
Token* SqlParser::Db2SizeSpecifier()
{
	Token *spec = GetNextToken();

	if(spec == NULL)
		return NULL;

	if(spec->Compare("K", L"K", 1) == false && spec->Compare("M", L"M", 1) == false &&
		spec->Compare("G", L"G", 1) == false)
	{
		PushBack(spec);
		return NULL;
	}

	return spec;
}

// DB2 for z/OS COMMENT ON tab (col IS 'comment' , ...)
bool SqlParser::ParseDb2Comment(Token *comment, Token *on, Token *name)
{
	if(comment == NULL)
		return false;

	Token *open = GetNextCharToken('(', L'(');

	if(open == NULL)
		return false;

	int num = 1;

	// Multipe comma separated comments can be specified
	while(true)
	{
		Token *col = GetNextIdentToken();

		if(col == NULL)
			break;

		// IS keyword must follow now
		Token *is = GetNextWordToken("IS", L"IS", 2);

		if(is == NULL)
			break;

		// Comment text
		Token *text = GetNextToken();

		if(_target == SQL_SQL_SERVER)
		{
			Append(text, ", 'table', ", L", 'table', ", 11);
			AppendCopy(text, name);
			Append(text, ", 'column', ", L", 'column', ", 12);
			AppendCopy(text, col);

			Token::Remove(col);
			Token::Remove(is);
		}
		else
		if(_target == SQL_ORACLE)
		{
			if(num == 1)
			{
				Append(on, " COLUMN", L" COLUMN", 7);
				Append(name, ".", L".", 1);
				AppendCopy(name, col);

				Token::Remove(col);
			}
		}

		Token *comma = GetNextCharToken(',', L',');

		if(comma == NULL)
			break;

		num++;
	}

	Token *close = GetNextCharToken(')', L')');

	if(_target == SQL_SQL_SERVER)
	{
		Token::Change(comment, "EXECUTE", L"EXECUTE", 7);
		AppendNoFormat(comment, " sp_addextendedproperty 'Comment',", L" sp_addextendedproperty 'Comment',", 34); 

		Token::Remove(on);
		Token::Remove(name);
		Token::Remove(open);
		Token::Remove(close);
	}
	else
	if(_target == SQL_ORACLE)
	{
		Token::Remove(open);
		Token::Remove(close);
	}

	return true;
}

// Convert DB2 identifier
bool SqlParser::ConvertDb2Identifier(Token *token, int expected_type)
{
	// Column, variable or parameter name
	if(expected_type == SQL_IDENT_COLVAR)
	{
		// Check for new.column in a trigger
		if(_spl_scope == SQL_SCOPE_TRIGGER && Token::Compare(token, "new.", L"new.", 0, 4) == true)
		{
			// Use :new.column in Oracle
			if(_target == SQL_ORACLE)
				PrependNoFormat(token, ":", L":", 1);
		}
	}

	return true;
}

// In DB2 trigger, column can be assigned without NEW correlation name
void SqlParser::Db2TriggerCorrelatedName(Token *var)
{
	if(var == NULL)
		return;

	bool exists = false;

	// Do not convert if the target name already set
	if(var->t_len > 0)
		return;

	if((_spl_new_correlation_name == NULL && Token::Compare(var, "NEW.", L"NEW.", 0, 4) == true) ||
		(Token::Compare(var, _spl_new_correlation_name, 3) == true && Token::Compare(var, ".", L".", 3, 1) == true))
		exists = true;

	if(exists)
		return;

	// In Oracle correlation name must be specified
	if(_target == SQL_ORACLE)
	{
		PrependNoFormat(var, ":", L":", 1);

		if(_spl_new_correlation_name != NULL)
			PrependCopy(var, _spl_new_correlation_name);
		else
			Prepend(var, "NEW", L"NEW", 3);

		PrependNoFormat(var, ".", L".", 1);
	}
}

// Parse DB2 partitioning clause
bool SqlParser::ParseDb2PartitioningClause(Token *partition, Token *by)
{
	if(partition == NULL || by == NULL)
		return false;

	// RANGE is optional in DB2 z/OS
	Token *range = GetNextWordToken("RANGE", L"RANGE", 5);

	// RANGE keyword is required for Oracle
	if(range == NULL && _target == SQL_ORACLE)
		Append(by, " RANGE", L" RANGE", 6);

	/*Token *open */ (void) GetNextCharToken('(', L'(');

	// Comma separated list of columns
	while(true)
	{
		Token *column = GetNextToken();

		if(column == NULL)
			break;

		Token *comma = GetNextCharToken(',', L',');

		if(comma == NULL)
			break;
	}

	/*Token *close */ (void) GetNextCharToken(')', L')');

	/*Token *open2 */ (void) GetNextCharToken('(', L'(');

	// Comma separated list of partition definitions
	while(true)
	{
		// PARTITION keyword
		Token *partition2 = GetNextWordToken("PARTITION", L"PARTITION", 9);

		if(partition2 == NULL)
			break;

		// Partition number
		Token *num = GetNextToken();

		if(num == NULL)
			break;

		// In Oracle a partition name is required so prepend 'p' to each number
		if(_target == SQL_ORACLE)
			PrependNoFormat(num, "p", L"p", 1);

		// ENDING [AT]
		Token *ending = GetNextWordToken("ENDING", L"ENDING", 6);
		Token *at = GetNextWordToken("AT", L"AT", 2);

		// VALUES LESS THAN in Oracle
		if(_target == SQL_ORACLE)
		{
			Token::Change(ending, "VALUES LESS THAN", L"VALUES LESS THAN", 16);
			Token::Remove(at);
		}

		// Each value is in ()
		/*Token *open3 */ (void) GetNextCharToken('(', L'(');

		/*Token *limit */ (void) GetNextToken();

		/*Token *close3 */ (void) GetNextCharToken(')', L')');
		
		Token *comma = GetNextCharToken(',', L',');

		if(comma == NULL)
			break;
	}

	/*Token *close2 */ (void) GetNextCharToken(')', L')');

	return true;
}

// Various SET options such as SET CURRENT SCHEMA
bool SqlParser::ParseDb2SetOptions(Token *set)
{
	bool exists = false;

	if(set == NULL)
		return false;

	// CURRENT is optional (SET SCHEMA is allowed)
	Token *current = GetNextWordToken("CURRENT", L"CURRENT", 7);

	Token *option = GetNextToken();

	if(option == NULL)
	{
		PushBack(current);
		return false;
	}

	// SET [CURRENT] PATH = list
	if(option->Compare("PATH", L"PATH", 4) == true)
	{
		// Optional =
		/*Token *equal */ (void) GetNextCharToken('=', L'=');

		// Comma-separated list of values
		while(true)
		{
			// Schema name
			/*Token *name */ (void) GetNextToken();

			Token *comma = GetNextCharToken(',', L',');

			if(comma == NULL)
				break;
		}

		// Remove the statement in Oracle
		if(_target == SQL_ORACLE)
			Token::Remove(set, Nvl(GetNextCharToken(';', L';'), GetLastToken()));

		exists = true;
	}
	else
	// SET [CURRENT] SCHEMA = name
	if(option->Compare("SCHEMA", L"SCHEMA", 6) == true)
	{
		// Optional =
		Token *equal = GetNextCharToken('=', L'=');

		// Schema name
		/*Token *name */ (void) GetNextToken();

		// ALTER SESSION SET CURRENT_SCHEMA = name in Oracle
		if(_target == SQL_ORACLE)
		{
			Prepend(set, "ALTER SESSION ", L"ALTER SESSION ", 14);
			
			Token::Remove(current);
			Token::Change(option, "CURRENT_SCHEMA", L"CURRENT_SCHEMA", 14);

			if(equal == NULL)
				AppendNoFormat(option, " =", L" =", 2);
		}

		exists = true;
	}
	// Not a SET option
	else
	{
		PushBack(option);
		PushBack(current);
	}

	return exists;
}

// VALUES NEXTVAL seq INTO var pattern
bool SqlParser::Db2ValuesNextValIntoPattern(Token *values)
{
	if(values == NULL || _target != SQL_ORACLE)
		return false;

	// NEXTVAL keyword
	Token *nextval = GetNextWordToken("NEXTVAL", L"NEXTVAL", 7);

	if(nextval == NULL)
		return false;

	// FOR keyword
	Token *for_ = GetNextWordToken("FOR", L"FOR", 3);

	// Sequence name
	Token *seq_name = GetNextToken(for_);

	// INTO keyword
	Token *into = GetNextWordToken(seq_name, "INTO", L"INTO", 4);

	// Variable name
	Token *var = GetNextToken(into);

	if(var == NULL)
	{
		PushBack(nextval);
		return false;
	}

	// SELECT seq.NEXTVAL INTO var FROM dual in Oracle
	if(_target == SQL_ORACLE)
	{
		Token::Change(values, "SELECT", L"SELECT", 6);

		PrependCopy(nextval, seq_name);
		PrependNoFormat(nextval, ".", L".", 1);

		Token::Remove(for_, seq_name);

		Append(var, " FROM ", L" FROM ", 6, values);
		AppendNoFormat(var, "dual", L"dual", 4);
	}

	return true;
}

// DB2 RESULT_SET_LOCATOR declaration
bool SqlParser::ParseDb2ResultSetLocatorDeclaration(Token *declare, Token *name, Token *result_set_locator)
{
	if(result_set_locator == NULL)
		return false;

	// VARYING keyword
	/*Token *varying */ (void) GetNextWordToken("VARYING", L"VARYING", 7);

	_spl_declared_rs_locators.Add(name);

	// Remove the entire declaration for other databases
	if(_target != SQL_DB2)
		Token::Remove(declare, Nvl(GetNextCharToken(';', L';'), GetLastToken()));

	return true;
}

// Parse specific DB2 AND syntax: (c1, c2, ...) = (v1, v2, ...)
bool SqlParser::ParseDb2AndBooleanExpression(Token *open)
{
	if(open == NULL)
		return false;

	Token *next = GetNextToken();

	if(next == NULL)
		return false;

	// Comma must follow after the first token
	Token *comma = GetNextCharToken(',', L',');

	if(comma == NULL)
	{
		PushBack(next);
		return false;
	}

	ListWM first;
	first.Add(next, comma);

	// Select left columns of AND expressions
	while(true)
	{
		Token *nextn = GetNextToken();

		if(next == NULL)
			break;

		Token *comma = GetNextCharToken(',', L',');

		first.Add(nextn, comma);

		if(comma == NULL)
			break;
	}

	Token *close = GetNextCharToken(')', L')');

	// = before list of right AND expressions
	Token *equal = GetNextCharToken('=', L'=');

	if(equal == NULL)
	{
		PushBack(next);
		return false;
	}
	
	Token *open2 = GetNextCharToken('(', L'(');

	ListwmItem *i = first.GetFirst();

	// Select right columns of AND expressions
	while(true)
	{
		Token *nextn = GetNextToken();

		if(next == NULL)
			break;

		Token *comma = GetNextCharToken(',', L',');

		Token *f_col = NULL;
		Token *f_comma = NULL;
		Token *f_append = NULL;

		if(i != NULL)
		{
			f_col = (Token*)i->value;
			f_comma = (Token*)i->value2;

			f_append = Nvl(f_comma, f_col);
		}

		// Oracle does not support this syntax, use c1 = v1 AND ...
		if(_target == SQL_ORACLE)
		{
			AppendNoFormat(f_append, " = ", L" = ", 3);
			AppendCopy(f_append, nextn);

			if(comma != NULL)
				AppendNoFormat(f_append, " AND", L" AND", 4);

			Token::Remove(f_comma);
			Token::Remove(nextn);
			Token::Remove(comma);
		}

		if(comma == NULL)
			break;

		if(i != NULL)
			i = i->next;
	}

	if(_target == SQL_ORACLE)
	{
		Token::Remove(close);
		Token::Remove(equal);
		Token::Remove(open2);
	}

	// Note: function mustn't parse closing ), it processed by the caller
	return true;
}

// MODIFIED BY option in EXPORT command
bool SqlParser::Db2ParseModifiedByOptions(Token **colsep_out)
{
	bool exists = false;

	Token *start = NULL;

	// Options
	while(true)
	{
		Token *next = GetNextToken();

		if(next == NULL)
			break;

		if(start == NULL)
			start = next;

		// COLDELx where x is a single characater column delimiter
		if(next->Compare("COLDEL", L"COLDEL", 0, 6) == true)
		{
			Token *del = NULL;

			// x is a special punctuation character it is selected as a separate token
			if(next->len == 6)
			{
				del = GetNextToken();

				if(colsep_out != NULL)
					*colsep_out = del;
			}

			exists = true;
		}
		else
		// NOCHARDEL
		if(next->Compare("NOCHARDEL", L"NOCHARDEL", 9) == true)
		{
			exists = true;
		}
		// Unknown option
		else
		{
			PushBack(next);
			break;
		}
	}

	if(exists == true)
	{
		// Remove options
		if(_target == SQL_ORACLE)
			Token::Remove(start, GetLastToken());
	}

	return exists;
}

// FOR COLUMN system_name in OS/400 to specify 10-char system name to access by old apps (like 8-char file name in Windows for DOS apps)
bool SqlParser::ParseDb2ForColumn()
{
	bool exists = false;

	Token *for_ = GetNextWordToken("FOR", L"FOR", 3);
			
	if(for_ == NULL)
		return false;

	// COLUMN keyword is optional
	/*Token *column */ (void) GetNextWordToken("COLUMN", L"COLUMN", 6);

	Token *system_name = GetNextToken();

	if(system_name != NULL)
	{
		Token::Remove(for_, system_name);
		exists = true;
	}

	return exists;
}
