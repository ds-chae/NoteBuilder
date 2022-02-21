// WinDebugDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NoteBuilderFrame.h"
#include "WinDebugDlg.h"
#if ENABLE_IQMATH
#include "IQmath.h"
#endif

// WinDebugDlg dialog

IMPLEMENT_DYNAMIC(WinDebugDlg, CDialog)

WinDebugDlg::WinDebugDlg(CWnd* pParent /*=NULL*/)
	: CDialog(WinDebugDlg::IDD, pParent)
{

}

WinDebugDlg::~WinDebugDlg()
{
}

void WinDebugDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(WinDebugDlg, CDialog)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// WinDebugDlg message handlers

void WinDebugDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages
	CDC *pSrcDC = new CDC();
	pSrcDC->CreateCompatibleDC(&dc);
	CBitmap *oBitmap = pSrcDC->SelectObject(pBitmap);
	RECT rect;
	GetClientRect(&rect);
	dc.StretchBlt(rect.left, rect.top, rect.right, rect.bottom, pSrcDC, 0, 0, w, sh, SRCCOPY);
	pSrcDC->SelectObject(oBitmap);
	pSrcDC->DeleteDC();
	delete pSrcDC;
}

BOOL WinDebugDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	int tp = 0;

	memcpy(&clen, debugdata+0, 4);
	tp += 4;

	memcpy(caption, debugdata+tp, clen);
	tp += clen;

	memcpy(&t, debugdata+tp, 4);
	tp += 4;
	
	memcpy(&w, debugdata+tp, 4);
	tp += 4;
	
	memcpy(&h, debugdata+tp, 4);
	tp += 4;
	
	unsigned char* imgptr = debugdata + clen+4+12;

	CDC *pDC = GetDC();
	CDC *BitmapDC = new CDC();
	BitmapDC->CreateCompatibleDC(pDC);
	pBitmap = new CBitmap();
	sh = h;
	if(h <= 1)
		sh = 256;
	else
		sh = h;

	pBitmap->CreateCompatibleBitmap(pDC, w, sh);
	CBitmap* oBitmap = BitmapDC->SelectObject(pBitmap);

	if(h <= 1) {
		for(int x = 0; x < w; x++) {
			if(x == 0)
				BitmapDC->MoveTo(x, 256-imgptr[x]);
			else
				BitmapDC->LineTo(x, 256-imgptr[x]);
		}
	} else {
		for(int y = 0; y < h; y++) {
			for(int x = 0; x < w; x++) {
				unsigned char v;
				switch(t){
				case 1 :
					v = *imgptr;
					imgptr += 1;
					break;
				case 4 :
					v = *(int*)imgptr;
					imgptr += 4;
#if ENABLE_IQMATH
				case 3 :
					v = _IQ20int(*(int*)imgptr);
#endif
				}
				BitmapDC->SetPixel(x, y, RGB(v, v, v));
			}
		}
	}
	BitmapDC->SelectObject(oBitmap);
	BitmapDC->DeleteDC();
	delete BitmapDC;

	RECT wrect;
	RECT crect;
	GetWindowRect(&wrect);
	GetClientRect(&crect);
	
	crect.right = (wrect.right-wrect.left) - (crect.right-crect.left) + w;
	crect.bottom = (wrect.bottom-wrect.top) - (crect.bottom-crect.top) + sh;
	
	MoveWindow(wrect.left, wrect.top, crect.right, crect.bottom);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
