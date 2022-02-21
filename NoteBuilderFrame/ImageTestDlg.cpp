// ImageTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NoteBuilderFrame.h"

#include "ICUMANDoc.h"
#include "Define.h"
#include "opencv_use.h"
#include "CarNumDetect.h"
#include "ICUMANView.h"
#include "ImageTestDlg.h"

CImageTestDlg* theImageTester = NULL;

extern int apply_undistort;
extern cv::Rect cropRect;
extern cv::Rect rectRects[];
extern double distortCoeffk1;
extern double distortCoeffk2;
extern double distortCoeffk3;
extern double focal_length;

extern int bCropImage;
extern int bScaleImage;

extern int min_char_height;
extern int max_char_height;

extern int horiline_min_pixels;  //수평선 최소에지 픽셀수
extern int horiline_var_allow;   // 수평선 분산 기준값
extern int vertline_min_pixels;  // 수직선 최소에지 픽셀수 
extern int vertline_var_allow;   // 수직선 분산 기준값..


// CImageTestDlg dialog

IMPLEMENT_DYNAMIC(CImageTestDlg, CDialog)

CImageTestDlg::CImageTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImageTestDlg::IDD, pParent)
{
	pWndView = (CICUMANView*)pParent;
	theImageTester = this;
}

CImageTestDlg::~CImageTestDlg()
{
}

void CImageTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PLATE_TYPE, m_PlateType);
	DDX_Control(pDX, IDC_BLUR_TYPE, m_BlurType);
	DDX_Control(pDX, IDC_IMG_SELECT, m_imgSelect);
	DDX_Control(pDX, IDC_IMG_TEXT, m_ImgText);
	DDX_Control(pDX, IDC_SELECTED_RECT, m_SelectedRect);
	DDX_Control(pDX, IDC_EDGES, m_Edges);
	DDX_Control(pDX, IDC_EQUIDIST_METHOD, m_EquidistMethod);
}


BEGIN_MESSAGE_MAP(CImageTestDlg, CDialog)
	ON_BN_CLICKED(IDC_CLOSE, &CImageTestDlg::OnBnClickedClose)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_APP_PIXELS2, &CImageTestDlg::OnBnClickedButtonAppPixels2)
	ON_BN_CLICKED(IDC_BUTTON_APP_PERCENT, &CImageTestDlg::OnBnClickedButtonAppPercent)
	ON_BN_CLICKED(IDC_BUTTON_APP_THRESHOLD, &CImageTestDlg::OnBnClickedButtonAppThreshold)
	ON_BN_CLICKED(IDC_RESET, &CImageTestDlg::OnBnClickedReset)
	ON_BN_CLICKED(IDC_CHECK_UNDISTORT, &CImageTestDlg::OnBnClickedCheckUndistort)
	ON_CBN_SELCHANGE(IDC_PLATE_TYPE, &CImageTestDlg::OnCbnSelchangePlateType)
	ON_CBN_SELCHANGE(IDC_BLUR_TYPE, &CImageTestDlg::OnCbnSelchangeBlurType)
	ON_BN_CLICKED(IDC_ENABLE_CROP, &CImageTestDlg::OnBnClickedEnableCrop)
	ON_BN_CLICKED(IDC_ENABLE_SCALE, &CImageTestDlg::OnBnClickedEnableScale)
	ON_BN_CLICKED(IDC_BROWSE_NNR_FOLDER, &CImageTestDlg::OnBnClickedBrowseNnrFolder)
	ON_CBN_SELCHANGE(IDC_IMG_SELECT, &CImageTestDlg::OnCbnSelchangeImgSelect)
	ON_BN_CLICKED(IDC_IMG_SAVE, &CImageTestDlg::OnBnClickedImgSave)
	ON_BN_CLICKED(IDC_TEST_TREE, &CImageTestDlg::OnBnClickedTestTree)
	ON_BN_CLICKED(IDC_TEST_SELECTION, &CImageTestDlg::OnBnClickedTestSelection)
	ON_CBN_SELCHANGE(IDC_EDGES, &CImageTestDlg::OnCbnSelchangeEdges)
	ON_BN_CLICKED(IDC_TEST_EDGES, &CImageTestDlg::OnBnClickedTestEdges)
	ON_CBN_SELCHANGE(IDC_EQUIDIST_METHOD, &CImageTestDlg::OnCbnSelchangeEquidistMethod)
	ON_BN_CLICKED(IDC_MARK_THE_EDGE, &CImageTestDlg::OnBnClickedMarkTheEdge)
	ON_BN_CLICKED(IDC_APPLY_FFT, &CImageTestDlg::OnBnClickedApplyFft)
	ON_BN_CLICKED(IDC_NEW_CANNY, &CImageTestDlg::OnBnClickedNewCanny)
END_MESSAGE_MAP()


// CImageTestDlg message handlers

void CImageTestDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	delete theImageTester;
	theImageTester = NULL;
}


void CImageTestDlg::OnBnClickedClose()
{
	DestroyWindow();
}

void CImageTestDlg::OnClose()
{
	CDialog::OnClose();

	DestroyWindow();
}

void CImageTestDlg::OnBnClickedButtonAppPixels2()
{
	// TODO: Add your control notification handler code here
}

extern int m_nSCR_WIDTH;// = 960; //640;//960;//640;   
extern int m_nSCR_HEIGHT;// = 669;//446;//669;//446;
extern int kernel_size;

void CannyThreshold(TCHAR* window_name, Mat& src, Mat& src_gray, Mat& dst);
void CannyThreshold();
void CannyCroppedImage();

TCHAR *IntArrToStr(TCHAR* tbuf, int arr[], int cnt)
{
	tbuf[0] = 0;
	for(int i = 0; i < cnt; i++) {
		wsprintf(tbuf+lstrlen(tbuf), L"%d", arr[i]);
		if(i != cnt-1)
			lstrcat(tbuf, L",");
	}

	return tbuf;
}

int StrToIntArr(TCHAR* tbuf, int arr[], int arr_size)
{
	int v = 0;
	int n = 0;
	while(1) {
		if(*tbuf >= '0' && *tbuf <= '9') {
			v = v * 10 + (*tbuf - '0');
		} else {
			arr[n++] = v;
			v = 0;
			if(n >= arr_size)
				break;
		}
		if(*tbuf == 0)
			break;
		tbuf++;
	}

	return n;
}

void CImageTestDlg::OnBnClickedButtonAppPercent()
{
	GetDataFromControl();

	OnBnClickedApplyUndistort();

	RegSetOptions();

	if(pMainView != NULL)
		pMainView ->ApplyThreshold(NULL, NULL);
}

BOOL CImageTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	TCHAR tbuf[128];
	IntArrToStr(tbuf, canny_th_min, canny_th_cnt);
	SetDlgItemText(IDC_TH_MIN, tbuf);
	IntArrToStr(tbuf, canny_th_max, canny_th_cnt);
	SetDlgItemText(IDC_TH_MAX, tbuf);

	ResetCoeffs();

	m_PlateType.AddString(L"3 Cars");	// 0
	m_PlateType.AddString(L"Han's plate");
	m_PlateType.AddString(L"Fill edge");
	m_PlateType.AddString(L"Edge Lines");
	// HDS-TEST-140926	
	m_PlateType.AddString(L"Han's Edge Lines"); // 4
	m_PlateType.AddString(L"Grid"); // 5

	m_PlateType.SetCurSel(nPlateType);

	m_BlurType.AddString(L"Normal");
	m_BlurType.AddString(L"Gaussian");
	m_BlurType.AddString(L"All");
	m_BlurType.SetCurSel(nBlurType);

	SetDlgItemText(IDC_TH_MIN, IntArrToStr(tbuf, canny_th_min, canny_th_cnt));
	SetDlgItemText(IDC_TH_MAX, IntArrToStr(tbuf, canny_th_max, canny_th_cnt));

	SetDlgItemInt(IDC_SCALE_WIDTH, m_nSCR_WIDTH);// = 960; //640;//960;//640;   
	SetDlgItemInt(IDC_SCALE_HEIGHT, m_nSCR_HEIGHT);// = 669;//446;//669;//446;
	GetDlgItem(IDC_ENABLE_SCALE)->SendMessage(BM_SETCHECK, bScaleImage ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_TEST_HOUGH_LINE)->SendMessage(BM_SETCHECK, bTestHoughLine ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_APPLY_FFT)->SendMessage(BM_SETCHECK, bApplyFFT ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_NEW_CANNY)->SendMessage(BM_SETCHECK,bNewCanny ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemInt(IDC_KERNEL_SIZE, kernel_size);
	SetDlgItemText(IDC_NNR_FOLDER, RegNNRFolder);

	m_EquidistMethod.AddString(L"Old");
	m_EquidistMethod.AddString(L"New");
	m_EquidistMethod.SetCurSel(equitest_method);

	SetDlgItemInt(IDC_MAX_CHAR_HEIGHT, max_char_height);

	SetDlgItemInt(IDC_HORILINE_MIN_PIXELS, horiline_min_pixels );
	SetDlgItemInt(IDC_HORILINE_VAR_ALLOW, horiline_var_allow );
	SetDlgItemInt(IDC_VERTLINE_MIN_PIXELS, vertline_min_pixels );
	SetDlgItemInt(IDC_VERTLINE_VAR_ALLOW, vertline_var_allow );

	SetDlgItemInt(IDC_VAR_LIMIT, var_limit);
	SetDlgItemInt(IDC_GRID_H, grid_cell_height);
	SetDlgItemInt(IDC_GRID_W, grid_cell_width);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CImageTestDlg::ResetCoeffs()
{
	SetDlgItemText(IDC_EDIT_K1, L"-4.1802327176423804e-001");
	SetDlgItemText(IDC_EDIT_K2, L"5.0715244063187526e-001");
	SetDlgItemText(IDC_EDIT_K3, L"-5.7843597214487474e-001");
	SetDlgItemText(IDC_EDIT_FOCUS, L"6.5746697944293521e+002");
	GetDlgItem(IDC_CHECK_UNDISTORT)->SendMessage(BM_SETCHECK, apply_undistort ? BST_CHECKED : BST_UNCHECKED, 0);

	SetDlgItemInt(IDC_EDIT_LEFT, cropRect.x);
	SetDlgItemInt(IDC_EDIT_TOP, cropRect.y);
	SetDlgItemInt(IDC_EDIT_WIDTH, cropRect.width);
	SetDlgItemInt(IDC_EDIT_HEIGHT, cropRect.height);
	GetDlgItem(IDC_ENABLE_CROP)->SendMessage(BM_SETCHECK, bCropImage ? BST_CHECKED : BST_UNCHECKED);

	CString str;
	str.Format(L"%d,%d,%d,%d", rectRects[0].x, rectRects[0].y, rectRects[0].width, rectRects[0].height);
	SetDlgItemText(IDC_EDIT_RECT0, str);
	str.Format(L"%d,%d,%d,%d", rectRects[1].x, rectRects[1].y, rectRects[1].width, rectRects[1].height);
	SetDlgItemText(IDC_EDIT_RECT1, str);
	str.Format(L"%d,%d,%d,%d", rectRects[2].x, rectRects[2].y, rectRects[2].width, rectRects[2].height);
	SetDlgItemText(IDC_EDIT_RECT2, str);
}

void CImageTestDlg::UpdateFromView()
{
}

void CImageTestDlg::ApplyPercent()
{
}

void CImageTestDlg::OnBnClickedButtonAppThreshold()
{
}

cv::Rect StrToCVRect(CString str)
{
	cv::Rect r = cv::Rect(0,0,0,0);
	LPCTSTR pstr = (LPCTSTR)str;
	while(*pstr && *pstr != ',') {
		r.x = r.x * 10 + (*pstr) - '0';
		pstr++;
	}
	if(*pstr == ',') pstr++;

	while(*pstr && *pstr != ',') {
		r.y = r.y * 10 + (*pstr) - '0';
		pstr++;
	}
	if(*pstr == ',') pstr++;

	while(*pstr && *pstr != ',') {
		r.width = r.width * 10 + (*pstr) - '0';
		pstr++;
	}
	if(*pstr == ',') pstr++;

	while(*pstr && *pstr != ',') {
		r.height = r.height * 10 + (*pstr) - '0';
		pstr++;
	}

	return r;
}

void do_undistort();

void CImageTestDlg::OnBnClickedApplyUndistort()
{
	GetDataFromControl();
}

void CImageTestDlg::GetDataFromControl()
{
	TCHAR tbuf[128];
	GetDlgItemText(IDC_TH_MIN, tbuf, 128);
	int cnt1 = StrToIntArr(tbuf, canny_th_min, 10);
	GetDlgItemText(IDC_TH_MAX, tbuf, 128);
	int cnt2 = StrToIntArr(tbuf, canny_th_max, 10);
	canny_th_cnt = cnt1 < cnt2 ? cnt1 : cnt2;

	if(cnt1 > canny_th_cnt)
		SetDlgItemText(IDC_TH_MIN, IntArrToStr(tbuf, canny_th_min, canny_th_cnt));
	if(cnt2 > canny_th_cnt)
		SetDlgItemText(IDC_TH_MAX, IntArrToStr(tbuf, canny_th_max, canny_th_cnt));
	kernel_size = GetDlgItemInt(IDC_KERNEL_SIZE);

	bTestHoughLine = GetDlgItem(IDC_TEST_HOUGH_LINE)->SendMessage(BM_GETCHECK, 0) == BST_CHECKED ? 1 : 0;
	bApplyFFT = GetDlgItem(IDC_APPLY_FFT)->SendMessage(BM_GETCHECK, 0) == BST_CHECKED ? 1 : 0;
	bNewCanny = GetDlgItem(IDC_NEW_CANNY)->SendMessage(BM_GETCHECK, 0) == BST_CHECKED ? 1 : 0;

	TCHAR tszBuf[64];
	char  szBuf[64];
	GetDlgItemText(IDC_EDIT_K1, tszBuf, 64);
	WideCharToMultiByte(CP_ACP, 0, tszBuf, lstrlen(tszBuf)+1, szBuf, 64, NULL, NULL);
	distortCoeffk1 = strtod(szBuf, NULL);

	GetDlgItemText(IDC_EDIT_K2, tszBuf, 64);
	WideCharToMultiByte(CP_ACP, 0, tszBuf, lstrlen(tszBuf)+1, szBuf, 64, NULL, NULL);
	distortCoeffk2 = strtod(szBuf, NULL);

	GetDlgItemText(IDC_EDIT_K3, tszBuf, 64);
	WideCharToMultiByte(CP_ACP, 0, tszBuf, lstrlen(tszBuf)+1, szBuf, 64, NULL, NULL);
	distortCoeffk3 = strtod(szBuf, NULL);

	GetDlgItemText(IDC_EDIT_FOCUS, tszBuf, 64);
	WideCharToMultiByte(CP_ACP, 0, tszBuf, lstrlen(tszBuf)+1, szBuf, 64, NULL, NULL);
	focal_length = strtod(szBuf, NULL);

	cropRect.x = GetDlgItemInt(IDC_EDIT_LEFT);
	cropRect.y = GetDlgItemInt(IDC_EDIT_TOP);
	cropRect.width = GetDlgItemInt(IDC_EDIT_WIDTH);
	cropRect.height = GetDlgItemInt(IDC_EDIT_HEIGHT);

	CString str;
	GetDlgItemText(IDC_EDIT_RECT0, str);
	rectRects[0] = StrToCVRect(str);
	GetDlgItemText(IDC_EDIT_RECT1, str);
	rectRects[1] = StrToCVRect(str);
	GetDlgItemText(IDC_EDIT_RECT2, str);
	rectRects[2] = StrToCVRect(str);

	min_char_height = GetDlgItemInt(IDC_MIN_CHAR_HEIGHT);
	max_char_height = GetDlgItemInt(IDC_MAX_CHAR_HEIGHT);

	horiline_min_pixels = GetDlgItemInt(IDC_HORILINE_MIN_PIXELS);
	horiline_var_allow = GetDlgItemInt(IDC_HORILINE_VAR_ALLOW);
	vertline_min_pixels = GetDlgItemInt(IDC_VERTLINE_MIN_PIXELS);
	vertline_var_allow = GetDlgItemInt(IDC_VERTLINE_VAR_ALLOW);

	var_limit = GetDlgItemInt(IDC_VAR_LIMIT);
	grid_cell_height = GetDlgItemInt(IDC_GRID_H);
	grid_cell_width = GetDlgItemInt(IDC_GRID_W);

	RegSetOptions();
}


void CImageTestDlg::OnBnClickedReset()
{
	ResetCoeffs();
	OnBnClickedApplyUndistort();
}


void CImageTestDlg::OnBnClickedCheckUndistort()
{
	apply_undistort = GetDlgItem(IDC_CHECK_UNDISTORT)->SendMessage(BM_GETCHECK) == BST_CHECKED;
	RegSetOptions();
}

void CImageTestDlg::SetEdgeCount(int ri, int count)
{
	switch(ri){
	case 0 :
		SetDlgItemInt(IDC_EDIT_E0, count);
		break;
	case 1 :
		SetDlgItemInt(IDC_EDIT_E1, count);
		break;
	case 2 :
		SetDlgItemInt(IDC_EDIT_E2, count);
	}
}


void CImageTestDlg::OnCbnSelchangePlateType()
{
	nPlateType = m_PlateType.GetCurSel();
	RegSetOptions();
}


void CImageTestDlg::OnCbnSelchangeBlurType()
{
	nBlurType = m_BlurType.GetCurSel();
	RegSetOptions(); // 옵션을 레지스트리에 저장한다.
}


void CImageTestDlg::OnBnClickedEnableCrop()
{
	bCropImage = GetDlgItem(IDC_ENABLE_CROP)->SendMessage(BM_GETCHECK) == BST_CHECKED ? 1 : 0;
	RegSetOptions();
}


void CImageTestDlg::OnBnClickedEnableScale()
{
	bScaleImage = GetDlgItem(IDC_ENABLE_SCALE)->SendMessage(BM_GETCHECK) == BST_CHECKED ? 1 : 0;
	RegSetOptions();
}


int ShowFolderDialog(HWND hWnd, TCHAR* buf);

void CImageTestDlg::OnBnClickedBrowseNnrFolder()
{
	TCHAR tmpFolder[MAX_PATH];
	if(ShowFolderDialog(this->m_hWnd, tmpFolder)) {
		lstrcpy(RegNNRFolder, tmpFolder);
		RegSetNNRFolder();
	}
}


void CImageTestDlg::OnCbnSelchangeImgSelect()
{
	int sel = this->m_imgSelect.GetCurSel();
	pMainView->SelectImageOnPlate(sel);
}

BOOL GetRawFileName(TCHAR* fullpath)
{
	CreateDirectory(fullpath, NULL);
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(fullpath, &wfd);
	if(hFind == INVALID_HANDLE_VALUE)
		return FALSE; // CreateDirectory failed
	FindClose(hFind);
	if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE; // It is not directory...

	TCHAR tmpPath[MAX_PATH];
	for(int i = 0; i < 1000; i++) {
		wsprintf(tmpPath, L"%s\\%03d.raw", fullpath, i);
		hFind = FindFirstFile(tmpPath, &wfd);
		if(hFind == INVALID_HANDLE_VALUE) {
			lstrcpy(fullpath, tmpPath);
			return TRUE;
		}
		FindClose(hFind);
	}

	return FALSE;
}


void CImageTestDlg::OnBnClickedImgSave()
{
	int sel = this->m_imgSelect.GetCurSel();
	Mat img;
	pMainView->GetSelectedROI(sel, img);

	// create folder ...\\rolxrow\\text
	TCHAR text[16];
	m_ImgText.GetWindowText(text, 16);
	TCHAR fullpath[MAX_PATH];
	wsprintf(fullpath, L"%s\\%dx%d", RegNNRFolder, img.cols, img.rows);
	CreateDirectory(fullpath, NULL);
	lstrcat(fullpath, L"\\"); lstrcat(fullpath, text);

	if(GetRawFileName(fullpath)) {
		HANDLE h = CreateFile(fullpath, GENERIC_WRITE|GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h != INVALID_HANDLE_VALUE) {
			for(int y = 0; y < img.rows; y++) {
				DWORD dwio = 0;
				WriteFile(h, img.data + y * img.step, img.cols, &dwio, NULL);
			}
			CloseHandle(h);
		}
		lstrcat(fullpath, L".bmp");
		char szfullpath[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, fullpath, lstrlen(fullpath)+1, szfullpath, MAX_PATH, NULL, NULL);
		imwrite(szfullpath, img);
	}
}


void CImageTestDlg::OnBnClickedTestTree()
{
	GetDataFromControl();
	pMainView->TestTree();
}


void CImageTestDlg::OnBnClickedTestSelection()
{
	pMainView->TestSelection();
}

void CImageTestDlg::FillEdges(int blobCandidateCount)
{
	m_Edges.ResetContent();
	for(int i = 0; i < blobCandidateCount; i++) {
		TCHAR tbuf[8];
		wsprintf(tbuf, L"%d", i);
		m_Edges.AddString(tbuf);
	}
}


void CImageTestDlg::OnCbnSelchangeEdges()
{
	int sel = m_Edges.GetCurSel();
	pMainView->DrawEdge(sel);
}


void CImageTestDlg::OnBnClickedTestEdges()
{
	TCHAR tszbuf[128];
	this->m_SelectedRect.GetWindowText(tszbuf, 128);
	pMainView->TestEdges(tszbuf);
}


void CImageTestDlg::OnCbnSelchangeEquidistMethod()
{
	equitest_method = m_EquidistMethod.GetCurSel();
}


void CImageTestDlg::FillTestEdgeText(CString &txt)
{
	SetDlgItemText(IDC_TESTED_EDGES, txt);
}


void CImageTestDlg::OnBnClickedMarkTheEdge()
{
	int edge_to_mark = GetDlgItemInt(IDC_EDGE_TO_MARK);
	pMainView->MarkEdge(edge_to_mark);
}


void CImageTestDlg::OnBnClickedApplyFft()
{
	bApplyFFT = GetDlgItem(IDC_APPLY_FFT)->SendMessage(BM_GETCHECK,0,0) == BST_CHECKED;
	RegSetOptions();
}


void CImageTestDlg::OnBnClickedNewCanny()
{
	bNewCanny = GetDlgItem(IDC_NEW_CANNY)->SendMessage(BM_GETCHECK,0,0) == BST_CHECKED;;
	RegSetOptions();
}
