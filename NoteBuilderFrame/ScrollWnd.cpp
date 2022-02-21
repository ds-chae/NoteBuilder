// Filename: ScrollWnd.cpp
// 2005-07-02 nschan Initial revision.
// 2005-09-08 nschan Use memory DC drawing to eliminate flickering on resize.

#include "stdafx.h"

using namespace cv;

#include "ScrollWnd.h"
#include "ScrollHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int _sc_start_x;
int _sc_start_y;
int _sc_end_x;
int _sc_end_y;
int _sc_old_x;
int _sc_old_y;
Mat _sc_mat;

BEGIN_MESSAGE_MAP(CScrollWnd, CWnd)
    //{{AFX_MSG_MAP(CScrollWnd)
    ON_WM_CREATE()
    ON_WM_ERASEBKGND()
    ON_WM_MOUSEACTIVATE()
    ON_WM_PAINT()
    ON_WM_HSCROLL()
    ON_WM_VSCROLL()
    ON_WM_MOUSEWHEEL()
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

CScrollWnd::CScrollWnd(CWnd* parentWnd)
{
    // Create the scroll helper class and attach to this wnd.
    m_scrollHelper = new CScrollHelper;
    m_scrollHelper->AttachWnd(this);

    // Create the GUI window. Notice we specify the styles WS_HSCROLL
    // and WS_VSCROLL. These are needed to enable the horizontal and
    // vertical scrollbars for this window.
//    Create(NULL, L"CScrollWnd", WS_OVERLAPPED | WS_VISIBLE |WS_HSCROLL | WS_VSCROLL | WS_POPUP, CRect(0,1280,0,960), parentWnd, 0, NULL);
//	CreateEx(0, NULL, L"CScrollWnd", WS_CHILD | WS_OVERLAPPED | WS_VISIBLE |WS_HSCROLL | WS_VSCROLL | WS_POPUP,
//		0, 0, 1280, 960, parentWnd->GetSafeHwnd(), (HMENU)(UINT_PTR)0, (LPVOID)NULL);
	CString strClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW, 0, (HBRUSH)(COLOR_WINDOW + 1));
	CreateEx(0, strClass, L"CScrollWnd", /*WS_CHILD | */WS_OVERLAPPEDWINDOW | WS_VISIBLE |WS_HSCROLL | WS_VSCROLL | WS_POPUP | WS_CAPTION,
		0, 0, 1280, 960, 
		parentWnd != NULL ? parentWnd->GetSafeHwnd() : NULL,
		(HMENU)(UINT_PTR)0, (LPVOID)NULL);
//m_pWndFrame->CreateEx( WS_EX_TOOLWINDOW|WS_EX_TOPMOST, strClass, _T("1234"), WS_POPUP, 5, 5, 500, 100, m_hWnd, NULL, NULL );
	m_bDown = 0;
	putLinesCnt = 0;
	XorRect = Rect(0,0,0,0);
	this->pEdgeDetector = NULL;
	edge_to_mark = -1;
}

CScrollWnd::~CScrollWnd()
{
    delete m_scrollHelper;
}

CScrollWnd *_ScrollWnd[20];
int _ScrollWndCnt = 0;

void CScrollWnd::PostNcDestroy()
{
    m_scrollHelper->DetachWnd();

	for(int i = 0; i < _ScrollWndCnt; i++) {
		if(_ScrollWnd[i] == this) {
			_ScrollWnd[i] = NULL;
		}
	}
    // Delete the C++ instance so the parent does not have
    // to worry about it.
    delete this;
}

int CScrollWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if ( CWnd::OnCreate(lpCreateStruct) == -1 )
        return -1;

    return 0;
}

BOOL CScrollWnd::OnEraseBkgnd(CDC* pDC)
{
    // Return TRUE to indicate further erasing is not needed.
    return TRUE;
}

int CScrollWnd::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
    int status = CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);

    // We handle this message so that when user clicks once in the
    // window, it will be given the focus, and this will allow
    // mousewheel messages to be directed to this window.
    SetFocus();

    return status;
}

static int Bpp(cv::Mat img) { return 8 * img.channels(); }

static void FillBitmapInfo(BITMAPINFO* bmi, int width, int height, int bpp, int origin) 
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

void CScrollWnd::OnPaint()
{
    CPaintDC dc(this);

    // Get the client rect.
    CRect rect;
    GetClientRect(&rect);

    // Prepare for memory DC drawing.
    CDC memoryDC;
    if ( rect.Width() <= 0 || rect.Height() <= 0)
		return;

	if(mat_src.empty()) {
		if(!memoryDC.CreateCompatibleDC(&dc) )
			return;

		CBitmap bitmap;
		if ( bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height()) )
		{
			CDC* pDC = &memoryDC;

			// Select bitmap into memory DC.
			CBitmap* pOldBitmap = pDC->SelectObject(&bitmap);

			// Set up the memory DC for drawing graphics and text.
			CPen pen(PS_SOLID, 1, RGB(0,0,255));
			CPen* pOldPen = pDC->SelectObject(&pen);
			CBrush* pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
			COLORREF oldTextColor = pDC->SetTextColor(RGB(0,0,255));
			int oldBkMode = pDC->SetBkMode(TRANSPARENT);

			// Fill the background.
			pDC->FillSolidRect(&rect, RGB(0,255,0));

			// Draw a rectangle representing the display size.
			// The rectangle needs to be offset due to scrolling position.
			CRect dispRect;
			dispRect.SetRect(0, 0, m_scrollHelper->GetDisplaySize().cx, m_scrollHelper->GetDisplaySize().cy);
			dispRect.OffsetRect(-m_scrollHelper->GetScrollPos().cx, -m_scrollHelper->GetScrollPos().cy);
			pDC->Rectangle(&dispRect);

			// Draw text information.
			DrawScrollInfo(pDC);

			// Do the bitblt.
			dc.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);

			// Restore DC state.
			pDC->SetBkMode(oldBkMode);
			pDC->SetTextColor(oldTextColor);
			pDC->SelectObject(pOldBrush);
			pDC->SelectObject(pOldPen);
			pDC->SelectObject(pOldBitmap);
		}
	} else {
		int height = mat_src.rows;
		int width = mat_src.cols;
		uchar buffer[sizeof( BITMAPINFOHEADER ) + 1024]; 
		BITMAPINFO* bmi = (BITMAPINFO* )buffer; 
		FillBitmapInfo(bmi, width, height, Bpp(mat_src),0);
		CPoint op = dc.GetViewportOrg();
		dc.SetViewportOrg(-m_scrollHelper->GetScrollPos().cx, -m_scrollHelper->GetScrollPos().cy);
		int row_size = mat_src.step[0];
		if((row_size % 4) == 0) {
			SetDIBitsToDevice(dc.GetSafeHdc(), 0, 0, width, height, 0, 0, 0, height, mat_src.data, bmi, DIB_RGB_COLORS);
		} else {
			for(int r = 0; r < height; r++) {
				SetDIBitsToDevice(dc.GetSafeHdc(), 0, r, width, 1, 0, r, r, 1, mat_src.data + r * row_size, bmi, DIB_RGB_COLORS);
			}
		}
		dc.SetViewportOrg(op);
	}

	DrawScrollInfo(&dc);

	for(int i = 0; i < putLinesCnt; i++) {
		if(putLinesTxt[i][0]) {
			dc.TextOut(putLinesPos[i].x, putLinesPos[i].y, putLinesTxt[i]);
		}
	}

	if(XorRect.width > 0 && XorRect.height > 0) {
		//    Saving the original object
		HGDIOBJ original = dc.SelectObject(GetStockObject(DC_PEN));

		//    Rectangle function is defined as...
		//    BOOL Rectangle(hdc, xLeft, yTop, yRight, yBottom);
    
		//    Set the Pen to Blue
		dc.SetDCPenColor(RGB(0,0,255));
        
		//    Drawing a rectangle with the current Device Context    
		dc.MoveTo(XorRect.x, XorRect.y);
		dc.LineTo(XorRect.x + XorRect.width, XorRect.y);
		dc.LineTo(XorRect.x + XorRect.width, XorRect.y + XorRect.height);
		dc.LineTo(XorRect.x, XorRect.y + XorRect.height);
		dc.LineTo(XorRect.x, XorRect.y);
	}

	if(pEdgeDetector != NULL) {
		DrawTestedEdges((void*)pEdgeDetector, &dc);
		if(edge_to_mark != -1)
			DrawMarkedEdge((void*)pEdgeDetector, &dc, edge_to_mark);
	}
}

void CScrollWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    m_scrollHelper->OnHScroll(nSBCode, nPos, pScrollBar);
    CString text;
//    text.Format(L"TestScroll (ScrollPos: %d, %d)", m_scrollHelper->GetScrollPos().cx, m_scrollHelper->GetScrollPos().cy);
//	SetWindowText(text);
}

void CScrollWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    m_scrollHelper->OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CScrollWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    BOOL wasScrolled = m_scrollHelper->OnMouseWheel(nFlags, zDelta, pt);
    return wasScrolled;
}

void CScrollWnd::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    CRect rect;
    GetWindowRect(&rect);

    // Initialization: Set the display size if needed.
    if ( m_scrollHelper->GetDisplaySize() == CSize(0,0) )
    {
        if ( rect.Width() > 0 && rect.Height() > 0 )
            m_scrollHelper->SetDisplaySize(rect.Width(), rect.Height());
    }

    m_scrollHelper->OnSize(nType, cx, cy);
}

void CScrollWnd::DrawScrollInfo(CDC* pDC)
{
    int x = 3, y = 2;

    // Offset starting position due to scrolling.
    x -= m_scrollHelper->GetScrollPos().cx;
    y -= m_scrollHelper->GetScrollPos().cy;
    CString text;

if(0) { // 이 부분 출력은 잠시 막아 두자
    // Draw the page size.
    text.Format(L"PageSize: %d x %d", m_scrollHelper->GetPageSize().cx, m_scrollHelper->GetPageSize().cy);
    pDC->TextOut(x, y, text);

    // Draw the display size.
    y += 16;
    text.Format(L"DisplaySize: %d x %d", m_scrollHelper->GetDisplaySize().cx, m_scrollHelper->GetDisplaySize().cy);
    pDC->TextOut(x, y, text);

    // Draw the GetClientRect() value.
    CRect rect;
    GetClientRect(&rect);
    y += 16;
    text.Format(L"ClientRect(x,y,cx,cy): %d, %d, %d, %d", rect.left, rect.top, rect.Width(), rect.Height());
    pDC->TextOut(x, y, text);

}
    // Change the caption text of the MDI child frame window to show
    // the current scroll position.
    text.Format(L"%s (ScrollPos: %d, %d)", szWndName, m_scrollHelper->GetScrollPos().cx, m_scrollHelper->GetScrollPos().cy);
    CWnd* pWnd = GetParentFrame();
    if ( pWnd != NULL )
    {
//        pWnd->SetWindowText(text);
    }
	SetWindowText(text);
}

void CScrollWnd::matShow(Mat& mat_img)
{
	RECT wrect, crect;
	GetWindowRect(&wrect);
	GetClientRect(&crect);
	int xdif = 0;
	int ydif = 0;
	if(mat_img.cols > 1600)
		xdif = 1600 - (crect.right-crect.left);
	else
		xdif = mat_img.cols - (crect.right-crect.left);
	if(mat_img.rows > 600)
		ydif = 600 - (crect.bottom-crect.top);
	else
		ydif = mat_img.rows - (crect.bottom-crect.top);
	if(xdif != 0 || ydif != 0) {
		wrect.right += xdif;
		wrect.bottom += ydif;
		SetWindowPos(NULL, wrect.left, wrect.top, wrect.right-wrect.left, wrect.bottom-wrect.top, SWP_NOMOVE);
	}

	mat_src = mat_img;
	m_scrollHelper->SetDisplaySize(mat_src.cols, mat_src.rows);
	Invalidate();
}

CScrollWnd *CreateScrollWnd(TCHAR* name)
{
	int empty_slot = -1;
	for(int i = 0; i < _ScrollWndCnt; i++) {
		if(_ScrollWnd[i] == NULL) {
			empty_slot = i;
			break;
		}

		if(lstrcmp(_ScrollWnd[i]->szWndName, name) == 0)
			return _ScrollWnd[i];
	}

	if(empty_slot == -1) {
		if(_ScrollWndCnt < 20) {
			empty_slot = _ScrollWndCnt;
			_ScrollWndCnt++;
		}
	}
	if(empty_slot == -1)
		return NULL;

	CScrollWnd* pwnd = new CScrollWnd(CWnd::GetDesktopWindow());
	lstrcpy(pwnd->szWndName, name);
	_ScrollWnd[empty_slot] = pwnd;

	return pwnd;
}

void ReleaseScrollWnd(TCHAR* name)
{
	if(name == NULL) {
		for(int i = 0; i < _ScrollWndCnt; i++) {
			if(_ScrollWnd[i]) {
				_ScrollWnd[i]->DestroyWindow();
				_ScrollWnd[i] = NULL;
			}
		}
	} else {
		for(int i = 0; i < _ScrollWndCnt; i++) {
			if(_ScrollWnd[i]) {
				if(lstrcmp(_ScrollWnd[i]->szWndName, name) == 0) {
					_ScrollWnd[i]->DestroyWindow();
					_ScrollWnd[i] = NULL;
				}
			}
		}
	}
}
// END


void CScrollWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	_sc_start_x = point.x;
	_sc_start_y = point.y;
	
	CString str;
	str.Format(L"(x=%d, y=%d) ", point.x, point.y);
	OutputDebugString(str);
	m_bDown = 1;
	_sc_old_x = -1;

	CWnd::OnLButtonDown(nFlags, point);
}


void CScrollWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bDown && point.x != (_sc_old_x || point.y != _sc_old_y)) {
		_sc_end_x = point.x;
		_sc_end_y = point.y;
		if(_sc_old_x != -1)
			DrawXorBox(_sc_start_x, _sc_start_y, _sc_old_x, _sc_old_y);
		DrawXorBox(_sc_start_x, _sc_start_y, _sc_end_x, _sc_end_y);
		_sc_old_x = _sc_end_x;
		_sc_old_y = _sc_end_y;
	} else {
		CDC *pdc = GetDC();
		COLORREF c = pdc->GetPixel(point);
		TCHAR buf[32];
		wsprintf(buf, L"  %d,%d = %d               ", point.x, point.y, c & 0x0FF);
		pdc->TextOut(200, 0, buf, lstrlen(buf));
		ReleaseDC(pdc);
	}

	CWnd::OnMouseMove(nFlags, point);
}


void CScrollWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_bDown) {
		if(_sc_old_x != -1)
			DrawXorBox(_sc_start_x, _sc_start_y, _sc_old_x, _sc_old_y);
		if(_sc_start_x != _sc_end_x && _sc_end_y != _sc_start_y) {
			int sx = _sc_start_x;
			int ex = _sc_end_x;
			int sy = _sc_start_y;
			int ey = _sc_end_y;
			if(sx > ex) {
				sx = _sc_end_x;
				ex = _sc_start_x;
			}
			if(sy > ey) {
				sy = _sc_end_y;
				ey = _sc_start_y;
			}
			_sc_mat = mat_src(cv::Rect(sx, sy, ex - sx + 1, ey - sy +1));
		}
		m_bDown = 0;
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CScrollWnd::DrawXorBox(int x1, int y1, int ex, int ey)
{
	CDC *pdc = GetDC();
	CGdiObject* pobj = pdc->SelectStockObject(WHITE_PEN);
	int oldrop = pdc->SetROP2(R2_XORPEN);
	pdc->MoveTo(x1, y1);
	pdc->LineTo(ex, y1);
	pdc->LineTo(ex, ey);
	pdc->LineTo(x1, ey);
	pdc->LineTo(x1, y1);
	pdc->SetROP2(oldrop);
	pdc->SelectObject(pobj);
	ReleaseDC(pdc);
}

void CScrollWnd::putText(char* sztmp, Point p)
{
	int i = -1;
	for( i = 0; i < putLinesCnt; i++) {
		if(putLinesPos[i] == p || putLinesTxt[i][0] == 0)
			break;
	}

	if( i >= MAX_PUTLINES)
		return;
	MultiByteToWideChar(CP_ACP, 0, sztmp, strlen(sztmp)+1, putLinesTxt[i], 128);
	putLinesPos[i] = p;

	if(i >= putLinesCnt)
		putLinesCnt = i + 1;
}
