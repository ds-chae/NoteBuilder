#include "StdAfx.h"

#define GLOBAL_VALUE_INIT
#define GLOBAL_CIS_DEF
//#define GLOBAL_MG_DEF

#include "B_Common.h"
#include "json.h"

#include "ST150Image.h"
#include "CurrencyInfo.h"

#include "libs\tiff-v3.6.1\libtiff\tiffio.h"
#include "libs\jpeg-6b\jpeglib.h"

#pragma warning(disable:4996)

WIN_ST_LOCAL WinCurrentLocal;

WIN_ST_LOCAL *pWinCurrentLocal = &WinCurrentLocal;

// Only One Instance
//ST150Image imgST150;

int laptop2;

ST150Image::ST150Image(void)
: m_nLaptop(0)
{
	ClearSize();

	TapeDisplayGap = 1;
	m_bLoadedFromJSN = 1;
}

ST150Image::~ST150Image(void)
{

}

static int Bpp(cv::Mat img) { return 8 * img.channels(); }

void ST150Image::OnDraw(CDC* pDC)
{
	if(mat_image.empty()) return;

	int height = mat_image.rows;
	int width = mat_image.cols;
	uchar buffer[sizeof( BITMAPINFOHEADER ) + 1024]; 
	BITMAPINFO* bmi = (BITMAPINFO* )buffer; 
	FillBitmapInfo(bmi, width, height, Bpp(mat_image),0);
	SetDIBitsToDevice(pDC->GetSafeHdc(), 0, 0, width, height, 0, 0, 0, height, mat_image.data, bmi, DIB_RGB_COLORS);
}

void ST150Image::FillBitmapInfo(BITMAPINFO* bmi, int width, int height, int bpp, int origin) 
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

void ST150Image::ClearSize()
{
	memset(&this->extr, 0, sizeof(extr));
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

void FlipImageX(CSize& size, unsigned char buf[][IMG_BUF_X])
{
	for(int y = 0; y < size.cy; y++) {
		int left = 0;
		int right = size.cx-1;
		while(left < right) {
			unsigned char c = buf[y][left];
			buf[y][left] = buf[y][right];
			buf[y][right] = c;

			left++;
			right--;
		}
	}
}

void ST150Image::FlipImageHorizontal()
{
	cv::flip(mat_image, mat_image, 1);
}


void FlipImageY(CSize &size, unsigned char buf[][IMG_BUF_X])
{
	for(int x = 0; x < size.cx; x++) {
		int low = 0;
		int high = size.cy - 1;
		while(low < high) {
			unsigned char c = buf[low][x];
			buf[low][x] = buf[high][x];
			buf[high][x] = c;

			low++;
			high--;
		}
	}
}

void ST150Image::FlipImageVertical()
{
	cv::flip(mat_image, mat_image, 0);
}


float GetImgAverage(unsigned char img[][IMG_BUF_X], int cx, int cy)
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

float GetMinMaxFactor(unsigned char img[][IMG_BUF_X], int cx, int cy, float* nMinFactor, float* nMaxFactor)
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

void SetDIBitsWidthReinforce(int height, CSize& size, unsigned char buf[][IMG_BUF_X], HDC hDC, HBITMAP hbitmap, int offset, BITMAPINFO& bmi)
{
	unsigned char *rowBuffer = (unsigned char*)malloc(bmi.bmiHeader.biWidth*3);
	int x, y;

	float nMaxFactor, nMinFactor;
	float nAverage = GetMinMaxFactor(buf, size.cx, size.cy, &nMinFactor, &nMaxFactor);

	for(y = 0; y < size.cy; y++) {
		unsigned char* dp = rowBuffer;
		for(x = 0; x < size.cx; x++) {
			unsigned short c = buf[y][x];
			float tmp = (float)c - nAverage;
			if(tmp < (float)0) c = (tmp*nMinFactor) + 127;
			else c = (tmp * nMaxFactor) + 127;

			*dp++ = c;
			*dp++ = c;
			*dp++ = c;
		}
		::SetDIBits(hDC, hbitmap, height - 1 - y - offset, 1, rowBuffer, &bmi, DIB_RGB_COLORS);
	}

	free(rowBuffer);
}

void SetDIBitsWidthRawImage(int height, CSize& size, unsigned char buf[][IMG_BUF_X], HDC hDC, HBITMAP hbitmap, int offset, BITMAPINFO& bmi, int w_threshold)
{
	unsigned char *rowBuffer = (unsigned char*)malloc(bmi.bmiHeader.biWidth*3);
	int x, y;

	for(y = 0; y < size.cy; y++) {
		unsigned char* dp = rowBuffer;
		for(x = 0; x < size.cx; x++) {
			unsigned char c = buf[y][x];
			if(c < w_threshold) c = 0;
			*dp++ = c;
			*dp++ = c;
			*dp++ = c;
		}
		::SetDIBits(hDC, hbitmap, height - 1 - y - offset, 1, rowBuffer, &bmi, DIB_RGB_COLORS);
	}

	free(rowBuffer);
}

/*
CBitmap* ST150Image::MakeBitmapFromRaw(CDC *pDC, int w_threshold, int ir_threshold, int ImgReinforce)
{
	CBitmap *bitmap = new CBitmap();
	bitmap->CreateCompatibleBitmap(pDC, mat_image.cols, mat_image.rows);

	BITMAP bm;
	bitmap->GetBitmap(&bm);

#if 1
	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = mat_image.cols;
	bmi.bmiHeader.biHeight = mat_image.rows;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;       
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = mat_image.rows * (mat_image.cols * 3);
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;

	HDC hdc = pDC->GetSafeHdc();
	HBITMAP hbitmap = (HBITMAP)bitmap->GetSafeHandle();
	if(ImgReinforce) {
		SetDIBitsWidthReinforce(m_ImageSize.cy, m_W_ImageSize, m_W_ImageBuffer, hdc, hbitmap, 0, bmi);
#if CURRENCY_IMAGE
		SetDIBitsWidthReinforce(m_ImageSize.cy, m_IR_ImageSize, m_IR_ImageBuffer, hdc, hbitmap, m_W_ImageSize.cy, bmi);
#endif
//		SetDIBitsWidthReinforce(m_ImageSize.cy, m_Y_ImageSize, m_Y_ImageBuffer, hdc, hbitmap, m_W_ImageSize.cy + m_IR_ImageSize.cy, bmi);
	} else {
		SetDIBitsWidthRawImage(m_ImageSize.cy, m_W_ImageSize, m_W_ImageBuffer, hdc, hbitmap, 0, bmi, w_threshold);
#if CURRENCY_IMAGE
		SetDIBitsWidthRawImage(m_ImageSize.cy, m_IR_ImageSize, m_IR_ImageBuffer, hdc, hbitmap, m_W_ImageSize.cy, bmi, ir_threshold);
#endif
//		SetDIBitsWidthRawImage(m_ImageSize.cy, m_Y_ImageSize, m_Y_ImageBuffer, hdc, hbitmap, m_W_ImageSize.cy + m_IR_ImageSize.cy, bmi, w_threshold);
	}
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
*/

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

int StrCaseCmp(TCHAR* s1, TCHAR* s2)
{
	while(*s1 && *s2) {
		if(toupper(*s1) != toupper(*s2))
			return toupper(*s1) - toupper(*s2);
		s1++;
		s2++;
	}
	return toupper(*s1) - toupper(*s2);
}

int IsTifFile(TCHAR* pathname)
{
	if(lstrcmp(pathname+lstrlen(pathname)-4, L".tif") == 0)
		return 1;
	
	return 0;
}

static BOOL FileExist(TCHAR* path, WIN32_FIND_DATA* pwfd)
{
	HANDLE h = FindFirstFile(path, pwfd);
	if(h != INVALID_HANDLE_VALUE) {
		FindClose(h);
		return TRUE;
	}

	return FALSE;
}

void GetWidthHeight(LPCTSTR str, CSize* size)
{
	CString s = str;

	size->SetSize(0,0);

	TCHAR pathname[MAX_PATH];

	lstrcpy(pathname, (LPCTSTR)s);
	TCHAR *fp = pathname;
	while(*fp) fp++; fp -= 4;
	while(fp != pathname) {
		fp--;
		if(*fp < _T('0') || *fp > _T('9')) {
			size->cy = GetDigitNumber(fp+1);
			break;
		}
	}

	while(fp != pathname) {
		fp--;
		if(*fp < _T('0') || *fp > _T('9')) {
			size->cx = GetDigitNumber(fp+1);
			break;
		}
	}
}

#if CURRENCY_IMAGE
int LoadSimilarFile(int Pos, TCHAR* orgpath, TCHAR type, CSize* size, unsigned char buf[][1728])
{
	TCHAR openpath[MAX_PATH];
	WIN32_FIND_DATA wfd;

	lstrcpy(openpath, orgpath);
	openpath[Pos+3] = type;
	if(!FileExist(openpath, &wfd)) {
		TCHAR srchpath[MAX_PATH];
		lstrcpy(srchpath, openpath);
		srchpath[Pos+14] = 0; // remove trailing ###.tif
		if(IsTifFile(orgpath))
			lstrcat(srchpath, L"*.tif");
		else
			lstrcat(srchpath, L"*.raw");

		HANDLE hFind = FindFirstFile(srchpath, &wfd);
		if(hFind != INVALID_HANDLE_VALUE) {
			openpath[Pos] = 0;
			lstrcat(openpath, wfd.cFileName);
			FindClose(hFind);
		} else {
			openpath[0] = 0;
		}
	}
	
	if(openpath[0] == 0)
		return 0;
	
	DWORD dwio = 0;
	
	if(IsTifFile(openpath)) {
		char szpathname[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, openpath, lstrlen(openpath)+1, szpathname, MAX_PATH, NULL, NULL);
		TIFF* tif = TIFFOpen(szpathname, "r");
		if(tif) {
	        uint32 w, h;
		    size_t npixels;
        
			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

			size->cy = h;
			size->cx = w;

			int size = TIFFScanlineSize(tif);
			for (int row = 0; row < h; row++)
				TIFFReadScanline(tif, buf[row], row);
			TIFFClose(tif);
		} else {
			size->cx = 0;
			size->cy = 0;
		}
	} else {
		// Load RAW Image
		HANDLE h = 	CreateFile(openpath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h != INVALID_HANDLE_VALUE) {
			GetWidthHeight(openpath, size);
			int y;
			for( y = 0; y < size->cy; y++) {
				if(!ReadFile(h, buf[y], size->cx, &dwio, NULL))
					break;
				if(size->cx != dwio)
					break;
			}
			CloseHandle(h);
			size->cy = y;
		}
	}

	return size->cy;
}
#endif

#if JPEG
	/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* libjpeg data structure for storing one row, that is, scanline of an image */
	JSAMPROW row_pointer[1];

	char filename[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, NULL, lstrlen(w_pathname)+1, filename, MAX_PATH, NULL, NULL);
	FILE *infile = fopen( filename, "rb" );
	unsigned long location = 0;
 int i = 0;

 if ( !infile )
 {
  printf("Error opening jpeg file %s\n!", filename );
  return -1;
 }
 /* here we set up the standard libjpeg error handler */
 cinfo.err = jpeg_std_error( &jerr );
 /* setup decompression process and source, then read JPEG header */
 jpeg_create_decompress( &cinfo );
 /* this makes the library read from infile */
 jpeg_stdio_src( &cinfo, infile );
 /* reading the image header which contains image information */
 jpeg_read_header( &cinfo, TRUE );
 /* Uncomment the following to output image information, if needed. */

 int width = 0;
 int height = 0;
 int bytes_per_pixel = 0;

 printf( "JPEG File Information: \n" );
 printf( "Image width and height: %d pixels and %d pixels.\n", width=cinfo.image_width, height=cinfo.image_height );
 printf( "Color components per pixel: %d.\n", bytes_per_pixel = cinfo.num_components );
 printf( "Color space: %d.\n", cinfo.jpeg_color_space );

 /* Start decompression jpeg here */
 jpeg_start_decompress( &cinfo );

  /* allocate memory to hold the uncompressed image */
	unsigned char* raw_image = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
 /* now actually read the jpeg into the raw buffer */
 row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );
 /* read one scan line at a time */
 while( cinfo.output_scanline < cinfo.image_height )
 {
  jpeg_read_scanlines( &cinfo, row_pointer, 1 );
  for( i=0; i<cinfo.image_width*cinfo.num_components;i++) 
   raw_image[location++] = row_pointer[0][i];
 }
 /* wrap up decompression, destroy objects, free pointers and close open files */
 jpeg_finish_decompress( &cinfo );
 jpeg_destroy_decompress( &cinfo );
 free( row_pointer[0] );
 fclose( infile );
 /* yup, we succeeded! */
#endif

 unsigned char tmpbuf[IMG_MAX_Y/2][1800];

int ST150Image::LoadImageFile(LPCTSTR cszPathName)
{
	m_DocumentType = DOC_TYPE_IMAGE;

	char szbuf[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, cszPathName, lstrlen(cszPathName)+1, szbuf, MAX_PATH, NULL, NULL);
	std::string stdstring = szbuf;
    mat_image = cv::imread(stdstring, CV_LOAD_IMAGE_COLOR);   // Read the file

    if(! mat_image.data )                              // Check for invalid input
    {
        MessageBox(NULL, L"Could not open or find the image", cszPathName, MB_OK);
        return -1;
    }

	return 1;
}

void ST150Image::ClearMem()
{
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

void MakeJsonPath(TCHAR* jsnpath, TCHAR* src)
{
	lstrcpy(jsnpath, src);
	int fnlen = lstrlen(jsnpath);
	if(StrCaseCmp(jsnpath+fnlen-5, L"B.raw") == 0) {
		lstrcpy(jsnpath+fnlen-5, L".jsn");
	} else {
		lstrcpy(jsnpath+fnlen-4, L".jsn");
	}
}

int ST150Image::SaveNoteInfo()
{
//	if(!this->m_bLoadedFromJSN)
//		return 0;
	
	TCHAR jsnpath[MAX_PATH];

	MakeJsonPath(jsnpath, this->m_Filename);

	Json::Value root;
	PutToJson(root);
	Json::StyledWriter writer;
	writer.save( jsnpath, root);

	return 1;
}

void ST150Image::PutToJson(Json::Value &root)
{
	root["extr"]["sx"] = extr.Region.Sx;
	root["extr"]["ex"] = extr.Region.Ex;
	root["extr"]["sy"] = extr.Region.Sy;
	root["extr"]["ey"] = extr.Region.Ey;
	root["extr"]["sfx"] = extr.Shift_X;
	root["extr"]["sfy"] = extr.Shift_Y;
	root["extr"]["smx"] = extr.Sum_X;
	root["extr"]["smy"] = extr.Sum_Y;
}

void ST150Image::GetFromJson(Json::Value &root)
{
	extr.Region.Sx = root["extr"].get("sx", 0).asInt();
	extr.Region.Ex = root["extr"].get("ex", 0).asInt();
	extr.Region.Sy = root["extr"].get("sy", 0).asInt();
	extr.Region.Ey = root["extr"].get("ey", 0).asInt();
	extr.Shift_X = root["extr"].get("sfx", 0).asInt();
	extr.Shift_Y = root["extr"].get("sfy", 0).asInt();
	extr.Sum_X = root["extr"].get("smx", 0).asInt();
	extr.Sum_Y = root["extr"].get("smy", 0).asInt();
}

int ST150Image::LoadNoteInfo()
{
	TCHAR jsnpath[MAX_PATH];

	MakeJsonPath(jsnpath, this->m_Filename);

	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	if(!reader.load(jsnpath, root)) {
		return 0;
	}

	GetFromJson(root);
	m_bLoadedFromJSN = 1;
	return 1;
/*
	// Get the value of the member of root named 'encoding', return 'UTF-8' if there is no
	// such member.
	std::string encoding = root.get("encoding", "UTF-8" ).asString();

	// Get the value of the member of root named 'encoding', return a 'null' value if
	// there is no such member.
	const Json::Value plugins = root["plug-ins"];
	for ( int index = 0; index < plugins.size(); ++index )  // Iterates over the sequence elements.
		loadPlugIn( plugins[index].asString() );
   
	setIndentLength( root["indent"].get("length", 3).asInt() );
	setIndentUseSpace( root["indent"].get("use_space", true).asBool() );

	// ...
	// At application shutdown to make the new configuration document:
	// Since Json::Value has implicit constructor for all value types, it is not
	// necessary to explicitly construct the Json::Value object:
	root["encoding"] = getCurrentEncoding();
	root["indent"]["length"] = getCurrentIndentLength();
	root["indent"]["use_space"] = getCurrentIndentUseSpace();

	Json::StyledWriter writer;
	// Make a new JSON document for the configuration. Preserve original comments.
	std::string outputConfig = writer.write( root );
*/
}

int ST150Image::LoadFromFile(LPCTSTR tszFileName)
{
	ClearSize();

	lstrcpy(m_Filename, tszFileName);

	CString s = tszFileName;

	if(s.Right(4).CompareNoCase(L".jpg") == 0 || s.Right(5).CompareNoCase(L".jpeg") == 0 ||
		s.Right(4).CompareNoCase(L".bmp") == 0) {
		int ret = LoadImageFile(tszFileName);
		LoadNoteInfo();
		return ret;
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
/*
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

	//*--------------------- 실제 파일에 기록함 --------------------------
	HANDLE pFile = NULL;

	pFile = CreateFile(bmpFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	bool bResult = false;
	if (pFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwio = 0;
	WriteFile(pFile, &hdr, sizeof(hdr), &dwio, NULL);
	WriteFile(pFile, &bi, sizeof(bi), &dwio, NULL);

	WriteRawToBMP(pFile, m_W_ImageSize, m_W_ImageBuffer, bi.biWidth*3);
	WriteRawToBMP(pFile, m_IR_ImageSize, m_IR_ImageBuffer, bi.biWidth*3);
	WriteRawToBMP(pFile, m_Y_ImageSize, m_Y_ImageBuffer, bi.biWidth*3);

	CloseHandle(pFile);
	return TRUE;
}
*/

int ST150Image::TestMousePoint(CPoint& point)
{
	if(point.x < 0 || point.y < 0 || point.y >= mat_image.rows)
		return 0;

	return 1;
}

void ST150Image::GetImageValue(CPoint point, int *iv)
{
	iv[0] = this->m_W_ImageBuffer[point.y][point.x];
#if CURRENCY_IMAGE
	iv[1] = this->m_IR_ImageBuffer[point.y][point.x];
#endif
}

int ST150Image::GetImageAverage(CPoint p1, CPoint p2, int *iv)
{
/*
	if(p1.x > p2.x) { int temp = p1.x; p1.x = p2.x; p2.x = temp; }
	if(p1.y > p2.y) { int temp = p1.y; p1.y = p2.y; p2.y = temp; }

	if(p1.x < 0 || p2.x < 0 || p1.y > m_ImageSize.cy)
		return 0;
	
	if(p1.y < m_W_ImageSize.cy && p2.y > m_W_ImageSize.cy)
		return 0;

	if(p1.y > m_W_ImageSize.cy) p1.y -= m_W_ImageSize.cy;
	if(p2.y > m_W_ImageSize.cy) p2.y -= m_W_ImageSize.cy;

	int Wsum = 0;
	int IRsum = 0;
	for(int y = p1.y; y < p2.y; y++) {
		for(int x = p1.x; x < p2.x; x++) {
			Wsum += m_W_ImageBuffer[y][x];
#if CURRENCY_IMAGE
			IRsum += m_IR_ImageBuffer[y][x];
#endif
		}
	}

	int size = (p2.x-p1.x)*(p2.y-p1.y);
	iv[0] = Wsum / size;
	iv[1] = IRsum / size;
*/
	return 1;
}


CBitmap* ST150Image::ExtractRegion(CDC* pDC, ST_IMG_REGION* pRegion)
{
	int i;
#if CURRENCY
	dMarkExtRegion = 1;
	AxisChange(&pRegion->Region, OneNote.Dir);
	ExtRegion(&OneNote.IAR_A, &gst_State.Rate_General, pRegion, g_iqRegionW_Buf[0], m_W_ImageBuffer);
	ExtRegion(&OneNote.IAR_B, &gst_State.Rate_General, pRegion, g_iqRegionIR_Buf[0], m_IR_ImageBuffer);
	dMarkExtRegion = 0;
#endif
	RegionSize = OneNote.IAR_A.Feature_X * OneNote.IAR_A.Feature_Y;
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

	int width = OneNote.IAR_A.Feature_X * 2 + 5;
	int height = OneNote.IAR_A.Feature_Y;

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	CBitmap *bitmap = new CBitmap();
	bitmap->CreateCompatibleBitmap(pDC, width, height );

	CBitmap* oldBitmap = dc.SelectObject(bitmap);

	int x, y;

	for(y = 0; y < height; y++) {
		for(x = 0; x < OneNote.IAR_A.Feature_X; x++) {
			unsigned char g = g_iqRegionW_Buf[0][y*OneNote.IAR_A.Feature_X+x];
			COLORREF c = RGB(g, g, g);
			dc.SetPixel( x, y, c);
		}
	}

	for(y = 0; y < height; y++) {
		for(x = 0; x < 5; x++) {
			dc.SetPixel( OneNote.IAR_A.Feature_X+x, y, RGB(255,255,255));
		}
	}

	for(y = 0; y < height; y++) {
		for(x = 0; x < OneNote.IAR_A.Feature_X; x++) {
			unsigned char g = g_iqRegionIR_Buf[0][y*OneNote.IAR_A.Feature_X+x];
			COLORREF c = RGB(g, g, g);
			dc.SetPixel( OneNote.IAR_A.Feature_X+5+x, y, c);
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

void ST150Image::SetLaptop(int nLaptop)
{
	m_nLaptop = nLaptop;
}

