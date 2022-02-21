#pragma once

class CTokenizer
{
public:
	CTokenizer(void);
	~CTokenizer(void);
	void Tokenize(TCHAR* str, TCHAR* seps);
	void Tokenize(char* str, char* seps);
	void ClearMemory();

	static const int TOKEN_MAX = 20;
	
	TCHAR* wszBuf;
	char*  szBuf;
	int    token_count;
	TCHAR* wtokens[TOKEN_MAX];
	char*  tokens[TOKEN_MAX];
};

