// Filename: ScrollWnd.h
// 2005-07-02 nschan Initial revision.
// 2005-09-08 nschan Use memory DC drawing to eliminate flickering on resize.

#ifndef SCROLL_WND_INCLUDED
#define SCROLL_WND_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	MAX_PUTLINES	10

// Forward class declarations.
class CScrollHelper;

// CScrollWnd implements a scrollable window using CScrollHelper.
class CScrollWnd : public CWnd
{
public:
    CScrollWnd(CWnd* parentWnd);
    virtual ~CScrollWnd();

public:
	TCHAR szWndName[128];
	Mat mat_src;
	void matShow(Mat &mat_img);
	int m_bDown;
	void* pEdgeDetector;
	int edge_to_mark;

	int putLinesCnt;
	Point putLinesPos[MAX_PUTLINES];
	TCHAR putLinesTxt[MAX_PUTLINES][128];
	Rect XorRect;

	void DrawXorBox(int x1, int y1, int x2, int y2);
	void SetName(char* name);
	void putText(char* sztmp, Point p);

protected:
    // ClassWizard generated virtual function overrides.
    //{{AFX_VIRTUAL(CScrollWnd)
    virtual void PostNcDestroy();
    //}}AFX_VIRTUAL

    // Generated message map functions.
    //{{AFX_MSG(CScrollWnd)
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg int  OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg void OnPaint();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void DrawScrollInfo(CDC* pDC);

    CScrollHelper* m_scrollHelper;
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

extern CScrollWnd *CreateScrollWnd(TCHAR* name);

extern int _sc_start_x;
extern int _sc_start_y;
extern int _sc_end_x;
extern int _sc_end_y;
extern int _sc_old_x;
extern int _sc_old_y;
extern Mat _sc_mat;

void DrawTestedEdges(void *pDrawer, CDC* pdc);
void DrawMarkedEdge(void *pEdgeDetector, CDC *pdc, int edge_to_mark);

#endif // SCROLL_WND_INCLUDED

// END

