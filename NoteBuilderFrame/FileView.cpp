
#include "stdafx.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "NoteBuilderFrame.h"
#include "CurrencyList.h"

#include "ICUMANDoc.h"
#include "Define.h"
#include "opencv_use.h"
#include "CarNumDetect.h"
#include "ICUMANView.h"
#include "CurrencyInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileView

CFileView::CFileView()
{
	m_hRoot = NULL;
}

CFileView::~CFileView()
{
}

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_COMMAND(ID_OPEN_WITH, OnFileOpenWith)
	ON_COMMAND(ID_DUMMY_COMPILE, OnDummyCompile)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_wndFileView.Create(dwViewStyle, rectDummy, this, 4))
	{
		TRACE0("Failed to create file view\n");
		return -1;      // fail to create
	}

	// Load view images:
	m_FileViewImages.Create(IDB_FILE_VIEW, 16, 0, RGB(255, 0, 255));
	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	// Fill in some static tree view data (dummy code, nothing magic here)
	AdjustLayout();

	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

int StrToInt(TCHAR* str)
{
	int v = 0;
	while(*str >= _T('0') && *str <= _T('9') ) {
		v = v * 10 + (*str - _T('0'));
		str++;
	}
	return v;
}

int hasImageExt(TCHAR *fn)
{
	int len = lstrlen(fn);
	return lstrcmp(fn+len-4, L".tif") == 0 || lstrcmp(fn+len-4, L".jpg") == 0 || lstrcmp(fn+len-4, L".bmp") == 0;
}

void CFileView::AddCurrencyFile(HTREEITEM hSrc, TCHAR* dir, TCHAR* fn, int level)
{
	if(hasImageExt(fn)) {
		HTREEITEM hChild = m_wndFileView.InsertItem(fn, 1, 1, hSrc);
		m_wndFileView.SetItemData(hChild, 0);
	}
}

void CFileView::AddCurrencyFolder(HTREEITEM hParent, TCHAR* Parent, TCHAR* Child, int level)
{
	HTREEITEM hSrc = m_wndFileView.InsertItem(Child, 0, 0, hParent);
	m_wndFileView.SetItemData(hSrc, 0);

	if(level > 1)
		return;

	TCHAR CurrentDir[MAX_PATH];
	wsprintf(CurrentDir, TEXT("%s\\%s"), Parent, Child);

	WIN32_FIND_DATA wfd;
	TCHAR FindPath[MAX_PATH];
	wsprintf(FindPath, TEXT("%s\\*.*"), CurrentDir);
	
	HANDLE hFind = FindFirstFile(FindPath, &wfd);
	while(hFind != INVALID_HANDLE_VALUE) {
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if( lstrcmp(wfd.cFileName, TEXT(".")) && lstrcmp(wfd.cFileName, TEXT(".."))) {
				AddCurrencyFolder(hSrc, CurrentDir, wfd.cFileName, level+1);
			}
		} else {
			AddCurrencyFile(hSrc, CurrentDir, wfd.cFileName, 0);
		}

		if(!FindNextFile(hFind, &wfd)) {
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
		}
	}

	m_wndFileView.SetItemData(hSrc, m_wndFileView.GetItemData(hSrc) | 1); // 기존 유형에 탐색했다는 마크를 추가
}

BOOL IsCurrencyNamedFolder(WIN32_FIND_DATA *pwfd)
{
#if CURRENCY_ONLY
	return
		(pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
		(lstrlen(pwfd->cFileName) == 3);
#else
	return (pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
		lstrcmp(pwfd->cFileName, L".") && lstrcmp(pwfd->cFileName, L"..");
#endif
}

// 맨 처음 루트를 위해 불리는 함수
// 루트 폴더 밑에 각 커런시의 폴더 이름을 채워준다.
// selectedRoot에 루트 폴더의 이름을 돌려준다. 이 함수는 루트 폴더용이다.
void CFileView::FillFileView(TCHAR* selectedRoot)
{
	m_wndFileView.DeleteAllItems();

	CMainFrame *pMainFrm = (CMainFrame*)this->GetParentFrame();
	pMainFrm->m_FirstPath[0] = 0;

	if(!m_CurrencyImageFolder[0])
		return;

	lstrcpy(selectedRoot, m_CurrencyImageFolder);
	// Root of Image Folder
	m_hRoot = m_wndFileView.InsertItem(selectedRoot, 0, 0);
	m_wndFileView.SetItemData(m_hRoot, 0);

	TCHAR CurrencySearch[MAX_PATH];
	wsprintf(CurrencySearch, TEXT("%s\\*.*"), selectedRoot);
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(CurrencySearch, &wfd);
	while(hFind != INVALID_HANDLE_VALUE ) {
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if(lstrcmp(wfd.cFileName, L".") && lstrcmp(wfd.cFileName, L"..") ) {
				AddCurrencyFolder(m_hRoot, selectedRoot, wfd.cFileName, 0);
			}
		} else {
			AddCurrencyFile(m_hRoot, selectedRoot, wfd.cFileName, 0);
		}

		if(!FindNextFile(hFind, &wfd)) {
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE ;
		}
	}
	m_wndFileView.Expand(m_hRoot, TVE_EXPAND);
	m_wndFileView.SetItemData(m_hRoot, 1); // 해달 폴더의 하부 데이터를 탐색했다는 표시

	CICUMANView *pView = (CICUMANView*)pMainFrm->GetActiveView();
	if(pView != NULL) {
		pView->SetPathName(selectedRoot); // 루트이므로 변환여부를 확인할 필요는 없다.
	}
}

void CFileView::FillFileView(HTREEITEM hRoot, TCHAR* selectedRoot, int ForceRework)
{
//	DeleteChildren(hRoot);

	CString R;
	GetTreeText(R, hRoot);
	lstrcpy(selectedRoot, (LPCTSTR)R);

	CMainFrame *pMainFrm = (CMainFrame*)this->GetParentFrame();
/* -- FillFileView를 호출하는 곳에서 이 기능을 수행하므로 여기서는 막는다.
	CICUMANView *pView = (CICUMANView*)pMainFrm->GetActiveView();
	if(pView != NULL) {
		pView->SetPathName(selectedRoot);
	}
*/
	if(!ForceRework && (m_wndFileView.GetItemData(hRoot) & 1)) // 폴더의 내용을 이미 검색했다면 귀환
		return;
	HTREEITEM ht = m_wndFileView.GetChildItem(hRoot);
	while(ht != NULL) {
		HTREEITEM hn = m_wndFileView.GetNextItem(ht, TVGN_NEXT);
		m_wndFileView.DeleteItem(ht);
		ht = hn;
	}

	TCHAR CurrencySearch[MAX_PATH];
	wsprintf(CurrencySearch, TEXT("%s\\*.*"), selectedRoot);
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(CurrencySearch, &wfd);
	while(hFind != INVALID_HANDLE_VALUE ) {
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if( lstrcmp(wfd.cFileName, TEXT(".")) && lstrcmp(wfd.cFileName, TEXT("..")))
				AddCurrencyFolder(hRoot, selectedRoot, wfd.cFileName, 0);
			// LoadCurrencyInfoFile(selectedRoot, wfd.cFileName); // 헤더와 Const.c에서 데이터를 불러오게 해서 더 이상 필요가 없다.
		} else {
			AddCurrencyFile(hRoot, selectedRoot, wfd.cFileName, 0);
		}

		if(!FindNextFile(hFind, &wfd)) {
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE ;
		}
	}
	m_wndFileView.Expand(hRoot, TVE_EXPAND);
	m_wndFileView.SetItemData(hRoot, 1);

#if 0
	HTREEITEM hSrc = m_wndFileView.InsertItem(_T("FakeApp Source Files"), 0, 0, hRoot);

	m_wndFileView.InsertItem(_T("FakeApp.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("FakeApp.rc"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("FakeAppDoc.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("FakeAppView.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("MainFrm.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("StdAfx.cpp"), 1, 1, hSrc);

	HTREEITEM hInc = m_wndFileView.InsertItem(_T("FakeApp Header Files"), 0, 0, hRoot);

	m_wndFileView.InsertItem((_T("FakeApp.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("FakeAppDoc.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("FakeAppView.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("Resource.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("MainFrm.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("StdAfx.h"), 2, 2, hInc);

	HTREEITEM hRes = m_wndFileView.InsertItem(_T("FakeApp Resource Files"), 0, 0, hRoot);

	m_wndFileView.InsertItem(_T("FakeApp.ico"), 2, 2, hRes);
	m_wndFileView.InsertItem(_T("FakeApp.rc2"), 2, 2, hRes);
	m_wndFileView.InsertItem(_T("FakeAppDoc.ico"), 2, 2, hRes);
	m_wndFileView.InsertItem(_T("FakeToolbar.bmp"), 2, 2, hRes);

	m_wndFileView.Expand(hSrc, TVE_EXPAND);
	m_wndFileView.Expand(hInc, TVE_EXPAND);
	m_wndFileView.Expand(hRoot, TVE_EXPAND);
#endif
}

void CFileView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_wndFileView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndFileView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::OnProperties()
{
	AfxMessageBox(_T("Properties...."));

}

void CFileView::OnFileOpen()
{
	// TODO: Add your command handler code here
}

void CFileView::OnFileOpenWith()
{
	// TODO: Add your command handler code here
}

void CFileView::OnDummyCompile()
{
	// TODO: Add your command handler code here
}

void CFileView::OnEditCut()
{
	// TODO: Add your command handler code here
}

void CFileView::OnEditCopy()
{
	// TODO: Add your command handler code here
}

void CFileView::OnEditClear()
{
	// TODO: Add your command handler code here
}

void CFileView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_wndFileView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndFileView.SetFocus();
}

void CFileView::OnChangeVisualStyle()
{
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);

	m_FileViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_FileViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_FileViewImages.Add(&bmp, RGB(255, 0, 255));

	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
}


void CFileView::GetTreeText(CString& R, HTREEITEM item)
{
	HTREEITEM pitem = m_wndFileView.GetParentItem(item);
	if(pitem != NULL) {
		CString P;
		GetTreeText(P, pitem);
		R = P + L"\\" + m_wndFileView.GetItemText(item);
	} else {
		R = m_wndFileView.GetItemText(item);
	}
}

BOOL CFileView::HasChildren(HTREEITEM hItem)
{
	return m_wndFileView.GetNextItem(hItem, TVGN_CHILD) != NULL;
}
   
void CFileView::DeleteChildren(HTREEITEM hCurr)
{
	HTREEITEM hChild = m_wndFileView.GetNextItem(hCurr, TVGN_CHILD);
	while(hChild != NULL) {
		HTREEITEM hNext = m_wndFileView.GetNextItem(hChild, TVGN_NEXT);
		m_wndFileView.DeleteItem(hChild);
		hChild = hNext;
	}
}

