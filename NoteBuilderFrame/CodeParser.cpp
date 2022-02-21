#include "StdAfx.h"
#include "CodeParser.h"

CCodeParser::CCodeParser(void)
{
	macro_count = 0;

	assign_buf = NULL;
	assign_next = 0;
	assign_count = 0;
}

CCodeParser::~CCodeParser(void)
{
	ClearMemory();
}

void CCodeParser::ClearMemory()
{
	macro_count = 0;

	if(assign_buf != NULL)
		free(assign_buf);
	
	assign_count = 0;
	assign_next = 0;
}

int CCodeParser::IsIdentifierChar(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '_');
}

#define	TOKEN_ID		'I'
#define	TOKEN_LBRACKET	'['
#define	TOKEN_NUMBER	'N'
#define	TOKEN_SEMICOLON	';'
#define	TOKEN_ASSIGN	'='
#define	TOKEN_RBRACKET	']'
#define	TOKEN_SHARP		'#'
#define	TOKEN_LF		'\n'
#define	TOKEN_STRING	's'

int CCodeParser::get_next_token(char**sp, char*tokenstr, int size)
{
	int tlen = 0;
	char* cp = *sp;
	tokenstr[0] = 0;

	while(*cp) {
		switch(*cp){
		case '/' :
			if(*(cp+1) == '/') {
				while(*cp && *cp != '\n' && *cp != '\r') cp++;
			} else {
				cp++;
				tokenstr[0] = '/';
				tokenstr[1] = 0;
				*sp = cp;
				return '/';
			}
			break;
		case '\n':
		case '#' :
		case ';' :
		case '[' :
		case ']' :
		case '=' :
		case '+':
		case '-' :
		case '(' :
		case ')' :
		case '*' :
			tokenstr[0] = *cp++;
			tokenstr[1] = 0;
			*sp = cp;
			return tokenstr[0];
		default :
			if(*cp == '-' || (*cp >= '0' && *cp <= '9')) {
				if(tlen < size-1)
					tokenstr[tlen++] = *cp++;
				tokenstr[tlen] = 0;
				while(*cp && ((*cp >= '0' && *cp <= '9') || *cp == '.')) {
					if(tlen < size-1)
						tokenstr[tlen++] = *cp++;
					tokenstr[tlen] = 0;
				}
				tlen = 0;
				*sp = cp;
				return TOKEN_NUMBER;
			} else if(IsIdentifierChar(*cp)) {
				while(*cp && IsIdentifierChar(*cp)) {
					if(tlen < size-1)
						tokenstr[tlen++] = *cp++;
					tokenstr[tlen] = 0;
				}
				tlen = 0;
				*sp = cp;
				return TOKEN_ID;
			} else if(*cp == '"') {
				cp++;
				tlen = 0;
				while(*cp) {
					switch(*cp){
					case '"' :
						tokenstr[tlen] = 0;
						cp++;
						*sp = cp;
						return TOKEN_STRING;
					case '\\':
						cp++;
						if(*cp == 0) {
							*sp = cp;
							return 0;
						}
						if(tlen < size-1)
							tokenstr[tlen++] = *cp;
						break;
					default :
						if(tlen < size-1)
							tokenstr[tlen++] = *cp++;
					}
				}
				return 0;
			} else {
				cp++;
			}
		}
	}

	*sp = cp;
	return 0;
}

#define	WAIT_TYPE			0
#define	WAIT_NAME			1
#define	WAIT_SEMICOLON		2
#define	WAIT_LBRACKET		3
#define	WAIT_DIMENSION		4
#define	WAIT_REGION_DATA	5
#define	WAIT_MACRO_NAME		6
#define	WAIT_COMMENT_BEGIN	7
#define	WAIT_END_OF_LINE	8
#define	WAIT_END_OF_COMMENT	9
#define	WAIT_MACRO_VALUE	10
#define	WAIT_DEFINE			11
#define	WAIT_ASSIGN			12
#define	WAIT_DATA	13

void CCodeParser::ParseText(char* dataBuf)
{
	int state = WAIT_TYPE;
	int vlen = 0;
	int token_type;
	char tokenstr[128];
	char t_dimension[128];
	ASSIGN_STRUCT *pCurAssign = NULL;
	int  last_token_type = 0;
	int  macro_length = 0;

	ClearMemory();
	int data_size = strlen(dataBuf);
	assign_buf = (char*)malloc(data_size+1);

	char* nsp = dataBuf;
	while(token_type = get_next_token(&nsp, tokenstr, sizeof(tokenstr))) {
		switch(state) {
		case WAIT_TYPE :
			switch(token_type){
			case TOKEN_ID :
				if(strcmp(tokenstr, "const") == 0)
					state = WAIT_TYPE;
				else if(strcmp(tokenstr, "extern") == 0)
					state = WAIT_END_OF_LINE;
				else {
					state = WAIT_NAME;
				}
				break;
			case TOKEN_SHARP :
				state = WAIT_DEFINE;
				break;
			case '/' :
				state = WAIT_COMMENT_BEGIN;
			}
			break;
		case WAIT_COMMENT_BEGIN :
			switch(token_type){
			case '/' :
				state = WAIT_END_OF_LINE;
				break;
			case '*' :
				state = WAIT_END_OF_COMMENT;
				break;
			default :
				state = WAIT_TYPE;
			}
			break;
		case WAIT_DEFINE :
			if(strcmp(tokenstr, "define") == 0) {
				state = WAIT_MACRO_NAME;
			} else {
				state = WAIT_END_OF_LINE;
			}
			break;
		case WAIT_MACRO_NAME :
			if(token_type == TOKEN_ID) {
				strcpy_s(macro_name, 64, tokenstr);
				state = WAIT_MACRO_VALUE;
				macro_value[0] = 0;
				macro_length = 0;
			} else {
				state = WAIT_TYPE;
			}
			break;
		case WAIT_END_OF_COMMENT :
			if(token_type == '/') {
				if(last_token_type == '*') {
					state = WAIT_TYPE;
				}
			}
			break;
		case WAIT_END_OF_LINE :
			if(token_type == '\n')
				state = WAIT_TYPE;
			break;
		case WAIT_MACRO_VALUE :
			if(token_type == '\r' || token_type == '\n') {
				macro_value[macro_length] = 0;
				AddToMacro(macro_name, macro_value);
				state = WAIT_TYPE;
				break;
			}
			strcpy_s(macro_value+macro_length, MACRO_VALUE_SIZE-macro_length, tokenstr);
			macro_length += strlen(tokenstr);
			break;
		case WAIT_NAME :
			switch(token_type){
			case TOKEN_SEMICOLON :
				state = WAIT_TYPE;
				break;
			case TOKEN_ID :
				pCurAssign = &assign[assign_count];
				strcpy_s(pCurAssign->name, 64, tokenstr);
				pCurAssign->dimension = 0;
				pCurAssign->data_count = 0;
				state = WAIT_LBRACKET;
				break;
			case '*' :
				break;
			default :
				state = WAIT_SEMICOLON;
			}
			break;
		case WAIT_SEMICOLON :
			if(token_type == TOKEN_SEMICOLON)
				state = WAIT_TYPE;
			break;
		case WAIT_LBRACKET :
			switch(token_type){
			case TOKEN_SEMICOLON :
				state = WAIT_TYPE;
				break;
			case TOKEN_LBRACKET :
				t_dimension[0] = 0;
				state = WAIT_DIMENSION;
				break;
			case TOKEN_ASSIGN :
				pCurAssign->start_pos = assign_next;
				state = WAIT_DATA;
				break;
			default :
				break;
			}
			break;
		case WAIT_DIMENSION :
			switch(token_type){
			case TOKEN_RBRACKET :
				strcpy_s(pCurAssign->dimension_size[pCurAssign->dimension], 64, t_dimension);
				pCurAssign->dimension++;
				state = WAIT_LBRACKET;
				break;
			case TOKEN_SEMICOLON :
				state = WAIT_TYPE;
				break;
			default :
				strcat_s(t_dimension, tokenstr);
			}
			break;
		case WAIT_DATA :
			switch(token_type){
			case TOKEN_NUMBER :
			case TOKEN_ID :
			case TOKEN_STRING :
				if(last_token_type == '-')
					assign_buf[assign_next++] = '-';

				strcpy_s(assign_buf+assign_next, data_size-assign_next, tokenstr);
				assign_next += (strlen(tokenstr)+1);
				pCurAssign->data_count++;
				break;
			case TOKEN_SEMICOLON :
				assign_count++;
				state = WAIT_TYPE;
			}
			break;
		}
		last_token_type = token_type;
	}
}

void CCodeParser::AddToMacro(char* name, char* value)
{
	strcpy_s(macro_names[macro_count], NAME_SIZE, name);
	strcpy_s(macro_values[macro_count], MACRO_VALUE_SIZE, value);
	macro_count++;
}

BOOL CCodeParser::LoadFile(TCHAR* fn)
{
	CFile file;
	if(!file.Open(fn, CFile::modeRead))
	{
		return FALSE;
	}

	int length = (int)file.GetLength();
	char *data = (char*)malloc(length+1);
	file.Read(data, length);
	file.Close();
	data[length] = 0;

	ParseText(data);
	free(data);

	return TRUE;
}
