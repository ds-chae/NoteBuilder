
// NoteBuilderFrameView.cpp : implementation of the CNoteBuilderFrameView class
//

#include "stdafx.h"
#include "NoteBuilderFrame.h"

#include "NoteBuilderFrameDoc.h"
#include "NoteBuilderFrameView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNoteBuilderFrameView

IMPLEMENT_DYNCREATE(CNoteBuilderFrameView, CView)

BEGIN_MESSAGE_MAP(CNoteBuilderFrameView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CNoteBuilderFrameView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CNoteBuilderFrameView construction/destruction

CNoteBuilderFrameView::CNoteBuilderFrameView()
{
	// TODO: add construction code here

}

CNoteBuilderFrameView::~CNoteBuilderFrameView()
{
}

BOOL CNoteBuilderFrameView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CNoteBuilderFrameView drawing

void CNoteBuilderFrameView::OnDraw(CDC* /*pDC*/)
{
	CNoteBuilderFrameDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CNoteBuilderFrameView printing


void CNoteBuilderFrameView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

BOOL CNoteBuilderFrameView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CNoteBuilderFrameView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CNoteBuilderFrameView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CNoteBuilderFrameView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CNoteBuilderFrameView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CNoteBuilderFrameView diagnostics

#ifdef _DEBUG
void CNoteBuilderFrameView::AssertValid() const
{
	CView::AssertValid();
}

void CNoteBuilderFrameView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CNoteBuilderFrameDoc* CNoteBuilderFrameView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNoteBuilderFrameDoc)));
	return (CNoteBuilderFrameDoc*)m_pDocument;
}
#endif //_DEBUG


// CNoteBuilderFrameView message handlers
