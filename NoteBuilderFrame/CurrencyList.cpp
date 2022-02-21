#include "StdAfx.h"
#include "CurrencyList.h"
#include "Tokenizer.h"

// One Global Instance
CCurrencyList CurrencyList;

CCurrencyList::CCurrencyList(void)
{
	currency_count = 0;
}

CCurrencyList::~CCurrencyList(void)
{
}

void CCurrencyList::ParseCurrencyListLine(TCHAR* buf)
{
	CTokenizer Tokenizer;
	
	Tokenizer.Tokenize(buf, L"\t" );

	if(Tokenizer.token_count > 3) {
		lstrcpy(this->CurrencyTable[currency_count].CurrencyNumber, Tokenizer.wtokens[0]);
		lstrcpy(this->CurrencyTable[currency_count].CountryName, Tokenizer.wtokens[1]);
		lstrcpy(this->CurrencyTable[currency_count].CurrencyName, Tokenizer.wtokens[2]);
		lstrcpy(this->CurrencyTable[currency_count].CurrencyCode, Tokenizer.wtokens[3]);
		currency_count++;
	}
}

void CCurrencyList::Load(CWnd *pWnd, TCHAR* path)
{
	currency_count = 0;

	TCHAR ListFileName[MAX_PATH];
	lstrcpy(ListFileName, path);
	lstrcat(ListFileName, L"\\currency_list.txt");

	CFile cf;
	if(!cf.Open(ListFileName, CFile::modeRead)) {
		pWnd->MessageBox(ListFileName, L"File not found.");
		return;
	}
	int FileLength = cf.GetLength();
	char* buffer = (char*)malloc(FileLength);
	cf.Read(buffer, FileLength);
	cf.Close();

	int WideLength = MultiByteToWideChar(CP_UTF8, 0, buffer, FileLength, NULL, 0);
	TCHAR* WideBuffer = (TCHAR*)calloc(WideLength+1, sizeof(TCHAR));
	MultiByteToWideChar(CP_UTF8, 0, buffer, FileLength, WideBuffer, WideLength);
	WideBuffer[WideLength] = 0;
	free(buffer);

	TCHAR *op = WideBuffer;
	TCHAR *bp = WideBuffer;
	while(*bp) {
		if(*bp == _T('\r') || *bp == _T('\n')) {
			*bp = 0;
			ParseCurrencyListLine(op);
			op = bp+1;
		}
		bp++;
	}
	free(WideBuffer);
}


CurrencyRecord* CCurrencyList::FindCurrencyCode(TCHAR *code)
{
	for(int i = 0; i < this->currency_count; i++) {
		if(lstrcmp(this->CurrencyTable[i].CurrencyCode, code) == 0) {
			return &this->CurrencyTable[i];
		}
	}
	
	return NULL;
}
