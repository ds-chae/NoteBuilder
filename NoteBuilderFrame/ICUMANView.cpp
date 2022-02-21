// ICUMANView.cpp : implementation of the CICUMANView class
//

#include "stdafx.h"
#include "mainfrm.h"
#include "string.h"
#include "NoteBuilderFrame.h"

//#define GLOBAL_VALUE_INIT
//#define GLOBAL_CIS_DEF

#include "ICUMANDoc.h"
#include "Define.h"
#include "opencv_use.h"
#include "CarNumDetect.h"
#include "ICUMANView.h"
#include "CurrencyInfo.h"
#include "RegionTable.h"
#include "RegionTestDlg.h"
#include "OCRDlg.h"
#include "libs\tiff-v3.6.1\libtiff\tiffio.h"
#include "ImageTestDlg.h"
#include "BuildLog.h"
#include "wdebug.h"
#include "ScrollWnd.h"

#include "Setting.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CICUMANView, CScrollView)

BEGIN_MESSAGE_MAP(CICUMANView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CICUMANView::OnFilePrintPreview)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_VIEW_DEBUGIMAGE, &CICUMANView::OnViewDebugimage)
	ON_COMMAND(ID_CURRENCY_CALIBRATIION, &CICUMANView::OnCurrencySetting) //ryuhs74
//	ON_COMMAND(ID_OCR, &CICUMANView::OnOcr)
	ON_COMMAND(ID_VIEW_IMAGE_TESTER, &CICUMANView::OnViewImageTester)
	ON_UPDATE_COMMAND_UI(ID_OCR, &CICUMANView::OnUpdateOcr)
//	ON_COMMAND(ID_VIEW_MG_UV_CIS, OnViewMgUvCIS)
ON_WM_CREATE()
END_MESSAGE_MAP()

// CICUMANView construction/destruction

CICUMANView* pMainView = NULL;
CCarNumDetect *pPlateDetector = NULL;

CICUMANView::CICUMANView()
{
	pMainView = this;

	// TODO: add construction code here
	EntryCount = 0;

	szPathName[0] = 0;
	szLeafName[0] = 0;
	
//	memset(&gst_State, 0, sizeof(gst_State));

//	m_hBitmap = NULL;
	ExtBitmap = NULL;;
	MergeBitmap = NULL;

#if TRY_LINEAR_ROTATION
	memset(m_RegionImageLinear, 0, sizeof(m_RegionImageLinear));
#endif

	ScrollSize.cx = 0;
	ScrollSize.cy = 0;
	
	SetScrollSizes(MM_TEXT, ScrollSize);

	tszRectAverage[0] = 0;
	m_bDown = FALSE;
	ThreadType = 0;
	ThreadWidth = 5;
	ThreadPos = -1;

	m_DownPoint.x = 0;
	m_DownPoint.y = 0;
	m_MovePoint.x = 0;
	m_MovePoint.y = 0;

	m_ImgReinforce = 0;

	//ryuhs74@20120313 - Calibration Struct init
	m_sSetting.nCFLevel[0] = m_sSetting.nCFLevel[1] = 7;
	m_sSetting.nDeinkedLevel = 87;
	m_sSetting.nHoleLevel = 10;
	m_sSetting.nSoiledLevel[0] = m_sSetting.nSoiledLevel[1] = m_sSetting.nSoiledLevel[2] = m_sSetting.nSoiledLevel[3] = 87;
	m_sSetting.nSoiledLevel[4] = m_sSetting.nSoiledLevel[5] = m_sSetting.nSoiledLevel[6] = 87;
	m_sSetting.nStainLevel = 50;
	m_sSetting.nTapeLevel = 50;
	m_sSetting.nWashedLevel = 50;

	//HDS
	carNumDetect.window_original = L"Blurred Map";
	carNumDetect.window_process = L"Process Map";
	carNumDetect.window_result = L"Result Map";
}

CICUMANView::~CICUMANView()
{
	ClearMem();
	if(pPlateDetector != NULL)
		delete pPlateDetector;
	pPlateDetector = NULL;
}

void CICUMANView::ClearMem()
{
	if(ExtBitmap != NULL) {
		delete ExtBitmap;
		ExtBitmap = NULL;
	}
	if(MergeBitmap != NULL) {
		delete MergeBitmap;
		MergeBitmap = NULL;
	}

	tszRectAverage[0] = 0;

//	if(m_hBitmap != NULL) delete 	m_hBitmap;
//	m_hBitmap = NULL;

	EntryCount = 0;

	DirFromDlg = 4; // set unknown dir

	ThreadPos = -1;
}

BOOL CICUMANView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style |= WS_HSCROLL;
	return CScrollView::PreCreateWindow(cs);
}

void DrawXorRect(CDC *pDC, CPoint& p1, CPoint& p2)
{
	int OldROP2 = pDC->SetROP2(R2_NOTXORPEN);
	pDC->MoveTo(p1);
	pDC->LineTo(p2.x, p1.y);
	pDC->LineTo(p2.x, p2.y);
	pDC->LineTo(p1.x, p2.y);
	pDC->LineTo(p1);
	pDC->SetROP2(OldROP2);
}

void CICUMANView::DisplayRegionFromSelf(CDC* pDC)
{
/*
	CPoint dp = m_DownPoint;
	CPoint mp = m_MovePoint;

	dp.x -= m_ViewportOrg.x;
	dp.y -= m_ViewportOrg.y;

	mp.x -= m_ViewportOrg.x;
	mp.y -= m_ViewportOrg.y;

	DrawXorRect(pDC, dp, mp);

	if(dp.y < imgST150.m_W_ImageSize.cy) {
		dp.y += imgST150.m_W_ImageSize.cy;
		mp.y += imgST150.m_W_ImageSize.cy;
	} else {
		dp.y -= imgST150.m_W_ImageSize.cy;
		mp.y -= imgST150.m_W_ImageSize.cy;
	}
	DrawXorRect(pDC, dp, mp);

	if(m_MovePoint.x != m_DownPoint.x && m_MovePoint.y != m_DownPoint.y) {
		int dir = (DirFromDlg < 4) ? DirFromDlg : imgST150.OneNote.Dir;
		int x1diff, x2diff;
		switch(dir) {
		case DIR_FF :
		case DIR_BB :
			x1diff = DownPoint.x - imgST150.OneNote.IAR_A.Center_X;
			x2diff = MovePoint.x - imgST150.OneNote.IAR_A.Center_X;
			break;
		default :
			x1diff = imgST150.OneNote.IAR_A.Center_X - DownPoint.x;
			x2diff = imgST150.OneNote.IAR_A.Center_X - MovePoint.x;
		}

		wsprintf(tszRectAverage, L"R(%d,%d)(%d,%d) Average: W=%d, IR=%d",
			x1diff, DownPoint.y - imgST150.OneNote.IAR_A.Center_Y,
			x2diff, MovePoint.y - imgST150.OneNote.IAR_A.Center_Y,
			AverageW, AverageIR);

		pDC->TextOutW(0, DrawHeight + 45, tszRectAverage, lstrlen(tszRectAverage));
	}
*/
}

void CICUMANView::ToggleImgReinforce()
{
	m_ImgReinforce = ! m_ImgReinforce;
//	MakeImageFromFiles();
	Invalidate();
}


static int Bpp(cv::Mat img) { return 8 * img.channels(); }

void FillBitmapInfo(BITMAPINFO* bmi, int width, int height, int bpp, int origin) 
{
	assert(bmi && width >= 0 && height >= 0 && (bpp == 8 || bpp == 24 || bpp == 32)); 

	BITMAPINFOHEADER* bmih = &(bmi->bmiHeader); 

	memset(bmih, 0, sizeof(*bmih)); 
	bmih->biSize = sizeof(BITMAPINFOHEADER); 
	bmih->biWidth = width; 
	bmih->biHeight = origin ? abs(height) : -abs(height); 
	bmih->biPlanes = 1; 
	bmih->biBitCount = (unsigned short)bpp; 
	bmih->biCompression = BI_RGB; 

	if (bpp == 8) 
	{ 
		RGBQUAD* palette = bmi->bmiColors; 

				for (int i = 0; i < 256; i++) 
		{ 
			palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i; 
			palette[i].rgbReserved = 0; 
		} 
	} 
}

//char* window_name = "Edge Map";
int edgeThresh = 1;

Mat mat_src, mat_src_gray;
Mat mat_dst, detected_edges;
Mat und_src;
Mat croppedImage, cropped_gray;
Mat cropped_edges, cropped_dst;

void CICUMANView::DrawMainImage(CDC *pDC)
{
	if(mat_src.empty()) return;

	int height = mat_src.rows;
	int width = mat_src.cols;
	uchar buffer[sizeof( BITMAPINFOHEADER ) + 1024]; 
	BITMAPINFO* bmi = (BITMAPINFO* )buffer; 
	FillBitmapInfo(bmi, width, height, Bpp(mat_src),0);
	SetDIBitsToDevice(pDC->GetSafeHdc(), 0, 0, width, height, 0, 0, 0, height, mat_src.data, bmi, DIB_RGB_COLORS);
/*
	pDC->TextOutW(0, DrawHeight + 5 + 18, tszbuf, lstrlen(tszbuf));
	SetTextColor(pDC->m_hDC, RGB(0,0,0));
*/
}

void CICUMANView::OnDraw(CDC* pDC)
{
	CICUMANDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	DrawHeight = 25;

	// TODO: add draw code for native data here
	if(EntryCount == 0 && mat_src.empty()) {
		pDC->TextOut(0, 0, L"EMPTY", 5);
		return;
	}
	
	if(EntryCount > 0) {
		int y;
		for(y = 0; y < EntryCount; y++) {
			pDC->TextOutW(0, y*20, DirEntries[y], lstrlen(DirEntries[y]));
		}
		return;
	}

	DrawHeight = 0;
	DrawMainImage(pDC);

	m_ViewportOrg = pDC->GetViewportOrg();
	// 선택 영역 그리기
	
	if(m_DownPoint.x != m_MovePoint.x && m_DownPoint.y != m_MovePoint.y)
		DisplayRegionFromSelf(pDC);
}


// CICUMANView printing


void CICUMANView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

BOOL CICUMANView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CICUMANView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CICUMANView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CICUMANView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CICUMANView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CICUMANView diagnostics

#ifdef _DEBUG
void CICUMANView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CICUMANView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CICUMANDoc* CICUMANView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CICUMANDoc)));
	return (CICUMANDoc*)m_pDocument;
}
#endif //_DEBUG


// CICUMANView message handlers

void CICUMANView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	CICUMANDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CMainFrame *pMainFrm = (CMainFrame*)this->GetParentFrame();
	if(pMainFrm->m_FirstPath[0]) {
		// 상위 폴더이므로 이것도 변환 여부를 검사하지 않는다.
		SetPathName(pMainFrm->m_FirstPath);
	}

	RECT r;
	GetClientRect(&r);
	ScrollSize.cx = r.right;
	ScrollSize.cy = r.bottom;
	SetScrollSizes(MM_TEXT, ScrollSize);
/*
	RECT r;
	GetClientRect(&r);
	CBrush WhiteBrush;
	WhiteBrush.CreateSolidBrush(RGB(255,255,255));
	pDC->FillRect(&r, &WhiteBrush);
	WhiteBrush.DeleteObject();
*/
}

void GetWidthHeight(TCHAR* path, int* width, int* height)
{
	int base;
	TCHAR *sp = path;
	
	*width = *height = 0;
	
	// 이름 끝까지 가기
	while(*sp) sp++;
	// 높이 구하기
	// 숫자 아닌 부분 지나가기
	while(sp != path) {
		sp--;
		if(*sp == '.')
			break;
	}
	if(*sp != '.')
		return;

	base = 1;
	sp--;
	while(*sp >= '0' && *sp <= '9') {
		*height = *height + base * (*sp - '0');
		base *= 10;
		sp--;
	}
	if(*sp != '_')
		return;
	
	base = 1;
	sp--;
	while(*sp >= '0' && *sp <= '9') {
		*width = *width + base * (*sp - '0');
		base *= 10;
		sp--;
	}
}

#if 0
void ConvertRawToTIFF(TCHAR* path)
{
	int width = 0;
	int height = 0;
	GetWidthHeight(path, &width, &height);
	if(width == 0 || height == 0)
		return;

	HANDLE h = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return;
	int fSize = GetFileSize(h, NULL);
	if(height != fSize/width)
		height = fSize/width;
	unsigned char *buf = (unsigned char*)malloc(fSize);
	DWORD dwio = 0;
	ReadFile(h, buf, fSize, &dwio, NULL);
	CloseHandle(h);

	TCHAR tiffName[MAX_PATH];
	lstrcpy(tiffName, path);
	lstrcpy(tiffName+lstrlen(tiffName)-4, L".tif");
	char szTiffName[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, tiffName, lstrlen(tiffName)+1, szTiffName, MAX_PATH, NULL, NULL);

	TIFF *tif = TIFFOpen(szTiffName, "w");
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,8);
	uint32 rowsperstrip = TIFFDefaultStripSize(tif, -1); 
	//<REC> gives better compression
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
	TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS); 
/*
			// Start CCITTFAX3 setting
			{uint32 group3options = GROUP3OPT_FILLBITS+GROUP3OPT_2DENCODING;
			TIFFSetField(tif, TIFFTAG_GROUP3OPTIONS, group3options);}
			TIFFSetField(tif, TIFFTAG_FAXMODE, FAXMODE_CLASSF);
			TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
			TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, -1L);
			// End CCITTFAX3 setting
*/
	//if(bitsPerPixel != 1) TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFSetField(tif, TIFFTAG_XRESOLUTION, 200.0);
	TIFFSetField(tif, TIFFTAG_YRESOLUTION, 200.0);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK); 

/* 
			char page_number[20];
			sprintf(page_number, "Page %d", 1);

			TIFFSetField(image, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
			TIFFSetField(image, TIFFTAG_PAGENUMBER, 1, 1);
			TIFFSetField(image, TIFFTAG_PAGENAME, page_number);
*/
/*			//   Some other essential fields to set that you do not have to understand for now.
			TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK); 
			{uint32 rowsperstrip = TIFFDefaultStripSize(tif, -1);
			TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip);}
*/
	for(int y = 0; y < height; y++) {
		TIFFWriteScanline(tif, buf + (y * width), y, 0);
	}
	TIFFWriteDirectory(tif);
	TIFFClose(tif);

	free(buf);
}
#endif

int CICUMANView::FillListFromDirectoryEntries()
{
	int scancount = 0;
	int ConversionCalled = 0;

	WIN32_FIND_DATA wfd;
	TCHAR szFindName[MAX_PATH];
	wsprintf(szFindName, L"%s\\*", szPathName);

	EntryCount = 0;
	HANDLE hFind = FindFirstFile(szFindName, &wfd);
	while(hFind != INVALID_HANDLE_VALUE ) {
		if(lstrcmp(wfd.cFileName, L".") && lstrcmp(wfd.cFileName, L"..")) {
			lstrcpy(DirEntries[EntryCount], wfd.cFileName);
			EntryCount++;
		}
		if(!FindNextFile(hFind, &wfd)) {
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE ;
		}
	}

	CSize s;
	CRect r;
	GetClientRect(&r);
	s.cx = r.Size().cx;
	s.cy = 20 * EntryCount;
	SetScrollSizes(MM_TEXT, s);

	return ConversionCalled;
}

#if 0
void CICUMANView::MakeImageFromFiles()
{
	if(m_hBitmap != NULL)
		delete m_hBitmap;

	CDC *pDC = GetDC();
	switch(this->m_ViewType){
	case 0 : // IAR_A
		m_hBitmap = imgST150.MakeBitmapFromRaw(pDC, m_WThreshold, m_IrThreshold, m_ImgReinforce);
		SetScrollSizes(MM_TEXT, imgST150.m_ImageSize);
		break;
/*
	case 1 :
		m_hBitmap = imgST150.MakeBitmapFromMG(pDC);
		SetScrollSizes(MM_TEXT, imgST150.size);
		break;
	case 2 :
		m_hBitmap = imgST150.MakeBitmapFromUV(pDC);
		SetScrollSizes(MM_TEXT, imgST150.size);
		break;
*/
	}

	ReleaseDC(pDC);
}
#endif

BOOL FileExist(TCHAR* fn, WIN32_FIND_DATA* pwfd);

char* LoadSrcFile(TCHAR* fn)
{
	HANDLE h = CreateFile(fn, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return NULL;

	DWORD fSize = GetFileSize(h, NULL);
	if(fSize > 512*1024 ) { //20000) { // Too Big //ryuhs74@20120305 - 이 부분에서NULL로 나온다.
		CloseHandle(h);
		return NULL;
	}

	char* cp = (char*)malloc(fSize + 128);
	if(cp == NULL) {
		CloseHandle(h);
		return NULL;
	}

	DWORD dwio = 0;
	ReadFile(h, cp, fSize, &dwio, NULL);
	cp[fSize] = 0;
	cp[fSize+1] = 0;
	CloseHandle(h);

	return cp;
}

BOOL ReplaceStrInStr(char* src, char* s1, char* s2)
{
	int l1 = strlen(s1);
	int l2 = strlen(s2);
	int ls = strlen(src);
	for(int i = 0; i < ls; i++) {
		if(strncmp(src+i, s1, l1) == 0) {
			if(l1 == l2) {
				// 같은 길이라면 그냥 복사만 하면 땡
			}
			if(l1 > l2) {
				// 뒤에 걸 앞으로 당겨 와야 한다.
				int j = i + l2;
				int k = i + l1;
				for( ; k < ls; k++) {
					src[j] = src[k];
					j++;
				}
				src[j] = 0;
			}
			if(l2 > l1) {
				// 뒤로 밀어야 한다.
				int b4 = ls;
				int last = ls + l2 - l1;
				int k = i + l1 - 1; // 마지막 0을 복사하므로 1자리를 추가해 준다.
				for( ; k < ls+1; k++) {
					src[last] = src[b4];
					last--;
					b4--;
				}
			}
			for(int j = 0; j < l2; j++)
				src[i+j] = s2[j];
			return TRUE;
		}
	}

	return FALSE;
}

void WriteStrToFile(char* pdllmain_cpp, TCHAR* tszfn)
{
	HANDLE h = CreateFile(tszfn, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return;

	DWORD dwio = 0;
	WriteFile(h, pdllmain_cpp, strlen(pdllmain_cpp), &dwio, NULL);
	CloseHandle(h);
}

/*
BOOL CICUMANView::Conv_TestDLL_vcproj(TCHAR* dst)
{
	TCHAR msg[MAX_PATH+128];

	char* ptestdll_proj = LoadSrcFile(dst);
	if(ptestdll_proj == NULL) {
		wsprintf(msg, L"%s is not found. Cannot load.", dst);
		return FALSE;
	}

	char hdr[24];
	sprintf_s(hdr, 24, "A_%d_%s", pWinCurrentLocal->CurrencyNum, pWinCurrentLocal->szCurrency);
	while(1) {
		if(!ReplaceStrInStr(ptestdll_proj, "A_13_EUR", hdr))
			break;
	}

	WriteStrToFile(ptestdll_proj, dst);
	free(ptestdll_proj);
	return TRUE;
}
*/

int CICUMANView::CopyIfDstIsOld(TCHAR* src, TCHAR* dst)
{
	WIN32_FIND_DATA src_wfd;
	WIN32_FIND_DATA dst_wfd;
	TCHAR msg[MAX_PATH+128];

	if(!FileExist(src, &src_wfd)) {
		wsprintf(msg, L"%s is not found. Please prepare A_13_EUR folder for template.", src);
		MessageBox(msg, L"Check source file.", MB_OK);
		return -1;
	}
	
	if(FileExist(dst, &dst_wfd)) {
		if(CompareFileTime(&src_wfd.ftLastWriteTime, &dst_wfd.ftLastWriteTime) <= 0) // src is earlier or equal
			return 0;
	}
	
	if(!CopyFile(src, dst, FALSE)) {
		wsprintf(msg, L"Copying %s\r\nto %s\r\nhas been failed. Continue?", src, dst);
		if(MessageBox(msg, L"Copying files", MB_YESNO) == IDNO)
			return -1;
	};

	return 1;
}

TCHAR CurImagePath[MAX_PATH] = L"";

/*
void CICUMANView::SetCurrentLocalByPath(TCHAR* path)
{
	lstrcpy(CurImagePath, path);
	CurImagePath[lstrlen(m_CurrencyImageFolder)+4] = 0;
	if(lstrcmp(m_CurrencyImageFolder, CurImagePath) && (lstrcmp(path, CurImagePath) == 0)) { // Local Currency Folder <image folder>\XXX
		TCHAR tszlocal[4];
		lstrcpy(tszlocal, &path[lstrlen(path)-3]);
		if(lstrcmp(tszlocal, pWinCurrentLocal->tszCurrency) != 0) {
			lstrcpy(pWinCurrentLocal->tszCurrency, tszlocal);
			WideCharToMultiByte(CP_ACP, 0, tszlocal, lstrlen(tszlocal)+1, pWinCurrentLocal->szCurrency, 4,NULL, NULL);
		}
	}
}
*/

void strlcat(char* dst, int size, char* src)
{
	while(size > 1) {
		dst++;
		size--;
	}

	while(size > 1) {
		*dst++ = *src++;
		size--;
	}

	*dst = 0;
}

extern int kernel_size;

void do_undistort();

/**
 * @function CannyThreshold
 * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
 */
void CannyThreshold(TCHAR* window_name, Mat& src, Mat& src_gray, Mat& edges, Mat& dst)
{
	/// Convert the image to grayscale
	cvtColor( src, src_gray, CV_BGR2GRAY );
	/// Reduce noise with a kernel 3x3
	blur( src_gray, edges, Size(3,3) );

	/// Canny detector
	Canny( edges, edges, canny_th_min[0], canny_th_max[0], kernel_size );

	/// Using Canny's output as a mask, we display our result
	dst = Scalar::all(0);

	src.copyTo( dst, edges);
	CreateScrollWnd(window_name)->matShow(dst);
}

void CannyThreshold()
{
	CannyThreshold(L"Edge Map 0", mat_src, mat_src_gray, detected_edges, mat_dst);
}

#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif

using namespace cv;
using namespace std;

static void help()
{
    cout <<  "This is a camera calibration sample." << endl
         <<  "Usage: calibration configurationFile"  << endl
         <<  "Near the sample file you'll find the configuration file, which has detailed help of "
             "how to edit it.  It may be any OpenCV supported file format XML/YAML." << endl;
}
class Settings
{
public:
    Settings() : goodInput(false) {}
    enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
    enum InputType {INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST};

    void write(FileStorage& fs) const                        //Write serialization for this class
    {
        fs << "{" << "BoardSize_Width"  << boardSize.width
                  << "BoardSize_Height" << boardSize.height
                  << "Square_Size"         << squareSize
                  << "Calibrate_Pattern" << patternToUse
                  << "Calibrate_NrOfFrameToUse" << nrFrames
                  << "Calibrate_FixAspectRatio" << aspectRatio
                  << "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
                  << "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint

                  << "Write_DetectedFeaturePoints" << bwritePoints
                  << "Write_extrinsicParameters"   << bwriteExtrinsics
                  << "Write_outputFileName"  << outputFileName

                  << "Show_UndistortedImage" << showUndistorsed

                  << "Input_FlipAroundHorizontalAxis" << flipVertical
                  << "Input_Delay" << delay
                  << "Input" << input
           << "}";
    }
    void read(const FileNode& node)                          //Read serialization for this class
    {
        node["BoardSize_Width" ] >> boardSize.width;
        node["BoardSize_Height"] >> boardSize.height;
        node["Calibrate_Pattern"] >> patternToUse;
        node["Square_Size"]  >> squareSize;
        node["Calibrate_NrOfFrameToUse"] >> nrFrames;
        node["Calibrate_FixAspectRatio"] >> aspectRatio;
        node["Write_DetectedFeaturePoints"] >> bwritePoints;
        node["Write_extrinsicParameters"] >> bwriteExtrinsics;
        node["Write_outputFileName"] >> outputFileName;
        node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
        node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;
        node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
        node["Show_UndistortedImage"] >> showUndistorsed;
        node["Input"] >> input;
        node["Input_Delay"] >> delay;
        interprate();
    }
    void interprate()
    {
        goodInput = true;
        if (boardSize.width <= 0 || boardSize.height <= 0)
        {
            cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
            goodInput = false;
        }
        if (squareSize <= 10e-6)
        {
            cerr << "Invalid square size " << squareSize << endl;
            goodInput = false;
        }
        if (nrFrames <= 0)
        {
            cerr << "Invalid number of frames " << nrFrames << endl;
            goodInput = false;
        }

        if (input.empty())      // Check for valid input
                inputType = INVALID;
        else
        {
            if (input[0] >= '0' && input[0] <= '9')
            {
                stringstream ss(input);
                ss >> cameraID;
                inputType = CAMERA;
            }
            else
            {
                if (readStringList(input, imageList))
                    {
                        inputType = IMAGE_LIST;
                        nrFrames = (nrFrames < (int)imageList.size()) ? nrFrames : (int)imageList.size();
                    }
                else
                    inputType = VIDEO_FILE;
            }
            if (inputType == CAMERA)
                inputCapture.open(cameraID);
            if (inputType == VIDEO_FILE)
                inputCapture.open(input);
            if (inputType != IMAGE_LIST && !inputCapture.isOpened())
                    inputType = INVALID;
        }
        if (inputType == INVALID)
        {
            cerr << " Inexistent input: " << input;
            goodInput = false;
        }

        flag = 0;
        if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
        if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
        if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;


        calibrationPattern = NOT_EXISTING;
        if (!patternToUse.compare("CHESSBOARD")) calibrationPattern = CHESSBOARD;
        if (!patternToUse.compare("CIRCLES_GRID")) calibrationPattern = CIRCLES_GRID;
        if (!patternToUse.compare("ASYMMETRIC_CIRCLES_GRID")) calibrationPattern = ASYMMETRIC_CIRCLES_GRID;
        if (calibrationPattern == NOT_EXISTING)
            {
                cerr << " Inexistent camera calibration mode: " << patternToUse << endl;
                goodInput = false;
            }
        atImageList = 0;

    }
    Mat nextImage()
    {
        Mat result;
        if( inputCapture.isOpened() )
        {
            Mat view0;
            inputCapture >> view0;
            view0.copyTo(result);
        }
        else if( atImageList < (int)imageList.size() )
            result = imread(imageList[atImageList++], CV_LOAD_IMAGE_COLOR);

        return result;
    }

    static bool readStringList( const string& filename, vector<string>& l )
    {
        l.clear();
        FileStorage fs(filename, FileStorage::READ);
        if( !fs.isOpened() )
            return false;
        FileNode n = fs.getFirstTopLevelNode();
        if( n.type() != FileNode::SEQ )
            return false;
        FileNodeIterator it = n.begin(), it_end = n.end();
        for( ; it != it_end; ++it )
            l.push_back((string)*it);
        return true;
    }
public:
    Size boardSize;            // The size of the board -> Number of items by width and height
    Pattern calibrationPattern;// One of the Chessboard, circles, or asymmetric circle pattern
    float squareSize;          // The size of a square in your defined unit (point, millimeter,etc).
    int nrFrames;              // The number of frames to use from the input for calibration
    float aspectRatio;         // The aspect ratio
    int delay;                 // In case of a video input
    bool bwritePoints;         //  Write detected feature points
    bool bwriteExtrinsics;     // Write extrinsic parameters
    bool calibZeroTangentDist; // Assume zero tangential distortion
    bool calibFixPrincipalPoint;// Fix the principal point at the center
    bool flipVertical;          // Flip the captured images around the horizontal axis
    string outputFileName;      // The name of the file where to write
    bool showUndistorsed;       // Show undistorted images after calibration
    string input;               // The input ->



    int cameraID;
    vector<string> imageList;
    int atImageList;
    VideoCapture inputCapture;
    InputType inputType;
    bool goodInput;
    int flag;

private:
    string patternToUse;


};

static void read(const FileNode& node, Settings& x, const Settings& default_value = Settings())
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };

bool runCalibrationAndSave(Settings& s, Size imageSize, Mat&  cameraMatrix, Mat& distCoeffs,
                           vector<vector<Point2f> > imagePoints );

int main(int argc, char* argv[])
{
    help();
    Settings s;
    const string inputSettingsFile = argc > 1 ? argv[1] : "default.xml";
    FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
    if (!fs.isOpened())
    {
        cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
        return -1;
    }
    fs["Settings"] >> s;
    fs.release();                                         // close Settings file

    if (!s.goodInput)
    {
        cout << "Invalid input detected. Application stopping. " << endl;
        return -1;
    }

    vector<vector<Point2f> > imagePoints;
    Mat cameraMatrix, distCoeffs;
    Size imageSize;
    int mode = s.inputType == Settings::IMAGE_LIST ? CAPTURING : DETECTION;
    clock_t prevTimestamp = 0;
    const Scalar RED(0,0,255), GREEN(0,255,0);
    const char ESC_KEY = 27;

    for(int i = 0;;++i)
    {
      Mat view;
      bool blinkOutput = false;

      view = s.nextImage();

      //-----  If no more image, or got enough, then stop calibration and show result -------------
      if( mode == CAPTURING && imagePoints.size() >= (unsigned)s.nrFrames )
      {
          if( runCalibrationAndSave(s, imageSize,  cameraMatrix, distCoeffs, imagePoints))
              mode = CALIBRATED;
          else
              mode = DETECTION;
      }
      if(view.empty())          // If no more images then run calibration, save and stop loop.
      {
            if( imagePoints.size() > 0 )
                runCalibrationAndSave(s, imageSize,  cameraMatrix, distCoeffs, imagePoints);
            break;
      }


        imageSize = view.size();  // Format input image.
        if( s.flipVertical )    flip( view, view, 0 );

        vector<Point2f> pointBuf;

        bool found;
        switch( s.calibrationPattern ) // Find feature points on the input format
        {
        case Settings::CHESSBOARD:
            found = findChessboardCorners( view, s.boardSize, pointBuf,
                CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
            break;
        case Settings::CIRCLES_GRID:
            found = findCirclesGrid( view, s.boardSize, pointBuf );
            break;
        case Settings::ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid( view, s.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID );
            break;
        default:
            found = false;
            break;
        }

        if ( found)                // If done with success,
        {
              // improve the found corners' coordinate accuracy for chessboard
                if( s.calibrationPattern == Settings::CHESSBOARD)
                {
                    Mat viewGray;
                    cvtColor(view, viewGray, COLOR_BGR2GRAY);
                    cornerSubPix( viewGray, pointBuf, Size(11,11),
                        Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
                }

                if( mode == CAPTURING &&  // For camera only take new samples after delay time
                    (!s.inputCapture.isOpened() || clock() - prevTimestamp > s.delay*1e-3*CLOCKS_PER_SEC) )
                {
                    imagePoints.push_back(pointBuf);
                    prevTimestamp = clock();
                    blinkOutput = s.inputCapture.isOpened();
                }

                // Draw the corners.
                drawChessboardCorners( view, s.boardSize, Mat(pointBuf), found );
        }

        //----------------------------- Output Text ------------------------------------------------
        string msg = (mode == CAPTURING) ? "100/100" :
                      mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
        int baseLine = 0;
        Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
        Point textOrigin(view.cols - 2*textSize.width - 10, view.rows - 2*baseLine - 10);

        if( mode == CAPTURING )
        {
            if(s.showUndistorsed)
                msg = format( "%d/%d Undist", (int)imagePoints.size(), s.nrFrames );
            else
                msg = format( "%d/%d", (int)imagePoints.size(), s.nrFrames );
        }

        putText( view, msg, textOrigin, 1, 1, mode == CALIBRATED ?  GREEN : RED);

        if( blinkOutput )
            bitwise_not(view, view);

        //------------------------- Video capture  output  undistorted ------------------------------
        if( mode == CALIBRATED && s.showUndistorsed )
        {
            Mat temp = view.clone();
            undistort(temp, view, cameraMatrix, distCoeffs);
        }

        //------------------------------ Show image and check for input commands -------------------
        imshow("Image View", view);
        char key = (char)waitKey(s.inputCapture.isOpened() ? 50 : s.delay);

        if( key  == ESC_KEY )
            break;

        if( key == 'u' && mode == CALIBRATED )
           s.showUndistorsed = !s.showUndistorsed;

        if( s.inputCapture.isOpened() && key == 'g' )
        {
            mode = CAPTURING;
            imagePoints.clear();
        }
    }

    // -----------------------Show the undistorted image for the image list ------------------------
    if( s.inputType == Settings::IMAGE_LIST && s.showUndistorsed )
    {
        Mat view, rview, map1, map2;
        initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
            getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
            imageSize, CV_16SC2, map1, map2);

        for(int i = 0; i < (int)s.imageList.size(); i++ )
        {
            view = imread(s.imageList[i], 1);
            if(view.empty())
                continue;
            remap(view, rview, map1, map2, INTER_LINEAR);
            imshow("Image View", rview);
            char c = (char)waitKey();
            if( c  == ESC_KEY || c == 'q' || c == 'Q' )
                break;
        }
    }


    return 0;
}

static double computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints,
                                         const vector<vector<Point2f> >& imagePoints,
                                         const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                         const Mat& cameraMatrix , const Mat& distCoeffs,
                                         vector<float>& perViewErrors)
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); ++i )
    {
        projectPoints( Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix,
                       distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);

        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    return std::sqrt(totalErr/totalPoints);
}

static void calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners,
                                     Settings::Pattern patternType /*= Settings::CHESSBOARD*/)
{
    corners.clear();

    switch(patternType)
    {
    case Settings::CHESSBOARD:
    case Settings::CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; ++i )
            for( int j = 0; j < boardSize.width; ++j )
                corners.push_back(Point3f(float( j*squareSize ), float( i*squareSize ), 0));
        break;

    case Settings::ASYMMETRIC_CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; i++ )
            for( int j = 0; j < boardSize.width; j++ )
                corners.push_back(Point3f(float((2*j + i % 2)*squareSize), float(i*squareSize), 0));
        break;
    default:
        break;
    }
}

static bool runCalibration( Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                            vector<vector<Point2f> > imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,
                            vector<float>& reprojErrs,  double& totalAvgErr)
{

    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if( s.flag & CV_CALIB_FIX_ASPECT_RATIO )
        cameraMatrix.at<double>(0,0) = 1.0;

    distCoeffs = Mat::zeros(8, 1, CV_64F);

    vector<vector<Point3f> > objectPoints(1);
    calcBoardCornerPositions(s.boardSize, s.squareSize, objectPoints[0], s.calibrationPattern);

    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    //Find intrinsic and extrinsic camera parameters
    double rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
                                 distCoeffs, rvecs, tvecs, s.flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

    cout << "Re-projection error reported by calibrateCamera: "<< rms << endl;

    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
                                             rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

    return ok;
}

bool runCalibrationAndSave(Settings& s, Size imageSize, Mat&  cameraMatrix, Mat& distCoeffs,vector<vector<Point2f> > imagePoints )
{
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;

    bool ok = runCalibration(s,imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs,
                             reprojErrs, totalAvgErr);
    cout << (ok ? "Calibration succeeded" : "Calibration failed")
        << ". avg re projection error = "  << totalAvgErr ;

    return ok;
}

double focal_length = 6.5746697944293521e+002;
double distortCoeffk1 = -4.1802327176423804e-001;
double distortCoeffk2 = 5.0715244063187526e-001;
double distortCoeffk3 = -5.7843597214487474e-001;
int apply_undistort = 1;
#if 0 // 3 number detect
cv::Rect cropRect = cv::Rect(300, 300, 1400, 500);
#else // HANDS Rect
cv::Rect cropRect = cv::Rect(0,0,960,664);
#endif

cv::Rect rectRects[10] =
{
		cv::Rect(165,258,124,30),
		cv::Rect(705, 248, 124, 30),
		cv::Rect(1165,262,124,30),
		cv::Rect(-1, -1, -1, -1),
};

cv::Rect srchRects[3] =
{
		cv::Rect(100, 100, 260, 320),
		cv::Rect(560, 100, 300, 320),
		cv::Rect(1020, 100, 260, 320)
};
int srchLevel[10] =
{
	500,
	650,
	500
};

cv::Rect foundRects[10];

/*
void CICUMANView::do_undistort()
{
	// undistort
    Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	Mat distCoeffs = Mat::zeros(8, 1, CV_64F);
	cameraMatrix.at<double>(0,0) = 1.0;

	cameraMatrix.at<double>(0,0) = focal_length; // focal length X
	cameraMatrix.at<double>(0,1) = 0.;
	cameraMatrix.at<double>(0,2) = mat_src.cols / 2; // center x 3.1950000000000000e+002;
	cameraMatrix.at<double>(1,0) = 0.;
	cameraMatrix.at<double>(1,1) = focal_length; // focal length y
	cameraMatrix.at<double>(1,2) = mat_src.rows / 2; // center y 2.3950000000000000e+002;
	cameraMatrix.at<double>(2,0) = 0.;
	cameraMatrix.at<double>(2,1) = 0.;
	cameraMatrix.at<double>(2,2) = 1.;

	distCoeffs.at<double>(0) = distortCoeffk1; // k1
	distCoeffs.at<double>(1) = distortCoeffk2; // k2
	distCoeffs.at<double>(2) = 0.; // p1
	distCoeffs.at<double>(3) = 0.; // p2
	distCoeffs.at<double>(4) = distortCoeffk3; // k3

	undistort(mat_src, und_src, cameraMatrix, distCoeffs);

	imshow("undistorted", und_src);
}
*/
void do_undistort()
{
	// undistort
    Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	Mat distCoeffs = Mat::zeros(8, 1, CV_64F);
	cameraMatrix.at<double>(0,0) = 1.0;

	cameraMatrix.at<double>(0,0) = focal_length; // focal length X
	cameraMatrix.at<double>(0,1) = 0.;
	cameraMatrix.at<double>(0,2) = mat_src.cols / 2; // center x 3.1950000000000000e+002;
	cameraMatrix.at<double>(1,0) = 0.;
	cameraMatrix.at<double>(1,1) = focal_length; // focal length y
	cameraMatrix.at<double>(1,2) = mat_src.rows / 2; // center y 2.3950000000000000e+002;
	cameraMatrix.at<double>(2,0) = 0.;
	cameraMatrix.at<double>(2,1) = 0.;
	cameraMatrix.at<double>(2,2) = 1.;

	distCoeffs.at<double>(0) = distortCoeffk1; // k1
	distCoeffs.at<double>(1) = distortCoeffk2; // k2
	distCoeffs.at<double>(2) = 0.; // p1
	distCoeffs.at<double>(3) = 0.; // p2
	distCoeffs.at<double>(4) = distortCoeffk3; // k3

// play with the multiplier to get good result. 
	// to get fully removed radial distortion, use 
	double multiplier = 1.7; // image size multiplier 
	double distMultiplier = multiplier * 1; 
	cv::Mat bufCameraMatrix = cameraMatrix.clone(); 
	cv::Mat bufDistCoeffs = distCoeffs.clone(); 
	cv::Mat bufInput(mat_src.rows * multiplier, mat_src.cols * multiplier, mat_src.type()); 

	cv::Size imageSize = mat_src.size(); 
	cv::Mat map1, map2; 

	// rescale the cameraMatrix into outputSize 
	for(int i = 0; i < 3; i++) 
		for(int j = 0; j < 3; j++) 
			if(!(i == 2 && j == 2)) 
				bufCameraMatrix.at<double>(i,j) = bufCameraMatrix.at<double>(i,j) * multiplier; 

	for (int i = 0; i < 5; i++){ 
		bufDistCoeffs.at<double>(i,0) = bufDistCoeffs.at<double>(i,0) * distMultiplier; 
	} 

	// make image at the center of bufInput 
	int originX = (bufInput.cols - mat_src.cols) / 2; 
	int originY = (bufInput.rows - mat_src.rows) / 2; 
	cv::Mat imageROI= bufInput(cv::Rect(originX, originY, mat_src.cols, mat_src.rows)); 
	cv::addWeighted(imageROI, 0.0, mat_src, 1.0, 0., imageROI); 
//qDebug("CENTERING DONE"); 

	// undistort bufInput 
	cv::undistort(bufInput, und_src, bufCameraMatrix, bufDistCoeffs); 
//qDebug("UNDISTORT DONE!!"); 

	imshow("undistorted", und_src);

	if(	cropRect.width == 0 || cropRect.height == 0)
		return;

	croppedImage = und_src(cropRect);
}

void undistort_center(Mat& src, Mat& dst)
{
	// undistort
    Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	Mat distCoeffs = Mat::zeros(8, 1, CV_64F);
	cameraMatrix.at<double>(0,0) = 1.0;

	cameraMatrix.at<double>(0,0) = focal_length; // focal length X
	cameraMatrix.at<double>(0,1) = 0.;
	cameraMatrix.at<double>(0,2) = src.cols / 2; // center x 3.1950000000000000e+002;
	cameraMatrix.at<double>(1,0) = 0.;
	cameraMatrix.at<double>(1,1) = focal_length; // focal length y
	cameraMatrix.at<double>(1,2) = src.rows / 2; // center y 2.3950000000000000e+002;
	cameraMatrix.at<double>(2,0) = 0.;
	cameraMatrix.at<double>(2,1) = 0.;
	cameraMatrix.at<double>(2,2) = 1.;

	distCoeffs.at<double>(0) = distortCoeffk1; // k1
	distCoeffs.at<double>(1) = distortCoeffk2; // k2
	distCoeffs.at<double>(2) = 0.; // p1
	distCoeffs.at<double>(3) = 0.; // p2
	distCoeffs.at<double>(4) = distortCoeffk3; // k3

	// play with the multiplier to get good result. 
	// to get fully removed radial distortion, use 
	cv::Mat bufCameraMatrix = cameraMatrix.clone(); 
	cv::Mat bufDistCoeffs = distCoeffs.clone(); 

	// undistort bufInput 
	cv::undistort(src, dst, bufCameraMatrix, bufDistCoeffs); 
//qDebug("UNDISTORT DONE!!"); 

	imshow("undistorted", dst);
}

void drawEdgeRect(Mat& edges, cv::Rect& rect)
{
	line( cropped_edges, Point(rect.x, rect.y), Point(rect.x+rect.width, rect.y), Scalar(255,255,255), 1, CV_AA);
	line( cropped_edges, Point(rect.x+rect.width, rect.y), Point(rect.x+rect.width, rect.y+rect.height), Scalar(255,255,255), 1, CV_AA);
	line( cropped_edges, Point(rect.x+rect.width, rect.y+rect.height), Point(rect.x, rect.y+rect.height), Scalar(255,255,255), 1, CV_AA);
	line( cropped_edges, Point(rect.x, rect.y+rect.height), Point(rect.x, rect.y), Scalar(255,255,255), 1, CV_AA);
}

/*
			cropped_edges.at<unsigned char>(y,x) = cropped_edges.at<unsigned char>(y+rectRect.y, x+rectRect.x) ;
			if(cropped_edges.at<unsigned char>(y,x) == 0x0ff) {
				cropped_edges.at<unsigned char>(y,x) = 0;
				edge_count++;
			}
*/
int CountEdgesRect(Mat& edges, int idx, cv::Rect& rect)
{
	unsigned char* input = edges.data;
	int edge_count = 0;
	unsigned char* dp = input + rect.y*edges.step + rect.x;
	unsigned char* ndp = dp + edges.step;
	for(int y = 0; y < rect.height; y++) {
		int yd = y + (rect.height * idx);
		for(int x = 0; x < rect.width; x++) {
			if(dp[x] == 0x0ff) {
				if(ndp[x] == 0x0ff) {
					if( ndp[x-1] == 0 && ndp[x+1] == 0) edge_count += 3;
					else edge_count++;
				} else {
					if( ndp[x-1] == 0x0ff || ndp[x+1] == 0x0ff) edge_count += 1;
				}
			}
		}
		dp += edges.step;
		ndp += edges.step;
//			input[yy*cropped_edges.step+x] = input[(y+rect.y)*cropped_edges.step+ x+rect.x];
	}

	//drawEdgeRect(cropped_edges, rect);

	return edge_count;
}

int CountEdgesRect(Mat& edges, int x, int y, int width, int height)
{
	unsigned char* input = edges.data;
	int edge_count = 0;
	unsigned char* dp = input + (y+height-1)*edges.step + x;
	unsigned char* ndp = dp - edges.step;
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(dp[x] == 0x0ff) {
				if(ndp[x] == 0x0ff) {
					if( ndp[x-1] == 0 && ndp[x+1] == 0) edge_count += 3;
					else edge_count++;
				} else {
					if( ndp[x-1] == 0x0ff || ndp[x+1] == 0x0ff) edge_count += 1;
				}
			}
		}
		dp -= edges.step;
		ndp -= edges.step;
	}

	return edge_count;
}

/*
int CountLongSegments(Mat& edges, int x1, int y1, int x2, int y2, int w)
{
	unsigned char* input = edges.data;
	int edge_count = 0;
	for(int y = y2-1; y >= y1; y--) {
		for(int x = x1; x < x2; x++) {
			unsigned char *p = input + y*edges.step+x;
			if(*p == 0x0ff && (*(p-1) != 0x0ff || *(p-1) != 0x0ff)) {
				int mx = x;
				int my = y-1;
				int ylen = 0;
				int xlen = 0;
				while(1) {
					if(input[my][mx] == 0x0ff) {
					} else if(input[my][mx-1] == 0x0ff) {
						my--;
						mx--;
						xlen--;
					} else if(input[my][mx+1] == 0x0ff) {
						my--;
						mx++;
						xlen++;
					} else {
						break;
					}
					if(xlen > 3)
						break;
					my--;
					ylen++;
					if(ylen > 8)
				}
			}
		}
	}

	return edge_count;
}
*/

int get_e_count(Mat& edges, int x, int width, int y)
{
	int e_count = 0;

	unsigned char* input = edges.data + y * edges.step + x;
	unsigned char oc = *input++;
	for(int x2 = 1; x2 < width; x2++) {
		unsigned char nc = *input++;
		if(oc != nc) {
			e_count++;
			oc = nc;
		}
	}

	return e_count;
}

void Filter2D(Mat& src, Mat& dst)
{
	Mat mat = Mat(3, 3, CV_32FC1);
	mat.at<float>(0,0) = 1;
	mat.at<float>(0,0) = 1;
	mat.at<float>(0,1) = 0;
	mat.at<float>(0,2) = -1;
	mat.at<float>(1,0) = 1;
	mat.at<float>(1,1) = 0;
	mat.at<float>(1,2) = -1;
	mat.at<float>(2,0) = 1;
	mat.at<float>(2,1) = 0;
	mat.at<float>(2,2) = -1;

	filter2D(cropped_edges, dst, -1, mat, cvPoint(-1,-1));

	imshow("filter", dst);
}

void FindPlateByCliffCount()
{
	memset(foundRects, 0, sizeof(foundRects));

	for(int ri = 0; ri < 3; ri++) {
		// X를 이동하면서 엣지 변화량이 12개가 넘는 위치를 찾는다.
		int x1 = srchRects[ri].x;
		int x2 = srchRects[ri].x + srchRects[ri].width - rectRects[ri].width;
		int y1 = srchRects[ri].y;
		int y2 = srchRects[ri].y + srchRects[ri].height - rectRects[ri].height;
		int _found = 0;
		for(int y = y2-1; !_found && y >= y1; y--) {
			for(int x = x1; x < x2; x++) {
				int e_count = get_e_count(cropped_edges, x, rectRects[ri].width, y);
				if(e_count > 24) {
					int yy = 0;
					for( yy = 1; yy < 10; yy++) {
						e_count = get_e_count(cropped_edges, x, rectRects[ri].width, y+yy);
						if(e_count < 24)
							break;
					}
					if(yy >= 10) {
						_found = 1;
						foundRects[ri].x = x;
						foundRects[ri].y = y;
						foundRects[ri].width = rectRects[ri].width;
						foundRects[ri].height = rectRects[ri].height;
						break;
					}
				}
			}
		}
	}
}

void FindPlateByShortTest(cv::Rect& srchRect, cv::Rect& rectRect, cv::Rect& foundRect, int srchLevel)
{
	memset(&foundRect, 0, sizeof(foundRect));

	// X를 이동하면서 엣지 변화량이 12개가 넘는 위치를 찾는다.
	int x1 = srchRect.x;
	int x2 = srchRect.x + srchRect.width - rectRect.width;
	int y1 = srchRect.y;
	int y2 = srchRect.y + srchRect.height - rectRect.height;
	int _found = 0;
	for(int y = y2-1; !_found && y >= y1; y--) {
		for(int x = x1; x < x2; x++) {
			int edge_count = CountEdgesRect(cropped_edges, x, y, rectRect.width, rectRect.height);
			if(edge_count >= srchLevel) {
				_found = 1;
				foundRect.x = x;
				foundRect.y = y;
				foundRect.width = rectRect.width;
				foundRect.height = rectRect.height;
				break;
			}
		}
	}
}

// X를 이동하면서 수직선 길이가 긴 것이 연속되는지 검사한다.
/*
void FindPlateByLongTest()
{
	memset(foundRects, 0, sizeof(foundRects));

	for(int ri = 0; ri < 3; ri++) {
		int x1 = srchRects[ri].x;
		int x2 = srchRects[ri].x + srchRects[ri].width - rectRects[ri].width;
		int y1 = srchRects[ri].y;
		int y2 = srchRects[ri].y + srchRects[ri].height - rectRects[ri].height;
		int _found = 0;
		int edge_count = CountLongSegments(cropped_edges, x1, y1, x2, y2, rectRects[ri].width);
		if(edge_count > srchLevel[ri]) {
			foundRects[ri].x = x;
			foundRects[ri].y = y;
			foundRects[ri].width = rectRects[ri].width;
			foundRects[ri].height = rectRects[ri].height;
			break;
		}
	}
}
*/

Mat plates[3];

void CannyCroppedImage()
{
	if(croppedImage.empty())
		return;

	CannyThreshold(L"Edge Map", croppedImage, cropped_gray, cropped_edges, cropped_dst);
	// Mat dst;
	// Filter2D(cropped_edges, dst);

	int rectCount = 0;
	int rows = cropped_edges.rows;
	int cols = cropped_edges.cols;
	int bpp = Bpp(cropped_edges);
#if 0
	for(int ri = 0; rectRects[ri].x != -1; ri++) {
		int edge_count = CountEdgesRect(cropped_edges, ri, rectRects[ri]);
		rectCount++;
		theImageTester->SetEdgeCount(ri, edge_count);
	}
#endif
	// search plate number area
	int xpos = 0;
	for(int ri = 0; rectRects[ri].x; ri++) {
		//FindPlateByCliffCount();
		FindPlateByShortTest(srchRects[ri], rectRects[ri], foundRects[ri], srchLevel[ri]);
		//FindPlateByLongTest();
		if(foundRects[ri].width) {
			foundRects[ri].width += 20;
			foundRects[ri].height += 10;
			foundRects[ri].y -= 15;
			drawEdgeRect(cropped_edges, foundRects[ri]);
			Mat plate = croppedImage(foundRects[ri]);
			cv::Rect roi(xpos, croppedImage.rows-foundRects[ri].height, foundRects[ri].width, foundRects[ri].height);
			plate.copyTo(croppedImage(roi));
			xpos += foundRects[ri].width;
		}
	}

	imshow("cropped edges", cropped_edges);

//	imshow("cropped gray", cropped_gray);
}

/*
opencv 3.0
void DetectLineSegments()
{
	Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_STD); 
// Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_NONE); 

	double start = double(getTickCount()); 
	vector<Vec4i> lines_std; 

	// Detect the lines 
	ls->detect(image, lines_std); 

	double duration_ms = (double(getTickCount()) - start) * 1000 / getTickFrequency(); 
	std::cout << "It took " << duration_ms << " ms." << std::endl; 

	// Show found lines 
	Mat drawnLines(image); 
	ls->drawSegments(drawnLines, lines_std); 
	imshow("Standard refinement", drawnLines); 
}
*/

int houghlines(Mat& edge_mat, Mat& cdst);

void RecognizeLicensePlate(int flags)
{
//	if(CANNY_ORIGINAL & flags) CannyThreshold(L"Edge Map 0", mat_src, mat_src_gray, detected_edges, mat_dst);

	if(CANNY_CROPPED & flags)
		CannyCroppedImage();
/*
	if(!cropped_edges.empty()) {
		houghlines(cropped_edges, cropped_dst);
	}
*/
	imshow("cropped", croppedImage);
}

void lpr2();

int CICUMANView::ProcessOneNote(FILE* retf, TCHAR* szPathName)
{

	char szbuf[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, szPathName, lstrlen(szPathName)+1, szbuf, MAX_PATH, NULL, NULL);
	std::string stdstring = szbuf;
    mat_src = cv::imread(stdstring, CV_LOAD_IMAGE_COLOR);   // Read the file

    if(! mat_src.data )                              // Check for invalid input
    {
        MessageBox(L"Could not open or find the image", szPathName, MB_OK);
        return -1;
    }

	/// Create a matrix of the same type and size as src (for dst)
	mat_dst.create( mat_src.size(), mat_src.type() );

	/// Create a window
//	namedWindow( window_name, CV_WINDOW_AUTOSIZE );
//	namedWindow( window_name, CV_WINDOW_NORMAL);
//	CreateScrollWnd(L"Edge Map 0")->ShowWindow(SW_SHOWNORMAL);

	do_undistort();
	
	// 1st try is use canny edge to find license plate area
	RecognizeLicensePlate(CANNY_ORIGINAL | CANNY_CROPPED);
	// 2nd try is to find rectangle using opencv
	//lpr2();

	return 1;
}

//////////////////////////////////////////////////////////////
extern int success_count;
extern int fail_count;

Mat tmpmat;

int CICUMANView::ProcessOneNote_HDS(FILE* retf, TCHAR* szPathName, Mat* imgmat)
{
	SYSTEMTIME st;
	GetSystemTime(&st);

	char szbuf[MAX_PATH];
	if(szPathName != NULL) {
		WideCharToMultiByte(CP_ACP, 0, szPathName, lstrlen(szPathName)+1, szbuf, MAX_PATH, NULL, NULL);
		//source image open...
		carNumDetect.OpenSrcImageFile(szbuf);
	} else {
		strcpy_s(szbuf, "Selected region");
		carNumDetect.SetSrcImg(*imgmat);
	}
	if(carNumDetect.src.empty())
		return 0;

	int ret_sub = 0;
	int ret = carNumDetect.GetNumberArea(retf, nBlurType, &CarNumPlate);
	SYSTEMTIME et;
	GetSystemTime(&et);
// houghlines는 영 쓸모가 없다...
	if(bTestHoughLine) {
		carNumDetect.src.copyTo(tmpmat);
		tmpmat = cv::Scalar(0);
		houghlines(carNumDetect.detected_edges, tmpmat);
	}

	int dt = ((((((et.wHour * 60) + et.wMinute) * 60) + et.wSecond) * 1000) + et.wMilliseconds) -
		((((((st.wHour * 60) + st.wMinute) * 60) + st.wSecond) * 1000) + st.wMilliseconds);

	if(nPlateType == 3) { // 엣지로부터 선 찾는 기능 실험
		// 에지를 디스플레이 하도록 이미지 복사
		cvtColor(carNumDetect.detected_edges, carNumDetect.labeledColorImage, CV_GRAY2BGR);
		carNumDetect.DrawEdgeLines(carNumDetect.labeledColorImage);
		if(theImageTester) {
			CString txt;
			carNumDetect.GetTestedEdgeText(txt);
			theImageTester->FillTestEdgeText(txt);
		}
	}
	// HDS-TEST-140926	
	if(nPlateType == 4) {
		cvtColor(carNumDetect.detected_edges, carNumDetect.labeledColorImage, CV_GRAY2BGR);
		carNumDetect.DrawEdgeLines_HDS(carNumDetect.labeledColorImage);
	}

	CreateScrollWnd(carNumDetect.window_original)->matShow(carNumDetect.src); // src 대신에 blur_src를 보여준다.
/*
	//process show...
	imshow( carNumDetect.window_process, carNumDetect.detected_edges );
*/
/*
	if(theImageTester)
		theImageTester->m_imgSelect.ResetContent();
*/
	if(ret && 0) {
		// 번호판 영역 이미지 보정을 시도한다
		// 아랫선의 기울기를 구한다
		CreateScrollWnd(L"NumPlate")->matShow(CarNumPlate);
		carNumDetect.GetSlope();
		if(pPlateDetector == NULL) {
			pPlateDetector = new CCarNumDetect();
			pPlateDetector->edge_name = L"plate edge";
		}
		pPlateDetector->SetSrcImg(CarNumPlate);
		ret_sub = pPlateDetector->GetNumberArea(retf, -1, NULL); // no blurring, no make plate area
		imshow("img 1", pPlateDetector->labeledColorImage);
		// Test DLG에 저장할 수 있는 이미지가 있음을 알린다.
		if(theImageTester) {
			theImageTester->m_imgSelect.AddString(L"0");
			theImageTester->m_imgSelect.AddString(L"1");
			theImageTester->m_imgSelect.AddString(L"2");
			theImageTester->m_imgSelect.AddString(L"3");

			theImageTester->FillEdges(pPlateDetector->blobCandidateCount);
		}
//		carNumDetect.CannyPlateArea();
	}

	char sztmp[128];
	if(ret == 1) {
		success_count++;
		sprintf(sztmp, "Success, E: %d ms, at %d", dt, carNumDetect.m_EdgeTable[carNumDetect.found_arr[0]].sx); // 최우선 후보....
		if(nBlurType == BLUR_GAUSSIAN)
			sprintf(sztmp+strlen(sztmp), " F:%4.2f", carNumDetect.ActiveSigma);
		if(retf != NULL) {
			fprintf(retf, "Success, %s", szbuf, dt); // 최우선 후보....
			if(nBlurType == BLUR_GAUSSIAN)
				fprintf(retf, sztmp+strlen(sztmp), " F:%4.2f", carNumDetect.ActiveSigma);
			fprintf(retf, "\n");
			carNumDetect.PrintTestDifference(retf);
		}
	} else {
		fail_count++;
		sprintf(sztmp, "Failed, E: %d ms", dt);
		if(retf != NULL) {
			fprintf(retf, "Failed, %s\n", szbuf, dt);
		}
	}

	if(!carNumDetect.labeledColorImage.empty()) {
		CScrollWnd* pwnd = CreateScrollWnd(carNumDetect.window_result);
		pwnd->pEdgeDetector = &carNumDetect; // 시험한 엣지들을 표시할 수 있도록 디텍터 주소 전달
		pwnd->matShow(carNumDetect.labeledColorImage);
		pwnd->putText(sztmp, Point(10,25));

		//result show...
		extern double slope_0, theta_0, slope_1, theta_1, slope_2, theta_2;
		sprintf(sztmp, "Slope %f %f %f", (float)slope_0, (float)slope_1, (float)slope_2); // 최우선 후보....
		pwnd->putText(sztmp, Point(10,50));
		if(ret_sub == 1)
			pwnd->putText("sub success", Point(10,75));
		else
			pwnd->putText("sub failed", Point(10,75));
	}

	return ret;

}

int CICUMANView::ProcessOneNote_HDS2(FILE* retf, TCHAR* szPathName, Mat* imgmat)
{
	SYSTEMTIME st;
	GetSystemTime(&st);

	char szbuf[MAX_PATH];
	if(szPathName != NULL) {
		WideCharToMultiByte(CP_ACP, 0, szPathName, lstrlen(szPathName)+1, szbuf, MAX_PATH, NULL, NULL);
		//source image open...
		carNumDetect.OpenSrcImageFile(szbuf);
	} else {
		strcpy_s(szbuf, "Selected region");
		carNumDetect.SetSrcImg(*imgmat);
	}
	if(carNumDetect.src.empty())
		return 0;

	int ret_sub = 0;
	int ret = carNumDetect.GetNumberArea(retf, 0, NULL);
	SYSTEMTIME et;
	GetSystemTime(&et);

	int dt = ((((((et.wHour * 60) + et.wMinute) * 60) + et.wSecond) * 1000) + et.wMilliseconds) -
		((((((st.wHour * 60) + st.wMinute) * 60) + st.wSecond) * 1000) + st.wMilliseconds);

	// HDS-TEST-140926	
	if(nPlateType == 4) {
		cvtColor(carNumDetect.detected_edges, carNumDetect.labeledColorImage, CV_GRAY2BGR);
		carNumDetect.DrawEdgeLines_HDS(carNumDetect.labeledColorImage);
	}


	CreateScrollWnd(carNumDetect.window_original)->matShow(carNumDetect.blur_src); // src 대신에 blur_src를 보여준다.
/*
	//process show...
	imshow( carNumDetect.window_process, carNumDetect.detected_edges );
*/
/*
	if(theImageTester)
		theImageTester->m_imgSelect.ResetContent();
*/

	char sztmp[128];
	if(ret == 1) {
		success_count++;
		
		if (carNumDetect.Found_PlateCandidateBoxIndex == -1) {
		   sprintf(sztmp, "<NUM> E: %d ms, at %d", dt, carNumDetect.m_EdgeTable[carNumDetect.found_arr[0]].sx); // 최우선 후보....
		}
		else {
		   sprintf(sztmp, "<BOX> E: %d ms, at %d", dt, carNumDetect.PlateCandidateBox[carNumDetect.Found_PlateCandidateBoxIndex].Box[0]); // 최우선 후보....
		}
		
		
		if(retf != NULL) {
			fprintf(retf, "Success, %s", szbuf);
			if(nBlurType == BLUR_GAUSSIAN)
				fprintf(retf, sztmp+strlen(sztmp), " F:%4.2f", carNumDetect.ActiveSigma);
			fprintf(retf, " %s", sztmp); 
			fprintf(retf, "\n");
			carNumDetect.PrintTestDifference(retf);
		}
	} else {
		fail_count++;
		sprintf(sztmp, "Failed, E: %d ms", dt);
		if(retf != NULL) {
			fprintf(retf, "Failed, %s\n", szbuf, dt);
		}
	}

	if(!carNumDetect.labeledColorImage.empty()) {
		CScrollWnd* pwnd = CreateScrollWnd(carNumDetect.window_result);
		pwnd->pEdgeDetector = &carNumDetect; // 시험한 엣지들을 표시할 수 있도록 디텍터 주소 전달
		pwnd->matShow(carNumDetect.labeledColorImage);
		pwnd->putText(sztmp, Point(10,25));

		//result show...
		extern double slope_0, theta_0, slope_1, theta_1, slope_2, theta_2;
		sprintf(sztmp, "Slope %f %f %f", (float)slope_0, (float)slope_1, (float)slope_2); // 최우선 후보....
		pwnd->putText(sztmp, Point(10,50));
		if(ret_sub == 1)
			pwnd->putText("sub success", Point(10,75));
		else
			pwnd->putText("sub failed", Point(10,75));
	}

	return ret;

}



int CICUMANView::ProcessOneNote_Fill(FILE* retf, TCHAR* szPathName, Mat* imgmat)
{
	SYSTEMTIME st;
	GetSystemTime(&st);

	char szbuf[MAX_PATH];
	if(szPathName != NULL) {
		WideCharToMultiByte(CP_ACP, 0, szPathName, lstrlen(szPathName)+1, szbuf, MAX_PATH, NULL, NULL);
		//source image open...
		carNumDetect.OpenSrcImageFile(szbuf);
	} else {
		strcpy_s(szbuf, "Selected region");
		carNumDetect.SetSrcImg(*imgmat);
	}
	if(carNumDetect.src.empty())
		return 0;

	int ret_sub = 0;
	int ret = carNumDetect.FillAndDetect(retf, nBlurType, &CarNumPlate);
	SYSTEMTIME et;
	GetSystemTime(&et);
// houghlines는 영 쓸모가 없다...
//	houghlines(carNumDetect.detected_edges, carNumDetect.src);
	carNumDetect.FindLinesByEdge(); // 에지에 따라 라인을 구한다. houghlines과는 상당히 다르다.

	int dt = ((((((et.wHour * 60) + et.wMinute) * 60) + et.wSecond) * 1000) + et.wMilliseconds) -
		((((((st.wHour * 60) + st.wMinute) * 60) + st.wSecond) * 1000) + st.wMilliseconds);

	CreateScrollWnd(carNumDetect.window_original)->matShow(carNumDetect.blur_src); // src 대신에 blur_src를 보여준다.
/*
	//process show...
	imshow( carNumDetect.window_process, carNumDetect.detected_edges );
*/
	if(theImageTester)
		theImageTester->m_imgSelect.ResetContent();
	if(ret && 0) {
		// 번호판 영역 이미지 보정을 시도한다
		// 아랫선의 기울기를 구한다
		CreateScrollWnd(L"NumPlate")->matShow(CarNumPlate);
		carNumDetect.GetSlope();
		if(pPlateDetector == NULL) {
			pPlateDetector = new CCarNumDetect();
			pPlateDetector->edge_name = L"plate edge";
		}
		pPlateDetector->SetSrcImg(CarNumPlate);
		ret_sub = pPlateDetector->GetNumberArea(retf, -1, NULL); // no blurring, no make plate area
		imshow("img 1", pPlateDetector->labeledColorImage);
		// Test DLG에 저장할 수 있는 이미지가 있음을 알린다.
		if(theImageTester) {
			theImageTester->m_imgSelect.AddString(L"0");
			theImageTester->m_imgSelect.AddString(L"1");
			theImageTester->m_imgSelect.AddString(L"2");
			theImageTester->m_imgSelect.AddString(L"3");

			theImageTester->FillEdges(pPlateDetector->blobCandidateCount);
		}
//		carNumDetect.CannyPlateArea();
	}

	char sztmp[128];
	if(ret == 1) {
		success_count++;
		sprintf(sztmp, "Success, E: %d ms, at %d", dt, carNumDetect.m_EdgeTable[carNumDetect.found_arr[0]].sx); // 최우선 후보....
		if(nBlurType == BLUR_GAUSSIAN)
			sprintf(sztmp+strlen(sztmp), " F:%4.2f", carNumDetect.ActiveSigma);
		if(retf != NULL) {
			fprintf(retf, "Success, %s", szbuf, dt); // 최우선 후보....
			if(nBlurType == BLUR_GAUSSIAN)
				fprintf(retf, sztmp+strlen(sztmp), " F:%4.2f", carNumDetect.ActiveSigma);
			fprintf(retf, "\n");
			carNumDetect.PrintTestDifference(retf);
		}
	} else {
		fail_count++;
		sprintf(sztmp, "Failed, E: %d ms", dt);
		if(retf != NULL) {
			fprintf(retf, "Failed, %s\n", szbuf, dt);
		}
	}

	if(!carNumDetect.labeledColorImage.empty()) {
		CScrollWnd* pwnd = CreateScrollWnd(carNumDetect.window_result);
		pwnd->matShow(carNumDetect.labeledColorImage);
		pwnd->putText(sztmp, Point(10,25));
		//result show...
		extern double slope_0, theta_0, slope_1, theta_1, slope_2, theta_2;
		sprintf(sztmp, "Slope %f %f %f", (float)slope_0, (float)slope_1, (float)slope_2); // 최우선 후보....
		pwnd->putText(sztmp, Point(10,50));
		if(ret_sub == 1)
			pwnd->putText("sub success", Point(10,75));
		else
			pwnd->putText("sub failed", Point(10,75));
	}
	return ret;

}

void CICUMANView::ApplyThreshold(FILE* retf, TCHAR* path)
{
	TCHAR* ActivePath = path ? path : szPathName; // if given path is NULL, use current path

#ifdef _DEBUG
	char szbuf[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, ActivePath, lstrlen(ActivePath)+1, szbuf, MAX_PATH, NULL, NULL);
	FILE *f = _tfopen(L"C:\\imgdebug.txt", L"at");
	fprintf_s(f, "%s\n", szbuf);
	fclose(f);
#endif

	//HDS
	switch(nPlateType){
	case 0 :
		ProcessOneNote(retf, ActivePath);
		break;
	case 1 :
	case 3 :
		ProcessOneNote_HDS(retf, ActivePath, NULL);
		break;
    // HDS-TEST-140926
	case 4 :
		ProcessOneNote_HDS2(retf, ActivePath, NULL);
		break;
	case 2 :
		ProcessOneNote_Fill(retf, ActivePath, NULL);
		break;
	case 5 :
		GridTest(retf, ActivePath);
	}
	Sleep(0);
}


//////////////////////////////////////////////////////////////
//HDS END
//////////////////////////////////
//////////////////////////////////////////////////////////////

int CICUMANView::SetPathName(TCHAR* path)
{
	int ConversionCalled = 0;

	CMainFrame *pMainFrm = (CMainFrame*)this->GetParentFrame();
	pMainFrm->SetWindowText(path);

	ClearMem();

	// 상위 이름이 없는 맨 마지막 이름을 구한다.
	lstrcpy(szPathName, path);
	TCHAR *bp = szPathName;
	TCHAR *sp = NULL;
	while(*bp) {
		if(*bp == _T('\\'))
			sp = bp;
		bp++;
	}
	if(sp != NULL)
		lstrcpy(szLeafName, sp+1);
	else
		lstrcpy(szLeafName, szPathName);

	// 폴더인 경우 리스트만 표시하고 리턴
	DWORD attrs = GetFileAttributes(path);
	if((attrs != 0xFFFFFFFF) && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
//		m_MainImageSize = CSize(0,0);
		ConversionCalled = FillListFromDirectoryEntries();
		Invalidate();
		return ConversionCalled;
	}

	TestFile(NULL, path);
	return 0;
}

void CICUMANView::TestFile(FILE* f, TCHAR* path)
{
	OutputDebugString(path); OutputDebugString(L"\r\n"); 

	ApplyThreshold(f, path);

	ScrollSize.cx = mat_src.cols;
	ScrollSize.cy = mat_src.rows;
	SetScrollSizes(MM_TEXT, ScrollSize);

	Invalidate();
}

void CICUMANView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(!mat_src.empty()) {
		if(!m_bDown) {
			CPoint gp = point;

			gp.x -= m_ViewportOrg.x;
			gp.y -= m_ViewportOrg.y;
/*
			if(imgST150.TestMousePoint(gp)) {
				int iv[2];
				imgST150.GetImageValue(gp, iv);
				TCHAR buf[128];
				swprintf_s(buf, 128, L"P %d,%d W=%d IR=%d           ", gp.x, gp.y, iv[0], iv[1]);
				CDC* pDC = GetDC();
				pDC->TextOutW(0, DrawHeight + 65, buf, lstrlen(buf));
				ReleaseDC(pDC);
			}
*/
		} else {
			if(m_MovePoint.x != point.x && m_MovePoint.y != point.y) {
				CDC* pDC = GetDC();
				if(m_MovePoint.x != m_DownPoint.x && m_MovePoint.y != m_DownPoint.y) {
					DrawXorRect(pDC, m_DownPoint, m_MovePoint);
				}
				if(m_DownPoint.x != point.x && m_DownPoint.y != point.y) {
					DrawXorRect(pDC, m_DownPoint, point);
				}
				ReleaseDC(pDC);
				m_MovePoint = point;
			}
		}
	}

	CScrollView::OnMouseMove(nFlags, point);
}

void CICUMANView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bDown = TRUE;
	m_DownPoint = point;
	m_MovePoint = point;

	CScrollView::OnLButtonDown(nFlags, point);
}

void CICUMANView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bDown = FALSE;
	ThreadPos = -1;
	if(ExtBitmap != NULL) {
		delete ExtBitmap;
		ExtBitmap = NULL;
	}
	if(MergeBitmap != NULL) {
		delete MergeBitmap;
		MergeBitmap = NULL;
	}

	m_MovePoint = point;
	if(m_MovePoint.x != m_DownPoint.x && m_MovePoint.y != m_DownPoint.y) {
		DownPoint = m_DownPoint;
		MovePoint = m_MovePoint;

		DownPoint.x -= m_ViewportOrg.x;
		DownPoint.y -= m_ViewportOrg.y;
		MovePoint.x -= m_ViewportOrg.x;
		MovePoint.y -= m_ViewportOrg.y;

		if(DownPoint.y > mat_src.rows) DownPoint.y -= mat_src.rows;
		if(MovePoint.y > mat_src.rows) MovePoint.y -= mat_src.rows;

		int iv[2];
/*
		if(imgST150.GetImageAverage(DownPoint, MovePoint, iv)) {
			AverageW = iv[0];
			AverageIR = iv[1];
		}
*/
	}

	Invalidate();

	CScrollView::OnLButtonUp(nFlags, point);
}

/*
void CICUMANView::FindThread(int tType, int tWidth)
{
	ThreadType = tType;
	ThreadWidth = tWidth;
	if(ExtBitmap == NULL)
		return;

	// 우선 걍 수직선만 찾자.
	memset(x_sum, 0, sizeof(x_sum));
	for(int x = 0; x < ExtSizeX; x++) {
		for(int y = 0; y < ExtSizeY; y++) {
			x_sum[x] += g_iqRegionIR_Buf[0][y*ExtSizeX+x];
		}
	}

	int wi;
	int min_sum = 0;
	for(wi = 0; wi < ThreadWidth; wi++) min_sum += x_sum[wi];
	ThreadPos = 0;
	for(int x = 1; x < ExtSizeX-4; x++) {
		int t_sum = 0;
		for(wi = 0; wi < ThreadWidth; wi++) t_sum += x_sum[x+wi];
		if(t_sum < min_sum) {
			min_sum = t_sum;
			ThreadPos = x;
		}
	}

	if(ThreadType == 0) {
		ThreadSizeX = ThreadWidth + 6;
		ThreadSizeY = ExtSizeY;
	} else {
		ThreadSizeX = ExtSizeX;
		ThreadSizeY = ThreadWidth + 6;
	}
	
	Invalidate();
}
*/
unsigned char RgnWriteBuf[1728*400];
/*
void CICUMANView::SaveThreadRegion(TCHAR* name)
{
	if(ExtBitmap == NULL)
		return;

	TCHAR tname[MAX_PATH];
	lstrcpy(tname, name);
	// 마지막 \\를 찾는다
	TCHAR *fp = tname;
	TCHAR *lp = fp; while(*fp) { if(*fp == '\\') lp = fp; fp++; }

	// 숫자를 찾는다.
	int index = 0;
	while(*lp && *lp < '0' || *lp > '9') lp++;
	while('0' <= *lp && *lp <= '9') {
		index = index * 10 + (*lp - '0');
		lp++;
	}

	// 다시 찾기. 파일 이름을 만들기 위해서
	lp = fp = tname;
	while(*fp) { if(*fp == '\\') lp = fp; fp++; }

	int i = 0;

	swprintf(lp, MAX_PATH, L"\\rgn_%03d_%d_%d.raw", index, ThreadWidth+6, ExtSizeY);

	for(int y = 0; y < ExtSizeY; y++) {
		for(int x = 0; x < ThreadWidth+6; x++) {
			RgnWriteBuf[i++] = g_iqRegionIR_Buf[0][y*ExtSizeX + ThreadPos + x - 2];
		}
	}

	HANDLE h = CreateFile(tname, GENERIC_WRITE|GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h != INVALID_HANDLE_VALUE) {
		DWORD dwio = 0;
		WriteFile(h, RgnWriteBuf, i, &dwio, NULL);
		CloseHandle(h);
	}
}
*/
void CICUMANView::GenerateRegionRawOnPath(TCHAR* path)
{
	int filecount = 0;
#define	MAX_FLIST	4000
	TCHAR *flist[MAX_FLIST];

	WIN32_FIND_DATA wfd;
	TCHAR FindPath[MAX_PATH];
	wsprintf(FindPath, TEXT("%s\\*.*"), path);
	
	HANDLE hFind = FindFirstFile(FindPath, &wfd);
	while(hFind) {
		if(lstrcmp(wfd.cFileName, L".") && lstrcmp(wfd.cFileName, L"..")) {
			TCHAR SubPath[MAX_PATH];
			wsprintf(SubPath, L"%s\\%s", path, wfd.cFileName);
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				GenerateRegionRawOnPath(SubPath);
			} else {
				if(filecount < MAX_FLIST) {
					flist[filecount] = (TCHAR*)malloc(sizeof(SubPath));
					lstrcpy(flist[filecount], SubPath);
					filecount++;
				}
			}
		}
		if(!FindNextFile(hFind, &wfd)) {
			FindClose(hFind);
			hFind = NULL;
			break;
		}
	}
	for(int fi = 0; fi < filecount; fi++) {
//		GenerateRegionRawOnFile(flist[fi]);
		free(flist[fi]);
	}
}

/*
void CICUMANView::OnGenerateRegionRaw(int tType, int tWidth)
{
	DirFromDlg = 4;

	ThreadType = tType;
	ThreadWidth = tWidth;

	TCHAR wpath[MAX_PATH];
	wsprintf(wpath, L"%s\\%s", m_CurrencyImageFolder, pWinCurrentLocal->tszCurrency);

	GenerateRegionRawOnPath(wpath);
}
*/

void CICUMANView::OnViewDebugimage()
{
	if(ExtBitmap != NULL) {
		delete ExtBitmap;
		ExtBitmap = NULL;
	}
	if(MergeBitmap != NULL) {
		delete MergeBitmap;
		MergeBitmap = NULL;
	}
/*	
	CDC *pDC = GetDC();
	ExtBitmap = imgST150.MakeDebugImage(pDC);
	ReleaseDC(pDC);
*/
	Invalidate();
}

void CICUMANView::MergeIrAndW()
{
	if(ExtBitmap == NULL)
		return;

	BITMAP bm;
	ExtBitmap->GetBitmap(&bm);
	CDC *pDC = GetDC();
	MergeBitmap = new CBitmap();
	int width = (bm.bmWidth-5)/2;
	MergeBitmap->CreateCompatibleBitmap(pDC, width, bm.bmHeight*2);

	CDC dc1;
	dc1.CreateCompatibleDC(pDC);
	CBitmap *ob1 = dc1.SelectObject(ExtBitmap);

	CDC dc2;
	dc2.CreateCompatibleDC(pDC);
	CBitmap *ob2 = dc2.SelectObject(MergeBitmap);

	ReleaseDC(pDC);

	for(int y = 0; y < bm.bmHeight; y++) {
		for(int x = 0; x < width; x++) {
			COLORREF rgb = dc1.GetPixel(x, y);
			dc2.SetPixel(x, y*2+1, rgb);
		}
	}

	for(int y = 0; y < bm.bmHeight; y++) {
		for(int x = 0; x < width; x++) {
			COLORREF rgb = dc1.GetPixel(x+width+5, y);
			dc2.SetPixel(x, y*2, rgb);
		}
	}

	dc1.SelectObject(ob1);
	dc1.DeleteDC();
	dc2.SelectObject(ob2);
	dc2.DeleteDC();

}

/*
void CICUMANView::OnOcr()
{
	HANDLE h = CreateFile(L"D:\\njkim.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h != INVALID_HANDLE_VALUE) {
		CloseHandle(h);
		if(theOCRDlg != NULL) {
			theOCRDlg->DestroyWindow();
		} else {
			if(pWinCurrentLocal == NULL) {
				MessageBox(L"Select local currency. Please.");
			} else {
				theOCRDlg = new COCRDlg(this);
				theOCRDlg->Create(IDD_OCR, this);
				theOCRDlg->ShowWindow(SW_SHOW);
			}
		}
	}
}
*/

void CICUMANView::OnViewImageTester()
{
	if(theImageTester != NULL) {
		theImageTester->DestroyWindow();
	} else {
		theImageTester = new CImageTestDlg(this);
		theImageTester->Create(IDD_IMAGE_TEST_DLG, this);
		theImageTester->ShowWindow(SW_SHOW);
	}
}

void CICUMANView::OnCurrencySetting()
{
	if(theHoughSetting!= NULL) {
		theHoughSetting->DestroyWindow();
	} else {
		theHoughSetting = new CSetting(this);
		theHoughSetting->Create(IDD_SETTING, GetDesktopWindow());
		theHoughSetting->ShowWindow(SW_SHOW);
	}
}

void CICUMANView::OnUpdateOcr(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

/*
void CICUMANView::OnViewMgUvCIS()
{
	// CIS -> MG -> UV -> CIS 로 순서대로 돌아가면서 데이터를 조회할 수 있게 한다.
	m_ViewType = (m_ViewType+1) % 3;

	Invalidate();
}
*/

int CICUMANView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	/// Create a Trackbar for user to enter threshold
	if(theImageTester == NULL) {
		theImageTester = new CImageTestDlg(this);
		theImageTester->Create(IDD_IMAGE_TEST_DLG, GetDesktopWindow()/*this*/);
		theImageTester->ShowWindow(SW_SHOW);
		theImageTester->m_Slider1.SetScrollRange(SB_CTL, 0, 100);
	}
//	createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold );

	return 0;
}

void CICUMANView::SelectImageOnPlate(int sel)
{
	if(pPlateDetector == NULL)
		return;

	carNumDetect.DrawPlateArea(pPlateDetector, CarNumPlate, sel);
}

void CICUMANView::GetSelectedROI(int sel, Mat& img)
{
	blob_info inf = pPlateDetector->m_EdgeTable[pPlateDetector->found_arr[sel]];
	Rect r(inf.sx, inf.sy, inf.width, inf.height);
	Mat m = CarNumPlate(r);
	m.copyTo(img);
}

int success_count = 0;
int fail_count = 0;

extern int bCropImage;
extern int bScaleImage;

void CICUMANView::TestTree()
{
	success_count = 0;
	fail_count = 0;

	TCHAR path[MAX_PATH];
	CMainFrame *pMainFrm = (CMainFrame*)this->GetParentFrame();
	pMainFrm->GetWindowText(path, MAX_PATH);

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(path, &wfd);
	if(hFind == INVALID_HANDLE_VALUE) {
		MessageBox(path, L"Cannot find");
		return;
	}
	FindClose(hFind);
	if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		MessageBox(path, L"It is not a folder.");
		return;
	}
	
	FILE* f = NULL;
	int findex = 0;
	while(1) {
		TCHAR txtname[MAX_PATH];
		wsprintf(txtname, L"%s\\test_%d.txt", path, findex);
		f = NULL;
		_tfopen_s(&f, txtname, L"rt");
		if(f == NULL) {
			_tfopen_s(&f, txtname, L"wt");
			if(f == NULL) {
				MessageBox(txtname, L"Cannot open to write");
				return;
			}
			break;
		} else {
			fclose(f);
		}
		findex++;
	}
	fprintf(f, "plate-type=%d\n", nPlateType);
	fprintf(f, "blur-type=%d\n", nBlurType);
	fprintf(f, "crop-image=%d\n", bCropImage);
	fprintf(f, "scale-image=%d\n", bScaleImage);
	fprintf(f, "kernel-size=%d\n", kernel_size);
	fprintf(f, "equidist-method=%d\n", equitest_method);
	fprintf(f, "test_houghline=%d\n", bTestHoughLine);
	fprintf(f, "apply-fft=%d\n", bApplyFFT);
	fprintf(f, "new-canny=%d\n", bNewCanny);

	fprintf(f, "canny-thr-count\%d\n", canny_th_cnt);
	fprintf(f, "canny-thr-min=%d,%d,%d,%d\n", canny_th_min[0], canny_th_min[1], canny_th_min[2], canny_th_min[3]);
	fprintf(f, "canny-thr-max=%d,%d,%d,%d\n", canny_th_max[0], canny_th_max[1], canny_th_max[2], canny_th_max[3]);

	TestTree(f, path);

	fprintf(f, "success=%d\n", success_count);
	fprintf(f, "fail=%d\n", fail_count);
	fclose(f);

	MessageBox(L"Finished", L"Test Tree");
}

void CICUMANView::TestTree(FILE *f, TCHAR* path)
{
	TCHAR tpath[MAX_PATH];
	wsprintf(tpath, L"%s\\*", path);
	
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(tpath, &wfd);
	while(hFind != INVALID_HANDLE_VALUE) {
		TCHAR jpgpath[MAX_PATH];
		if(lstrcmp(wfd.cFileName, L".") && lstrcmp(wfd.cFileName, L"..") && lstrcmp(wfd.cFileName, L"test.txt")) {
			// if it is neither current directory nor parent directory..
			TCHAR subpath[MAX_PATH];
			wsprintf(subpath, L"%s\\%s", path, wfd.cFileName);
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				CString str("Fail");
				if(str.CompareNoCase(wfd.cFileName))
					TestTree(f, subpath);
			} else {
				TestFile(f, subpath);
			}
		}
		if(!FindNextFile(hFind, &wfd)) {
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
		}
	}
}

// 이미지 위에서 마우스로 드래그 한 영역을 가지고 인식을 시도한다.
// 좁은 영역에서 실험해서 디버깅을 쉽게 하도록 도와주는 기능이다.
void CICUMANView::TestSelection()
{
	ProcessOneNote_HDS(NULL, NULL, &_sc_mat);
}

void CICUMANView::DrawEdge(int sel)
{
	if(pPlateDetector == NULL)
		return;

	CScrollWnd *pWnd = CreateScrollWnd(pPlateDetector->edge_name);
	int blob_index = pPlateDetector->tmp_blob[sel].index;
	blob_info inf = pPlateDetector->m_EdgeTable[blob_index];
	pWnd->XorRect = Rect(inf.sx, inf.sy, inf.width, inf.height);
	pWnd->Invalidate();
}

void CICUMANView::TestEdges(TCHAR* tszbuf)
{
	int   select_arr[4];
	int vcnt = 0;
	int v = 0;
	int p = 0;
	while(1) {
		if(tszbuf[p] >= '0' && tszbuf[p] <= '9') {
			v = v * 10 + tszbuf[p] - '0';
		} else {
			select_arr[vcnt] = v;
			vcnt++;
			v = 0;
		}
		if(vcnt >= 4)
			break;
		if(tszbuf[p] == 0)
			break;
		p++;
	}
	if(vcnt < 4)
		return;

	int ret = pPlateDetector->call_check_condition(select_arr);
}

void CICUMANView::MarkEdge(int em)
{
	CScrollWnd* pwnd = CreateScrollWnd(carNumDetect.window_result);
	pwnd->edge_to_mark = em;
	pwnd->Invalidate();
}

Mat grid_src;
Mat grid_cell;

int grid_cell_height = 8;
int grid_cell_width = 8;
int var_limit = 2000;

void CICUMANView::GridTest(FILE* retf, TCHAR* szPathName)
{
	SYSTEMTIME st;
	GetSystemTime(&st);

	char szbuf[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, szPathName, lstrlen(szPathName)+1, szbuf, MAX_PATH, NULL, NULL);
	//source image open...
	grid_src = imread( szbuf , CV_LOAD_IMAGE_GRAYSCALE);
	if(grid_src.empty())
		return;
	grid_src.copyTo(grid_cell);

	for(int y = 0; y < grid_src.rows; y += grid_cell_height) {
		for(int x = 0; x < grid_src.cols; x += grid_cell_width) {
			VarAnalysisGrid(x, y);
		}
	}

	imshow("cell", grid_cell);
}

void CICUMANView::VarAnalysisGrid(int x, int y)
{
	// make accumulation table
	int cell_accumulator[256];
	memset(cell_accumulator, 0, sizeof(cell_accumulator));

	for(int yy = y; yy < y + grid_cell_height; yy++) {
		unsigned char *ucp = grid_src.data + grid_src.step * yy + x;
		for(int xx = x; xx < x + grid_cell_width; xx++) {
			cell_accumulator[*ucp] += 1;
			ucp++;
		}
	}

	// divide into 2 classes
	float cell_total = 0;
	for(int ci = 0; ci < 256; ci++)
		cell_total += cell_accumulator[ci] * ci;
	int average_position = (int)(cell_total / (grid_cell_width * grid_cell_height));
	
	// find the position where the variance difference is minimized
	float curr_0_total = 0; // low part
	float curr_0_average = 0;
	float curr_0_count = 0;
	float curr_1_total = 0;
	float curr_1_average = 0;
	float curr_1_count = 0;
	
	int first_time = 1;
	float curr_0_variance = 0;
	float curr_1_variance = 0;
	float prev_variance_sum = 0;
	int   prev_average_position = 0;

	while(1) {
		for(int i = 0; i < average_position; i++) {
			if(cell_accumulator[i]) {
				curr_0_total += cell_accumulator[i] * i;
				curr_0_count += cell_accumulator[i];
			}
		}
		for(int i = average_position; i < 256; i++) {
			if(cell_accumulator[i]) {
				curr_1_total += cell_accumulator[i] * i;
				curr_1_count += cell_accumulator[i];
			}
		}

		curr_0_average = curr_0_total / curr_0_count;
		curr_1_average = curr_1_total / curr_1_count;

		// compute variance
		for(int i = 0; i < average_position; i++) {
			if(cell_accumulator[i]) {
				float diff = i - curr_0_average;
				curr_0_variance += cell_accumulator[i] * (diff * diff);
			}
		}
		for(int i = average_position; i < 256; i++) {
			if(cell_accumulator[i]) {
				float diff = i - curr_1_average;
				curr_1_variance += cell_accumulator[i] * (diff * diff);
			}
		}

		float curr_variance_sum = curr_0_variance + curr_1_variance;
		if(!first_time) {
			if(prev_variance_sum <= curr_variance_sum)
				break;
		}
		first_time = 0;
		prev_variance_sum = curr_variance_sum;
		prev_average_position = average_position;
		if(curr_0_variance < curr_1_variance) {
			average_position -= 1;
		} else {
			average_position += 1;
		}
		if(curr_1_average - curr_0_average < 50) {
			break; // flat image, no distintive marked area in this grid cell
		}
	}

	unsigned char fill_value = 255;
	if(curr_1_average - curr_0_average < 50)
		fill_value = 0;
	if(prev_variance_sum > var_limit)
		fill_value = 120;
	for(int yy = y; yy < y + grid_cell_height; yy++) {
		unsigned char *ucp = grid_cell.data + grid_cell.step * yy + x;
		for(int xx = x; xx < x + grid_cell_width; xx++) {
			*ucp++ = fill_value;
		}
	}
}
