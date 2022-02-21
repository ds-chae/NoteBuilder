// CurrencyInfoDlg.cpp : implementation file
//

#include "stdafx.h"

#include "B_Common.h"

#include "NoteBuilderFrame.h"
#include "ST150Image.h"
#include "ICUMANDoc.h"
#include "ICUMANView.h"
#include "CurrencyInfoDlg.h"
#include "CurrencyList.h"
#include "CurrencyInfo.h"
#include "SelectRegionDlg.h"
#include "MakeRefDataDlg.h"
#include "CodeParser.h"

TCHAR tszPathToExtract[MAX_PATH];
TCHAR tszExtToExtract[16];

char* szRecognitionTypes[] = {
	"RECOG_METHOD_V1",
	"RECOG_METHOD_V2",
	"RECOG_METHOD_V3",
	"RECOG_METHOD_V4",
	NULL
};

// CCurrencyInfoDlg dialog

IMPLEMENT_DYNAMIC(CCurrencyInfoDlg, CDialog)

CCurrencyInfoDlg::CCurrencyInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCurrencyInfoDlg::IDD, pParent)
{
	pViewWnd = (CICUMANView*)pParent;
	pImgST150 = new ST150Image();
	m_bContinue = 1;
}

CCurrencyInfoDlg::~CCurrencyInfoDlg()
{
	delete pImgST150;
}

void CCurrencyInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CURRENCY_LIST, m_ComboCurrencyList);
	DDX_Control(pDX, IDC_EDIT_CURCODE, m_EditCurCode);
	DDX_Control(pDX, IDC_EDIT_CURNUMBER, m_EditCurNumber);
	DDX_Control(pDX, IDC_EDIT_RECT_REG1, m_EditRectRegion1);
	DDX_Control(pDX, IDC_EDIT_DECI_REG1, m_EditDeciReg1);
	DDX_Control(pDX, IDC_EDIT_SUM_REG1, m_EditSumReg1);
	DDX_Control(pDX, IDC_EDIT_THRES_REG1, m_EditThresReg1);
	DDX_Control(pDX, IDC_EDIT_CURDENOMVALUES, m_EditDenomValues);
	DDX_Control(pDX, IDC_EDIT_CURVERSIONS, m_EditCurVersions);
	DDX_Control(pDX, IDC_EDIT_CURVERSIONS2, m_EditNoteSize);
	//	DDX_Control(pDX, IDC_EDIT_IMG_NUM, m_EditImgNum);
	DDX_Control(pDX, IDC_COMBO_REINFORCEMENT, m_ComboReinforcement);
	DDX_Control(pDX, IDC_COMBO_RECOG_METHOD, m_ComboRecogMethod);
}


BEGIN_MESSAGE_MAP(CCurrencyInfoDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_CURRENCY_LIST, &CCurrencyInfoDlg::OnCbnSelchangeComboCurrencyList)
	ON_BN_CLICKED(IDOK, &CCurrencyInfoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_DEF_REG1, &CCurrencyInfoDlg::OnBnClickedButtonDefReg1)
	ON_BN_CLICKED(IDC_BUTTON_TESTREF, &CCurrencyInfoDlg::OnBnClickedButtonTestref)
	ON_BN_CLICKED(IDC_BUTTON_LOADREF, &CCurrencyInfoDlg::OnBnClickedButtonLoadref)
	ON_BN_CLICKED(IDC_BUTTON_SEL1, &CCurrencyInfoDlg::OnBnClickedButtonSel1)
	ON_CBN_SELCHANGE(IDC_COMBO_REINFORCEMENT, &CCurrencyInfoDlg::OnCbnSelchangeComboReinforcement)
	ON_BN_CLICKED(IDC_BUTTON_KEY_FOLDER_1, &CCurrencyInfoDlg::OnBnClickedButtonKeyFolder1)
	ON_BN_CLICKED(IDC_BUTTON_GENKEYS_1, &CCurrencyInfoDlg::OnBnClickedButtonGenkeys1)
	ON_BN_CLICKED(IDC_BUTTON_MAKEREF_1, &CCurrencyInfoDlg::OnBnClickedButtonMakeref1)
	ON_BN_CLICKED(IDC_BUTTON_EXT_FOLDER, &CCurrencyInfoDlg::OnBnClickedButtonExtFolder)
	ON_BN_CLICKED(IDC_BUTTON_DST_FOLDER, &CCurrencyInfoDlg::OnBnClickedButtonDstFolder)
	ON_BN_CLICKED(IDC_BUTTON_EXTRACT_DATA, &CCurrencyInfoDlg::OnBnClickedButtonExtractData)
	ON_BN_CLICKED(IDC_BUTTON_GATHER_DATA, &CCurrencyInfoDlg::OnBnClickedButtonGatherData)
	ON_BN_CLICKED(IDC_BUTTON_EXECUTE_NNR, &CCurrencyInfoDlg::OnBnClickedButtonExecuteNnr)
	ON_BN_CLICKED(IDC_CHECK_REGION_SET, &CCurrencyInfoDlg::OnBnClickedCheckRegionSet)
	ON_BN_CLICKED(IDC_CHECK_VIA_DLL, &CCurrencyInfoDlg::OnBnClickedCheckViaDll)
	ON_CBN_SELCHANGE(IDC_COMBO_FUNCTION, &CCurrencyInfoDlg::OnCbnSelchangeComboFunction)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_RECOG_METHOD, &CCurrencyInfoDlg::OnBnClickedButtonSaveRecogMethod)
	ON_CBN_SELCHANGE(IDC_COMBO_RECOG_METHOD, &CCurrencyInfoDlg::OnCbnSelchangeComboRecogMethod)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_NNR, &CCurrencyInfoDlg::OnBnClickedButtonLoadNnr)
	ON_BN_CLICKED(IDC_CHECK_TEST_THRU_TESTDLL, &CCurrencyInfoDlg::OnBnClickedCheckTestThruTestdll)
	ON_BN_CLICKED(IDCANCEL, &CCurrencyInfoDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CCurrencyInfoDlg message handlers

void CCurrencyInfoDlg::OnCbnSelchangeComboCurrencyList()
{
	// TODO: Add your control notification handler code here
}

void CCurrencyInfoDlg::FillRegionSetOrDllFunctions()
{
	if(GetDlgItem(IDC_CHECK_VIA_DLL)->SendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED) {
		GetDlgItem(IDC_COMBO_FUNCTION)->SendMessage(CB_RESETCONTENT, 0, 0);
		GetDlgItem(IDC_COMBO_FUNCTION)->SendMessage(CB_ADDSTRING, 0, (LPARAM)L"WIN_CIS_Analysis");
		GetDlgItem(IDC_COMBO_FUNCTION)->SendMessage(CB_ADDSTRING, 0, (LPARAM)L"WIN_CIS_Verify");
		GetDlgItem(IDC_COMBO_FUNCTION)->SendMessage(CB_ADDSTRING, 0, (LPARAM)L"WIN_CIS_Result");
		GetDlgItem(IDC_COMBO_FUNCTION)->SendMessage(CB_ADDSTRING, 0, (LPARAM)L"WIN_MG_Result");
		GetDlgItem(IDC_COMBO_FUNCTION)->SendMessage(CB_ADDSTRING, 0, (LPARAM)L"WIN_UV_Result");
		GetDlgItem(IDC_COMBO_FUNCTION)->SendMessage(CB_SETCURSEL, 0, 0);
	} else {
		GetDlgItem(IDC_COMBO_FUNCTION)->SendMessage(CB_RESETCONTENT, 0, 0);
		for(int ri = 0; ri < currencyInfo.RegionSetCount; ri++) {
			REGION_SET *pSet = &currencyInfo.RegionSet[ri];
			TCHAR txtbuf[128];
			MultiByteToWideChar(CP_ACP, 0, pSet->ImgRegionName, strlen(pSet->ImgRegionName)+1, txtbuf, 128);
			GetDlgItem(IDC_COMBO_FUNCTION)->SendMessage(CB_ADDSTRING, 0, (LPARAM)txtbuf);
		}
		GetDlgItem(IDC_COMBO_FUNCTION)->SendMessage(CB_SETCURSEL, 0, 0);
	}
}

void CCurrencyInfoDlg::SetDialogItems()
{
	m_ComboReinforcement.SetCurSel(gst_State.pCurrentLocal->ImageAverageReinforce);

	TCHAR str[128];
	ST_IMG_REGION *pReg = &pWinCurrentLocal->Region_Value[0];
	wsprintf(str, L"%d,%d,%d,%d", pReg->Region.Sx, pReg->Region.Sy, pReg->Region.Ex, pReg->Region.Ey);
	m_EditRectRegion1.SetWindowTextW(str);

	wsprintf(str, L"%d,%d", pReg->Shift_X, pReg->Shift_Y);
	m_EditDeciReg1.SetWindowTextW(str);
	wsprintf(str, L"%d,%d", pReg->Sum_X, pReg->Sum_Y);
	m_EditSumReg1.SetWindowTextW(str);
	swprintf_s(str, 128, L"%5.3f", pReg->Threshold);
	m_EditThresReg1.SetWindowTextW(str);

	SetDlgItemText(IDC_EDIT_KEY_FOLDER_1, m_tszKeyFolder1);
	SetDlgItemText(IDC_EDIT_KEY_FOLDER_2, m_tszKeyFolder2);

	TCHAR tszBuf[128]; tszBuf[0] = 0;
	for(int i = 0; i < currencyInfo.MaxDenom; i++) {
		TCHAR numstr[16];
		wsprintf(numstr, L"%d", currencyInfo.denomValue[i]);
		if(tszBuf[0]) lstrcat(tszBuf, L",");
		lstrcat(tszBuf, numstr);
	}
	this->m_EditDenomValues.SetWindowTextW(tszBuf);

	tszBuf[0] = 0;
	for(int i = 0; i < currencyInfo.MaxDenom; i++) {
		TCHAR numstr[32];
		swprintf_s(numstr, 32, L"%.1f,%.1f", currencyInfo.CUR_MM_X[i], currencyInfo.CUR_MM_Y[i]);
		if(tszBuf[0]) lstrcat(tszBuf, L", ");
		lstrcat(tszBuf, numstr);
	}
	this->m_EditNoteSize.SetWindowTextW(tszBuf);

	GetDlgItem(IDC_CHECK_REGION_SET)->SendMessage(BM_SETCHECK, BST_CHECKED, 0);
	FillRegionSetOrDllFunctions();
}

BOOL CCurrencyInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	TCHAR wCurCode[8];
	if(pWinCurrentLocal != NULL) {
		MultiByteToWideChar(CP_ACP, 0, pWinCurrentLocal->szCurrency, 4, wCurCode, 8);
	} else {
		wCurCode[0] = 0;
	}

	int FoundIndex = -1;
	for(int i = 0; i < CurrencyList.currency_count; i++) {
		TCHAR text[128];
		wsprintf(text, L"%s %s,%s,%s",
			CurrencyList.CurrencyTable[i].CurrencyCode,
			CurrencyList.CurrencyTable[i].CurrencyNumber,
			CurrencyList.CurrencyTable[i].CurrencyName,
			CurrencyList.CurrencyTable[i].CountryName);
		m_ComboCurrencyList.AddString(text);
		text[3] = 0;
		if(lstrcmp(text, wCurCode) == 0) 
		{
			FoundIndex = i;
			break;
		}
	}

	m_EditCurCode.SetWindowTextW(wCurCode);
	if(FoundIndex != -1) {
		m_ComboCurrencyList.SetCurSel(FoundIndex);
		m_EditCurNumber.SetWindowTextW(CurrencyList.CurrencyTable[FoundIndex].CurrencyNumber);
	} else {
		TCHAR str[16];
		wsprintf(str, L"%d", pWinCurrentLocal->CurrencyNum);
		m_EditCurNumber.SetWindowTextW(str);
	}

	m_ComboReinforcement.AddString(L"0");
	m_ComboReinforcement.AddString(L"1");
	m_ComboReinforcement.AddString(L"2");

	m_ComboRecogMethod.AddString(L"Undefined");
	char** sp = szRecognitionTypes;
	while(*sp != NULL) {
		CString tmp(*sp);
		m_ComboRecogMethod.AddString((LPCTSTR)tmp);
		sp++;
	}
	m_ComboRecogMethod.SetCurSel(pWinCurrentLocal->RecognitionType);

	SetDialogItems();

	if(bTestThruTestDLL == 1)
		GetDlgItem(IDC_CHECK_TEST_THRU_TESTDLL)->SendMessage(BM_SETCHECK, BST_CHECKED, 0);
	else
		GetDlgItem(IDC_CHECK_TEST_THRU_TESTDLL)->SendMessage(BM_SETCHECK, 0, 0);

	GetDlgItem(IDC_EDIT_NNR_LOADED)->SetWindowTextW(bNNRLoaded ? L"Loaded" : L"Not loaded yet");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CCurrencyInfoDlg::UpdateCurrencyInfo()
{
	TCHAR str[128];

	if(pWinCurrentLocal != NULL) {
		ST_IMG_REGION *pReg = &pWinCurrentLocal->Region_Value[0];

		m_EditRectRegion1.GetWindowTextW(str, 128);
		ScanSquareValuesW(str, &pReg->Region);

		m_EditDeciReg1.GetWindowTextW(str, 128);
		ScanDecimationW(str, pReg);

		m_EditSumReg1.GetWindowTextW(str, 128);
		ScanSummationW(str, pReg);
		
		m_EditThresReg1.GetWindowTextW(str, 128);
		pReg->Threshold = _wtof(str);
	}
}

void CCurrencyInfoDlg::OnBnClickedOk()
{
	UpdateCurrencyInfo();

	OnOK();
}

void CCurrencyInfoDlg::OnBnClickedButtonDefReg1()
{
	if(pWinCurrentLocal == NULL)
		return;
	
	currencyInfo.SetRegionDefault(&pWinCurrentLocal->Region_Value[0]);
	SetDialogItems();
}

void CCurrencyInfoDlg::TestKeysOnPath(TCHAR* path)
{
/*
	int filecount = 0;
#define	MAX_FLIST	4000
	TCHAR *flist[MAX_FLIST];

	WIN32_FIND_DATA wfd;
	TCHAR FindPath[MAX_PATH];
	wsprintf(FindPath, TEXT("%s\\*.*"), path);
	
	HANDLE hFind = FindFirstFile(FindPath, &wfd);
	while(hFind) {
		if(lstrcmp(wfd.cFileName, L".") && lstrcmp(wfd.cFileName, L"..")) {
			TCHAR SubPath[MAX_PATH];
			wsprintf(SubPath, L"%s\\%s", path, wfd.cFileName);
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				TestKeysOnPath(SubPath);
			} else {
				if(filecount < MAX_FLIST) {
					flist[filecount] = (TCHAR*)malloc(sizeof(SubPath));
					lstrcpy(flist[filecount], SubPath);
					filecount++;
				}
			}
		}
		if(!FindNextFile(hFind, &wfd)) {
			FindClose(hFind);
			hFind = NULL;
			break;
		}
	}
	for(int fi = 0; fi < filecount; fi++) {
		TestKeysOnFile(flist[fi]);
		free(flist[fi]);
	}
*/
}

void CCurrencyInfoDlg::OnBnClickedButtonTestref()
{
	UpdateCurrencyInfo();

	TCHAR wpath[MAX_PATH];
	wsprintf(wpath, L"%s\\%s", m_CurrencyImageFolder, pWinCurrentLocal->tszCurrency);

	TestKeysOnPath(wpath);
}


TCHAR CurImagePath[];

void CCurrencyInfoDlg::OnBnClickedButtonLoadref()
{
	currencyInfo.LoadCurrencyConfiguration(this, CurImagePath);
}

void CCurrencyInfoDlg::OnBnClickedButtonSel1()
{
	TCHAR tszBuf[128];
	CSelectRegionDlg dlg;
	if(dlg.DoModal() == IDOK) {
		ST_IMG_REGION *pRegion = currencyInfo.FindRegionByIndex(dlg.m_SelectedRegion);
		if(pRegion != NULL) {
			swprintf_s(tszBuf, 128, L"%d,%d %d,%d", pRegion->Region.Sx, pRegion->Region.Sy, pRegion->Region.Ex, pRegion->Region.Ey);
			m_EditRectRegion1.SetWindowTextW(tszBuf);
			swprintf_s(tszBuf, 128, L"%d,%d", pRegion->Shift_X, pRegion->Shift_Y);
			m_EditDeciReg1.SetWindowTextW(tszBuf);
			swprintf_s(tszBuf, 128, L"%d,%d", pRegion->Sum_X, pRegion->Sum_Y);
			m_EditSumReg1.SetWindowTextW(tszBuf);
		}
	}
}

void CCurrencyInfoDlg::OnCbnSelchangeComboReinforcement()
{
	// 인식을 위한 샘플의 이미지 향상 여부
	gst_State.pCurrentLocal->ImageAverageReinforce = m_ComboReinforcement.GetCurSel();
}

BOOL BrowseFolder(HWND hWnd, TCHAR *pszPathname, LPARAM lParam, DWORD flag);
//extern TCHAR tszKeyFolder[MAX_PATH];

void CCurrencyInfoDlg::OnBnClickedButtonKeyFolder1()
{
	TCHAR tszKeyFolder[MAX_PATH];
	GetDlgItemText(IDC_EDIT_KEY_FOLDER_1, tszKeyFolder, MAX_PATH);
	if(BrowseFolder(GetSafeHwnd(), tszKeyFolder, (LPARAM)m_tszKeyFolder1, BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_NEWDIALOGSTYLE)) {
		lstrcpy(m_tszKeyFolder1, tszKeyFolder);
		SetDlgItemText(IDC_EDIT_KEY_FOLDER_1, m_tszKeyFolder1);
		RegSetNNRFolder();
	}
}

void CCurrencyInfoDlg::OnBnClickedButtonGenkeys1()
{
	UpdateCurrencyInfo();

	TCHAR wpath[MAX_PATH];
	wsprintf(wpath, L"%s\\%s", m_CurrencyImageFolder, pWinCurrentLocal->tszCurrency);

	TCHAR tszKeyFolder[MAX_PATH];
	GetDlgItemText(IDC_EDIT_KEY_FOLDER_1, tszKeyFolder, MAX_PATH);
	WIN32_FIND_DATA wfd;
	HANDLE h = FindFirstFile(tszKeyFolder, &wfd);
	if(h == INVALID_HANDLE_VALUE) {
		MessageBox(L"Key folder must be specified or exist.");
		return;
	}
	FindClose(h);

	if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		MessageBox(L"Specified path is not a folder.");
		return;
	}

	MessageBox(L"Key generation finished.");
}

void CCurrencyInfoDlg::OnBnClickedButtonMakeref1()
{
	CMakeRefDataDlg *pDlg = new CMakeRefDataDlg(this);
	pDlg->DoModal();
	delete pDlg;
}

void CCurrencyInfoDlg::OnBnClickedButtonExtFolder()
{
	TCHAR tszExtFolder[MAX_PATH];
	GetDlgItemText(IDC_EDIT_EXT_FOLDER, tszExtFolder, MAX_PATH);
	if(BrowseFolder(GetSafeHwnd(), tszExtFolder, (LPARAM)m_tszExtFolder, BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_NEWDIALOGSTYLE)) {
		lstrcpy(m_tszExtFolder, tszExtFolder);
		SetDlgItemText(IDC_EDIT_EXT_FOLDER, m_tszExtFolder);
		RegSetNNRFolder();
	}
}


void CCurrencyInfoDlg::OnBnClickedButtonDstFolder()
{
	TCHAR tszDstFolder[MAX_PATH];
	GetDlgItemText(IDC_EDIT_DST_FOLDER, tszDstFolder, MAX_PATH);
	if(BrowseFolder(GetSafeHwnd(), tszDstFolder, (LPARAM)m_tszDstFolder, BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_NEWDIALOGSTYLE)) {
		lstrcpy(m_tszDstFolder, tszDstFolder);
		SetDlgItemText(IDC_EDIT_DST_FOLDER, m_tszDstFolder);
		RegSetNNRFolder();
	}
}

void CCurrencyInfoDlg::OnBnClickedButtonExtractData()
{
	UpdateCurrencyInfo();

	TCHAR wpath[MAX_PATH];
	wsprintf(wpath, L"%s\\%s", m_CurrencyImageFolder, pWinCurrentLocal->tszCurrency);

	TCHAR tszExtFolder[MAX_PATH];
	GetDlgItemText(IDC_EDIT_EXT_FOLDER, tszExtFolder, MAX_PATH);
	WIN32_FIND_DATA wfd;
	HANDLE h = FindFirstFile(tszExtFolder, &wfd);
	if(h == INVALID_HANDLE_VALUE) {
		MessageBox(L"Extracted data folder must be specified or exist.");
		return;
	}
	FindClose(h);

	if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		MessageBox(L"Specified path is not a folder.");
		return;
	}

	TCHAR tszPrefix[16];
	GetDlgItemText(IDC_EDIT_PREFIX, tszPrefix, 16);
	StrTrim(tszPrefix, TEXT(" ")); // 마지막에 블랭크가 딱 하나 들어가도록 한다.
	lstrcat(tszPrefix, TEXT(" "));
	SetDlgItemText(IDC_EDIT_PREFIX, tszPrefix);

	GetDlgItemText(IDC_EDIT_EXTENSION, tszExtToExtract, 16);
	tszExtToExtract[6] = 0;
	StrTrim(tszExtToExtract, TEXT(" "));
	SetDlgItemText(IDC_EDIT_EXTENSION, tszExtToExtract);

	if(tszPrefix[0] == 0 || tszExtToExtract[0] == 0) {
		MessageBox(L"Prefix and Extension should be specified.");
	}

	tszPathToExtract[0] = 0; // 데이터 추출 중인지 아닌지 표시
	if(GetDlgItem(IDC_CHECK_VIA_DLL)->SendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED) {
	int function = GetDlgItem(IDC_COMBO_FUNCTION)->SendMessage(LB_GETSEL);
		ExtractDataOnPath(function, NULL, pViewWnd, wpath, tszExtFolder, tszPrefix);
	} else {
		TCHAR regionName[MAX_PATH];
		GetDlgItem(IDC_COMBO_FUNCTION)->GetWindowTextW(regionName, MAX_PATH);
		ExtractDataOnPath(-1, regionName, pViewWnd, wpath, tszExtFolder, tszPrefix);
	}
	tszPathToExtract[0] = 0; // 데이터 추출 끝났슴

	MessageBox(L"Extracting data finished.");
}

void CCurrencyInfoDlg::ExtractDataOnPath(int function, TCHAR* tszRegion, CICUMANView* pViewWnd, TCHAR* path, TCHAR* keypath, TCHAR* tszPrefix)
{
	int filecount = 0;
#define	MAX_FLIST	4000
	TCHAR *flist[MAX_FLIST];
	TCHAR *keylist[MAX_FLIST];

	WIN32_FIND_DATA wfd;
	TCHAR FindPath[MAX_PATH];
	wsprintf(FindPath, TEXT("%s\\*.*"), path);
	
	HANDLE hFind = FindFirstFile(FindPath, &wfd);
	while(hFind) {
		if(lstrcmp(wfd.cFileName, L".") && lstrcmp(wfd.cFileName, L"..")) {
			TCHAR SubPath[MAX_PATH];
			TCHAR SubKeyPath[MAX_PATH];
			wsprintf(SubPath, L"%s\\%s", path, wfd.cFileName);
			wsprintf(SubKeyPath, L"%s\\%s", keypath, wfd.cFileName);
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				CreateDirectory(SubKeyPath, NULL);
				ExtractDataOnPath(function, tszRegion, pViewWnd, SubPath, SubKeyPath, tszPrefix);
			} else {
				if(StrStr(wfd.cFileName, tszPrefix)) {
					// data prefix 관련 정보는 무조건 복사한다
					CopyFile(SubPath, SubKeyPath, FALSE);
				} else {
					if(filecount < MAX_FLIST) {
						flist[filecount] = (TCHAR*)malloc(sizeof(SubPath));
						lstrcpy(flist[filecount], SubPath);
						keylist[filecount] = (TCHAR*)malloc(sizeof(SubKeyPath));
						lstrcpy(keylist[filecount], SubKeyPath);
						filecount++;
					}
				}
			}
		}

		if(!FindNextFile(hFind, &wfd)) {
			FindClose(hFind);
			hFind = NULL;
			break;
		}
	}
	for(int fi = 0; fi < filecount; fi++) {
//		ExtractDataOnFile(function, tszRegion, pViewWnd, flist[fi], keylist[fi]);
		free(flist[fi]);
		free(keylist[fi]);
	}
}

void SaveToBitmap(TCHAR* path, unsigned char* buf, int x_size, int y_size);
extern "C" int Size_X[8], Size_Y[8]; // 이 변수들은 D_CIS_Extraction.c 파일에 있다.

BOOL FileExist(TCHAR* fn, WIN32_FIND_DATA* pwfd);

void CCurrencyInfoDlg::CopyExtractedFileToNNRFolder(TCHAR* currentRefIndex, TCHAR* subPath, TCHAR* subFile, TCHAR* tszRefFolder)
{
	WIN32_FIND_DATA wfd;

	if(currentRefIndex[0] == 0) {
		MessageBox(L"DENOM index is not found.");
		m_bContinue = 0;
		return;
	}

	// 대상 폴더가 없으면 생성한다.
	TCHAR dstDirName[MAX_PATH];
	wsprintf(dstDirName, L"%s\\%s", tszRefFolder, currentRefIndex);
	if(!FileExist(dstDirName, &wfd))
		CreateDirectory(dstDirName, NULL);
	if(!FileExist(dstDirName, &wfd)) {
		TCHAR msg[MAX_PATH];
		wsprintf(msg, L"Cannot create %s", tszRefFolder);
		MessageBox(msg);
		m_bContinue = 0;
		return;
	}
	
	// 이름이 겹치지 않는 이름을 찾는다.
	int seq = 0;
	TCHAR dstFileName[MAX_PATH];
	for(seq = 0; seq < 10000; seq++) {
		wsprintf(dstFileName, L"%s\\%s\\%04d_%s", tszRefFolder, currentRefIndex, seq, subFile);
		if(!FileExist(dstFileName, &wfd))
			break;
	}
	if(seq >= 10000) {
		TCHAR msg[MAX_PATH];
		wsprintf(msg, L"Cannot find subsequent filename in %s\\%s.", tszRefFolder, currentRefIndex);
		MessageBox(msg);
		m_bContinue = 0;
		return;
	}
	
	// 파일 이름을 찾았으니 사용하자.
	TCHAR srcPath[MAX_PATH];
	wsprintf(srcPath, L"%s\\%s", subPath, subFile);
	CopyFile(srcPath, dstFileName, FALSE);

	if(srcPath[lstrlen(srcPath)-4] == '.') {
		lstrcpy(srcPath+lstrlen(srcPath)-4, L".bmp");
		lstrcpy(dstFileName+lstrlen(dstFileName)-4, L".bmp");
		CopyFile(srcPath, dstFileName, FALSE);
	}
}


void CCurrencyInfoDlg::CopyExtractFolderToNNRFolder(TCHAR* RefIndex, TCHAR* tszExtFolder, TCHAR* tszNNRFolder)
{
	TCHAR tempPath[MAX_PATH];
	wsprintf(tempPath, L"%s\\CLS *", tszExtFolder);
	WIN32_FIND_DATA wfd;
	
	// 폴더 밑에 있는 CLS ##.txt 파일을 찾는다.
	TCHAR currentRefIndex[10];
	currentRefIndex[0] = 0;
	HANDLE hFind = FindFirstFile(tempPath, &wfd);
	if(hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		TCHAR* cp = wfd.cFileName;
		while(*cp && *cp < '0' || *cp > '9') cp++;
		int i = 0;
		while(*cp >= '0' && *cp <= '9' && i < 3) {
			currentRefIndex[i] = *cp++;
			i++;
		}
		currentRefIndex[i] = 0;
	}
	
	// 못 찾았으면 상위 인덱스를 이용한다.
	if(currentRefIndex[0] == 0) {
		if(RefIndex != NULL)
			lstrcpy(currentRefIndex, RefIndex);
	}

	// 자 이제 파일을 나머지 찾도록 한다.
	wsprintf(tempPath, L"%s\\*", tszExtFolder);
	hFind = FindFirstFile(tempPath, &wfd);
	while(hFind != INVALID_HANDLE_VALUE) {
		TCHAR dotExt[16];
		lstrcpy(dotExt, L".");
		lstrcat(dotExt, tszExtToExtract);
		int len = lstrlen(wfd.cFileName);
		int elen = lstrlen(dotExt);
		if(lstrcmp(wfd.cFileName+len-elen, dotExt) == 0) {
			// 아직까지 인덱스가 없다면 오류를 알리고 종료한다.
			if(currentRefIndex[0] == 0) {
				TCHAR msg[MAX_PATH];
				wsprintf(msg, L"Following path requires class index file, CLS ##.txt\r\n%s", tszExtFolder);
				MessageBox(msg);
				m_bContinue = 0;
				break;
			}
			CopyExtractedFileToNNRFolder(currentRefIndex, tszExtFolder, wfd.cFileName, tszNNRFolder);
		} else {
			if(lstrcmp(wfd.cFileName, L".") && lstrcmp(wfd.cFileName, L"..")) {
				if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					TCHAR subPath[MAX_PATH];
					wsprintf(subPath, L"%s\\%s", tszExtFolder, wfd.cFileName);
					CopyExtractFolderToNNRFolder(currentRefIndex, subPath, tszNNRFolder);
				}
			}
		}
		if(!FindNextFile(hFind, &wfd))
			break;
		if(!m_bContinue)
			break;
	}
	FindClose(hFind);
}

void CCurrencyInfoDlg::OnBnClickedButtonGatherData()
{
	GetDlgItemText(IDC_EDIT_EXT_FOLDER, m_tszExtFolder, MAX_PATH);
	GetDlgItemText(IDC_EDIT_DST_FOLDER, m_tszDstFolder, MAX_PATH);

	GetDlgItemText(IDC_EDIT_EXTENSION, tszExtToExtract, 16);
	tszExtToExtract[6] = 0;
	StrTrim(tszExtToExtract, TEXT(" "));
	SetDlgItemText(IDC_EDIT_EXTENSION, tszExtToExtract);

	m_bContinue = 1;
	CreateDirectory(m_tszDstFolder, NULL);
	CopyExtractFolderToNNRFolder(L"", m_tszExtFolder, 	m_tszDstFolder);

	MessageBox(L"Gathering data finished");
}

void CCurrencyInfoDlg::OnBnClickedButtonExecuteNnr()
{
	while(1) {
		if(m_tszNNRExeFile[0]) {
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			memset(&si, 0, sizeof(si));
			memset(&pi, 0, sizeof(pi));
			si.cb = sizeof(si);
			memset(&pi, 0, sizeof(pi));
			if(CreateProcess(m_tszNNRExeFile, L"", NULL, NULL, FALSE, 0, NULL, L".", &si, &pi))
				return;
		}

		CString str = L"exe 파일(*.exe)|*.exe|";
		CFileDialog Dlg(TRUE, L"exe", NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, str, NULL);
		if(Dlg.DoModal() == IDOK)
		{
			lstrcpy(m_tszNNRExeFile, Dlg.GetFileName());
			RegSetNNRFolder();
		} else {
			break;
		}
	}
}

void CCurrencyInfoDlg::OnBnClickedCheckRegionSet()
{
	if(GetDlgItem(IDC_CHECK_REGION_SET)->SendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED) {
		GetDlgItem(IDC_CHECK_VIA_DLL)->SendMessage(BM_SETCHECK, 0, 0);
		FillRegionSetOrDllFunctions();
	}
}

void CCurrencyInfoDlg::OnBnClickedCheckViaDll()
{
	if(GetDlgItem(IDC_CHECK_VIA_DLL)->SendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED) {
		GetDlgItem(IDC_CHECK_REGION_SET)->SendMessage(BM_SETCHECK, 0, 0);
		FillRegionSetOrDllFunctions();
	}
}


void CCurrencyInfoDlg::OnCbnSelchangeComboFunction()
{
	int index = 0;
	TCHAR tszBuf[128];
	CSelectRegionDlg dlg;


	CComboBox* test = ((CComboBox*)GetDlgItem(IDC_COMBO_FUNCTION));
	index = test->GetCurSel();
	ST_IMG_REGION *pRegion = currencyInfo.FindRegionByIndex(index);
	if(pRegion != NULL) 
	{		
	}	
}

#define	TXT_BUFSIZ	16384
static char dstbuf[2048];
static char srcbuf[2048];

void CCurrencyInfoDlg::OnBnClickedButtonSaveRecogMethod()
{
	if(pWinCurrentLocal == NULL)
		return;
	if(pWinCurrentLocal->RecognitionType <= 0)
		return;
	
	char** sp = szRecognitionTypes;

	char txt1[128];
	sprintf_s(txt1, sizeof(txt1),
			"#define\t%s_RECOGNITON_TYPE\t%s\r\n",
			pWinCurrentLocal->szCurrency,
			szRecognitionTypes[pWinCurrentLocal->RecognitionType-1]);

	strcpy_s(dstbuf, sizeof(dstbuf), srcbuf);

	char tag1[128];

	sprintf_s(tag1, 128, "#define\t%s_RECOGNITON_TYPE", pWinCurrentLocal->szCurrency);

	ReplaceTagText(dstbuf, sizeof(dstbuf), tag1, txt1);

	TCHAR backFile[MAX_PATH];
	wsprintf(backFile, TEXT("%s.bak"), CUR_FunctionsPath);
	MoveFile(CUR_FunctionsPath, backFile);
	
	HANDLE h = CreateFile(CUR_FunctionsPath, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h != INVALID_HANDLE_VALUE) {
		DWORD dwio = 0;
		WriteFile(h, dstbuf, strlen(dstbuf), &dwio, NULL);
		CloseHandle(h);
		MessageBox(CUR_FunctionsPath, TEXT("Recognition type was saved to..."), MB_OK);

		goto _return;
	}

	MessageBox(CUR_FunctionsPath, TEXT("Failed to write to..."), MB_OK);

_return:
	;

	// TODO: Add your control notification handler code here
}

void CCurrencyInfoDlg::OnCbnSelchangeComboRecogMethod()
{
	if(pWinCurrentLocal == NULL)
		return;

	pWinCurrentLocal->RecognitionType = m_ComboRecogMethod.GetCurSel(); // 0 -> Undefined...
	gst_State.pCurrentLocal->RecognitionType = pWinCurrentLocal->RecognitionType;
}

void CCurrencyInfoDlg::OnBnClickedButtonLoadNnr()
{
	currencyInfo.LoadNNRData(this);
	bNNRLoaded = 1;
	GetDlgItem(IDC_EDIT_NNR_LOADED)->SetWindowTextW(L"Loaded");
}

void CCurrencyInfoDlg::OnBnClickedCheckTestThruTestdll()
{
	if(GetDlgItem(IDC_CHECK_TEST_THRU_TESTDLL)->SendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED)
		bTestThruTestDLL = 1;
	else
		bTestThruTestDLL = 0;
	
	RegSetNNRFolder();
}

void CCurrencyInfoDlg::OnBnClickedCancel()
{
	OnCancel();
}
