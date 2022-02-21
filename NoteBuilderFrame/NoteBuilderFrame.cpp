//
// NoteBuilderFrame.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "NoteBuilderFrame.h"
#include "MainFrm.h"

#include "ICUMANDoc.h"
#include "Define.h"
#include "opencv_use.h"
#include "CarNumDetect.h"
#include "ICUMANView.h"
#include "CurrencyInfo.h"
#include "MacroValue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNoteBuilderFrameApp

BEGIN_MESSAGE_MAP(CNoteBuilderFrameApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CNoteBuilderFrameApp::OnAppAbout)
	ON_COMMAND(ID_FILE_NEW_FRAME, &CNoteBuilderFrameApp::OnFileNewFrame)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CNoteBuilderFrameApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CNoteBuilderFrameApp construction

CNoteBuilderFrameApp::CNoteBuilderFrameApp()
{

	m_bHiColorIcons = TRUE;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CNoteBuilderFrameApp object

CNoteBuilderFrameApp theApp;


// CNoteBuilderFrameApp initialization
BOOL ReplaceStrInStr(char* src, char* s1, char* s2);

BOOL CNoteBuilderFrameApp::InitInstance()
{
#if 0 // ReplaceStrInStr 시험한 코드임.
	char src[20];
	strcpy_s(src, 20, "123456789");
	ReplaceStrInStr(src, "34", "ab");
	strcpy_s(src, 20, "123456789");
	ReplaceStrInStr(src, "34", "abc");
	strcpy_s(src, 20, "123456789");
	ReplaceStrInStr(src, "345", "ab");
#endif

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


#if CURRENCY
	InitializeRecognitionVariables();
#endif
	FILE *fdebug = _tfopen(L"C:\\imgdebug.txt", L"wt");
	fclose(fdebug);

	RegGetNNRFolder();
	RegGetOptions();

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CICUMANDoc),
		RUNTIME_CLASS(CMainFrame), // custom MDI child frame
		RUNTIME_CLASS(CICUMANView));
	if (!pDocTemplate)
		return FALSE;
	m_pDocTemplate = pDocTemplate;
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand

	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CNoteBuilderFrameApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CNoteBuilderFrameApp customization load/save methods

void CNoteBuilderFrameApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CNoteBuilderFrameApp::LoadCustomState()
{
}

void CNoteBuilderFrameApp::SaveCustomState()
{
}

// CNoteBuilderFrameApp message handlers

void CNoteBuilderFrameApp::OnFileNewFrame() 
{
	ASSERT(m_pDocTemplate != NULL);

	CDocument* pDoc = NULL;
	CFrameWnd* pFrame = NULL;

	// Create a new instance of the document referenced
	// by the m_pDocTemplate member.
	if (m_pDocTemplate != NULL)
		pDoc = m_pDocTemplate->CreateNewDocument();

	if (pDoc != NULL)
	{
		// If creation worked, use create a new frame for
		// that document.
		pFrame = m_pDocTemplate->CreateNewFrame(pDoc, NULL);
		if (pFrame != NULL)
		{
			// Set the title, and initialize the document.
			// If document initialization fails, clean-up
			// the frame window and document.

			m_pDocTemplate->SetDefaultTitle(pDoc);
			if (!pDoc->OnNewDocument())
			{
				pFrame->DestroyWindow();
				pFrame = NULL;
			}
			else
			{
				// Otherwise, update the frame
				m_pDocTemplate->InitialUpdateFrame(pFrame, pDoc, TRUE);
			}
		}
	}

	// If we failed, clean up the document and show a
	// message to the user.

	if (pFrame == NULL || pDoc == NULL)
	{
		delete pDoc;
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
	}
}

void CNoteBuilderFrameApp::OnFileNew() 
{
	CDocument* pDoc = NULL;
	CFrameWnd* pFrame;
	pFrame = DYNAMIC_DOWNCAST(CFrameWnd, CWnd::GetActiveWindow());
	
	if (pFrame != NULL)
		pDoc = pFrame->GetActiveDocument();

	if (pFrame == NULL || pDoc == NULL)
	{
		// if it's the first document, create as normal
		CWinApp::OnFileNew();
	}
	else
	{
		// Otherwise, see if we have to save modified, then
		// ask the document to reinitialize itself.
		if (!pDoc->SaveModified())
			return;

		CDocTemplate* pTemplate = pDoc->GetDocTemplate();
		ASSERT(pTemplate != NULL);

		if (pTemplate != NULL)
			pTemplate->SetDefaultTitle(pDoc);
		pDoc->OnNewDocument();
	}
}

#define	APP_NAME	AXIAL_LPR
#define	NNR_FOLDER	"NNRFolder"
#define	NUM_OF_CHARS	"NumberOfCharacters"
#define	SAVE_HEIGHT	"SaveHeight"
#define	SAVE_WIDTH	"SaveWidth"
#define	OCR_IMAGE_SOURCE	"OCRImageSource"
#define	OCR_H_WINDOW		"OCRHWindow"
#define	OCR_V_WINDOW		"OCRVWindow"
#define	OCR_IR_PERCENT		"OCRIrPercent"
#define	OCR_W_PERCENT		"OCRWPercent"
#define	OCR_XWINDOW_METHOD	"OCRXWindowMethod"
#define	OCR_THR_TYPE		"OCRThrType"

#define	KEY_FOLDER_1			"KeyFolder1"
#define	KEY_FOLDER_2			"KeyFolder2"

#define	EXTRACT_FOLDER		"ExtractFolder"
#define	EXTDST_FOLDER		"ExtDestFolder"
#define	NNREXE_FILE			"NNRExeFile"

#define	LEAST_PIXEL			"LeastPixel"

#define	TEST_THRU_TESTDLL	"TestThruTestDLL"

TCHAR RegNNRFolder[MAX_PATH];
TCHAR m_tszKeyFolder1[MAX_PATH];
TCHAR m_tszKeyFolder2[MAX_PATH];
TCHAR m_tszRefFolder[MAX_PATH];

TCHAR m_tszExtFolder[MAX_PATH];
TCHAR m_tszDstFolder[MAX_PATH] = L"";

TCHAR m_tszNNRExeFile[MAX_PATH] = L"";

int SaveHeight = 11;
int SaveWidth = 16;

CMacroValue OCR_HWindow("_OCR_H_WINDOW", 16);

CMacroValue OCR_VWindow("_OCR_V_WINDOW_1", 13);
// 0	= IR Transmit, 1 = W Reflection
CMacroValue OCR_ImageSource("_OCR_IMGSOURCE", 0);

CMacroValue OCR_VWindow_2("_OCR_V_WINDOW_2", 21);
CMacroValue OCR_maxchar("_OCR_MAXCHAR", 12);
CMacroValue OCR_minchar("_OCR_MINCHAR", 10);

CMacroValue OCR_save_width_height_count("_OCR_SAVE_WIDTH_HEIGHT_COUNT", 1);

//CMacroValue OCR_x_window("_OCR_X_WINDOW", 18); OCR_H_WINDOW와 중복으로 삭제함 dschae@20121222
CMacroValue OCR_y_window("_OCR_Y_WINDOW", 14);// 14 - 이미지가 있는 곳을 찾을 때 사용하는 높이
CMacroValue OCR_f_height("_OCR_F_HEIGHT", 14);// 14 - 인식을 위해 상하를 잘라낸 후에 사용하는 높이

CMacroValue OCR_IR_Percent("_OCR_IR_PCT", 89);
CMacroValue OCR_W_Percent("_OCR_W_PCT", 120);
CMacroValue least_pixel("_OCR_MIN_PIXELS", 100);
CMacroValue OCR_xwindow_method("_OCR_XWIN_METHOD", 0);
CMacroValue OCR_ThrType("_OCR_THR_TYPE", 0);
CMacroValue OCR_scaleX("_OCR_SCALE_X", 2);
CMacroValue OCR_scaleY("_OCR_SCALE_Y", 1);

int bTestThruTestDLL = 1;
int bNNRLoaded = 0;

void RegSetNNRFolder()
{
	DWORD dwDisp = 0;
	HKEY hkey = NULL;
	LONG err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, APP_NAME, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisp);
	if(err != ERROR_SUCCESS)
		return;

	RegSetValueEx(hkey, TEXT(NNR_FOLDER), 0, REG_SZ, (LPBYTE)RegNNRFolder, (lstrlen(RegNNRFolder)+1)*sizeof(TCHAR));

	RegSetValueEx(hkey, TEXT(SAVE_HEIGHT), 0, REG_BINARY, (LPBYTE)&SaveHeight, sizeof(SaveHeight));
	RegSetValueEx(hkey, TEXT(SAVE_WIDTH), 0, REG_BINARY, (LPBYTE)&SaveWidth, sizeof(SaveWidth));
	RegSetValueEx(hkey, TEXT(OCR_IMAGE_SOURCE), 0, REG_BINARY, (LPBYTE)&OCR_ImageSource, sizeof(OCR_ImageSource));
	RegSetValueEx(hkey, TEXT(OCR_XWINDOW_METHOD), 0, REG_BINARY, (LPBYTE)&OCR_xwindow_method, sizeof(OCR_xwindow_method));
	RegSetValueEx(hkey, TEXT(OCR_THR_TYPE), 0, REG_BINARY, (LPBYTE)&OCR_ThrType, sizeof(OCR_ThrType));

	RegSetValueEx(hkey, TEXT(KEY_FOLDER_1), 0, REG_BINARY, (LPBYTE)m_tszKeyFolder1, sizeof(m_tszKeyFolder1));
	RegSetValueEx(hkey, TEXT(KEY_FOLDER_2), 0, REG_BINARY, (LPBYTE)m_tszKeyFolder2, sizeof(m_tszKeyFolder2));
	RegSetValueEx(hkey, TEXT(EXTRACT_FOLDER), 0, REG_BINARY, (LPBYTE)m_tszExtFolder, sizeof(m_tszExtFolder));
	RegSetValueEx(hkey, TEXT(EXTDST_FOLDER), 0, REG_BINARY, (LPBYTE)m_tszDstFolder, sizeof(m_tszDstFolder));
	RegSetValueEx(hkey, TEXT(NNREXE_FILE), 0, REG_BINARY, (LPBYTE)m_tszNNRExeFile, sizeof(m_tszNNRExeFile));

	RegSetValueEx(hkey, TEXT(TEST_THRU_TESTDLL), 0, REG_BINARY, (LPBYTE)&bTestThruTestDLL, sizeof(bTestThruTestDLL));

	RegCloseKey(hkey);
}

void RegGetNNRFolder()
{
	GetCurrentDirectory(MAX_PATH, RegNNRFolder);
	HKEY hkey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, APP_NAME, 0, KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS)
		return;

	DWORD dwType, dwSize;
	dwType = 0;
	dwSize = MAX_PATH;
	TCHAR temp[MAX_PATH];
	RegNNRFolder[0] = 0;
	if(RegQueryValueEx(hkey, TEXT(NNR_FOLDER), NULL, &dwType, (LPBYTE)temp, &dwSize) == ERROR_SUCCESS) {
		lstrcpy(RegNNRFolder, temp);
	}

	dwSize = sizeof(SaveHeight);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT(SAVE_HEIGHT), NULL, &dwType, (LPBYTE)&SaveHeight, &dwSize);

	dwSize = sizeof(SaveWidth);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT(SAVE_WIDTH), NULL, &dwType, (LPBYTE)&SaveWidth, &dwSize);

	dwSize = sizeof(m_tszKeyFolder1);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT(KEY_FOLDER_1), NULL, &dwType, (LPBYTE)m_tszKeyFolder1, &dwSize);

	dwSize = sizeof(m_tszKeyFolder2);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT(KEY_FOLDER_2), NULL, &dwType, (LPBYTE)m_tszKeyFolder2, &dwSize);

	dwSize = sizeof(m_tszExtFolder);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT(EXTRACT_FOLDER), NULL, &dwType, (LPBYTE)m_tszExtFolder, &dwSize);

	dwSize = sizeof(m_tszDstFolder);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT(EXTDST_FOLDER), NULL, &dwType, (LPBYTE)m_tszDstFolder, &dwSize);

	dwSize = sizeof(m_tszNNRExeFile);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT(NNREXE_FILE), NULL, &dwType, (LPBYTE)m_tszNNRExeFile, &dwSize);

	dwSize = sizeof(least_pixel);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT(TEST_THRU_TESTDLL), NULL, &dwType, (LPBYTE)&bTestThruTestDLL, &dwSize);

	RegCloseKey(hkey);
}

BOOL bDebugEnabled = TRUE;
int  nPlateType;
int  nBlurType = BLUR_NORMAL; // normal
int bCropImage = 1;
int bScaleImage = 1;
int kernel_size = 3;
int equitest_method = 0;
int bTestHoughLine = 0;
int bApplyFFT = 0;
int bNewCanny = 0;

extern int apply_undistort;

int canny_th_min[10] = { 60,  80,  40, 25 };
int canny_th_max[10] = {180, 240, 120, 75 };
int canny_th_cnt = 4;

extern int horiline_min_pixels;  //수평선 최소에지 픽셀수
extern int horiline_var_allow;   // 수평선 분산 기준값
extern int vertline_min_pixels;  // 수직선 최소에지 픽셀수 
extern int vertline_var_allow;   // 수직선 분산 기준값..



void RegSetOptions()
{
	DWORD dwDisp = 0;
	HKEY hkey = NULL;
	LONG err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, APP_NAME, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisp);
	if(err != ERROR_SUCCESS)
		return;

	RegSetValueEx(hkey, TEXT("DEBUGDLG"), 0, REG_BINARY, (LPBYTE)&bDebugEnabled, sizeof(bDebugEnabled));
	RegSetValueEx(hkey, TEXT("PLATETYPE"), 0, REG_BINARY, (LPBYTE)&nPlateType, sizeof(nPlateType));
	RegSetValueEx(hkey, TEXT("BLURTYPE"), 0, REG_BINARY, (LPBYTE)&nBlurType, sizeof(nBlurType));
	RegSetValueEx(hkey, TEXT("CROPIMAGE"), 0, REG_BINARY, (LPBYTE)&bCropImage, sizeof(bCropImage));
	RegSetValueEx(hkey, TEXT("SCALEIMAGE"), 0, REG_BINARY, (LPBYTE)&bScaleImage, sizeof(bScaleImage));
	RegSetValueEx(hkey, TEXT("CANNYTHCNT"), 0, REG_BINARY, (LPBYTE)&canny_th_cnt, sizeof(canny_th_cnt));
	RegSetValueEx(hkey, TEXT("CANNYTHMIN"), 0, REG_BINARY, (LPBYTE)&canny_th_min, sizeof(canny_th_min));
	RegSetValueEx(hkey, TEXT("CANNYTHMAX"), 0, REG_BINARY, (LPBYTE)&canny_th_max, sizeof(canny_th_max));

	RegSetValueEx(hkey, TEXT("UNDISTORT"), 0, REG_BINARY, (LPBYTE)&apply_undistort, sizeof(apply_undistort));
	RegSetValueEx(hkey, TEXT("KERNELSIZE"), 0, REG_BINARY, (LPBYTE)&kernel_size, sizeof(kernel_size));
	RegSetValueEx(hkey, TEXT("HOUGHLINETEST"), 0, REG_BINARY, (LPBYTE)&bTestHoughLine, sizeof(bTestHoughLine));
	RegSetValueEx(hkey, TEXT("APPLYFFT"), 0, REG_BINARY, (LPBYTE)&bApplyFFT, sizeof(bApplyFFT));
	RegSetValueEx(hkey, TEXT("NEWCANNY"), 0, REG_BINARY, (LPBYTE)&bNewCanny, sizeof(bNewCanny));

	
	RegSetValueEx(hkey, TEXT("HORIMINPIXELS"), 0, REG_BINARY, (LPBYTE)&horiline_min_pixels, sizeof(horiline_min_pixels));
	RegSetValueEx(hkey, TEXT("HORIVARALLOW"), 0, REG_BINARY, (LPBYTE)&horiline_var_allow, sizeof(horiline_var_allow));
	RegSetValueEx(hkey, TEXT("VERTMINPIXELS"), 0, REG_BINARY, (LPBYTE)&vertline_min_pixels, sizeof(vertline_min_pixels));
    RegSetValueEx(hkey, TEXT("VERTVARALLOW"), 0, REG_BINARY, (LPBYTE)&vertline_var_allow, sizeof(vertline_var_allow));
	
	RegCloseKey(hkey);
}

void RegGetOptions()
{
	HKEY hkey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, APP_NAME, 0, KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS)
		return;

	DWORD dwType, dwSize;

	dwSize = sizeof(bDebugEnabled);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("DEBUGDLG"), NULL, &dwType, (LPBYTE)&bDebugEnabled, &dwSize);

	dwSize = sizeof(nPlateType);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("PLATETYPE"), NULL, &dwType, (LPBYTE)&nPlateType, &dwSize);

	dwSize = sizeof(nPlateType);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("BLURTYPE"), NULL, &dwType, (LPBYTE)&nBlurType, &dwSize);

	dwSize = sizeof(nBlurType);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("CROPIMAGE"), NULL, &dwType, (LPBYTE)&bCropImage, &dwSize);

	dwSize = sizeof(nBlurType);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("SCALEIMAGE"), NULL, &dwType, (LPBYTE)&bScaleImage, &dwSize);

	dwSize = sizeof(canny_th_cnt);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("CANNYTHCNT"), NULL, &dwType, (LPBYTE)&canny_th_cnt, &dwSize);

	dwSize = sizeof(canny_th_min);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("CANNYTHMIN"), NULL, &dwType, (LPBYTE)&canny_th_min, &dwSize);

	dwSize = sizeof(canny_th_max);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("CANNYTHMAX"), NULL, &dwType, (LPBYTE)&canny_th_max, &dwSize);

	dwSize = sizeof(apply_undistort);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("UNDISTORT"), NULL, &dwType, (LPBYTE)&apply_undistort, &dwSize);

	dwSize = sizeof(kernel_size);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("KERNELSIZE"), NULL, &dwType, (LPBYTE)&kernel_size, &dwSize);

	dwSize = sizeof(bTestHoughLine);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("HOUGHLINETEST"), NULL, &dwType, (LPBYTE)&bTestHoughLine, &dwSize);

	dwSize = sizeof(bApplyFFT);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("APPLYFFT"), NULL, &dwType, (LPBYTE)&bApplyFFT, &dwSize);
	
	dwSize = sizeof(bNewCanny);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("NEWCANNY"), NULL, &dwType, (LPBYTE)&bNewCanny, &dwSize);

	dwSize = sizeof(horiline_min_pixels);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("HORIMINPIXELS"), NULL, &dwType, (LPBYTE)&horiline_min_pixels, &dwSize);

	dwSize = sizeof(horiline_var_allow);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("HORIVARALLOW"), NULL, &dwType, (LPBYTE)&horiline_var_allow, &dwSize);

	dwSize = sizeof(vertline_min_pixels);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("VERTMINPIXELS"), NULL, &dwType, (LPBYTE)&vertline_min_pixels, &dwSize);

	dwSize = sizeof(vertline_var_allow);
	dwType = 0;
	RegQueryValueEx(hkey, TEXT("VERTVARALLOW"), NULL, &dwType, (LPBYTE)&vertline_var_allow, &dwSize);
	
	RegCloseKey(hkey);
}


void ReleaseScrollWnd(TCHAR* name);

int CNoteBuilderFrameApp::ExitInstance()
{
	ReleaseScrollWnd(NULL);

	return CWinAppEx::ExitInstance();
}
