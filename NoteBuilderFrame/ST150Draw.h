#pragma once

#define	MAX_CUR_DENOMS		20
#define	MAX_CUR_VERSIONS	10
#define	MAX_DENOM_SET	(MAX_CUR_DENOMS*MAX_CUR_VERSIONS)	// 10 Versions, 10 Denoms

#define MAX_AREA_X		150
#define MAX_AREA_Y		150
#define CUR_MAX_DATA	150

#define	MAX_UV_ADC 200

#define	DOC_TYPE_IMAGE	1
#define	DOC_TYPE_MG		2
#define	DOC_TYPE_SIMPLE_GRAPH	3
#define	DOC_TYPE_TAPE	4
#define	DOC_TYPE_UV		5

class ST150Image
{
public:
	ST150Image(void);
	~ST150Image(void);

public:
	Mat mat_image;

	ST_NOTE OneNote;

	TCHAR m_Filename[MAX_PATH];

	CSize m_WImageSize;

	CSize m_IImageSize;

	CSize m_ImageSize;

	CSize size;

	int m_DocumentType;

	int m_TapeImageSize;
	unsigned char m_TapeImageBuffer[8192];

	int m_UVImageSize;
	unsigned char m_UVImageBuffer[8192];

	int m_MGImageSize;
	unsigned char m_MGImageBuffer[8192];

	int m_RawDataSize;
	unsigned char m_RawData[1024];

	unsigned char	m_IR_ImageBuffer[800][1728];
	unsigned char	m_W_ImageBuffer[800][1728];
	char g_KeyTxBuf[8192];

	int t_IR, t_W;
	int RegionSize;

	int ExtMinW;
	int ExtMaxW;
	int ExtMinIR;
	int ExtMaxIR;

	int ExtVarW;
	int ExtVarIR;

	int TapeTrackStart[12];
	int TapeTrackEnd[12];
	int TapeDisplayGap;

	int m_WThreshold;
	int m_IrThreshold;

	int m_WPixels;
	int m_IrPixels;

public:
	void ApplyThreshold(int w_threshold, int ir_threshold);
	void ApplyPercent(int w_percent, int ir_percent);
	void ResetRateGeneral();
	void LoadRegionSet(char* sp, char* ep);
	CBitmap* ExtractRegion(CDC* pDC, ST_IMG_REGION* pRegion);

	int TestMousePoint(CPoint& point);
	void GetImageValue(CPoint point, int *iv);
	int GetImageAverage(CPoint p1, CPoint p2, int *iv);

	int ReadFileData(LPCTSTR tszFileName, int size, unsigned char* buf);
	void DrawMG(CDC* pDC);
	int SaveToBitmap(TCHAR* bmpFileName);
	void DrawUV(CDC* pDC);
	void DrawTape(CDC* pDC);
	void OnDraw(CDC *pDC);
	int LoadImageFile(LPCTSTR szPathName);
	void DrawSimpleGraph(CDC *pDC);
	void ParseUVData();
	void ParseTapeData();

	int FillIntArraySymbols(char* begin, char* end, int Result[], int size);
	void SetImgRegion(ST_IMG_REGION *pReg, int *CUR_Regions);
	int GetSymbolValue(int index);
	int GetSymbolValue(char* name);

	void FillDenVersion(int* data);
	void FillDen2(int* data);

	void SetKeyRef(int* CUR_KeyRef);
	void Set_TDenom(int* CUR_TDenom);
	void SetRef_Cnt(int* CUR_Ref_Cnt);
	void ClearMem();
	int LoadFromFile(LPCTSTR tszFileName);
	void FlipImageHorizontal();
	void FlipImageVertical();
	CBitmap* MakeBitmapFromRaw(CDC *pDC, int w_threshold, int ir_threshold, int ImgReinforce);
	CBitmap* MakeDebugImage(CDC* pDC);
	void RecognizeCurrency(int ImageSource);
	void GetWidthHeight(LPCTSTR str);
	CBitmap* MakeRegionImage(CDC *pDC, int RegionNumber);
	int GetLastNamePosition(TCHAR *name);
	void V1_SaveKeyToFile();
	void V2_SaveKeyToFile();
};

int GetDigitNumber(TCHAR *s);
