#include "StdAfx.h"

#include "B_Common.h"

#include "ST150Image.h"
#include "CurrencyInfo.h"

#include "libs\tiff-v3.6.1\libtiff\tiffio.h"

#pragma warning(disable:4996)

extern WIN_ST_LOCAL *pWinCurrentLocal;

extern "C" int dMarkExtRegion;

long t_Wbuf[2][2000];
long t_IRbuf[2][2000];

CBitmap* MakeRegionImage(CWnd* pWnd, NOTE_STRUCT* pNote, long* wbuf)
{
	int width = pNote->IAR_A.Feature_X;
	int height = pNote->IAR_A.Feature_Y;

	CDC *pDC = pWnd->GetDC();

	CBitmap *bitmap = new CBitmap();
	bitmap->CreateCompatibleBitmap(pDC, width, height );

	BITMAP bm;
	bitmap->GetBitmap(&bm);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	pWnd->ReleaseDC(pDC);

	CBitmap* oldBitmap = dc.SelectObject(bitmap);

	int x, y;
//	for(x = 0; x < bm.bmWidth; x++) {
//		for(y = 0; y < bm.bmHeight; y++) dc.SetPixel(x, y, RGB(255,255,255));
//	}

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			unsigned char g = (unsigned char)g_iqRegionW_Buf[0][y*width+x];
			COLORREF c = RGB(g, g, g);
			dc.SetPixel( x, y, c);
		}
	}

	dc.SelectObject(oldBitmap);
	dc.DeleteDC();


	return bitmap;
}

//20121029 sapa82 V3에서 region 영역 보이게 하는 함수. 기존거랑 파라메터가 하나 달라져서(Long -> Uin8) 오버로딩 했음.
CBitmap* MakeRegionImage(CWnd* pWnd, NOTE_STRUCT *pNote, Uint8* wbuf)
{
	int width = pNote->IAR_A.Feature_X;
	int height = pNote->IAR_A.Feature_Y;

	CDC *pDC = pWnd->GetDC();

	CBitmap *bitmap = new CBitmap();
	bitmap->CreateCompatibleBitmap(pDC, width, height );

	BITMAP bm;
	bitmap->GetBitmap(&bm);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	pWnd->ReleaseDC(pDC);

	CBitmap* oldBitmap = dc.SelectObject(bitmap);

	int x, y;
//	for(x = 0; x < bm.bmWidth; x++) {
//		for(y = 0; y < bm.bmHeight; y++) dc.SetPixel(x, y, RGB(255,255,255));
//	}

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			unsigned char g = (unsigned char)g_iqRegionW_Buf[0][y*width+x];
			COLORREF c = RGB(g, g, g);
			dc.SetPixel( x, y, c);
		}
	}

	dc.SelectObject(oldBitmap);
	dc.DeleteDC();


	return bitmap;
}


/////////// Double 체크용 평균값 구하기
void GetImageAverageForDouble(ST150Image* pImg, Uint16 RegionSize)
{
	int i;

	pImg->ExtMinW = pImg->ExtMaxW = g_iqRegionW_Buf[0][0];
	pImg->ExtMinIR = pImg->ExtMaxIR = g_iqRegionIR_Buf[0][0];

	pImg->ExtVarW = pImg->ExtVarIR = 0; // 이미지의 분산을 계산하기 위해 초기화 한다.

	for(i = 1; i < RegionSize; i++) {
		if(pImg->ExtMinIR > g_iqRegionIR_Buf[0][i]) pImg->ExtMinIR = g_iqRegionIR_Buf[0][i];
		if(pImg->ExtMaxIR < g_iqRegionIR_Buf[0][i]) pImg->ExtMaxIR = g_iqRegionIR_Buf[0][i];

		if(pImg->ExtMinW > g_iqRegionW_Buf[0][i]) pImg->ExtMinW = g_iqRegionW_Buf[0][i];
		if(pImg->ExtMaxW < g_iqRegionW_Buf[0][i]) pImg->ExtMaxW = g_iqRegionW_Buf[0][i];

		int d_ir = g_iqRegionIR_Buf[0][i] -  pImg->OneNote.IAR_A.DDValue;
		pImg->ExtVarIR += d_ir * d_ir;
		int d_w = g_iqRegionW_Buf[0][i] -  pImg->OneNote.IAR_A.DDValueW;
		pImg->ExtVarW += d_w * d_w;
	}

#ifdef _DEBUG
#ifndef _WINDOWS
	sprintf(debugString, "IR=%d, W=%d\r\n", (int)t_IR, (int)t_W);
	debugStrExist = 1;

	for(j = 0; j < RegionSize; j++){
		g_RegionW_Buf[RegionNumber][j]  = (Uint8)g_iqRegionW_Buf[0][j];
	}
#endif
#endif
}
