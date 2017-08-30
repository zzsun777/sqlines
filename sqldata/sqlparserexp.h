
// SQLParser exports
// Copyright (c) 2012 SQLines. All rights reserved.

#ifndef sqlines_sqlparserexp_h
#define sqlines_sqlparserexp_h

// SQL dialect types
#define SQL_SQL_SERVER			1
#define SQL_ORACLE				2
#define SQL_DB2					3
#define SQL_MYSQL				4
#define SQL_POSTGRESQL			5
#define SQL_SYBASE				6
#define SQL_INFORMIX			7
#define SQL_GREENPLUM			8

extern void* CreateParserObject();
extern void SetParserTypes(void *parser, short source, short target);
extern int SetParserOption(void *parser, const char *option, const char *value);
extern int ConvertSql(void *parser, const char *input, int size, const char **output, int *out_size, int *lines);
extern void FreeOutput(const char *output);

#endif // sqlines_sqlparserexp_h