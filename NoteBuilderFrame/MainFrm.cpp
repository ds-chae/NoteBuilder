
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "NoteBuilderFrame.h"

#include "MainFrm.h"
#include "ICUMANDoc.h"
#include "Define.h"
#include "opencv_use.h"
#include "CarNumDetect.h"
#include "ICUMANView.h"
#include "CurrencyList.h"
#include "WinDebugDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

TCHAR m_CurrencyImageFolder[MAX_PATH];
int ShowFolderDialog(HWND hWnd, TCHAR* buf);

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_CLOSE, &CMainFrame::OnFileClose)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnUpdateApplicationLook)
	ON_COMMAND(ID_EDIT_CURRENCY, &CMainFrame::OnEditCurrency)
	ON_COMMAND(ID_FILE_RELOAD, &CMainFrame::OnFileReload)
	ON_COMMAND(ID_FILE_OPEN, &CMainFrame::OnFileOpen)
	ON_COMMAND(ID_DEBUG_IPC, &CMainFrame::OnOcr)
	ON_COMMAND(ID_CURRENCY_IMAGE_FOLDER, &CMainFrame::OnCurrencyImageFolder)
	ON_WM_INITMENU()
	ON_COMMAND(ID_VIEW_REINFORCE_IMAGE, &CMainFrame::OnViewReinforceImage)

	ON_COMMAND(ID_DEBUG_SHOWDEBUG, &CMainFrame::OnDebugShowdebug)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_FirstPath[0] = 0;

	m_CurrencyImageFolder[0] = 0;

	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2005);
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;
	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	// Allow user-defined toolbars operations:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Load menu item image (not placed on any standard toolbars):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

	FindCurrencyImageFolder();
//	LoadCurrencyList();

	// create docking windows
	if (!CreateDockingWindows())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}

	m_wndFileView.EnableDocking(CBRS_ALIGN_ANY);
#if 0
	m_wndClassView.EnableDocking(CBRS_ALIGN_ANY);
#endif
	DockPane(&m_wndFileView);
#if 0
	CDockablePane* pTabbedBar = NULL;
	m_wndClassView.AttachToTabWnd(&m_wndFileView, DM_SHOW, TRUE, &pTabbedBar);
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutput);
#endif
#if ENABLE_PROP
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProperties);
#endif

	// Enable toolbar and docking window menu replacement
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == NULL)
	{
		// load user-defined toolbar images
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			m_UserImages.SetImageSize(CSize(16, 16), FALSE);
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}

	// enable menu personalization (most-recently used commands)
	// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
	CList<UINT, UINT> lstBasicCommands;

	lstBasicCommands.AddTail(ID_FILE_NEW);
	lstBasicCommands.AddTail(ID_FILE_OPEN);
	lstBasicCommands.AddTail(ID_FILE_SAVE);
	lstBasicCommands.AddTail(ID_FILE_PRINT);
	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_EDIT_CUT);
	lstBasicCommands.AddTail(ID_EDIT_PASTE);
	lstBasicCommands.AddTail(ID_EDIT_UNDO);
	lstBasicCommands.AddTail(ID_APP_ABOUT);
	lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
	lstBasicCommands.AddTail(ID_FILE_NEW_FRAME);
	lstBasicCommands.AddTail(ID_FILE_CLOSE);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);
	lstBasicCommands.AddTail(ID_SORTING_SORTALPHABETIC);
	lstBasicCommands.AddTail(ID_SORTING_SORTBYTYPE);
	lstBasicCommands.AddTail(ID_SORTING_SORTBYACCESS);
	lstBasicCommands.AddTail(ID_SORTING_GROUPBYTYPE);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);

	m_tszSelectedRoot[0] = 0;
	m_wndFileView.FillFileView(m_tszSelectedRoot);
	SetWindowText(m_tszSelectedRoot);

	unsigned int WinDebugWait(void *pData);
	AfxBeginThread(WinDebugWait, this);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
	BOOL bNameValid;

#if 0
	// Create class view
	CString strClassView;
	bNameValid = strClassView.LoadString(IDS_CLASS_VIEW);
	ASSERT(bNameValid);
	if (!m_wndClassView.Create(strClassView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CLASSVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Class View window\n");
		return FALSE; // failed to create
	}
#endif

	// Create file view
	CString strFileView;
	bNameValid = strFileView.LoadString(IDS_FILE_VIEW);
	ASSERT(bNameValid);
	if (!m_wndFileView.Create(strFileView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_FILEVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create File View window\n");
		return FALSE; // failed to create
	}

	// Create output window
	CString strOutputWnd;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
#if 0
	if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Output window\n");
		return FALSE; // failed to create
	}
#endif

	// Create properties window
	CString strPropertiesWnd;
	bNameValid = strPropertiesWnd.LoadString(IDS_PROPERTIES_WND);
	ASSERT(bNameValid);
#if ENABLE_PROP
	if (!m_wndProperties.Create(strPropertiesWnd, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}
#endif
	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hFileViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndFileView.SetIcon(hFileViewIcon, FALSE);
#if 0
	HICON hClassViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndClassView.SetIcon(hClassViewIcon, FALSE);
	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);
#endif

#if ENABLE_PROP
	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);
#endif
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	CWinApp* pApp = AfxGetApp();
	if (pApp->m_pMainWnd == NULL)
		pApp->m_pMainWnd = this;

	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}

void CMainFrame::OnFileClose()
{
	DestroyWindow();
}

BOOL IsDirectory(TCHAR* FolderFound)
{
	DWORD attrs = GetFileAttributes(FolderFound);
	if((attrs != 0xFFFFFFFF) && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
		return TRUE;
	}
	return FALSE;
}

void FindMatchFolder(TCHAR* FolderToFind, TCHAR* FolderFound)
{
	TCHAR CurrentDir[MAX_PATH];
	GetCurrentDirectory(sizeof(CurrentDir), CurrentDir);
	lstrcat(CurrentDir, L"\\ICUMAN");

	// 현재 폴더에서 찾아 보고, 올라가면서 찾아본다.
	for(int i = 0; i < 4; i++) {
		//wsprintf(FolderFound, L"%s\\CUR_IMAGES", CurrentDir, FolderToFind);
		wsprintf(FolderFound, L"%s\\%s", CurrentDir, FolderToFind);
		if(IsDirectory(FolderFound))
			return;
		int l = lstrlen(CurrentDir);
		while(l > 0) {
			l--;
			if(CurrentDir[l] == '\\') {
				CurrentDir[l] = 0;
				break;
			}
		}
	}

	FolderFound[0] = 0;
}

/*
void CMainFrame::LoadCurrencyList()
{
	CurrencyList.Load(this, m_CurrencyImageFolder);
}
*/

void CMainFrame::RegSetImageFolder()
{
	DWORD dwDisp = 0;
	HKEY hkey = NULL;
	LONG err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, AXIAL_LPR, 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisp);
	if(err == ERROR_SUCCESS) {
		RegSetValueEx(hkey, TEXT("ImagePath"), 0, REG_SZ, (LPBYTE)m_CurrencyImageFolder, (lstrlen(m_CurrencyImageFolder)+1)*sizeof(TCHAR));
		RegCloseKey(hkey);
	}
}

void CMainFrame::FindCurrencyImageFolder()
{
	m_CurrencyImageFolder[0] = 0;
	// 먼저 레지스트리를 검사한다.
	HKEY hk;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, AXIAL_LPR, 0, KEY_ALL_ACCESS, &hk) == ERROR_SUCCESS) {
		DWORD dwType = 0;
		DWORD cbData = sizeof(m_CurrencyImageFolder);
		RegQueryValueEx(hk, TEXT("ImagePath"), NULL, &dwType, (BYTE*)m_CurrencyImageFolder, &cbData);
		RegCloseKey(hk);
	}

	if(m_CurrencyImageFolder[0]) {
		if(!IsDirectory(m_CurrencyImageFolder))
			m_CurrencyImageFolder[0] = 0;
	}
	
	if(!m_CurrencyImageFolder[0]) {
		FindMatchFolder(TEXT("CUR_IMAGES"), m_CurrencyImageFolder);
	}

	if(!m_CurrencyImageFolder[0]) {
		// ask image folder
		if(ShowFolderDialog(this->GetSafeHwnd(), m_CurrencyImageFolder)) {
			RegSetImageFolder();
		}
	}
}

void xbreaker()
{
}

BOOL CMainFrame::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: Add your specialized code here and/or call the base class

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT(pNMHDR != NULL);

	if(pNMHDR->code == TVN_SELCHANGED) {
		if(::GetParent(pNMHDR->hwndFrom) == this->m_wndFileView.m_hWnd) {
			int reworkCount = 0;
			LPNMTREEVIEW ntv = (LPNMTREEVIEW)lParam;
			HTREEITEM m_hSelectedTreeItem = ntv->itemNew.hItem;
_rework:
			m_wndFileView.FillFileView(m_hSelectedTreeItem, m_tszSelectedRoot, reworkCount);
			SetWindowText(m_tszSelectedRoot);

			CICUMANView *pView = (CICUMANView*)this->GetActiveView();
			if(pView != NULL) {
				if(pView->SetPathName(m_tszSelectedRoot) && reworkCount == 0) {
					// 만약 변환된 파일이 있었다면 트리를 다시 채운다.
					reworkCount = 1;
					goto _rework;
				}
			}
			//CICUMANDoc *pDoc = (CICUMANDoc*)GetActiveDocument();
			//if(pDoc != NULL)
			//	pDoc->OnOpenDocument(S);
			TVITEMEX TvEx;
			TvEx.mask = TVIF_PARAM;// | TVIF_TEXT;
			TvEx.hItem = ntv->itemNew.hItem;
			TreeView_GetItem(pNMHDR->hwndFrom, &TvEx);
//            Param *pParam = (Param *)TvEx.lParam;
		}
	}

	return CFrameWndEx::OnNotify(wParam, lParam, pResult);
}

void CMainFrame::OnEditCurrency()
{
	CICUMANView *pView = (CICUMANView*)this->GetActiveView();
	if(pView != NULL) {
//		pView->EditCurrencyInfo();
	}
}

void CMainFrame::OnFileReload()
{
	m_wndFileView.FillFileView(m_tszSelectedRoot);
	SetWindowText(m_tszSelectedRoot);
}

// 초기 셋팅 시 '경로'를 무조건 나오게 할려면
int CALLBACK BICallBackFunc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
 switch(uMsg)
 {
 case BFFM_INITIALIZED:
  if(lpData)
   ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);      // lpData 가 경로
  break;
 }

 return 0;
}

BOOL BrowseFolder(HWND hWnd, TCHAR *pszPathname, LPARAM lParam, DWORD flag)
{
	ITEMIDLIST*  pildBrowse;
	BROWSEINFO  bInfo;

	memset(&bInfo, 0, sizeof(bInfo));

	bInfo.hwndOwner   = hWnd;
	bInfo.pidlRoot   = NULL;
	bInfo.pszDisplayName = pszPathname;
	bInfo.lpszTitle   = _T("디렉토리를 선택하세요");             // dialog title
	bInfo.ulFlags   = flag;                 // 여러가지 옵션이 존재합니다.. msdn을 참고해보세요.
	bInfo.lpfn    = BICallBackFunc;                        // 옵션? 입니다.. 초기 경로 설정등을 할때는 여기에 추가하세요.
	bInfo.lParam  = (LPARAM)(LPCTSTR)lParam;

	pildBrowse    = ::SHBrowseForFolder(&bInfo);

	if(!pildBrowse)
		return FALSE;

	SHGetPathFromIDList(pildBrowse, pszPathname);
	return TRUE;
}

int ShowFolderDialog(HWND hWnd, TCHAR* buf)
{
	TCHAR   pszPathname[MAX_PATH];
	if(BrowseFolder(hWnd, pszPathname, (LPARAM)buf, BIF_RETURNONLYFSDIRS | BIF_VALIDATE)) {
		lstrcpy(buf, (LPCTSTR)pszPathname);                                // => 실제 선택한 폴더가 전체 경로로 나옵니다.
		return TRUE;
	}
	return FALSE;
}

void CMainFrame::OnFileOpen()
{
	if(ShowFolderDialog(this->GetSafeHwnd(), m_CurrencyImageFolder)) {
		RegSetImageFolder();

		m_wndFileView.FillFileView(m_tszSelectedRoot);
		SetWindowText(m_tszSelectedRoot);
	}
}

int GetIPCDataHeader(unsigned char* debugdata, char* caption, int* type, int* width, int* height)
{
	int tp = 0;
	int clen = 0;

	memcpy(&clen, debugdata+0, 4); // caption length
	tp += 4;

	memcpy(caption, debugdata+tp, clen); // caption
	tp += clen;

	memcpy(type, debugdata+tp, 4);
	tp += 4;
	
	memcpy(width, debugdata+tp, 4);
	tp += 4;
	
	memcpy(height, debugdata+tp, 4);
	tp += 4;

	return tp;
}

void CMainFrame::OnOcr()
{
	int GetDebugIPCPtr(unsigned char** pptr, DWORD* len);

	while(1) {
		unsigned char* ptr;
		DWORD len;
		if(!GetDebugIPCPtr(&ptr, &len))
			break;

		// 일단 데이터의 형식을 판별해서 필요한 함수를 부른다.
		char caption[MAX_PATH];
		int type, width, height;
		int tp = GetIPCDataHeader(ptr, caption, &type, &width, &height);

			CICUMANView *pView = (CICUMANView*)this->GetActiveView();
/* 디버그 윈도우를 따로 열지 않고 CICUMANView의 debug 영역에 표시하도록 수정함. 20120331 dschae
		if(bDebugEnabled) {
			WinDebugDlg* pDlg = new WinDebugDlg();
			pDlg->debugdata = ptr;
			pDlg->Create(IDD_WINDEBUG, this);
			pDlg->ShowWindow(SW_SHOW);
		}
*/
		LocalFree(ptr);
	}
}

void CMainFrame::OnCurrencyImageFolder()
{
	// TODO: Add your command handler code here
}

BOOL FileExist(TCHAR* path, WIN32_FIND_DATA* pwfd)
{
	HANDLE h = FindFirstFile(path, pwfd);
	if(h != INVALID_HANDLE_VALUE) {
		FindClose(h);
		return TRUE;
	}

	return FALSE;
}

void CMainFrame::OnViewReinforceImage()
{
	CICUMANView *pView = (CICUMANView*)this->GetActiveView();
	if(pView != NULL) {
		pView->ToggleImgReinforce();
	}
}

void CMainFrame::OnDebugShowdebug()
{
	bDebugEnabled = !bDebugEnabled;
	RegSetOptions();
}
