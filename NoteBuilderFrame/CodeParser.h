#ifndef _CODEPARSER_H_
#define	_CODEPARSER_H_

#pragma once

#define	NAME_SIZE	64
#define	MACRO_VALUE_SIZE	128
#define	MAX_MACRO	100
#define	MAX_ASSIGN	200

typedef struct TAG_ASSIGN_STRUCT {
	char name[NAME_SIZE];
	int  start_pos;
	int  data_count;
	int  dimension;
	char dimension_size[4][NAME_SIZE];
} ASSIGN_STRUCT;

class CCodeParser
{
public:
	CCodeParser(void);
	~CCodeParser(void);

	char macro_name[NAME_SIZE];
	char macro_value[MACRO_VALUE_SIZE];

	int macro_count;
	char macro_names[MAX_MACRO][NAME_SIZE];
	char macro_values[MAX_MACRO][MACRO_VALUE_SIZE];

	char *assign_buf;
	int  assign_next;

	int  assign_count;
	ASSIGN_STRUCT assign[MAX_ASSIGN];

public:
	int  get_next_token(char**sp, char*tokenstr, int size);
	void ParseText(char* dataBuf);
	int IsIdentifierChar(char c);
	void AddToMacro(char* name, char* value);
	void ClearMemory();
	BOOL LoadFile(TCHAR* fn);
};

int LoadFile(TCHAR* tfn, char* buf, int bufsize);
void ReplaceTagText(char* dstbuf, int bufsize, char* tag, char* txt);

#endif // _CODEPARSER_H_
