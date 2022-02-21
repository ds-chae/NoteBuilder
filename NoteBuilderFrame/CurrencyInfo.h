#include "CodeParser.h"

#pragma once

#define	MAX_CUR_DENOMS		20
#define	MAX_CUR_VERSIONS	10
#define	MAX_DENOM_SET	(MAX_CUR_DENOMS*MAX_CUR_VERSIONS)	// 10 Versions, 10 Denoms

#define MAX_AREA_X		150
#define MAX_AREA_Y		150
#define CUR_MAX_DATA	150

#define	MAX_UV_ADC 200

#define	MAX_REGION_SET	100

// 10 Denominations * 4 Direction * 5 Emissions = 200
#define	MAX_CLASS	(10*4*5)
#define	MAX_INPUT_NODE	4000
#define	MAX_HIDDEN_NODE	(MAX_CLASS+5)
#define	MAX_HIDDEN_WEIGHT_SIZE	(MAX_INPUT_NODE*MAX_HIDDEN_NODE)
#define	MAX_OUTPUT_WEIGHT_SIZE	(MAX_HIDDEN_NODE*MAX_CLASS)

typedef struct {
	int m_HiddenNodeSize;
	int  m_InputNodeSize;
	int  m_ClassSize;

	int  ClassNameCount;
	char ClassName[MAX_CLASS][NAME_SIZE];

	int  HiddenWeightCount;
	long HiddenWeight[MAX_HIDDEN_WEIGHT_SIZE];
	int  OutputWeightCount;
	long OutputWeight[MAX_OUTPUT_WEIGHT_SIZE];
} NNR_DATA_TYPE;

typedef struct TAG_WIN_ST_LOCAL {
	int CurrencyNum;    // num of Currency
	int CountOfValues;
	TCHAR tszCurrency[8];
	char  szCurrency[8];
	int  Versions;
#define	CURR_DATA_NONE	0
#define	CURR_DATA_LOAD	1
#define	CURR_DATA_JSON	2
	int  CurrencyDataType;
	int RecognitionType;
} WIN_ST_LOCAL;

//extern WIN_ST_LOCAL *pWinCurrentLocal;
//extern WIN_ST_LOCAL WinCurrentLocal;

typedef struct tag_evaltoken {
	int type;
	int ivalue;
	float fvalue;
	char str[64];
} evaltoken;

class ST_REGIONPTRS
{
public:
	char szArrayName[64];
	int  Count;
	char szRegions[64][64];

public:
	ST_REGIONPTRS()
	{
		clear();
	}

	void clear()
	{
		szArrayName[0] = 0;
		Count = 0;
		memset(szRegions, 0, sizeof(szRegions));
	}

	void add(char* name)
	{
		strcpy_s(szRegions[Count], sizeof(szRegions[Count]), name);
		Count++;
	}
};
/*
class CurrencyInfo
{
public:
	CurrencyInfo(void);
	~CurrencyInfo(void);

	WIN_ST_LOCAL *m_pLocal;

public:
	void SetDDLV(int* CUR_DDLV);
	void SetDenoms(int*denarr);

	void AddRegionSet(REGION_SET* prs);
	ST_IMG_REGION* GetImgRegion(int index);
	ST_IMG_REGION *FindRegionByName(LPCTSTR name, LPCTSTR indx);
	ST_IMG_REGION *FindRegionByIndex(int index);
	Uint16 FindAndCopyRegionSet(TCHAR* regionName, ST_IMG_REGION** ppRegionValue);
	BOOL GetRegionByIndex(int SetIndex, int index, ST_IMG_REGION* pReg, char** name);

	void SetImgRegion(ST_IMG_REGION *pReg, int *CUR_Regions);
	void LoadCurrencyData(CWnd* wnd, TCHAR *fn, char* dataBuf);
	void LoadCurrencyConfiguration(CWnd *wnd, TCHAR* cur_path);
	void LoadCurrencyConst(CWnd *wnd, TCHAR *fn, char* dataBuf);
	void LoadRegionSet(char* sp);
	void LoadCurrencyDefinition(CWnd *wnd, TCHAR* fn, char* dataBuf);
	void LoadCurrencyRecogMethod(CWnd *wnd, TCHAR *fn, char* dataBuf);
	void LoadNNRData(CWnd *pWnd);
	void ResetCurrentLocal();

	void ParseDefinitionLine(char* line);

	void PutArrToRegion(int *varr, int vlen, REGION_SET *pRegSet);
	void InitializeCurrencyInfo(WIN_ST_LOCAL *pLocal);
	void LoadCurrencyInfoFile(TCHAR* File, WIN_ST_LOCAL *pLocal);
	void ParseConfigOneLine(char* ptr);
	void SetRegionDefault(ST_IMG_REGION *pReg);
	int GetIntValueWithPrefix(char* prefix, char* body);
	int FillIntArraySymbols(char* begin, char* end, int Result[], int size);
	void FillDenVersion(int* data);
	void FillDen2(int* data);
	void SetRef_Cnt(int* CUR_Ref_Cnt);
	void Set_TDenom(int* CUR_TDenom);
	void SetKeyRef(int* CUR_KeyRef);

//20120921 sapa82	v3 적용
	void V3_CIS_Analysis(NOTE_STRUCT *pNote); //권종별 key matching routine
	void V3_ST150_Denom(NOTE_STRUCT *Note);
	void V3_ST150_Value(NOTE_STRUCT *pNote);

public:
	REGION_SET	RegionSet[100];
	int  RegionSetCount;

	int RegionPtrArrayCount;
	ST_REGIONPTRS RegionPtrArray[20];

	int denomValue[MAX_CUR_DENOMS];
	int MaxDenom; // USD_MAXDENOM

	int MaxCurrencyNum; // USD_CURRENCY_NUM
	int MaxAreaY; // USD_AREA_X
	int MaxAreaX; // USD_AREA_Y
	int MaxDataZ; // USD_MAX_DATA

	float	CUR_MM_X[MAX_DENOM_SET];
	float	CUR_MM_Y[MAX_DENOM_SET];
	Uint16 CUR_DDLV[MAX_DENOM_SET];

	int Temp_Ref_Cnt[MAX_AREA_Y][MAX_AREA_X];
	int Temp_Tdenom[MAX_AREA_Y][MAX_AREA_X][CUR_MAX_DATA];
	int Temp_Ref_Key[MAX_AREA_Y][MAX_AREA_X][CUR_MAX_DATA];
	int V1_Ref_Key[MAX_AREA_Y][MAX_AREA_X][CUR_MAX_DATA][SIFT_KEY_VECTOR];

//20121008 sapa82 V3용 Match_X 배열값 처리 추가
	int Temp_Match_X[MAX_CUR_DENOMS][5];

	ST_DENOM_VERSION CUR_VersionTable[MAX_DENOM_SET];
	Uint16 CUR_Denom2[MAX_DENOM_SET];

	char VerString[64];
	char DenString[64];

	int IsDDLV_Set;
	int IsDenomValue_Set;
	int IsMMX_Set;
	int IsMMY_Set;
	int IsDenVer_Set;
	int IsDen2_Set;
	int IsRefCnt_Set;
	int IsTDenom_Set;
	int IsRefKey_Set;

	int IsNNRDataLoaded;
	NNR_DATA_TYPE NNR_Data;

	int  data_type;
	int  name_type;
	char data_name[128];

	int  varr[100];
	char sarr[100][128];
};
*/

//extern CurrencyInfo currencyInfo;

extern	TCHAR m_CurrencyImageFolder[MAX_PATH];

BOOL LoadNNR_Data(CWnd *pWnd, TCHAR* Folder, TCHAR* tszFileName, NNR_DATA_TYPE* pNNR_Data);

class SymbolTable {
public:
	SymbolTable();

#define	MAX_SYMBOL	200
public:
	int SymbolCount;
	char SymbolValue[MAX_SYMBOL][64];
	char SymbolNames[MAX_SYMBOL][64];

public:
	void Init();
	void AddSymbol(char* symbolname, char* tokenstr);
	int GetSymbolStr(char *vstr, char* rstr, int size);
	int GetSymbolValue(int index);
	float GetSymbolValueF(int index);
	float GetSymbolValueF(char* name);
	int GetSymbolValue(char* name, evaltoken& et);
	int GetSymbolIndex(char *vstr);
};
