#include "StdAfx.h"

#define GLOBAL_VALUE_INIT
#define GLOBAL_CIS_DEF
//#define GLOBAL_MG_DEF

#include "B_Common.h"

#include "ST150Image.h"
#include "CurrencyInfo.h"

#include "libs\tiff-v3.6.1\libtiff\tiffio.h"

#pragma warning(disable:4996)

WIN_ST_LOCAL *pWinCurrentLocal = NULL;

// Only One Instance
//ST150Image imgST150;

int laptop2;

ST150Image::ST150Image(void)
: m_nLaptop(0)
{
	m_ImageSize.SetSize(0,0);
	m_IImageSize.SetSize(0,0);
	m_WImageSize.SetSize(0,0);

	TapeDisplayGap = 1;
}

ST150Image::~ST150Image(void)
{

}

/*
CBitmap* CST150ExplorerView::MakeBitmapFromRaw(bool AutoRGB)
{
	CST150ExplorerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return NULL;

	int x, y;
	if(AutoRGB) {
		m_bViewRGB = false;
		int colorCount = 0;
		unsigned char *rp = pDoc->m_RawImageBuffer;
		for(y = 0; y < pDoc->m_RawHeight; y++) {
			if(*rp == 0x1A || *rp == 0x1B || *rp == 0x1C)
				colorCount++;
			rp += pDoc->m_RawWidth;
		}
		if(colorCount > 6)
			m_bViewRGB = true;
	}

	CDC dc;
	CDC *pDC = GetDC();
	dc.CreateCompatibleDC(pDC);
	CBitmap *bitmap = new CBitmap();
	if(m_bViewRGB)
		bitmap->CreateCompatibleBitmap(pDC, pDoc->m_RawWidth, (pDoc->m_RawHeight*5) / 2);
	else
		bitmap->CreateCompatibleBitmap(pDC, pDoc->m_RawWidth, pDoc->m_RawHeight);

	BITMAP bm;
	bitmap->GetBitmap(&bm);
	CBitmap* oldBitmap = dc.SelectObject(bitmap);

	for(x = 0; x < bm.bmWidth; x++) {
		for(y = 0; y < bm.bmHeight; y++) dc.SetPixel(x, y, RGB(255,255,255));
	}

	unsigned char *rp = pDoc->m_RawImageBuffer;
	int rHeight = 0;
	int gHeight = 0;
	int bHeight = 0;
	int rBaseY = pDoc->m_RawHeight + 2;
	int gBaseY = rBaseY + pDoc->m_RawHeight/2;
	int bBaseY = gBaseY + pDoc->m_RawHeight/2;
	for(y = 0; y < pDoc->m_RawHeight; y++) {
		unsigned char header = *rp; rp++;
		if(!m_bViewRGB) header = 0;
		for(x = 1; x < pDoc->m_RawWidth; x++) {
			unsigned char rv;
			if(m_bMedian3) {
				if(x == 1)
					rv = fmedian3(rp[0], rp[1], rp[2]);
				else if(x == pDoc->m_RawWidth-1)
					rv = fmedian3(rp[-2], rp[-1], rp[0]);
				else
					rv = fmedian3(rp[-1], rp[0], rp[1]);
			} else {
				rv = rp[0];
			}

			COLORREF c;
			switch(header){
			case 0x1A:
				c = RGB(rv, 0, 0);
				break;
			case 0x1B:
				c = RGB(0, rv, 0);
				break;
			case 0x1C:
				c = RGB(0, 0, rv);
				break;
			default:
				c = RGB(rv, rv, rv);
			}
			int xx = (m_bFlipHorizontal) ? pDoc->m_RawWidth-x : x;
			int yy = (m_bFlipVertical) ? pDoc->m_RawHeight-y : y;

			dc.SetPixel( xx, yy, c);

			switch(header){
			case 0x1A:
				dc.SetPixel(x, rHeight+rBaseY, c);
				break;
			case 0x1B:
				dc.SetPixel(x, gHeight+gBaseY, c);
				break;
			case 0x1C:
				dc.SetPixel(x, bHeight+bBaseY, c);
				break;
			}
			rp++;
		}
		switch(header){
		case 0x1A:
			rHeight++;
			break;
		case 0x1B:
			gHeight++;
			break;
		case 0x1C:
			bHeight++;
			break;
		}
	}
	dc.SelectObject(oldBitmap);
	dc.DeleteDC();

	ReleaseDC(pDC);

	return bitmap;
}
*/

void ST150Image::FlipImageHorizontal()
{
	for(int y = 0; y < m_WImageSize.cy; y++) {
		int left = 0;
		int right = m_WImageSize.cx-1;
		while(left < right) {
			unsigned char c = m_W_ImageBuffer[y][left];
			m_W_ImageBuffer[y][left] = m_W_ImageBuffer[y][right];
			m_W_ImageBuffer[y][right] = c;

			c = m_IR_ImageBuffer[y][left];
			m_IR_ImageBuffer[y][left] = m_IR_ImageBuffer[y][right];
			m_IR_ImageBuffer[y][right] = c;

			left++;
			right--;
		}
	}
}

void ST150Image::FlipImageVertical()
{
	for(int x = 0; x < m_WImageSize.cx; x++) {
		int low = 0;
		int high = m_WImageSize.cy - 1;
		while(low < high) {
			unsigned char c = m_IR_ImageBuffer[low][x];
			m_IR_ImageBuffer[low][x] = m_IR_ImageBuffer[high][x];
			m_IR_ImageBuffer[high][x] = c;

			c = m_W_ImageBuffer[low][x];
			m_W_ImageBuffer[low][x] = m_W_ImageBuffer[high][x];
			m_W_ImageBuffer[high][x] = c;

			low++;
			high--;
		}
	}
}

float GetImgAverage(unsigned char img[][1728], int cx, int cy)
{
	int nImageSize = cx * cy;
	int nSum = 0;
	for(int y = 0; y < cy; y++) {
		for(int x = 0; x < cx; x++) {
			nSum += img[y][x];
		}
	}

	return (float)nSum / (float)nImageSize;
}

float GetMinMaxFactor(unsigned char img[][1728], int cx, int cy, float* nMinFactor, float* nMaxFactor)
{
	float nAverage = GetImgAverage(img, cx, cy);

	float nMax = 127;
	float nMin = -128;
	for(int y = 0; y < cy; y++) {
		for(int x = 0; x < cx; x++) {
			unsigned short c = img[y][x];
			float tmp = (float)c - nAverage;
			if(tmp > nMax) nMax = tmp;
			if(tmp < nMin) nMin = tmp;
		}
	}

	*nMaxFactor = (float)127 / nMax;
	*nMinFactor = (float)-127 / nMin;

	return nAverage;
}

CBitmap* ST150Image::MakeBitmapFromRaw(CDC *pDC, int w_threshold, int ir_threshold, int ImgReinforce)
{
	CBitmap *bitmap = new CBitmap();
	bitmap->CreateCompatibleBitmap(pDC, m_ImageSize.cx, m_ImageSize.cy);

	BITMAP bm;
	bitmap->GetBitmap(&bm);

#if 1
	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = m_ImageSize.cx;
	bmi.bmiHeader.biHeight = m_ImageSize.cy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;       
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = m_ImageSize.cx * m_ImageSize.cy* 3;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;

	unsigned char *rowBuffer = (unsigned char*)malloc(bmi.bmiHeader.biWidth*3);
	int x, y;

	if(ImgReinforce) {
		float nMaxFactor, nMinFactor;
		float nAverage = GetMinMaxFactor(m_W_ImageBuffer, m_WImageSize.cx, m_WImageSize.cy, &nMinFactor, &nMaxFactor);

		for(y = 0; y < m_WImageSize.cy; y++) {
			unsigned char* dp = rowBuffer;
			for(x = 0; x < m_WImageSize.cx; x++) {
				unsigned short c = m_W_ImageBuffer[y][x];
				float tmp = (float)c - nAverage;
				if(tmp < (float)0) c = (tmp*nMinFactor) + 127;
				else c = (tmp * nMaxFactor) + 127;

				*dp++ = c;
				*dp++ = c;
				*dp++ = c;
			}
			::SetDIBits(pDC->GetSafeHdc(), (HBITMAP)bitmap->GetSafeHandle(), m_ImageSize.cy - 1 - y, 1, rowBuffer, &bmi, DIB_RGB_COLORS);
		}

		nAverage = GetMinMaxFactor(m_IR_ImageBuffer, m_IImageSize.cx, m_IImageSize.cy, &nMinFactor, &nMaxFactor);

		for(y = 0; y < m_IImageSize.cy; y++) {
			unsigned char* dp = rowBuffer;
			for(x = 0; x < m_IImageSize.cx; x++) {
				unsigned short c = m_IR_ImageBuffer[y][x];
				float tmp = (float)c - nAverage;
				if(tmp < (float)0) c = (tmp*nMinFactor) + 127;
				else c = (tmp * nMaxFactor) + 127;

				*dp++ = c;
				*dp++ = c;
				*dp++ = c;
			}
			::SetDIBits(pDC->GetSafeHdc(), (HBITMAP)bitmap->GetSafeHandle(), m_ImageSize.cy - 1 - y - m_WImageSize.cy, 1, rowBuffer, &bmi, DIB_RGB_COLORS);
		}
	} else {
		for(y = 0; y < m_WImageSize.cy; y++) {
			unsigned char* dp = rowBuffer;
			for(x = 0; x < m_WImageSize.cx; x++) {
				unsigned char c = m_W_ImageBuffer[y][x];
				if(c < w_threshold) c = 0;
				*dp++ = c;
				*dp++ = c;
				*dp++ = c;
			}
			::SetDIBits(pDC->GetSafeHdc(), (HBITMAP)bitmap->GetSafeHandle(), m_ImageSize.cy - 1 - y, 1, rowBuffer, &bmi, DIB_RGB_COLORS);
		}

		for(y = 0; y < m_IImageSize.cy; y++) {
			unsigned char* dp = rowBuffer;
			for(x = 0; x < m_IImageSize.cx; x++) {
				unsigned char c = m_IR_ImageBuffer[y][x];
				if(c < ir_threshold) c = 0;
				*dp++ = c;
				*dp++ = c;
				*dp++ = c;
			}
			::SetDIBits(pDC->GetSafeHdc(), (HBITMAP)bitmap->GetSafeHandle(), m_ImageSize.cy - 1 - y - m_WImageSize.cy, 1, rowBuffer, &bmi, DIB_RGB_COLORS);
		}
	}
	free(rowBuffer);
#else
	CDC dc;
	dc.CreateCompatibleDC(pDC);
	CBitmap* oldBitmap = dc.SelectObject(bitmap);

	int x, y;
//	for(x = 0; x < bm.bmWidth; x++) {
//		for(y = 0; y < bm.bmHeight; y++) dc.SetPixel(x, y, RGB(255,255,255));
//	}

	for(y = 0; y < m_WImageSize.cy; y++) {
		for(x = 0; x < 1728; x++) {
			COLORREF c = RGB(m_W_ImageBuffer[y][x], m_W_ImageBuffer[y][x], m_W_ImageBuffer[y][x]);
			dc.SetPixel( x, y, c);
		}
	}

	for(y = 0; y < m_IImageSize.cy; y++) {
		for(x = 0; x < 1728; x++) {
			COLORREF c = RGB(m_IR_ImageBuffer[y][x], m_IR_ImageBuffer[y][x], m_IR_ImageBuffer[y][x]);

			dc.SetPixel( x, y + m_WImageSize.cy, c);
		}
	}
	dc.SelectObject(oldBitmap);
	dc.DeleteDC();
#endif
	return bitmap;
}

int ST150Image::GetLastNamePosition(TCHAR *name)
{
	int Pos = lstrlen(name);
	while(Pos > 0) {
		Pos--;
		if(name[Pos] == _T('\\')) break;
	}
	if(name[Pos] == _T('\\'))
		return Pos + 1;

	return -1;
}

#define TEST_DUAL_DSP 1 //ryuhs74@20121106 - For Dual DSP image
int ST150Image::LoadImageFile(LPCTSTR cszPathName)
{
#if TEST_DUAL_DSP
	BOOL bUpperImaage = FALSE;
#endif

	TCHAR pathname[MAX_PATH];
	lstrcpy(pathname, cszPathName);

	m_DocumentType = DOC_TYPE_IMAGE;

	int Pos = GetLastNamePosition(pathname);
	if(Pos == -1)
		return 0;

	GetWidthHeight(pathname);

	if(wcsncmp(pathname+Pos, L"rgn_", 4) == 0) {
		HANDLE h = 	CreateFile(pathname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h != INVALID_HANDLE_VALUE) {
			unsigned char *temp = (unsigned char*)malloc(m_WImageSize.cx * m_WImageSize.cy);
			DWORD dwio = 0;
			ReadFile(h, temp, m_WImageSize.cx * m_WImageSize.cy, &dwio, NULL);
			CloseHandle(h);
			int ti = 0;
			for(int y = 0; y < m_WImageSize.cy; y++) {
				for(int x = 0; x < m_WImageSize.cx; x++) {
					m_W_ImageBuffer[y][x] = temp[ti++];
				}
			}
			free(temp);
		}
		m_IImageSize.SetSize(0,0);
		return 1;
	}

#if TEST_DUAL_DSP //ryuhs74@20121106 - Upper DSP의 캡쳐 이미지 파일 이름은 X,Y이다. 파일명이 틀려 이미지가 보지이 않았다.
	if(
		(wcsncmp(pathname+Pos, L"imgI", 4) != 0 && wcsncmp(pathname+Pos, L"imgW", 4) != 0) && 
		(wcsncmp(pathname+Pos, L"imgX", 4) != 0 && wcsncmp(pathname+Pos, L"imgY", 4) != 0)) {
		MessageBox(NULL, L"RAW file should start with imgI or imgW or imgY or imgX.", L"Check File Name", MB_OK);
		return 0;
	}


	if(wcsncmp(pathname+Pos, L"imgX", 4) != 0 && wcsncmp(pathname+Pos, L"imgY", 4) != 0) {
		pathname[Pos+3] = _T('W');
		bUpperImaage = FALSE;
	} else {
		pathname[Pos+3] = _T('Y');
		bUpperImaage = TRUE;
	}
	// Grey 부터 처리
	DWORD dwio = 0;
	
	if(lstrcmp(pathname+lstrlen(pathname)-4, L".tif") == 0) {
		char szpathname[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, pathname, lstrlen(pathname)+1, szpathname, MAX_PATH, NULL, NULL);
		TIFF* tif = TIFFOpen(szpathname, "r");
		if(tif) {
	        uint32 w, h;
		    size_t npixels;
        
			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

			m_WImageSize.cy = h;
			m_WImageSize.cx = w;

			int size = TIFFScanlineSize(tif);
			for (int row = 0; row < h; row++)
				TIFFReadScanline(tif, m_W_ImageBuffer[row], row);
			TIFFClose(tif);
		} else {
		}
	} else {
		// Load RAW Image
		HANDLE h = 	CreateFile(pathname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h != INVALID_HANDLE_VALUE) {
			int y;
			for( y = 0; y < m_WImageSize.cy; y++) {
				if(!ReadFile(h, m_W_ImageBuffer[y], m_WImageSize.cx, &dwio, NULL))
					break;
				if(m_WImageSize.cx != dwio)
					break;
			}
			CloseHandle(h);
			if(y != m_WImageSize.cy)
				m_WImageSize.cy = y;
		} else {
			m_WImageSize.SetSize(0,0);
		}
	}

	// IR 처리	
	dwio = 0;
	pathname[Pos+3] = _T('I');

	if(bUpperImaage == FALSE) {
		if(lstrcmp(pathname+lstrlen(pathname)-4, L".tif") == 0) 
		{
			char szpathname[MAX_PATH];
			WideCharToMultiByte(CP_ACP, 0, pathname, lstrlen(pathname)+1, szpathname, MAX_PATH, NULL, NULL);
			TIFF* tif = TIFFOpen(szpathname, "r");
			
			if(tif) {
				uint32 w, h;
				size_t npixels;
				uint32* raster;
        
				TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
				TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

				m_IImageSize.cy = h;
				m_IImageSize.cx = w;

				int size = TIFFScanlineSize(tif);
				for (int row = 0; row < h; row++)
					TIFFReadScanline(tif, m_IR_ImageBuffer[row], row);
				TIFFClose(tif);
			} else {
			}
		} else {
			HANDLE h = CreateFile(pathname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			
			if(h != INVALID_HANDLE_VALUE) {
				int y;
				for( y = 0; y < m_IImageSize.cy; y++) {
					if(!ReadFile(h, m_IR_ImageBuffer[y], m_IImageSize.cx, &dwio, NULL))
						break;
					if(m_IImageSize.cx != dwio)
						break;
				}
				CloseHandle(h);
				if(y != m_IImageSize.cy)
					m_IImageSize.cy = y;
				else {
	//			szPathName[Pos+3] = _T('X');
	//			h = CreateFile(szPathName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//			WriteFile(h, m_IR_ImageBuffer, dwio, &dwio, NULL);
	//			CloseHandle(h);
				}
			} else {
				m_IImageSize.SetSize(0,0);
			}
		}
	}

	m_ImageSize.cx = m_WImageSize.cx ? m_WImageSize.cx : m_IImageSize.cx ;
	m_ImageSize.cy = m_WImageSize.cy + m_IImageSize.cy;

#else
	if(wcsncmp(pathname+Pos, L"imgI", 4) != 0 && wcsncmp(pathname+Pos, L"imgW", 4) != 0) {
		MessageBox(NULL, L"RAW file should start with imgI or imgW.", L"Check File Name", MB_OK);
		return 0;
	}

		// Grey 부터 처리
	DWORD dwio = 0;
	pathname[Pos+3] = _T('Y');//_T('W');
	if(lstrcmp(pathname+lstrlen(pathname)-4, L".tif") == 0) {
		char szpathname[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, pathname, lstrlen(pathname)+1, szpathname, MAX_PATH, NULL, NULL);
		TIFF* tif = TIFFOpen(szpathname, "r");
		if(tif) {
	        uint32 w, h;
		    size_t npixels;
        
			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

			m_WImageSize.cy = h;
			m_WImageSize.cx = w;

			int size = TIFFScanlineSize(tif);
			for (int row = 0; row < h; row++)
				TIFFReadScanline(tif, m_W_ImageBuffer[row], row);
			TIFFClose(tif);
		} else {
		}
	} else {
		// Load RAW Image
		HANDLE h = 	CreateFile(pathname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h != INVALID_HANDLE_VALUE) {
			int y;
			for( y = 0; y < m_WImageSize.cy; y++) {
				if(!ReadFile(h, m_W_ImageBuffer[y], m_WImageSize.cx, &dwio, NULL))
					break;
				if(m_WImageSize.cx != dwio)
					break;
			}
			CloseHandle(h);
			if(y != m_WImageSize.cy)
				m_WImageSize.cy = y;
		} else {
			m_WImageSize.SetSize(0,0);
		}
	}

	// IR 처리
	dwio = 0;
	pathname[Pos+3] = _T('X');//_T('I');
	if(lstrcmp(pathname+lstrlen(pathname)-4, L".tif") == 0) {
		char szpathname[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, pathname, lstrlen(pathname)+1, szpathname, MAX_PATH, NULL, NULL);
		TIFF* tif = TIFFOpen(szpathname, "r");
		if(tif) {
	        uint32 w, h;
		    size_t npixels;
			uint32* raster;
        
			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

			m_IImageSize.cy = h;
			m_IImageSize.cx = w;

			int size = TIFFScanlineSize(tif);
			for (int row = 0; row < h; row++)
				TIFFReadScanline(tif, m_IR_ImageBuffer[row], row);
			TIFFClose(tif);
		} else {
		}
	} else {
		HANDLE h = CreateFile(pathname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h != INVALID_HANDLE_VALUE) {
			int y;
			for( y = 0; y < m_IImageSize.cy; y++) {
				if(!ReadFile(h, m_IR_ImageBuffer[y], m_IImageSize.cx, &dwio, NULL))
					break;
				if(m_IImageSize.cx != dwio)
					break;
			}
			CloseHandle(h);
			if(y != m_IImageSize.cy)
				m_IImageSize.cy = y;
			else {
	//			szPathName[Pos+3] = _T('X');
	//			h = CreateFile(szPathName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//			WriteFile(h, m_IR_ImageBuffer, dwio, &dwio, NULL);
	//			CloseHandle(h);
			}
		} else {
			m_IImageSize.SetSize(0,0);
		}
	}

	m_ImageSize.cx = m_WImageSize.cx ? m_WImageSize.cx : m_IImageSize.cx ;
	m_ImageSize.cy = m_WImageSize.cy + m_IImageSize.cy;
#endif

	return 1;
}

void ST150Image::ClearMem()
{
	m_ImageSize.SetSize(0,0);
	m_WImageSize.SetSize(0,0);
	m_IImageSize.SetSize(0,0);
}

extern "C" int dMarkExtRegion;

int GetDigitNumber(TCHAR *s)
{
	int v = 0;
	while(*s >= _T('0') && *s <= _T('9')) {
		v = v * 10 + (*s - _T('0'));
		s++;
	}

	return v;
}


void ST150Image::GetWidthHeight(LPCTSTR str)
{
	CString s = str;

	m_WImageSize.SetSize(0,0);
	m_IImageSize.SetSize(0,0);

	TCHAR pathname[MAX_PATH];

	lstrcpy(pathname, (LPCTSTR)s);
	TCHAR *fp = pathname;
	while(*fp) fp++; fp -= 4;
	while(fp != pathname) {
		fp--;
		if(*fp < _T('0') || *fp > _T('9')) {
			m_WImageSize.cy = GetDigitNumber(fp+1);
			break;
		}
	}

	while(fp != pathname) {
		fp--;
		if(*fp < _T('0') || *fp > _T('9')) {
			m_WImageSize.cx = GetDigitNumber(fp+1);
			break;
		}
	}

	m_IImageSize = m_WImageSize;
}

void ST150Image::V1_SaveKeyToFile(TCHAR* keyname)
{
	char g_KeyTxBuf[8192];

	int i = 0;
	int j = 0;
	int KeyTxLength = 0;
	// 전송할 내용을 준비한다.
	KeyTxLength += sprintf_s(g_KeyTxBuf+KeyTxLength, 8192, "SIFT_DN=%d\r\n", V1_SIFT_DESCRIPTER_NUM);
	KeyTxLength += sprintf_s(g_KeyTxBuf+KeyTxLength, 8192, "SIFT_KN=%d\r\n", SIFT_KEY_VECTOR);
	for(i = 0; i < V1_SIFT_DESCRIPTER_NUM; i++){
		KeyTxLength += sprintf_s(g_KeyTxBuf+KeyTxLength, 8192, "%d\t%d\t", OneNote.CIS.V1_descriptor[i].x, OneNote.CIS.V1_descriptor[i].y);
		for(j = 0; j < SIFT_KEY_VECTOR; j++){
			KeyTxLength += sprintf_s(g_KeyTxBuf+KeyTxLength, 8192, "%d\t", OneNote.CIS.V1_descriptor[i].V1_DesVector[j]);
		}
		KeyTxLength += sprintf_s(g_KeyTxBuf+KeyTxLength, 8192, "\r\n");
	}
	KeyTxLength += sprintf_s(g_KeyTxBuf+KeyTxLength, 8192, "END\r\n");

	TCHAR KeyFileName[MAX_PATH];
	wsprintf(KeyFileName, L"%s.%dkey.txt", keyname, 0);
	HANDLE h = CreateFile(KeyFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h != INVALID_HANDLE_VALUE) {
		DWORD dwio = 0;
		WriteFile(h, g_KeyTxBuf, KeyTxLength, &dwio, NULL);
		CloseHandle(h);
	}
}

void ST150Image::V2_SaveKeyToFile(TCHAR* keyname)
{
	int i = 0;
	int j = 0;

	// 전송할 내용을 준비한다.
	TCHAR KeyFileName[MAX_PATH];
	wsprintf(KeyFileName, L"%s.%dkey.txt", keyname, 0);
	HANDLE h = CreateFile(KeyFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return;

	sprintf_s(g_KeyTxBuf, 8192, "SIFT_DN=%d\r\n", SIFT_DESCRIPTER_NUM);
	DWORD dwio = 0;
	WriteFile(h, g_KeyTxBuf, strlen(g_KeyTxBuf), &dwio, NULL);

	sprintf_s(g_KeyTxBuf, 8192, "SIFT_KN=%d\r\n", SIFT_KEY_VECTOR);
	dwio = 0;
	WriteFile(h, g_KeyTxBuf, strlen(g_KeyTxBuf), &dwio, NULL);

	for(i = 0; i < SIFT_DESCRIPTER_NUM; i++){
		sprintf_s(g_KeyTxBuf, 8192, "%d\t%d\t", OneNote.CIS.descriptor[i].x, OneNote.CIS.descriptor[i].y);
		dwio = 0;
		WriteFile(h, g_KeyTxBuf, strlen(g_KeyTxBuf), &dwio, NULL);

		sprintf_s(g_KeyTxBuf, 8192, "%d\t", OneNote.CIS.descriptor[i].DesVector);
		dwio = 0;
		WriteFile(h, g_KeyTxBuf, strlen(g_KeyTxBuf), &dwio, NULL);

		sprintf_s(g_KeyTxBuf, 8192, "\r\n");
		dwio = 0;
		WriteFile(h, g_KeyTxBuf, strlen(g_KeyTxBuf), &dwio, NULL);
	}
	sprintf_s(g_KeyTxBuf, 8192, "END\r\n");
	dwio = 0;
	WriteFile(h, g_KeyTxBuf, strlen(g_KeyTxBuf), &dwio, NULL);

	CloseHandle(h);
}

//권종별 key matching routine

// CST150ExplorerDoc serialization
void ST150Image::ParseTapeData()
{
	int index = 0;

	memset(&OneNote, 0, sizeof(OneNote));
	for(int i = 0; i < m_TapeImageSize; ) {
		unsigned int hb = m_TapeImageBuffer[i++];
		unsigned int lb = m_TapeImageBuffer[i++];
		int count;

		count = (hb << 8) | lb;
		if(count > OneNote.TDS.MaxADC) OneNote.TDS.MaxADC = count;
		OneNote.TDS.ADCnt[index] = count;
		for(int x = 0; x < count; x++) {
			hb = m_TapeImageBuffer[i++];
			lb = m_TapeImageBuffer[i++];
			OneNote.TDS.ThickBuf[index][x] = (hb << 8) | lb;
			OneNote.TDS.MultSum[index] += OneNote.TDS.ThickBuf[index][x];
		}

		if(index < 12)
			index++;
		else
			break;
	}
}

/*
// UV 데이터를 Seq_UVCh()를 이용하여 저장하게 변경하면서 이 함수는 필요가 없어졌다. 20120331 dschae
void ST150Image::ParseUVData()
{
	int index = 0;
	memset(&OneNote, 0, sizeof(OneNote));

	for(int i = 0; i < m_UVImageSize; ) {
		unsigned int hb = m_UVImageBuffer[i++];
		unsigned int lb = m_UVImageBuffer[i++];
		int count;

		count = (hb << 8) | lb;
		OneNote.UV.ADCnt[index] = count;
		if(count > OneNote.UV.MaxADC)
			OneNote.UV.MaxADC = count;
		for(int x = 0; x < count; x++) {
			hb = m_UVImageBuffer[i++];
			lb = m_UVImageBuffer[i++];
			if(x < MAX_UV_ADC)
				OneNote.UV.Buf_F[index][x] = (hb << 8) | lb;
			hb = m_UVImageBuffer[i++];
			lb = m_UVImageBuffer[i++];
			if(x < MAX_UV_ADC)
				OneNote.UV.Buf_R[index][x] = (hb << 8) | lb;
		}

		if(index < UV_CH)
			index++;
		else
			break;
	}
}
*/

void ST150Image::DrawSimpleGraph(CDC *pDC)
{
	if(m_RawDataSize <= 0)
		return;

	int i;
	int h = 0;
	for(i = 0; i < m_RawDataSize; i++) {
		if(i == 0) pDC->MoveTo(i, m_RawData[i]);
		else pDC->LineTo(i, m_RawData[i]);
	}
}

int ST150Image::ReadFileData(LPCTSTR tszFileName, int size, unsigned char* buf)
{
	HANDLE h = CreateFile(tszFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return FALSE;
	DWORD fSizeLow, fSizeHigh;
	fSizeLow = GetFileSize(h, &fSizeHigh);
	if(fSizeHigh > 0 || fSizeLow > size) {
		CloseHandle(h);
		return 0;
	}

	DWORD dwio = 0;
	ReadFile(h, buf, fSizeLow, &dwio, NULL);
	CloseHandle(h);

	return fSizeLow;
}

unsigned char* LoadDataFromFile(LPCTSTR mgpath, int* size)
{
	HANDLE h = CreateFile(mgpath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return NULL;

	int fsize = GetFileSize(h, NULL);
	unsigned char* mgdata = (unsigned char*)malloc(fsize);
	DWORD dwio = 0;
	ReadFile(h, mgdata, fsize, &dwio, NULL);
	CloseHandle(h);

	*size = fsize;
	return mgdata;
}

void AddUVTrack(ST_NOTE* pNote, unsigned char* data, int count, int Ch)
{
	int i = 0;
	for(int x = 0; x < count; x++) {
		unsigned int hb = data[i++];
		unsigned int lb = data[i++];
		gst_State.UVMG_ADBuf[UV_AD_F] = (hb << 8) | lb;
		hb = data[i++];
		lb = data[i++];
		gst_State.UVMG_ADBuf[UV_AD_R] = (hb << 8) | lb;
		Seq_UVCh(pNote, Ch);
	}
}

void SetUVData(ST_NOTE* pNote, unsigned char* data, int uvsize)
{
	memset(&pNote->UV, 0, sizeof(pNote->UV));
	gst_State.ArgosVers = 0x030b;

	int index = 0;
//20121010 sapa82 UV의 진짜 시작점이 없이 0부터 그냥 주욱 탄다. i를 진짜 UV 시작점에서 시작하게 하면 끗.
	for(int i = 0; i < uvsize; ) {
		unsigned int hb = data[i++];
		unsigned int lb = data[i++];
		int count;

		count = (hb << 8) | lb;
		if(count > UV_BUF) // 데이터에 오류가 있는 경우 작업을 중지한다.
			return;
		
		pNote->Length = count * 4; // MG와 달리 UV는 4먹스이므로 UV길이에 대해 4를 곱해줘야 지폐 길이가 된다.
		AddUVTrack(pNote, data+i, count, index);
		index++;

		i += count * 4;
		if(i > uvsize) i = uvsize; // 원인미상의 오류에 대응한다.
	}
	Seq_UVEnd(pNote); //ryuhs74@20120416 - 이 부분이 빠져서 0으로 나오고 있던 것임
}

void AddMGTrack(ST_NOTE* pNote, unsigned char* buf, int count, int Ch)
{
 	// 센터 이외의 좌우 두 개를 각각 하나처럼 합산하게 하는 변수이다. 총 3개가 됨
	int index_table[5] = { 0, 0, 1, 2, 2 };
	// MG 값의 유효치를 보는 변수, 센터는 40미만이면 값으로 치지 않는다는 말이다.
	int thresh_table[5] = { 15, 15, 40, 15, 15 };

	if(Ch >= MG_CH) // 최대치를 넘어서 저장되지 않도록
		return;
	
	for(int x = 0; x < count; x++) {
		if(x < MG_BUF) {
			gst_State.UVMG_ADBuf[Ch] = buf[x];
			Seq_MGCh(pNote, Ch, thresh_table[Ch], index_table[Ch]);
		}
	}
}

void SetMGData(ST_NOTE* pNote, unsigned char* data, int mgsize)
{
	memset(&pNote->MG, 0, sizeof(pNote->MG));

	int index = 0;

	for(int i = 0; i < mgsize; ) {
		unsigned int hb = data[i++];
		unsigned int lb = data[i++];
		int count;

		count = (hb << 8) | lb;

		if(count > MG_BUF)
			return; // 데이터에 오류가 있는 경우 작업을 중지한다. 일부 데이터가 전송 중에 오류가 발생한 경우가 있다.

		pNote->Length = count; // 기계에서 받은 길이 정보가 없으므로 일단 MG 데이터 전체 길이로 정해준다.
		if(index == 0)
			Seq_MGStart(pNote); // 최초에 한 번 시작하는 것 처럼 호출해 준다.
		AddMGTrack(pNote, data+i, count, index);

		i += (count);
		index += 1;
	}

}


int ST150Image::LoadFromFile(LPCTSTR tszFileName)
{
	lstrcpy(m_Filename, tszFileName);

	CString s = tszFileName;

	if(s.Right(4).CompareNoCase(L".raw") == 0 || s.Right(4).CompareNoCase(L".tif") == 0) {
		return LoadImageFile(tszFileName);
	}

	if(s.Right(3).CompareNoCase(L".mg") == 0) {
		m_DocumentType = DOC_TYPE_MG;
#if 0
	// UV 데이터를 Seq_UVCh()를 이용하여 저장하게 변경하면서 이 루틴이 수정되었다. 20120331 dschae
		m_MGImageSize = ReadFileData(tszFileName, sizeof(m_MGImageBuffer), m_MGImageBuffer);
		return m_MGImageSize;
#else
		unsigned char* mgdata = LoadDataFromFile(tszFileName, &m_MGImageSize);
		if(mgdata != NULL) {
			SetMGData(&OneNote, mgdata, m_MGImageSize);
			free(mgdata);
			return m_MGImageSize;
		}
#endif
	}

	if(s.Right(3).CompareNoCase(L".tp") == 0) {
		m_DocumentType = DOC_TYPE_TAPE;
		m_TapeImageSize = ReadFileData(tszFileName, sizeof(m_TapeImageBuffer), m_TapeImageBuffer);
		if(m_TapeImageSize)
			ParseTapeData();
		return m_TapeImageSize;
	}

	if(s.Right(3).CompareNoCase(L".uv") == 0) {
		m_DocumentType = DOC_TYPE_UV;
#if 0
	// UV 데이터를 Seq_UVCh()를 이용하여 저장하게 변경하면서 이 루틴이 수정되었다. 20120331 dschae
		m_UVImageSize = ReadFileData(tszFileName, sizeof(m_UVImageBuffer), m_UVImageBuffer);
		if(m_UVImageSize)
			ParseUVData();
		return m_UVImageSize;
#else
		unsigned char* uvdata = LoadDataFromFile(tszFileName, &m_UVImageSize);
		if(uvdata != NULL) {
			SetUVData(&OneNote, uvdata, m_UVImageSize);
			free(uvdata);
		}
		return m_UVImageSize;
#endif
	}

	return FALSE;
}

/*
void imgST150::GetWidthHeight(CArchive& ar)
{
	CString s = ar.m_strFileName;

	m_FileSize = GetFileSize(ar.GetFile()->m_hFile, NULL);

	m_RawWidth = 0;
	m_RawHeight = 0;
	TCHAR pathname[MAX_PATH];

	lstrcpy(pathname, (LPCTSTR)s);
	TCHAR *fp = pathname;
	while(*fp) fp++; fp -= 4;
	while(fp != pathname) {
		fp--;
		if(*fp < _T('0') || *fp > _T('9')) {
			m_RawHeight = GetDigitNumber(fp+1);
			break;
		}
	}

	while(fp != pathname) {
		fp--;
		if(*fp < _T('0') || *fp > _T('9')) {
			m_RawWidth = GetDigitNumber(fp+1);
			break;
		}
	}

	if(m_RawWidth == 0 || m_RawHeight == 0) {
		CImgSizeDlg dlg;
		dlg.m_width = m_RawWidth;
		dlg.m_height = m_RawHeight;
		dlg.m_FileSize = m_FileSize;
		if(dlg.DoModal() == IDOK) {
			if(dlg.m_IsSimpleGraph)
				this->m_DocumentType = DOC_TYPE_SIMPLE_GRAPH;
			m_RawWidth = dlg.m_width;
			m_RawHeight = dlg.m_height;
		}
	}
}
*/

void ST150Image::OnDraw(CDC *pDC)
{
	switch(m_DocumentType){
	case DOC_TYPE_MG:
		DrawMG(pDC, size);
		break;
	case DOC_TYPE_UV:
		DrawUV(pDC, size);
		break;
	case DOC_TYPE_SIMPLE_GRAPH:
		DrawSimpleGraph(pDC);
		break;
	case DOC_TYPE_TAPE:
		DrawTape(pDC);
	}
}

void DrawGraph(CDC *pDC, unsigned char* buf, int count, int h, int b, int m)
{
	int x;
	for(x = 0; x < count; x++) {
		if(x == 0) pDC->MoveTo(h + x*m, b + 255 - buf[x]);
		else pDC->LineTo(h + x*m, b + 255 - buf[x]);
	}

//	for(x = count*m; x < count*m+3; x++) {
//		pDC->MoveTo(h+x, b+0);
//		pDC->LineTo(h+x, b+255);
//	}
}

void DrawGraph16(CDC *pDC, Uint16* buf, int count, int h, int b, int m)
{
	int x;

	if (laptop2 == 1)	{
		for(x = 0; x < count; x+=2) {
			if(x == 0) pDC->MoveTo(h + x*m, b + 255 - buf[x]);
			else pDC->LineTo(h + (x/2)*m, b + 255 - buf[x]);
		}
	} else {
		for(x = 0; x < count; x++) {
			if(x == 0) pDC->MoveTo(h + x*m, b + 255 - buf[x]);
			else pDC->LineTo(h + x*m, b + 255 - buf[x]);
		}
	}


//	for(x = count*m; x < count*m+3; x++) {
//		pDC->MoveTo(h+x, b+0);
//		pDC->LineTo(h+x, b+255);
//	}
}

void DrawGraph2(CDC *pDC, Uint16* buf, int count, int h, int base, int m, int divider)
{
	int x;
	for(x = 0; x < count; x++) {
		int v = buf[x] / divider;
		if(x == 0)
			pDC->MoveTo(h + x*m, base - v);
		else
			pDC->LineTo(h + x*m, base - v);
	}

//	for(x = count*m; x < count*m+3; x++) {
//		pDC->MoveTo(h+x, b+0);
//		pDC->LineTo(h+x, b+255);
//	}
}

#define FILTER_TAP_NUMBER 5

#define WAVEFORM_MAX					255
#define WAVEFORM_CENTER				128
#define WAVEFORM_MIN					0
#define WAVEFORM_CUT_OFF_THRESHOLD	16

int filter_tap[FILTER_TAP_NUMBER] =
{
	1376, 
	6291,
	9568,
	6291, 
	1376 
};

void DrawLPF(CDC *pDC, unsigned char* buf, int count, int h, int b)
{
	int i,j;

	unsigned char output[1024];

	memset(output, 0, sizeof(output));

	int py = buf[0];
	for(i=0;i<count;i++)
	{
		output[i] = abs(buf[i]-128);
	}

	pDC->MoveTo(h + 0, b + 130 - output[0]);

	for(i = 1; i < count-1; i++) {
		if( output[i-1] < output[i] && output[i] == output[i+1]) {
			pDC->LineTo(h + i, b + 130 - output[i]);
		}
		else if( output[i-1] == output[i] && output[i] > output[i+1]) {
			pDC->LineTo(h + i, b + 130 - output[i]);
		}
		else if(output[i-1] < output[i] && output[i] > output[i+1]) {
			pDC->LineTo(h + i, b + 130 - output[i]);
		}
	}
	output[0] = output[1];
}

// DrawLPF의 16비트 파라미터 버전이다.
void DrawLPF16(CDC *pDC, Uint16* buf, int count, int h, int b)
{
	int i,j;

	unsigned char output[1024];

	memset(output, 0, sizeof(output));

	int py = buf[0];
	for(i=0;i<count;i++)
	{
		output[i] = abs(buf[i]-128);
	}

	pDC->MoveTo(h + 0, b + 130 - output[0]);

	if(laptop2 == 1)
	{
		for(i = 1; i < count-1; i+=2) {
			if( output[i-1] < output[i] && output[i] == output[i+1]) {
				pDC->LineTo(h + i/2, b + 130 - output[i]);
			}
			else if( output[i-1] == output[i] && output[i] > output[i+1]) {
				pDC->LineTo(h + i/2, b + 130 - output[i]);
			}
			else if(output[i-1] < output[i] && output[i] > output[i+1]) {
				pDC->LineTo(h + i/2, b + 130 - output[i]);
			}
		}
	}
	else
	{
	for(i = 1; i < count-1; i++) {
		if( output[i-1] < output[i] && output[i] == output[i+1]) {
			pDC->LineTo(h + i, b + 130 - output[i]);
		}
		else if( output[i-1] == output[i] && output[i] > output[i+1]) {
			pDC->LineTo(h + i, b + 130 - output[i]);
		}
		else if(output[i-1] < output[i] && output[i] > output[i+1]) {
			pDC->LineTo(h + i, b + 130 - output[i]);
		}
	}
	}

	output[0] = output[1];
}

int GetMedium3(unsigned char* dst, unsigned char* src, int cnt)
{
	unsigned char p, q;
	p = src[0];
	q = src[1];
	int j = 0;
	for(int i = 2; i < cnt-1; i++) {
		unsigned char r = src[i];
		unsigned char m;
		if(p == q) {
			m = p;
		} else if(p > q) {
			if( q > r)
				m = q;
			else {
				if(r > p)
					m = p;
				else
					m = r;
			}
		} else { // p < q
			if(r < p)
				m = p;
			else {
				if(r > q)
					m = q;
				else
					m = r;
			}
		}
		dst[j++] = m;
		p = q;
		q = r;
	}

	return j;
}

int GetMedium32(Uint16 * dst, Uint16* src, int cnt)
{
	int p, q;
	p = src[0];
	q = src[1];
	int j = 0;
	for(int i = 2; i < cnt-1; i++) {
		int r = src[i];
		int m;
		if(p == q) {
			m = p;
		} else if(p > q) {
			if( q > r)
				m = q;
			else {
				if(r > p)
					m = p;
				else
					m = r;
			}
		} else { // p < q
			if(r < p)
				m = p;
			else {
				if(r > q)
					m = q;
				else
					m = r;
			}
		}
		dst[j++] = m;
		p = q;
		q = r;
	}

	return j;
}

int SumBytes(unsigned char* src, int cnt)
{
	int sum = 0;
	for(int i = 0; i < cnt; i++) sum += src[i];
	return sum;
}

int SumInts(int * src, int cnt)
{
	int sum = 0;
	for(int i = 0; i < cnt; i++) sum += src[i];
	return sum;
}

unsigned char* LoadDataBySequence(TCHAR* szPathName, int* size, TCHAR* format)
{
	// 이미지의 일련번호 찾기
	TCHAR *cp = szPathName;
	TCHAR *lp = NULL;
	while(*cp) {
		if(*cp == '\\') lp = cp; // 어쨌든 마지막 '\\'의 위치가 저장된다.
		cp++;
	}
	if(lp == NULL)
		return NULL;

	int seq = 0;
	while(*lp && (*lp < '0' || *lp > '9')) lp++; // 숫자 아닌 부분 통과
	while(*lp >= '0' && *lp <= '9') {
		seq = seq * 10 + (*lp - '0');
		lp++;
	}
		
	TCHAR mgpath[1024/*MAX_PATH*/] = {0,};
	lstrcpy((LPTSTR)mgpath, (LPTSTR)szPathName);
	cp = mgpath;
	while(*cp) {
		if(*cp == '\\') lp = cp; // 어쨌든 마지막 '\\'의 위치가 저장된다.
		cp++;
	}
	wsprintf(lp, format, seq);

	return LoadDataFromFile(mgpath, size);
}

unsigned char* LoadMGDataBySequence(TCHAR* szPathName, int *mgsize)
{
	return LoadDataBySequence(szPathName, mgsize, L"\\mg_%03d.mg");
}

unsigned char* LoadUVDataFromSequence(TCHAR* szPathName, int *mgsize)
{
	return LoadDataBySequence(szPathName, mgsize, L"\\uv_%03d.uv");
}

int ST150Image::SetMGDataBySequence(TCHAR* szPathName, ST_NOTE* pNote)
{
	memset(&pNote->MG, 0, sizeof(pNote->MG));

	m_MGImageSize = 0;
	unsigned char *data = LoadMGDataBySequence(szPathName, &m_MGImageSize);
	if(data == NULL)
		return -1;

	SetMGData(pNote, data, m_MGImageSize);
	free(data);

	return 0;
}

int ST150Image::SetUVDataBySequence(TCHAR* szPathName, ST_NOTE* pNote)
{
	memset(&pNote->UV, 0, sizeof(pNote->UV));

	unsigned char *data = LoadUVDataFromSequence(szPathName, &m_UVImageSize);
	if(data == NULL)
		return -1;

	SetUVData(pNote, data, m_UVImageSize);

	free(data);

	return 0;
}

void ClearRect(CDC* pDC, int left, int top, int right, int bottom)
{
	// 배경 지우기
	CBrush *wbrush = new CBrush(RGB(255,255,255));
	RECT rect;
	rect.left = left; rect.top = top;
	rect.right = right; rect.bottom = bottom;
	pDC->FillRect(&rect, wbrush);
	delete wbrush;
}


void ST150Image::DrawMG(CDC* pDC, CSize& size)
{
	if(m_MGImageSize <= 0)
		return;

	int h = 0;
	int index = 0;

	if(pDC != NULL) {
		// 실제로 그린다.
		ClearRect(pDC, 0, 0, size.cx, size.cy); // 배경 지우기

		for(index = 0; index < MG_CH; index++) {
			int count = OneNote.MG.ADCnt[index];

			TCHAR msgbuf[24];
			wsprintf(msgbuf, L"%d", OneNote.MG.Buf[index][0]);
			laptop2 = m_nLaptop;
//20121019 sapa82 플래그 체크 여부에 따라 그래프를 그릴때 샘플링을 다 할건지 반만 할건지 결정한다.
			if (laptop2 == 1) {
				pDC->MoveTo(h, 0); pDC->LineTo(h, 258); pDC->LineTo(h+count/2, 258); pDC->LineTo(h+count/2, 0);		
			} else {
				pDC->MoveTo(h, 0); pDC->LineTo(h, 258); pDC->LineTo(h+count, 258); pDC->LineTo(h+count, 0);
			}

			pDC->TextOutW(h, 0, msgbuf, lstrlen(msgbuf));
			
			DrawGraph16(pDC, OneNote.MG.Buf[index], count-1, h, 0, 1);

			// 이것은 MG 신호 강도를 그리는 부분이다
			DrawLPF16(pDC, OneNote.MG.Buf[index], count-1, h, 258);
			if (laptop2 == 1) {
				h += count/2 + 10;
			} else {
				h += count + 10;
			}
		}
	} else {
		// 이미지의 크기만 구한다.
		for(index = 0; index < MG_CH; index++) 
		{
			int count = OneNote.MG.ADCnt[index];
			h += count + 10;
		}
	}
	size = CSize(h, 258);
}

void ST150Image::DrawTape(CDC* pDC)
{
	size = CSize(0,0);
	if(this->m_TapeImageSize <= 0)
		return;

	ST_NOTE *Note = &OneNote;

	int h = 0;

	TapeDisplayGap = 1;

	int ci;
	int Avg[TDS_CH];
	int TapeStart, TapeEnd;
	int VTh, HTh;

	VTh = 3500;
	HTh = 3200;

	TapeStart = 0;
	TapeEnd = 11;

	Note->TDS.g_Tape_Channel = -1;
	Note->TDS.g_Tape_Pos = -1;
	for(ci = 0; ci < TDS_CH; ci++) {
		int AdCnt = Note->TDS.ADCnt[ci];
		Avg[ci] = AdCnt > 30 ? Note->TDS.MultSum[ci]/AdCnt : 0;
	}

	for(ci = 0; ci < TDS_CH; ci++) {
		h = ci * (Note->TDS.MaxADC+10);
		int count = Note->TDS.ADCnt[ci];
		pDC->MoveTo(h, 0); pDC->LineTo(h, 258); pDC->LineTo(h+count*TapeDisplayGap, 258); pDC->LineTo(h+count*TapeDisplayGap, 0);

		TapeTrackStart[ci] = h;
		TapeTrackEnd[ci] = h+count;
		DrawGraph2(pDC, Note->TDS.ThickBuf[ci], count, h, 255, TapeDisplayGap, 100);

		Uint16 mediums[1024];
		int len3 = GetMedium32(mediums, Note->TDS.ThickBuf[ci], count);
		DrawGraph2(pDC, mediums, len3, h, 255+256, TapeDisplayGap, 100);
	}

	// 길게 붙인 테이프 검출
	for( ci = TapeStart+1; ci <= TapeEnd; ci++) {
		h = ci * (Note->TDS.MaxADC+10);
		if(Avg[ci] > 10000+VTh ||
			Avg[ci-1] > 7000 && Avg[ci] > 7000)
		{
			int diff = (Avg[ci-1] > Avg[ci]) ? (Avg[ci-1] - Avg[ci]) : (Avg[ci] - Avg[ci-1]);
			if(diff >= VTh) {
				Note->TDS.g_Tape_Channel = ci;
				Note->Result = S_CMD_ER_TDS;

				CPen pen;
				pen.CreatePen(PS_SOLID, 1, RGB(255,0,0));
				CPen *open = pDC->SelectObject(&pen);
				for(int hc = 0; hc < 3; hc++) {
					pDC->MoveTo(h, hc+12);
					pDC->LineTo(h+Note->TDS.ADCnt[ci], hc+12);
				}
				pDC->SelectObject(open);
			}
		}
		TCHAR buf[16];
		wsprintf(buf, L"%d  ", Avg[ci]);
		pDC->TextOut(h, 256-20, buf, lstrlen(buf));
	}

#if 0
	if(DoubleTest > 5) {
		Note->TDS.g_Tape_Channel = 5;
		Note->Result = S_CMD_ER_DOUBLE;
		return;
	}
#endif

#define	SEE_DOWN_HILL	8
#define	PRE_DOWN_HILL	17

#if 1 // 대각선으로 붙은 테이프 검출. 나중으로 미루자.
	for( ci = TapeStart; ci <= TapeEnd; ci++) {
		int xx;
		int upcount = 0;
		Uint16 *pbuf = Note->TDS.ThickBuf[ci];
		int count = Note->TDS.ADCnt[ci] - SEE_DOWN_HILL;
		int count10 = count - SEE_DOWN_HILL;
		for(xx = PRE_DOWN_HILL; xx < count10; xx++) {
			if(pbuf[xx] > 12800) {
				upcount++;
				if(upcount > 25) {
					Note->TDS.g_Tape_Channel = ci;
					Note->Result = S_CMD_ER_TDS;

					CPen pen;
					pen.CreatePen(PS_SOLID, 1, RGB(255,0,0));
					CPen *open = pDC->SelectObject(&pen);
					for(int hc = 0; hc < 3; hc++) {
						pDC->MoveTo(h, hc+12);
						pDC->LineTo(h+30, hc+12);
					}
					pDC->SelectObject(open);
					return;
				}
			}
		}
	}
#endif			

	// 자신의 채널에서만 테이프를 검출한다.
	for( ci = TapeStart; ci <= TapeEnd; ci++) {
		h = ci * (Note->TDS.MaxADC+10);
		int xx, count;
		int upcount = 0;
		int count10;
		Uint16 *pbuf = Note->TDS.ThickBuf[ci];
		if(Avg[ci] < 7000)
			continue;

		count = Note->TDS.ADCnt[ci] - SEE_DOWN_HILL;
		count10 = count-(10-SEE_DOWN_HILL);
		for(xx = PRE_DOWN_HILL; xx < count; xx++) {
			if(pbuf[xx] > pbuf[xx+2]) { // falling
				int d = pbuf[xx] - pbuf[xx+2];
				if(d >= HTh && pbuf[xx+2] >= 7500) {
					int idx;
					int qqcnt = 0;
					int pavg = pbuf[xx]+pbuf[xx-1]+pbuf[xx-2]+pbuf[xx-3]+pbuf[xx-4];
					pavg = pavg/5 - HTh;
					for( idx = 0; idx < SEE_DOWN_HILL; idx++)
						if(pbuf[xx+2+idx] > 7500 && pbuf[xx+2+idx] <= pavg) qqcnt++;
					if(qqcnt >= SEE_DOWN_HILL-2) {
						int ppcnt = 0;
						int ppbegin = xx - PRE_DOWN_HILL;
						for(idx = ppbegin; idx < xx; idx++)
							if(pbuf[idx] < pavg) ppcnt++;
						if(ppcnt < 2) {
							Note->TDS.g_Tape_Channel = ci;
							Note->TDS.g_Tape_Pos = xx;
							Note->Result = S_CMD_ER_TDS;

							CPen pen;
							pen.CreatePen(PS_SOLID, 1, RGB(0,0,255));
							CPen *open = pDC->SelectObject(&pen);
							pDC->MoveTo(h+xx, 40);
							pDC->LineTo(h+xx, 200);
							for(int hc = 0; hc < 3; hc++) {
								pDC->MoveTo(h+xx+hc, 20);
								pDC->LineTo(h+xx+hc, 40);
							}
							pDC->SelectObject(open);
							TCHAR msgbuf[16];
							wsprintf(msgbuf, L"F%d", xx);
							pDC->TextOut(h+xx+4, 20, msgbuf, lstrlen(msgbuf));
						} else {
							CPen pen;
							pen.CreatePen(PS_SOLID, 1, RGB(255,0,255));
							CPen *open = pDC->SelectObject(&pen);
							for(int hc = 0; hc < 2; hc++) {
								pDC->MoveTo(h+xx+hc, 40);
								pDC->LineTo(h+xx+hc, 60);
							}
							pDC->SelectObject(open);
						}
					}
				}
			} else {
				int d = pbuf[xx+2] - pbuf[xx];
				if(d >= HTh && pbuf[xx] >= 7500) { // rising
					int idx;
					int qqcnt = 0;
					int pavg = pbuf[xx]+pbuf[xx-1]+pbuf[xx-2]+pbuf[xx-3]+pbuf[xx-4];
					pavg = pavg/5 + HTh;
					for( idx = 0; idx < 7; idx++)
						if( pbuf[xx+2+idx] >= pavg) qqcnt++;
					if(qqcnt >= 5) {
						int ppow = 0;
						for( idx = 0; idx < 5; idx++) ppow += (pbuf[xx-idx]-pavg)*(pbuf[xx-idx]-pavg);
						if(ppow <= 80000000) {
							Note->TDS.g_Tape_Channel = ci;
							Note->TDS.g_Tape_Pos = xx;
							Note->Result = S_CMD_ER_TDS;
							CPen pen;
							pen.CreatePen(PS_SOLID, 1, RGB(255,0,0));
							CPen *open = pDC->SelectObject(&pen);
							for(int hc = 0; hc < 3; hc++) {
								pDC->MoveTo(h+xx+hc, 20);
								pDC->LineTo(h+xx+hc, 40);
							}
							pDC->SelectObject(open);
							pDC->TextOut(h+xx+4, 20, L"R", 1);
						} else {
							CPen pen;
							pen.CreatePen(PS_SOLID, 1, RGB(0,255,255));
							CPen *open = pDC->SelectObject(&pen);
							for(int hc = 0; hc < 2; hc++) {
								pDC->MoveTo(h+xx+hc, 40);
								pDC->LineTo(h+xx+hc, 60);
							}
							pDC->SelectObject(open);
						}
					}
				}
			}
		}
	}

	size.cx = h;
	size.cy = 256;
}

void ST150Image::DrawUV(CDC* pDC, CSize& size)
{
	if(this->m_UVImageSize <= 0)
		return;

	ST_NOTE *Note = &OneNote;

	int h1 = 0; // 각 채널 그래프의 x좌표.

	int ci;

	if(pDC != NULL) {
		// 실제로 그린다.
		ClearRect(pDC, 0, 0, size.cx, size.cy);

	for(ci = 0; ci < UV_CH; ci++) {
		int head = 20;
		int count = Note->UV.ADCnt[ci];

		pDC->MoveTo(h1, 0+head); pDC->LineTo(h1, 258+head); pDC->LineTo(h1+count, 258+head); pDC->LineTo(h1+count, 0+head);
		DrawGraph2(pDC, Note->UV.Buf_F[ci], count, h1, 258, 1, 1);

		pDC->MoveTo(h1, 260); pDC->LineTo(h1, 260+255); pDC->LineTo(h1+count, 260+255); pDC->LineTo(h1+count, 260+0);
		DrawGraph2(pDC, Note->UV.Buf_R[ci], count, h1, 260+255, 1, 1);

		h1 += (count + 10);
	}
	} else {
		// 최종 이미지의 크기만 돌려준다.
		for(ci = 0; ci < UV_CH; ci++) {
			int count = Note->UV.ADCnt[ci];
			h1 += (count + 10);
		}
	}

	size.cx = h1;
	size.cy = 260+255;
}

int ST150Image::SaveToBitmap(TCHAR* bmpFileName)
{
	BITMAPINFOHEADER bi;

	//*------------------------- 비트멥 헤더를 기록함 -------------------------

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = m_ImageSize.cx;
	bi.biHeight = -m_ImageSize.cy;
	bi.biPlanes = 1;
	bi.biBitCount = 24;       
	bi.biCompression = BI_RGB;
	bi.biSizeImage = m_ImageSize.cx* m_ImageSize.cy* 3;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	BITMAPFILEHEADER hdr;

	hdr.bfType = ((WORD)('M' << 8) | 'B');        
	hdr.bfSize = bi.biSizeImage + sizeof(bi) + sizeof(hdr);   
	hdr.bfReserved1 = 0;                                 
	hdr.bfReserved2 = 0;                                 
	int nColors = 0; // Color Index 없음
	hdr.bfOffBits=(DWORD)(sizeof(hdr) + bi.biSize + nColors * sizeof(RGBQUAD));

	/*--------------------- 실제 파일에 기록함 --------------------------*/
	HANDLE pFile = NULL;

	pFile = CreateFile(bmpFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	bool bResult = false;
	if (pFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwio = 0;
	WriteFile(pFile, &hdr, sizeof(hdr), &dwio, NULL);
	WriteFile(pFile, &bi, sizeof(bi), &dwio, NULL);
	unsigned char *rowBuffer = (unsigned char*)malloc(bi.biWidth*3);
	int x, y;

	for(y = 0; y < m_WImageSize.cy; y++) {
		unsigned char* dp = rowBuffer;
		for(x = 0; x < m_WImageSize.cx; x++) {
			*dp++ = m_W_ImageBuffer[y][x];
			*dp++ = m_W_ImageBuffer[y][x];
			*dp++ = m_W_ImageBuffer[y][x];
		}
		WriteFile(pFile, rowBuffer, bi.biWidth*3, &dwio, NULL);
	}

	for(y = 0; y < m_WImageSize.cy; y++) {
		unsigned char* dp = rowBuffer;
		for(x = 0; x < m_WImageSize.cx; x++) {
			*dp++ = m_IR_ImageBuffer[y][x];
			*dp++ = m_IR_ImageBuffer[y][x];
			*dp++ = m_IR_ImageBuffer[y][x];
		}
		WriteFile(pFile, rowBuffer, bi.biWidth*3, &dwio, NULL);
	}
	free(rowBuffer);

	CloseHandle(pFile);
	return TRUE;
}

int ST150Image::TestMousePoint(CPoint& point)
{
	if(point.x < 0 || point.y < 0 || point.y >= this->m_ImageSize.cy)
		return 0;

	if(point.y > m_WImageSize.cy)
		point.y -= m_WImageSize.cy;

	return 1;
}

void ST150Image::GetImageValue(CPoint point, int *iv)
{
	iv[0] = this->m_W_ImageBuffer[point.y][point.x];
	iv[1] = this->m_IR_ImageBuffer[point.y][point.x];
}

int ST150Image::GetImageAverage(CPoint p1, CPoint p2, int *iv)
{
	if(p1.x > p2.x) { int temp = p1.x; p1.x = p2.x; p2.x = temp; }
	if(p1.y > p2.y) { int temp = p1.y; p1.y = p2.y; p2.y = temp; }

	if(p1.x < 0 || p2.x < 0 || p1.y > m_ImageSize.cy)
		return 0;
	
	if(p1.y < m_WImageSize.cy && p2.y > m_WImageSize.cy)
		return 0;

	if(p1.y > m_WImageSize.cy) p1.y -= m_WImageSize.cy;
	if(p2.y > m_WImageSize.cy) p2.y -= m_WImageSize.cy;

	int Wsum = 0;
	int IRsum = 0;
	for(int y = p1.y; y < p2.y; y++) {
		for(int x = p1.x; x < p2.x; x++) {
			Wsum += m_W_ImageBuffer[y][x];
			IRsum += m_IR_ImageBuffer[y][x];
		}
	}

	int size = (p2.x-p1.x)*(p2.y-p1.y);
	iv[0] = Wsum / size;
	iv[1] = IRsum / size;

	return 1;
}


CBitmap* ST150Image::ExtractRegion(CDC* pDC, ST_IMG_REGION* pRegion)
{
	int i;

	dMarkExtRegion = 1;
	AxisChange(&pRegion->Region, OneNote.Dir);
	ExtRegion(&OneNote.CIS, &gst_State.Rate_General, pRegion,
		g_iqRegionW_Buf[0], g_iqRegionIR_Buf[0], m_W_ImageBuffer, m_IR_ImageBuffer);
	dMarkExtRegion = 0;
	RegionSize = OneNote.CIS.Feature_X * OneNote.CIS.Feature_Y;
	if(RegionSize <= 0)
		return NULL;

/////////// Double을 체크
	int t_IR = 0;
	int t_W = 0;
	i = 0;
	for(i = 0; i < RegionSize; i++) {
		t_IR += g_iqRegionIR_Buf[0][i];
		t_W += g_iqRegionW_Buf[0][i];
	}

	OneNote.CIS.DDValue  = t_IR / RegionSize;
	OneNote.CIS.DDValueW = t_W / RegionSize;

	int width = OneNote.CIS.Feature_X * 2 + 5;
	int height = OneNote.CIS.Feature_Y;

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	CBitmap *bitmap = new CBitmap();
	bitmap->CreateCompatibleBitmap(pDC, width, height );

	CBitmap* oldBitmap = dc.SelectObject(bitmap);

	int x, y;

	for(y = 0; y < height; y++) {
		for(x = 0; x < OneNote.CIS.Feature_X; x++) {
			unsigned char g = g_iqRegionW_Buf[0][y*OneNote.CIS.Feature_X+x];
			COLORREF c = RGB(g, g, g);
			dc.SetPixel( x, y, c);
		}
	}

	for(y = 0; y < height; y++) {
		for(x = 0; x < 5; x++) {
			dc.SetPixel( OneNote.CIS.Feature_X+x, y, RGB(255,255,255));
		}
	}

	for(y = 0; y < height; y++) {
		for(x = 0; x < OneNote.CIS.Feature_X; x++) {
			unsigned char g = g_iqRegionIR_Buf[0][y*OneNote.CIS.Feature_X+x];
			COLORREF c = RGB(g, g, g);
			dc.SetPixel( OneNote.CIS.Feature_X+5+x, y, c);
		}
	}

	dc.SelectObject(oldBitmap);
	dc.DeleteDC();

	return bitmap;
}

CBitmap* ST150Image::MakeDebugImage(CDC* pDC)
{
	CDC dc;
	dc.CreateCompatibleDC(pDC);
	CBitmap *bitmap = new CBitmap();
	bitmap->CreateCompatibleBitmap(pDC, 40, 17);
	CBitmap *obitmap = dc.SelectObject(bitmap);
	for(int y = 0; y < 17; y++) {
		for(int x = 0; x < 40; x++) {
			unsigned char c = this->m_W_ImageBuffer[0][y * 40 + x];
			dc.SetPixel(x, y, RGB(c,c,c));
		}
	}
	dc.SelectObject(obitmap);
	dc.DeleteDC();

	return bitmap;
}

void ST150Image::ApplyPercent(int w_percent, int ir_percent)
{
}

void ST150Image::ApplyThreshold(int w_threshold, int ir_threshold)
{

}

CBitmap* ST150Image::MakeBitmapFromMG(CDC* pDC)
{
	// 이미지 크기 가져오기
	size = CSize(0,0);
	DrawMG(NULL, size);

	// 가져온 크기로 이미지 만들기
	CBitmap *bitmap = new CBitmap();
	bitmap->CreateCompatibleBitmap(pDC, size.cx, size.cy);

	// 이미지 위에 그리기
	CDC *dc = new CDC();
	dc->CreateCompatibleDC(pDC);
	CBitmap* obit = dc->SelectObject(bitmap);
	DrawMG(dc, size);
	// 전체 그리를 생성된 이미지 크기로 세팅
	dc->SelectObject(obit);
	dc->DeleteDC();
	delete dc;

	return bitmap;
}

CBitmap* ST150Image::MakeBitmapFromUV(CDC* pDC)
{
	// 이미지 크기 가져오기
	size = CSize(0,0);
	DrawUV(NULL, size);

	// 가져온 크기로 이미지 만들기
	CBitmap *bitmap = new CBitmap();
	bitmap->CreateCompatibleBitmap(pDC, size.cx, size.cy);

	// 이미지 위에 그리기
	CDC *dc = new CDC();
	dc->CreateCompatibleDC(pDC);
	CBitmap* obit = dc->SelectObject(bitmap);
	DrawUV(dc, size);
	dc->SelectObject(obit);
	dc->DeleteDC();
	delete dc;

	return bitmap;
}

void ST150Image::SetLaptop(int nLaptop)
{
	m_nLaptop = nLaptop;
}
