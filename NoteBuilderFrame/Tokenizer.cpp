#include "StdAfx.h"
#include "Tokenizer.h"

CTokenizer::CTokenizer(void)
{
	wszBuf = NULL;
	szBuf = NULL;
	token_count = 0;
}

CTokenizer::~CTokenizer(void)
{
	ClearMemory();
}

void CTokenizer::ClearMemory()
{
	if(szBuf != NULL) {
		free(szBuf);
		szBuf = NULL;
	}
	if(wszBuf != NULL) {
		free(wszBuf);
		wszBuf = NULL;
	}
	token_count = 0;
}

void CTokenizer::Tokenize(char *str, char *sep)
{
	ClearMemory();

	int buflen = strlen(str) * 2 + 1;
	szBuf = (char*)malloc(buflen);
	char* sp = szBuf;
	char* tp = szBuf;

	int tlen = 0;
	while(1) {
		if(strchr(sep, *str) || *str == 0) {
			*sp++ = 0;
			if(tlen > 0) {
				tokens[token_count] = tp;
				token_count++;
			}
			tp = sp;
		} else {
			*sp++ = *str;
			tlen++;
		}
		if(*str == 0)
			break;
		str++;
	}
			/*
	char* token = strtok(str, sep);
	while(token && token_count < TOKEN_MAX) {
		tokens[token_count++] = sp;
		strcpy_s(sp, buflen, token);
		sp += strlen(token)+1;
		token = strtok(NULL, sep);
	}
	*/
}

void CTokenizer::Tokenize(TCHAR *str, TCHAR *seps)
{
	ClearMemory();

	wszBuf = (TCHAR*)calloc(lstrlen(str)*2+1, sizeof(TCHAR));
	TCHAR* sp = wszBuf;

	TCHAR* next_token = NULL;
	TCHAR* token = wcstok_s(str, seps, &next_token);
	while(token && token_count < TOKEN_MAX) {
		wtokens[token_count++] = sp;
		lstrcpy(sp, token);
		sp += lstrlen(token)+1;
		token = wcstok_s(NULL, seps, &next_token);
	}
}
