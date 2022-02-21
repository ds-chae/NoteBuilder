#include "StdAfx.h"

#include "B_Common.h"

#include "CurrencyList.h"
#include "CurrencyInfo.h"
#include "Tokenizer.h"

#pragma warning(disable:4996)

ST_LOCAL	g_LocalZero;

TCHAR m_CurrencyImageFolder[MAX_PATH];

CurrencyInfo currencyInfo;

CurrencyInfo::CurrencyInfo(void)
{
	m_pLocal = NULL;

	InitializeCurrencyInfo(NULL);
}

CurrencyInfo::~CurrencyInfo(void)
{
}

void CurrencyInfo::SetRegionDefault(ST_IMG_REGION *pReg)
{
	pReg->Region.State = 1; // Default Active
	pReg->Region.Sx = -164; // 이 값들은 지폐의 중앙 기준이다.
	pReg->Region.Sy = -55;
	pReg->Region.Ex = 284;
	pReg->Region.Ey = 45;

	// X 방향으로 4개 건너 뛰고 4개를 평균.
	pReg->Shift_X = 4; // X방향으로 4개를 건너뛴다.
	pReg->Shift_Y = 1; // Y방향으로 2개를 건너뛴다.
	pReg->Sum_X = 2; // X방향으로 4개를 평균
	pReg->Sum_Y = 2; // Y방향으로 2개를 평균

	pReg->Threshold = 17.0;
}

SymbolTable::SymbolTable()
{
	Init();
}

void SymbolTable::Init()
{
	// 로컬에 초기 데이터를 세팅하는 기능이다. 개발 중이라서 루블을 기준으로 세팅했다.
	strcpy_s(SymbolNames[0], 64, "DIR_FF"); strcpy_s(SymbolValue[0], 64, "0");
	strcpy_s(SymbolNames[1], 64, "DIR_FB"); strcpy_s(SymbolValue[1], 64, "1");
	strcpy_s(SymbolNames[2], 64, "DIR_BF"); strcpy_s(SymbolValue[2], 64, "2");
	strcpy_s(SymbolNames[3], 64, "DIR_BB"); strcpy_s(SymbolValue[3], 64, "3");
	SymbolCount = 4;
}

SymbolTable st;

void CurrencyInfo::InitializeCurrencyInfo(WIN_ST_LOCAL *pLocal)
{
	// 심볼 테이블 초기화
	st.Init();

	RegionSetCount = 0;
	memset(RegionSet, 0, sizeof(RegionSet));

	RegionPtrArrayCount = 0;

	MaxCurrencyNum = 0; // USD_CURRENCY_NUM
	MaxAreaY = 0; // USD_AREA_X
	MaxAreaX = 0; // USD_AREA_Y
	MaxDataZ = 0; // USD_MAX_DATA

	if(pLocal != NULL) {
		memset(pLocal , 0, sizeof(ST_LOCAL) );
		pLocal->Region_Value_Num = 1;
		
		SetRegionDefault(&pLocal->Region_Value[0]);
		SetRegionDefault(&pLocal->Region_Value[1]);
	}

	IsNNRDataLoaded = 0;
}

enum CurrencyInfoSymbolNumber {
	//REG_VALUE_NUM = 1,
	//REG_IMGSOURCE = 2,
	//REG_METHODTYPE = 3,
	//REG_SQUARE = 4,
	//REG_DECIMATION = 5,
	//REG_SUMMATION = 6,
	REG_THRESHOLD = 7,
	CUR_VERSIONS = 8,
	//CUR_VALUES = 9,
	//NOTE_SIZE = 10,
	NOTE_AREA = 11,
	NOTE_MAXDATA = 12,
	//IMAGE_FOLDERS = 13,
};

struct TAG_CurrencyInfoSymbol {
	char* Symbol;
	int   Token;
};

typedef struct TAG_CurrencyInfoSymbol CurrencyInfoSymbol;

CurrencyInfoSymbol CurrencyInfoSymbols[] = {
	{ "Threshold",	REG_THRESHOLD },
	{ "Versions",   CUR_VERSIONS },
	{ "NoteArea",	NOTE_AREA },
	{ "NoteMaxData",NOTE_MAXDATA },
	{ NULL, -1 }
};

int GetSymbolNumber(char* sym)
{
	for(CurrencyInfoSymbol* cis = CurrencyInfoSymbols; cis->Symbol != NULL; cis++) {
		if(strcmp(sym, cis->Symbol) == 0)
			return cis->Token;
	}

	return -1;
}

char* GetSymbolString(int num)
{
	for(CurrencyInfoSymbol* cis = CurrencyInfoSymbols; cis->Symbol != NULL; cis++) {
		if(num == cis->Token)
			return cis->Symbol;
	}

	return "";
}

void SaveRegion(HANDLE h, int RegionNumber, ST_IMG_REGION *pReg)
{
	DWORD dwio = 0;
	char text[128];

	sprintf_s(text, sizeof(text), "%s=%f\r\n", GetSymbolString(REG_THRESHOLD), pReg->Threshold);
	WriteFile(h, text, strlen(text), &dwio, NULL);

}

void CurrencyInfo::LoadCurrencyInfoFile(TCHAR* File, WIN_ST_LOCAL *pLocal)
{
	m_pLocal = pLocal;
	InitializeCurrencyInfo(pLocal);

	WideCharToMultiByte(CP_ACP, 0, File, 4, pLocal->szCurrency, 4, NULL, NULL);
	lstrcpy(pLocal->tszCurrency, File);

	CurrencyRecord *rec = CurrencyList.FindCurrencyCode(File);
	if(rec != NULL)
		pLocal->CurrencyNum = StrToInt(rec->CurrencyNumber);
	else {
		TCHAR tStrMsg[102] = {0,};
		wsprintf(tStrMsg,L"%s, Not Found in currency_list", File);
		MessageBox(NULL, tStrMsg, L"Not Found", MB_OK);
	}

}

void CurrencyInfo::ParseConfigOneLine(char* ptr)
{
	CTokenizer t;
	t.Tokenize(ptr, "=");
	if(t.token_count > 1) {
		switch(GetSymbolNumber(t.tokens[0])) {
		case REG_THRESHOLD :
			m_pLocal->Region_Value[m_pLocal->Region_Value_Num-1].Threshold = atof(t.tokens[1]); // 0.032는 0.032000002로 변환된다.
			break;
		case CUR_VERSIONS :
			m_pLocal->Versions = atoi(t.tokens[1]);
			break;
		}
	}
}

void ScanSquareValuesW(TCHAR* str, ST_SQUARE *pSQ)
{
	int arr[4];
	if(StrToIntArrW(str, arr, 4) == 4) {
		pSQ->Sx = arr[0];
		pSQ->Sy = arr[1];
		pSQ->Ex = arr[2];
		pSQ->Ey = arr[3];
	}
}

void ScanSquareValuesA(char* str, ST_SQUARE *pSQ)
{
	TCHAR wstr[64];
	int   len = strlen(str) + 1;
	MultiByteToWideChar(CP_ACP, 0, str, len, wstr, 64);
	ScanSquareValuesW(wstr, pSQ);
}

void ScanDecimationA(char* str, ST_IMG_REGION *pReg)
{
	TCHAR wstr[64];
	int len = strlen(str) + 1;
	MultiByteToWideChar(CP_ACP, 0, str, len, wstr, 64);
	ScanDecimationW(wstr, pReg);
}

void ScanDecimationW(TCHAR* str, ST_IMG_REGION *pReg)
{
	int arr[2];
	if(StrToIntArrW(str, arr, 2) == 2) {
		pReg->Shift_X = arr[0];
		pReg->Shift_Y = arr[1];
	}
}

void ScanSummationA(char* str, ST_IMG_REGION *pReg)
{
	TCHAR wstr[64];
	int len = strlen(str) + 1;
	MultiByteToWideChar(CP_ACP, 0, str, len, wstr, 64);
	ScanSummationW(wstr, pReg);
}

void ScanSummationW(TCHAR* str, ST_IMG_REGION *pReg)
{
	int arr[2];
	if(StrToIntArrW(str, arr, 2) == 2) {
		pReg->Sum_X = arr[0];
		pReg->Sum_Y = arr[1];
	}
}

int StrToIntArrA(char* str, int* arr, int maxcnt)
{
	int len = strlen(str)+1;
	TCHAR* wstr = (TCHAR*)calloc(len, sizeof(TCHAR));
	MultiByteToWideChar(CP_ACP, 0, str, len, wstr, len);

	int ret = StrToIntArrW(wstr, arr, maxcnt);
	free(wstr);
	return ret;
}

int StrToIntArrW(TCHAR* str, int* arr, int maxcnt)
{
	int v = 0;
	int num = 0;
	int cnt = 0;
	int sign = 1;
	while(cnt < maxcnt && *str) {
		if(*str == _T('-')) {
			sign = -1;
		} else if(*str >= _T('0') && *str <= _T('9')) {
			v = v * 10 + (*str - _T('0'));
			num = 1;
		} else {
			if(num) {
				arr[cnt] = v * sign;
				cnt++;
				sign = 1;
			}
			num = 0;
			v = 0;
		}
		str++;
	}
	if(num && cnt < maxcnt) {
		arr[cnt] = v;
		cnt++;
	}

	return cnt;
}

int ScanInt32Arr(char*s, Uint32 *arr)
{
	int v = 0;
	int cnt = 0;
	int len = 0;

	while(*s) {
		if(*s >= '0' && *s <= '9') {
			v = v * 10 + (*s - '0');
			len++;
		} else {
			if(len) {
				len = 0;
				arr[cnt] = v;
				cnt++;
			}
			v = 0;
		}
		s++;
	}
	if(len) {
		arr[cnt] = v;
		cnt++;
	}
	return cnt;
}

int SymbolTable::GetSymbolStr(char *vstr, char* rstr, int size)
{
	for(int i = 0; i < SymbolCount; i++) {
		if(strcmp(vstr, SymbolNames[i]) == 0) {
			strcpy_s(rstr, size, SymbolValue[i]);
			return 1;
		}
	}

	*rstr = 0;
	return 0;
}

int SymbolTable::GetSymbolIndex(char *vstr)
{
	for(int i = 0; i < SymbolCount; i++) {
		if(strcmp(vstr, SymbolNames[i]) == 0)
			return i;
	}

	return -1;
}

int IsNumber(char *str)
{
	int dot_count = 0;
	if(*str == '-' || *str == '+') // skip sign
		str++;

	if(*str == 0) // 0 length string
		return 0;

	while(*str) {
		if(*str == '.') {
			dot_count++;
			if(dot_count > 1)
				return 0;
		} else {
			if(*str < '0' || *str > '9')
				return 0;
		}
		str++;
	}

	if(dot_count > 0)
		return 2;
	
	return 1;
}

#define	MAXTOKENS	16

class tokenlist {
public:
	int count;
	evaltoken tokens[MAXTOKENS];

public:
	tokenlist() {
		count = 0;
		memset(tokens, 0, sizeof(tokens));
	}

	void add(evaltoken* ptoken)
	{
		tokens[count++] = *ptoken;
	}

	void add(int t)
	{
		tokens[count].type = t;
		tokens[count].str[0] = t;
		tokens[count].str[1] = 0;
		count++;
	}

	void strbegin(int t, int c)
	{
		tokens[count].type = t;
		stradd(c);
	}

	void stradd(int c)
	{
		int slen = strlen(tokens[count].str);
		tokens[count].str[slen++] = c;
		tokens[count].str[slen] = 0;
	}
	
	void strend()
	{
		count++;
	}
};

int ParseStrToList(char* str, tokenlist* list)
{
	int state = 0;

	while(*str) {
		switch(state) {
		case 0 :
			if(*str > ' ') {
				switch(*str) {
				case '(' :
				case ')' :
				case '+' :
				case '-' :
				case '*' :
				case '/' :
					list->add(*str);
					break;
				default :
					if(*str >= '0' && *str <= '9') {
						list->strbegin('0', *str);
						state = 1;
					} else if(*str >= 'a' && *str <= 'z' || *str >= 'A' && *str <= 'Z' || *str == '_') {
						list->strbegin('A', *str);
						state = 2;
					}
				}
			}
			break;
		case 1 :
			if(*str >= '0' && *str <= '9') {
				list->stradd(*str);
			} else {
				list->strend();
				str--;
				state = 0;
			}
			break;
		case 2 :
			if(*str >= 'a' && *str <= 'z' || *str >= 'A' && *str <= 'Z' || *str == '_' || *str >= '0' && *str <= '9') {
				list->stradd(*str);
			} else {
				list->strend();
				str--;
				state = 0;
			}
		}
		str++;
	}

	if(state == 1 || state == 2) {
		list->strend();
	}

	return list->count;
}

#define	STACKSIZE	10
#define	EMPTY	-1

class stack {
public:
	evaltoken data[STACKSIZE];
	int top;  

public:
	stack()
	{
		emptystack();
	}

	int isempty() {
		return (top == EMPTY) ? 1 : 0;
	}

	void emptystack()  
	{
		top = EMPTY;
	}

	void push(evaltoken* token)
	{  
		if(top == (STACKSIZE-1))
			return;

		++top;  
		data[top] = *token;
	}

	void pop(evaltoken* ptoken)
	{
		ptoken->type = EMPTY;
		if(!isempty()) {
			*ptoken = data[top];  
			--top;  
		}
	}

	void display()
	{
		while(top != EMPTY) {
			printf("\n%d, %s", data[top].type, data[top].str);
			top--;
		}
	}
};

int isoperator(char e)  
{
	if(e == '+' || e == '-' || e == '*' || e == '/' || e == '%')  
		return 1;  
	else 
		return 0;  
}

int priority(char e)  
{  
	switch(e) {
	case '*' :
	case '/' :
	case '%' :
		return 2;

	case '+' :
	case '-' :
		return 1;
	}
	
	return 0;
}

int MakePostfix(tokenlist* infix, tokenlist* postfix)
{
	evaltoken *i;
	stack X;  
	evaltoken n1;  

	X.emptystack();

	i = infix->tokens;

	while(i->type) {
		if( i->type == '0' || i->type == 'A' ) {
			postfix->add(i);
			i++;
		}

		if( i->type == '(' )  {  
			X.push(i);  
			i++;  
		}  

		if( i->type == ')')  
		{  
			X.pop(&n1);
			while( n1.type != '(' )  
			{
				postfix->add(&n1);
				X.pop(&n1);
			}  
			i++;  
		}  

		if( isoperator(i->type) )  
		{  
			if(X.isempty())  
				X.push(i);  
			else 
			{  
				X.pop(&n1);  
				while(priority(n1.type) >= priority(i->type))  
				{  
					postfix->add(&n1);
					X.pop(&n1);
				}  
				X.push(&n1);  
				X.push(i);  
			}
			i++;  
		}  
	}

	while(!X.isempty())  
	{  
		X.pop(&n1);
		postfix->add(&n1);
	}

	return postfix->count;
}

void Evaluate(char* str, evaltoken* pet)
{
	tokenlist postfix;
	tokenlist list;
	stack evalstack;

	ParseStrToList(str, &list);
	MakePostfix(&list, &postfix);

	for(int i = 0; i < postfix.count; i++) {
		evaltoken t = postfix.tokens[i];
		evaltoken v;
		v.type = 'i';
		if(isoperator(t.type)) {
			v.ivalue = 0;
			if(!evalstack.isempty()) {
				evaltoken t0, t1;
				evalstack.pop(&t0);
				if(!evalstack.isempty()) {
					evalstack.pop(&t1);
					switch(t.type){
					case '+' :
						v.ivalue = t1.ivalue + t0.ivalue;
						break;
					case '-':
						v.ivalue = t1.ivalue - t0.ivalue;
						break;
					case '/':
						v.ivalue = t1.ivalue / t0.ivalue;
						break;
					case '*':
						v.ivalue = t1.ivalue * t0.ivalue;
					}
				} else {
				}
			}
			evalstack.push(&v);
		} else {
			switch(t.type){
			case '0': // 숫자문자열이면 숫자로 변환
				t.type = 'i';
				t.ivalue = atoi(t.str);
				break;
			case 'A': // 문자열이면 그 값을 계산한다.
				st.GetSymbolStr(t.str, v.str, sizeof(v.str));
				if(IsNumber(v.str))
					v.ivalue = atoi(v.str);
				else
					Evaluate(t.str, &v);
				t = v;
			}
			evalstack.push(&t);
		}
	}

	evalstack.pop(pet);
}

int SymbolTable::GetSymbolValue(char* name, evaltoken& et)
{
	char rstr[128];

	switch(IsNumber(name)) {
	case 1 :
		et.type = 'i';
		et.ivalue = atoi(name);
		return 1;

	case 2 :
		et.type = 'f';
		et.fvalue = atof(name);
		return 1;

	default :
		if(GetSymbolStr(name, rstr, sizeof(rstr))) {
			Evaluate(rstr, &et);
			if(et.type == '0') { // 숫자문자열이면 숫자로 변환
				et.type = 'i';
				et.ivalue = atoi(et.str);
			}
			return 1;
		}
		return 0;
	}
}

float SymbolTable::GetSymbolValueF(char* name)
{
	if(IsNumber(name))
		return (float)atof(name);

	int NextIndex = GetSymbolIndex(name);
	if(NextIndex != -1)
		return GetSymbolValueF(NextIndex);

	return -1.0;
}

int SymbolTable::GetSymbolValue(int index)
{
	if(IsNumber(SymbolNames[index]))
		return atoi(SymbolNames[index]);
	evaltoken tkn;
	Evaluate(SymbolValue[index], &tkn);
	if(tkn.type == 'i')
		return tkn.ivalue;

	return -1;
}

float SymbolTable::GetSymbolValueF(int index)
{
	if(IsNumber(SymbolNames[index]))
		return atof(SymbolNames[index]);
	if(IsNumber(SymbolValue[index]))
		return atof(SymbolValue[index]);
	int NextIndex = GetSymbolIndex(SymbolValue[index]);
	if(NextIndex != -1)
		return GetSymbolValueF(NextIndex);

	return -1;
}

void CurrencyInfo::ParseDefinitionLine(char* line)
{
	char tokenstr[128];
	int  tokenlen = 0;
	char symbolname[128];

	// skip leading blank
	while(*line && *line <= ' ')
		line++;

	// test #define
	if(*line != '#')
		return;
	line++;

	// get string
	tokenlen = 0;
	while(*line >= 'a' && *line <= 'z')
		tokenstr[tokenlen++] = *line++;
	tokenstr[tokenlen] = 0;
	if(strcmp(tokenstr, "define") != 0)
		return;

	// get symbol
	// skip leading blank
	while(*line && *line <= ' ') line++;
	tokenlen = 0;
	while(*line == '_' || (*line >= '0' && *line <= '9') || (*line >= 'A' && *line <= 'Z') || (*line >= 'a' && *line <= 'z'))
		tokenstr[tokenlen++] = *line++;
	if(tokenlen == 0)
		return;
	
	tokenstr[tokenlen] = 0;
	strcpy_s(symbolname, sizeof(symbolname), tokenstr);
	
	// get until EOL
	// skip leading blank
	while(*line && *line <= ' ')
		line++;
	tokenlen = 0;
	while(*line && *line != '\r' && *line != '\n') {
		if( *line == '/' && *(line+1) == '/')
			break;
		tokenstr[tokenlen++] = *line++;
	}
	tokenstr[tokenlen] = 0;
	
	// remove trailing blank
	while(tokenlen > 0) {
		if(tokenstr[tokenlen-1] > ' ')
			break;
		tokenlen--;
		tokenstr[tokenlen] = 0;
	}

	if(tokenlen)
		st.AddSymbol(symbolname, tokenstr);
}

void SymbolTable::AddSymbol(char* symbolname, char* tokenstr)
{
	if(SymbolCount < MAX_SYMBOL) {
		strcpy_s(SymbolNames[SymbolCount], 64, symbolname);
		strcpy_s(SymbolValue[SymbolCount], 64, tokenstr);
		SymbolCount++;
	}
}

void CurrencyInfo::LoadCurrencyDefinition(CWnd *wnd, TCHAR* fn, char* dataBuf)
{
	char lineBuffer[256];
	int  lineLength = 0;
	int inComment = 0;
	for( ; *dataBuf; dataBuf++){
		if(inComment) {
			if(*dataBuf == '*' && *(dataBuf+1) == '/') {
				inComment = 0;
				dataBuf++; // skip '/'
			}
		} else {
			if(*dataBuf == '/' && *(dataBuf+1) == '*') {
				lineBuffer[lineLength] = 0;
				lineLength = 0;
				ParseDefinitionLine(lineBuffer);
				dataBuf++;
			} else {
				if(*dataBuf == '\r' || *dataBuf == '\n') {
					lineBuffer[lineLength] = 0;
					lineLength = 0;
					ParseDefinitionLine(lineBuffer);
				} else {
					if(lineLength < 255)
						lineBuffer[lineLength++] = *dataBuf;
				}
			}
		}
	}

	pWinCurrentLocal->Region_Value_Num = currencyInfo.GetIntValueWithPrefix(pWinCurrentLocal->szCurrency, "_REGION_VALUE_NUM");
	if(pWinCurrentLocal->Region_Value_Num == 0) {
		wnd->MessageBox(fn, L"No _REGION_VALUE_NUM");
	}
	MaxCurrencyNum = GetIntValueWithPrefix(pWinCurrentLocal->szCurrency, "_CNRRENCY_NUM"); // USD_CURRENCY_NUM
	if(MaxCurrencyNum <= 0) {
		wnd->MessageBox(fn, L"No _CNRRENCY_NUM");
		pWinCurrentLocal->CurrencyDataType = CURR_DATA_NONE;
	}
	MaxAreaY = GetIntValueWithPrefix(pWinCurrentLocal->szCurrency, "_AREA_Y"); // USD_AREA_X
	if(MaxAreaY <= 0) {
		wnd->MessageBox(fn, L"No _AREA_Y");
		pWinCurrentLocal->CurrencyDataType = CURR_DATA_NONE;
	}
	MaxAreaX = GetIntValueWithPrefix(pWinCurrentLocal->szCurrency, "_AREA_X"); // USD_AREA_Y
	if(MaxAreaX <= 0) {
		wnd->MessageBox(fn, L"No _AREA_X");
		pWinCurrentLocal->CurrencyDataType = CURR_DATA_NONE;
	}
	MaxDataZ = GetIntValueWithPrefix(pWinCurrentLocal->szCurrency, "_MAX_DATA"); // USD_MAX_DATA
	if(MaxDataZ <= 0) {
		wnd->MessageBox(fn, L"No _MAX_DATA");
		pWinCurrentLocal->CurrencyDataType = CURR_DATA_NONE;
	}
	MaxDenom = GetIntValueWithPrefix(pWinCurrentLocal->szCurrency, "_MAXDENOM");
	if(MaxDenom == 0) {
		wnd->MessageBox(fn, L"No _MAXDENOM");
		pWinCurrentLocal->CurrencyDataType = CURR_DATA_NONE;
	}
}

void CurrencyInfo::SetImgRegion(ST_IMG_REGION *pReg, int *CUR_Regions)
{
	pReg->Region.Sx = CUR_Regions[0];
	pReg->Region.Sy = CUR_Regions[1];
	pReg->Region.Ex = CUR_Regions[2];
	pReg->Region.Ey = CUR_Regions[3];
	pReg->Region.State = CUR_Regions[4];
	pReg->Shift_X = CUR_Regions[5];
	pReg->Shift_Y = CUR_Regions[6];
	pReg->Sum_X = CUR_Regions[7];
	pReg->Sum_Y = CUR_Regions[8];
}

int IsDigitChar(char c)
{
	return (c >= '0' && c <= '9');
}

int IsIdentifierChar(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '_');
}

void CurrencyInfo::PutArrToRegion(int *varr, int vlen, REGION_SET *pRegSet)
{
	int ri = 0;

	while(vlen >= 9) {
		SetImgRegion(&pRegSet->Regions[ri], varr);
		vlen -= 9;
		varr += 9;
		ri++;
	}
}

#define	TOKEN_ID		'I'
#define	TOKEN_LBRACKET	'['
#define	TOKEN_NUMBER	'N'
#define	TOKEN_SEMICOLON	';'
#define	TOKEN_ASSIGN	'='
#define	TOKEN_RBRACKET	']'
#define	TOKEN_PTR		'*'

int get_next_token(char**sp, char*tokenstr)
{
	int tlen = 0;
	char* cp = *sp;
	tokenstr[0] = 0;

	while(*cp) {
		if(*cp == '/') {
			cp++;
			switch(*cp){
			case '/' : // 한 줄 주석은 줄 끝까지 통과한다.
				cp++;
				while(*cp && *cp != '\n' && *cp != '\r') cp++;
				break;
			case '*' : // 주석 열림은 주석 닫힘까지 통과한다.
				cp++; // '/*' 중에서 '*'까지 통과
				while(*cp) {
					if(*cp == '*' && *(cp+1) == '/') { // '*/'가 발견되면
						cp += 2;
						break;
					}
					cp++; // 발견이 안 되면 하나씩 증가하면서 검사
				}
				break;
			}
		} else if(*cp == '#') {
			cp++;
			while(*cp && *cp != '\n' && *cp != '\r') cp++;
		} else if(*cp == ';' || *cp == '[' || *cp == ']' || *cp == '=' || *cp == '*') {
			int token = *cp++;
			tokenstr[0] = token;
			tokenstr[1] = 0;
			*sp = cp;
			return token;
		} else if(*cp == '-' || (*cp >= '0' && *cp <= '9')) {
			tokenstr[tlen++] = *cp++;
			tokenstr[tlen] = 0;
			while(*cp && ((*cp >= '0' && *cp <= '9') || *cp == '.')) {
				tokenstr[tlen++] = *cp++;
				tokenstr[tlen] = 0;
			}
			tlen = 0;
			*sp = cp;
			return TOKEN_NUMBER;
		} else if(IsIdentifierChar(*cp)) {
			while(*cp && IsIdentifierChar(*cp)) {
				tokenstr[tlen++] = *cp++;
				tokenstr[tlen] = 0;
			}
			tlen = 0;
			*sp = cp;
			return TOKEN_ID;
		} else {
			cp++;
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
#define	WAIT_DENOM_DATA		6
#define	WAIT_DDLV_DATA		7
#define	WAIT_MMX_DATA		8
#define	WAIT_MMY_DATA		9
#define	WAIT_VERSION_DATA	10
#define	WAIT_DEN_2			11
#define	WAIT_ASSIGN			12
#define	WAIT_DATA	13
#define	WAIT_REGION_PTR		14

#define	NT_UNKNOWN	0
#define	NT_REGION	1
#define	NT_DENOM	2
#define	NT_DDLV		3
#define	NT_MM_X		4
#define	NT_MM_Y		5
#define	NT_VERSION_TBL	6
#define	NT_DEN_2		7
#define	NT_REFCNT	8
#define	NT_TDENOM	9
#define	NT_REFKEY	10
#define	NT_Match_X	11
#define	NT_REGIONPTR	12

int GetDataType(char* tokenstr)
{
	if(strcmp(tokenstr, "ST_IMG_REGION") == 0)
		return NT_REGION;
	return NT_UNKNOWN;
}

int GetNameType(char* str)
{
	char temp[128];
	sprintf_s(temp, 128, "c_%s_Denom", pWinCurrentLocal->szCurrency);
	if(strcmp(str, temp) == 0)
		return NT_DENOM;

	sprintf_s(temp, 128, "c_%s_DDLv", pWinCurrentLocal->szCurrency);
	if(strcmp(str, temp) == 0)
		return NT_DDLV;

	sprintf_s(temp, 128, "%s_MM_X", pWinCurrentLocal->szCurrency);
	if(strcmp(str, temp) == 0)
		return NT_MM_X;

	sprintf_s(temp, 128, "%s_MM_Y", pWinCurrentLocal->szCurrency);
	if(strcmp(str, temp) == 0)
		return NT_MM_Y;

	sprintf_s(temp, 128, "%s_VersionTable", pWinCurrentLocal->szCurrency);
	if(strcmp(str, temp) == 0)
		return NT_VERSION_TBL;

	sprintf_s(temp, 128, "%s_Denom2", pWinCurrentLocal->szCurrency);
	if(strcmp(str, temp) == 0)
		return NT_DEN_2;

	sprintf_s(temp, 128, "%s_Ref_Cnt", pWinCurrentLocal->szCurrency);
	if(strcmp(str, temp) == 0)
		return NT_REFCNT;

	sprintf_s(temp, 128, "%s_Tdenom", pWinCurrentLocal->szCurrency);
	if(strcmp(str, temp) == 0)
		return NT_TDENOM;

	sprintf_s(temp, 128, "%s_Ref_Key", pWinCurrentLocal->szCurrency);
	if(strcmp(str, temp) == 0)
		return NT_REFKEY;

	sprintf_s(temp, 128, "%s_Match_X", pWinCurrentLocal->szCurrency);
	if(strcmp(str, temp) == 0)
		return NT_Match_X;

	return NT_UNKNOWN;
}


int CurrencyInfo::GetIntValueWithPrefix(char* prefix, char* body)
{
	char symbolName[128];
	sprintf_s(symbolName, "%s%s", prefix, body);
	evaltoken ev;
	if(st.GetSymbolValue(symbolName, ev))
		return ev.ivalue;
	return -1;
}

int FillIntArray(char* begin, char *end, int result[], int size)
{
	int count = 0;
	char *sp;
	int len = 0;
	int v = 0;
	int sign = 1;

	for(sp = begin; sp < end && count < size; sp++) {
		if((*sp >= '0' && *sp <= '9')) {
			len++;
			v = v * 10 + (*sp -'0');
		} else if(*sp == '-') {
			sign = -1;
		} else {
			if(len > 0) {
				result[count++] = v * sign;
				len = 0;
				v = 0;
				sign = 1;
			}
		}
	}
	if(len > 0 && count < size) {
		result[count++] = v * sign;
	}

	return count;
}

int FillFloatArray(char* begin, char *end, float result[], int size)
{
	float f;
	char floatstr[64];
	int count = 0;
	char *sp;
	int len = 0;

	for(sp = begin; sp < end && count < size; sp++) {
		if((*sp == '.') || (*sp >= '0' && *sp <= '9')) {
			if(len < 64-1) floatstr[len++] = *sp;
		} else {
			if(len > 0) {
				floatstr[len] = 0;
				sscanf(floatstr, "%f", &f);
				result[count++] = f;
				len = 0;
			}
		}
	}
	if(len > 0 && count < size) {
		floatstr[len] = 0;
		sscanf(floatstr, "%f", &f);
		result[count++] = f;
		len = 0;
	}

	return count;
}

void CurrencyInfo::LoadCurrencyConst(CWnd *wnd, TCHAR *fn, char* dataBuf)
{
	int state = WAIT_TYPE;
	int vlen = 0;
	int token_type;
	char tokenstr[128];

	IsDDLV_Set = 0;
	IsDenomValue_Set = 0;
	IsMMX_Set = 0;
	IsMMY_Set = 0;
	IsDenVer_Set = 0;
	IsDen2_Set = 0;

	RegionSetCount = 0;
	memset(RegionSet, 0, sizeof(RegionSet));

	RegionPtrArrayCount = 0;

	char* nsp = dataBuf;
	while(token_type = get_next_token(&nsp, tokenstr)) {
		switch(state) {
		case WAIT_TYPE :
			if(token_type == TOKEN_ID) {
				if(strcmp(tokenstr, "const") == 0)
					state = WAIT_TYPE;
				else {
					data_type = GetDataType(tokenstr);
					state = WAIT_NAME;
					vlen = 0;
				}
			}
			break;
		case WAIT_NAME :
			state = WAIT_SEMICOLON;
			if(token_type == TOKEN_PTR) { // '*'가 나왔는데
				if(data_type == NT_REGION) { // 기본 유형이 영역이면
					data_type = NT_REGIONPTR; // 영역 포인터의 배열이란 말이다.
					state = WAIT_NAME; // 다시 이름을 기다리러 간다.
					break;
				}
			}
			if(token_type == TOKEN_SEMICOLON) {
				state = WAIT_TYPE;
				break;
			}

			// 이름을 맞이하게 된 경우
			strcpy_s(data_name, 64, tokenstr);
			name_type = GetNameType(data_name);
			if(data_type == NT_REGION) {
				strcpy_s(RegionSet[RegionSetCount].ImgRegionName, sizeof(RegionSet[RegionSetCount].ImgRegionName), tokenstr);
				state = WAIT_LBRACKET;
				break;
			}
			if(data_type == NT_REGIONPTR) {
				RegionPtrArray[RegionPtrArrayCount].clear();
				strcpy_s(RegionPtrArray[RegionPtrArrayCount].szArrayName, sizeof(RegionPtrArray[RegionPtrArrayCount].szArrayName), tokenstr);
				state = WAIT_LBRACKET;
				break;
			}

			switch(name_type){
			case NT_DENOM :
			case NT_DDLV :
			case NT_REGION :
			case NT_MM_X :
			case NT_MM_Y :
			case NT_VERSION_TBL :
			case NT_DEN_2 :
				state = WAIT_LBRACKET;
				break;
			default :
				;
			}
			break;
		case WAIT_SEMICOLON :
			if(token_type == TOKEN_SEMICOLON)
				state = WAIT_TYPE;
			break;
		case WAIT_LBRACKET :
			state = WAIT_SEMICOLON;
			if(token_type == TOKEN_SEMICOLON) {
				state = WAIT_TYPE;
				break;
			}
			if(token_type == TOKEN_LBRACKET) {
				state = WAIT_DIMENSION;
				break;
			}
			if(token_type == TOKEN_ASSIGN) {
				if(data_type == NT_REGION) {
					strcpy_s(RegionSet[RegionSetCount].szRegionsCount, sizeof(RegionSet[RegionSetCount].szRegionsCount), "1");
					evaltoken tkn;
					st.GetSymbolValue(RegionSet[RegionSetCount].szRegionsCount, tkn);
					RegionSet[RegionSetCount].intRegionsCount = tkn.ivalue;
					state = WAIT_REGION_DATA;
				}
				if(data_type == NT_REGIONPTR) {
					RegionPtrArray[RegionPtrArrayCount].clear();
					state = WAIT_REGION_PTR;
				}
			}
			break;
		case WAIT_DIMENSION :
			state = WAIT_SEMICOLON;
			if(token_type == TOKEN_ID || token_type == TOKEN_NUMBER) {
				if(data_type == NT_REGION) {
					strcpy_s(RegionSet[RegionSetCount].szRegionsCount, sizeof(RegionSet[RegionSetCount].szRegionsCount), tokenstr);
					evaltoken tkn;
					st.GetSymbolValue(RegionSet[RegionSetCount].szRegionsCount, tkn);
					RegionSet[RegionSetCount].intRegionsCount = tkn.ivalue;
					state = WAIT_REGION_DATA;
				} else if(data_type == NT_REGIONPTR) {
					RegionPtrArray[RegionPtrArrayCount].clear();
					state = WAIT_REGION_PTR;
				} else {
					switch(name_type){
					case NT_DENOM :
						state = WAIT_DENOM_DATA;
						break;
					case NT_DDLV :
						state = WAIT_DDLV_DATA;
						break;
					case NT_MM_X :
						state = WAIT_MMX_DATA;
						break;
					case NT_MM_Y :
						state = WAIT_MMY_DATA;
						break;
					case NT_VERSION_TBL :
						state = WAIT_VERSION_DATA;
						break;
					case NT_DEN_2 :
						state = WAIT_DEN_2;
					}
				}
			}
			vlen = 0; // 여기서 0으로 리셋한다.
			break;
		case WAIT_REGION_DATA :
			if(token_type == TOKEN_NUMBER)
				varr[vlen++] = atoi(tokenstr);
			else if(token_type == TOKEN_SEMICOLON) {
				PutArrToRegion(varr, vlen, &RegionSet[RegionSetCount]);
				RegionSetCount++;
				state = WAIT_TYPE;
			}
			break;
		case WAIT_REGION_PTR :
			if(token_type == TOKEN_ID) {
				RegionPtrArray[RegionPtrArrayCount].add(tokenstr);
			} else if(token_type == TOKEN_SEMICOLON) {
				RegionPtrArrayCount++;
				state = WAIT_TYPE;
			}
			break;
		case WAIT_DENOM_DATA :
			if(token_type == TOKEN_NUMBER) {
				if(vlen < currencyInfo.MaxDenom) {
					denomValue[vlen] = atoi(tokenstr);
					vlen++;
				}
			} else if(token_type == TOKEN_SEMICOLON) {
				IsDenomValue_Set = 1;
				state = WAIT_TYPE;
			}
			break;
		case WAIT_DDLV_DATA :
			if(token_type == TOKEN_NUMBER) {
				if(vlen < currencyInfo.MaxDenom) {
					CUR_DDLV[vlen] = atoi(tokenstr);
					vlen++;
				}
			} else if(token_type == TOKEN_SEMICOLON) {
				IsDDLV_Set = 1;
				state = WAIT_TYPE;
			}
			break;
		case WAIT_MMX_DATA :
			if(token_type == TOKEN_NUMBER) {
				if(vlen < MaxDenom) {
					CUR_MM_X[vlen] = atof(tokenstr);
					vlen++;
				}
			} else if(token_type == TOKEN_ID) {
				CUR_MM_X[vlen] = st.GetSymbolValueF(tokenstr);
				vlen++;
			} else if(token_type == TOKEN_SEMICOLON) {
				IsMMX_Set = 1;
				state = WAIT_TYPE;
			}
			break;
		case WAIT_MMY_DATA :
			if(token_type == TOKEN_NUMBER) {
				if(vlen < MaxDenom) {
					CUR_MM_Y[vlen] = atof(tokenstr);
					vlen++;
				}
			} else if(token_type == TOKEN_SEMICOLON) {
				IsMMY_Set = 1;
				state = WAIT_TYPE;
			}
			break;
		case WAIT_VERSION_DATA :
			if(token_type == TOKEN_ID ) {
				varr[vlen++] = st.GetSymbolIndex(tokenstr);
			} else if(token_type == TOKEN_SEMICOLON) {
				IsDenVer_Set = 1;
				FillDenVersion(varr);
				state = WAIT_TYPE;
			}
			break;
		case WAIT_DEN_2 :
			if(token_type == TOKEN_ID)
				varr[vlen++] = st.GetSymbolIndex(tokenstr);
			else if(token_type == TOKEN_SEMICOLON) {
				IsDen2_Set = 1;
				FillDen2(varr);
//				RegionSetCount++; // 20121206dschae 이 줄이 문제였군. 여기서 이걸 증가시키면 안 된다. Den2는 Region이 아니므로
				state = WAIT_TYPE;
			}
		}
	}

	char NameOfRegionValue[64];
	sprintf_s(NameOfRegionValue, sizeof(NameOfRegionValue), "c_%s_Region_Value", pWinCurrentLocal->szCurrency);
	for(int ri = 0; ri < RegionSetCount; ri++) {
		if(strcmp(RegionSet[ri].ImgRegionName, NameOfRegionValue) == 0) {
			memcpy(pWinCurrentLocal->Region_Value, RegionSet[ri].Regions, sizeof(pWinCurrentLocal->Region_Value));
			pWinCurrentLocal->Region_Value_Num = RegionSet[ri].intRegionsCount;
			break;
		}
	}

	sprintf_s(NameOfRegionValue, sizeof(NameOfRegionValue), "c_%s_Region_Value_NNR", pWinCurrentLocal->szCurrency);
	for(int ri = 0; ri < RegionSetCount; ri++) {
		if(strcmp(RegionSet[ri].ImgRegionName, NameOfRegionValue) == 0) {
			memcpy(pWinCurrentLocal->Region_ValueNNR, RegionSet[ri].Regions, sizeof(pWinCurrentLocal->Region_ValueNNR));
			pWinCurrentLocal->Region_Value_NumNNR = RegionSet[ri].intRegionsCount;
			break;
		}
	}

	if(!IsDDLV_Set) {
		wnd->MessageBox(fn, L"Not found _DDLv[");
	}
	if(!IsDenomValue_Set) {
		wnd->MessageBox(fn, L"Not found _Denom[");
	}
	if(!IsMMX_Set) {
		wnd->MessageBox(fn, L"Not found _MMX[");
	}
	if(!IsMMY_Set) {
		wnd->MessageBox(fn, L"Not found _MMY[");
	}
	if(!IsDenVer_Set) {
		wnd->MessageBox(fn, L"Not found _VersionTable[");
	}
	if(!IsDen2_Set) {
		wnd->MessageBox(fn, L"Not found _Denom2[");
	}
}

void CurrencyInfo::LoadCurrencyRecogMethod(CWnd *wnd, TCHAR *fn, char* dataBuf)
{
}

BOOL FileExist(TCHAR* fn, WIN32_FIND_DATA* pwfd);

void CurrencyInfo::LoadCurrencyConfiguration(CWnd *wnd, TCHAR* cur_path)
{
	TCHAR tMsgTxt[MAX_PATH];
	TCHAR tMsgCap[MAX_PATH];
	TCHAR fn[MAX_PATH];

	// TRUE로 해 뒀다가 차후에 불리는 함수에서 데이터가 부족하면 FALSE가 된다.
	pWinCurrentLocal->CurrencyDataType = CURR_DATA_LOAD;

	// 일단 디폴트 영역을 지정해 준다. 안 그러면 실행 중에 죽는 문제가 있어서... 20120927 dschae 추가
	pWinCurrentLocal->Region_Value_Num = 1;
	pWinCurrentLocal->Region_Value[0].Region.Sx = -352;
	pWinCurrentLocal->Region_Value[0].Region.Ex = 352;
	pWinCurrentLocal->Region_Value[0].Region.Sy = -40;
	pWinCurrentLocal->Region_Value[0].Region.Ey = 40;
	pWinCurrentLocal->Region_Value[0].Region.State = 1;
	pWinCurrentLocal->Region_Value[0].Shift_X = 4;
	pWinCurrentLocal->Region_Value[0].Shift_Y = 4;
	pWinCurrentLocal->Region_Value[0].Sum_X = 2;
	pWinCurrentLocal->Region_Value[0].Sum_Y = 2;

	TCHAR no_info_text[MAX_PATH];
	wsprintf(no_info_text, L"%s\\no_support_files.txt", cur_path);
	WIN32_FIND_DATA wfd;
	if(FileExist(no_info_text, &wfd)) {
		pWinCurrentLocal->CurrencyDataType = CURR_DATA_JSON;
		return;
	}
}

void CurrencyInfo::LoadCurrencyData(CWnd* wnd, TCHAR *fn, char* dataBuf)
{
	int state = 0;
	int token_type;
	char tokenstr[128];

	IsRefCnt_Set = 0;
	IsTDenom_Set = 0;
	IsRefKey_Set = 0;
	IsNNRDataLoaded = 0;

	int x, y, z, v;

	int vlen = 0;
	int vmax = currencyInfo.MaxAreaX * currencyInfo.MaxAreaY * currencyInfo.MaxDataZ * SIFT_KEY_VECTOR;
	if(vmax <= 0)
		return;
	int *intarr = (int*)calloc(vmax, sizeof(int));

	char* nsp = dataBuf;
	state = WAIT_TYPE;
	while(token_type = get_next_token(&nsp, tokenstr)) {
		switch(state) {
		case WAIT_TYPE :
			if(token_type == TOKEN_ID) {
				if(strcmp(tokenstr, "const") == 0)
					state = WAIT_TYPE;
				else {
					data_type = GetDataType(tokenstr);
					state = WAIT_NAME;
				}
			}
			break;
		case WAIT_NAME :
			state = WAIT_SEMICOLON;
			if(token_type == TOKEN_SEMICOLON) {
				state = WAIT_TYPE;
				break;
			}
			if(token_type != TOKEN_ID) {
				state = WAIT_SEMICOLON;
				break;
			}
			strcpy_s(data_name, 64, tokenstr);
			state = WAIT_ASSIGN;
			break;
		case WAIT_ASSIGN :
			if(token_type == TOKEN_SEMICOLON) {
				state = WAIT_TYPE;
				break;
			}
			if(token_type == TOKEN_ASSIGN){
				name_type = GetNameType(data_name);
				state = WAIT_DATA;
				vlen = 0;
			}
			break;
		case WAIT_DATA :
			if(token_type == TOKEN_NUMBER) {
				if(vlen < vmax) {
					intarr[vlen] = atoi(tokenstr);
					vlen++;
				} else {
					if(vlen > 0)
					intarr[vlen-1] = 0; // ERROR
				}
				break;
			}
			if(token_type == TOKEN_SEMICOLON) {
				int vidx = 0;
				state = WAIT_TYPE;
				switch(name_type){
				case NT_REFCNT :
					IsRefCnt_Set = 1;
					for(y = 0; y < currencyInfo.MaxAreaY; y++) {
						for(x = 0; x < currencyInfo.MaxAreaX; x++) {
							Temp_Ref_Cnt[y][x] = intarr[vidx];
							vidx++;
						}
					}
					break;
				case NT_TDENOM :
					IsTDenom_Set = 1;
					for(y = 0; y < currencyInfo.MaxAreaY; y++) {
						for(x = 0; x < currencyInfo.MaxAreaX; x++) {
							for(z = 0; z < currencyInfo.MaxDataZ; z++) {
								Temp_Tdenom[y][x][z] = intarr[vidx];
								vidx++;
							}
						}
					}
					break;
//20121008 sapa82 V3용 Match_X 배열값 처리 추가
				case NT_Match_X :
					IsTDenom_Set = 1;
					for(y = 0; y < currencyInfo.MaxAreaY; y++) {
						for(x = 0; x < currencyInfo.MaxAreaX; x++) {							
							Temp_Match_X[y][x] = intarr[vidx];
							vidx++;							
						}
					}
					break;
				case NT_REFKEY :
					IsRefKey_Set = 1;
					// 이 문장에 iNR이 포함되어 있어서 INR의 경우에 인식이 안 되었던 것이다.
					// 초창기 INR이 V1을 사용하였지만 이제는 V1을 사용하지 않는다.
//					if(pWinCurrentLocal->CurrencyNum == EUR || pWinCurrentLocal->CurrencyNum == INR) {
					if(pWinCurrentLocal->RecognitionType == RECOG_METHOD_V1) { // EUR의 경우에 V1이다.
						for(y = 0; y < currencyInfo.MaxAreaY; y++) {
							for(x = 0; x < currencyInfo.MaxAreaX; x++) {
								for(z = 0; z < currencyInfo.MaxDataZ; z++) {
									for(v = 0; v < SIFT_KEY_VECTOR; v++) {
										V1_Ref_Key[y][x][z][v] = intarr[vidx];
										vidx++;
									}
								}
							}
						}
					} else {
						for(y = 0; y < currencyInfo.MaxAreaY; y++) {
							for(x = 0; x < currencyInfo.MaxAreaX; x++) {
								for(z = 0; z < currencyInfo.MaxDataZ; z++) {
									Temp_Ref_Key[y][x][z] = intarr[vidx];
									vidx++;
								}
							}
						}
					}
				}
			}
			break;
		case WAIT_SEMICOLON :
			if(token_type == TOKEN_SEMICOLON)
				state = WAIT_TYPE;
			break;
		}
	}

	if(!IsRefCnt_Set) {
		wnd->MessageBox(fn, L"Not found _Ref_Cnt[");
	}
	if(!IsTDenom_Set) {
		wnd->MessageBox(fn, L"Not found _TDenom[");
	}
	if(!IsRefKey_Set) {
		wnd->MessageBox(fn, L"Not found _Ref_Key[");
	}

	free(intarr);
}

int CurrencyInfo::FillIntArraySymbols(char* begin, char* end, int Result[], int size)
{
	int count = 0;
	char *sp;
	char vstr[64];
	int len = 0;
	
	for(sp = begin; sp < end && count < size; sp++) {
		if( *sp == '_' || (*sp >= '0' && *sp <= '9') || (*sp >= 'a' && *sp <= 'z') || (*sp >= 'A' && *sp <= 'Z') ) {
			if(len < 63)
				vstr[len++] = *sp;
		} else {
			if(len > 0) {
				vstr[len] = 0;
				Result[count++] = st.GetSymbolIndex(vstr);
				len = 0;
			}
		}
	}

	return count;
}


void CurrencyInfo::FillDenVersion(int* data)
{
	for(int i = 0; i < currencyInfo.MaxCurrencyNum; i++) {
		this->CUR_VersionTable[i].Vers = *data++;
		this->CUR_VersionTable[i].DenomIndex = *data++;
		char buf[128];
		sprintf_s(buf, 128, "%s,%s\r\n",
			st.SymbolNames[CUR_VersionTable[i].Vers],
			st.SymbolNames[CUR_VersionTable[i].DenomIndex] );
		TCHAR tbuf[128];
		MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf)+1, tbuf, 128);
		TRACE1("%s", tbuf);
	}
}

void CurrencyInfo::FillDen2(int* data)
{
	for(int i = 0; i < currencyInfo.MaxCurrencyNum; i++) {
		this->CUR_Denom2[i] = *data++;
	}
}

void CurrencyInfo::Set_TDenom(int* CUR_TDenom)
{
}

void CurrencyInfo::SetKeyRef(int* CUR_KeyRef)
{
}

//Uint16 matched_x[MAX_DENOM_SET][currencyInfo.MaxAreaX];
Uint16 matched_x[MAX_DENOM_SET][MAX_AREA_X];
Uint16 s_matched_x[5], sa_matched_x[3];

ST_IMG_REGION* CurrencyInfo::FindRegionByIndex(int index)
{
	int no = 0;
	for(int ri = 0; ri < RegionSetCount; ri++) {
		for(int i = 0; i < RegionSet[ri].intRegionsCount; i++) {
			if(no == index)
				return &RegionSet[ri].Regions[i];
			no++;
		}
	}
	
	return NULL;
}

ST_IMG_REGION* CurrencyInfo::FindRegionByName(LPCTSTR name, LPCTSTR szindx)
{
	char szbuf[128];
	WideCharToMultiByte(CP_ACP, 0, name, lstrlen(name)+1, szbuf, 128, NULL, NULL);
	int indx = StrToInt(szindx);
	if(indx < 0) indx = 0;

	for(int ri = 0; ri < RegionSetCount; ri++) {
		if(strcmp(RegionSet[ri].ImgRegionName, szbuf) == 0) {
			return &RegionSet[ri].Regions[indx];
		}
	}
	
	return NULL;
}

ST_IMG_REGION* CurrencyInfo::GetImgRegion(int index)
{
	int no = 0;
	for(int ri = 0; ri < RegionSetCount; ri++) {
		for(int ii = 0; ii < RegionSet[ri].intRegionsCount; ii++) {
			if(no == index)
				return &RegionSet[ri].Regions[ii];
			no++;
		}
	}
	
	return NULL;
}

void CurrencyInfo::AddRegionSet(REGION_SET* prs)
{
	if(RegionSetCount >= MAX_REGION_SET)
		return;

	RegionSet[RegionSetCount] = *prs;
	RegionSetCount++;
}

Uint16 CurrencyInfo::FindAndCopyRegionSet(TCHAR* regionName, ST_IMG_REGION** ppRegionValue)
{
	char szbuf[128];
	WideCharToMultiByte(CP_ACP, 0, regionName, lstrlen(regionName)+1, szbuf, 128, NULL, NULL);

	for(int ri = 0; ri < RegionSetCount; ri++) {
		if(strcmp(RegionSet[ri].ImgRegionName, szbuf) == 0) {
			*ppRegionValue = RegionSet[ri].Regions;
			return RegionSet[ri].intRegionsCount;
		}
	}

	return 0;
}

void CurrencyInfo::LoadNNRData(CWnd* pWnd)
{
	if(LoadNNR_Data(pWnd, NULL, NULL, &NNR_Data))
		this->IsNNRDataLoaded = 1;
}

BOOL LoadNNR_Data(CWnd *pWnd, TCHAR* Folder, TCHAR* tszFileName, NNR_DATA_TYPE* pNNR_Data)
{
	if(Folder != NULL)
		SetCurrentDirectory(Folder);

	CString FileName;
	if(tszFileName != NULL && tszFileName[0])
		FileName = tszFileName;
	else {
		CString FileFormat;
		FileFormat = "iq";
		CString str = FileFormat + " 파일(*." + FileFormat + ")|*." + FileFormat+ "| 모든파일(*.*)|*.*|";
		CFileDialog Dlg(TRUE, FileFormat, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, str, NULL);
		if(Dlg.DoModal() != IDOK)
			return FALSE;

		FileName =  Dlg.GetFileName();
	}

	CFile file;
	if(!file.Open(FileName, CFile::modeRead)) {
		if(pWnd != NULL)
			pWnd->MessageBox(FileName, L"해당파일을 찾을수 없습니다.");
		return FALSE;
	}

	int length = file.GetLength();
	char *data = (char*)malloc(length+1);
	file.Read(data, length);
	file.Close();

	CCodeParser* cp = new CCodeParser();
	cp->ParseText(data);
	free(data);

	for(int i = 0; i < cp->macro_count; i++) {
		if(strstr(cp->macro_names[i], "NNR_FHIDDEN_SIZE")) {
			pNNR_Data->m_HiddenNodeSize = atoi(cp->macro_values[i]);
		}
		if(strstr(cp->macro_names[i], "NNR_INPUTNODE_SIZE")) {
			pNNR_Data->m_InputNodeSize = atoi(cp->macro_values[i]);
		}
		if(strstr(cp->macro_names[i], "NNR_CLASS_NUM")) {
			pNNR_Data->m_ClassSize = atoi(cp->macro_values[i]);
		}
	}
	
	for(int i = 0; i < cp->assign_count; i++) {
		if(strstr(cp->assign[i].name, "HiddenWeight")) {
			int pos = cp->assign[i].start_pos;
			char* data_pos = cp->assign_buf;
			for(pNNR_Data->HiddenWeightCount = 0; pNNR_Data->HiddenWeightCount < cp->assign[i].data_count; pNNR_Data->HiddenWeightCount++) {
				pNNR_Data->HiddenWeight[pNNR_Data->HiddenWeightCount] = atoi(data_pos+pos);
				pos += strlen(data_pos+pos) + 1;
			}
		}
		if(strstr(cp->assign[i].name, "OutputWeight")) {
			int pos = cp->assign[i].start_pos;
			char* data_pos = cp->assign_buf;
			for(pNNR_Data->OutputWeightCount = 0; pNNR_Data->OutputWeightCount < cp->assign[i].data_count; pNNR_Data->OutputWeightCount++) {
				pNNR_Data->OutputWeight[pNNR_Data->OutputWeightCount] = atoi(data_pos+pos);
				pos += strlen(data_pos+pos) + 1;
			}
		}
		if(strstr(cp->assign[i].name, "ClassName")) {
			int pos = cp->assign[i].start_pos;
			char* data_pos = cp->assign_buf;
			for(pNNR_Data->ClassNameCount = 0; pNNR_Data->ClassNameCount < cp->assign[i].data_count; pNNR_Data->ClassNameCount++) {
				strcpy_s(pNNR_Data->ClassName[pNNR_Data->ClassNameCount], NAME_SIZE, data_pos+pos);
				pos += strlen(data_pos+pos) + 1;
				if(pNNR_Data->ClassNameCount >= MAX_CLASS)
					break;
			}
		}
	}

	delete cp;

	return TRUE;
}

BOOL CurrencyInfo::GetRegionByIndex(int SetIndex, int index, ST_IMG_REGION* pReg, char** name)
{
	*name = NULL;
	if(SetIndex >= RegionPtrArrayCount)
		return FALSE;
	
	if(index >= RegionPtrArray[SetIndex].Count)
		return FALSE;
	
	char* regName = RegionPtrArray[SetIndex].szRegions[index];
	*name = regName;

	for(int i = 0; i < RegionSetCount; i++) {
		if(strcmp(regName, RegionSet[i].ImgRegionName) == 0) {
			*pReg = RegionSet[i].Regions[0];
			return TRUE;
		}
	}
	
	*name = NULL;
	return FALSE;
}

void CurrencyInfo::ResetCurrentLocal()
{
	if(pWinCurrentLocal != NULL) {
		pWinCurrentLocal->tszCurrency[0] = pWinCurrentLocal->szCurrency[0] = 0;
	}
}
