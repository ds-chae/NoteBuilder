#pragma once

typedef struct CurrencyRecord {
	TCHAR	CurrencyNumber[4];
	TCHAR	CountryName[64];
	TCHAR	CurrencyName[64];
	TCHAR	CurrencyCode[4];
} CurrencyRecord;

class CCurrencyList
{
public:
	CCurrencyList(void);
	~CCurrencyList(void);

	static const int MAX_CURRENCY = 300;

	CurrencyRecord CurrencyTable[MAX_CURRENCY];
	int currency_count;

	void Load(CWnd *pWnd, TCHAR* path);
	void ParseCurrencyListLine(TCHAR* buf);
	CurrencyRecord* FindCurrencyCode(TCHAR *code);
};

//extern CCurrencyList CurrencyList;

extern TCHAR m_CurrencyImageFolder[MAX_PATH];
