// RawFormatConvertDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NNR.h"
#include "RawFormatConvertDlg.h"

#include "NNRDoc.h"
#include "NNRView.h"
#include "OCRStuff.h"

// CRawFormatConvertDlg dialog

IMPLEMENT_DYNCREATE(CRawFormatConvertDlg, CDHtmlDialog)

CRawFormatConvertDlg::CRawFormatConvertDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CRawFormatConvertDlg::IDD, CRawFormatConvertDlg::IDH, pParent)
{
	srcWidth = 0;
	srcHeight = 0;

	m_ScaleX = 2;
	m_ScaleY = 1;
}

CRawFormatConvertDlg::~CRawFormatConvertDlg()
{
}

void CRawFormatConvertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SCALE1, m_ComboScaleX);
	DDX_Control(pDX, IDC_COMBO_SCALE_Y, m_ComboScaleY);
}

BOOL CRawFormatConvertDlg::OnInitDialog()
{
	RegGetNNRFolder();

	SetDlgItemText(IDC_STATIC_PATH, RegNNRFolder);
	SetDlgItemInt(IDC_EDIT_DST_WIDTH, m_RaxW);
	SetDlgItemInt(IDC_EDIT_DST_HEIGHT, m_RaxH);
	SetDlgItemInt(IDC_EDIT_SRC_WIDTH, 0);
	SetDlgItemInt(IDC_EDIT_SRC_HEIGHT, 0);

	SetDlgItemText(IDC_EDIT_SRC_EXT, ".raw");
	SetDlgItemText(IDC_EDIT_DST_EXT, ".rax");

	CDHtmlDialog::OnInitDialog();

	m_ComboScaleX.AddString("1");
	m_ComboScaleX.AddString("2");
	m_ComboScaleX.AddString("3");
	m_ComboScaleX.SetCurSel(1);

	m_ComboScaleY.AddString("1");
	m_ComboScaleX.SetCurSel(1);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CRawFormatConvertDlg, CDHtmlDialog)
	ON_BN_CLICKED(IDC_BUTTON_PATH_SELECT, &CRawFormatConvertDlg::OnBnClickedButtonPathSelect)
	ON_BN_CLICKED(IDC_BUTTON_SRC_FIND, &CRawFormatConvertDlg::OnBnClickedButtonSrcFind)
	ON_BN_CLICKED(IDOK, &CRawFormatConvertDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_DST_WIDTH, &CRawFormatConvertDlg::OnEnChangeEditDstWidth)
	ON_CBN_SELCHANGE(IDC_COMBO_SCALE1, &CRawFormatConvertDlg::OnCbnSelchangeComboScale1)
	ON_CBN_SELCHANGE(IDC_COMBO_SCALE_Y, &CRawFormatConvertDlg::OnCbnSelchangeComboScaleY)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CRawFormatConvertDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()

void SaveToBitmap(TCHAR* path, unsigned char* buf, int x_size, int y_size);

// CRawFormatConvertDlg message handlers

HRESULT CRawFormatConvertDlg::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CRawFormatConvertDlg::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}

void CRawFormatConvertDlg::OnBnClickedButtonPathSelect()
{
	TCHAR pszPathname[MAX_PATH];

	if(!BrowseFolder(GetSafeHwnd(), pszPathname))
		return;
	
	NNRFolder = pszPathname;
	RegSetNNRFolder();
	SetDlgItemText(IDC_STATIC_PATH, pszPathname);
}

void CRawFormatConvertDlg::OnBnClickedButtonPathSelect2()
{
	// TODO: Add your control notification handler code here
}

BOOL GetSrcDimensionFromFile(TCHAR* path, TCHAR* sub, TCHAR* ext, int* sw, int *sh)
{
	TCHAR srchPath[MAX_PATH];
	lstrcpy(srchPath, path);
	lstrcat(srchPath, "\\");
	lstrcat(srchPath, sub);

	if(ext[0] != '.')
		lstrcat(srchPath, "\\*.");
	else
		lstrcat(srchPath, "\\*");
	lstrcat(srchPath, ext);

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(srchPath, &wfd);
	if(hFind == INVALID_HANDLE_VALUE)
		return FALSE;
	
	while(1) {
		// 현재 파일 이름에서 숫자를 찾는다.
		int num_found[4]; // maxmum count of numbers found
		int num_count = 0;
		int in_number = 0;

		memset(num_found, 0, sizeof(num_found));
		for(int i = 0; wfd.cFileName[i] && num_count < 4 ; i++) {
			if(wfd.cFileName[i] >= '0' && wfd.cFileName[i] <= '9') {
				num_found[num_count] = num_found[num_count] * 10 + (wfd.cFileName[i] - '0');
				in_number = 1; // 숫자를 보고 있었다고 표시한다.
			} else {
				if(in_number) num_count++; // 숫자를 보고 있었으면 숫자 카운트 증가
				in_number = 0;
			}
			if(wfd.cFileName[i] == '.') // 점을 만나면 그만 보도록 한다.
				break;
		}

		if(num_count >= 3) {
			// 숫자가 3개 이상이면 원하는 파일이름이다.
			*sw = num_found[num_count-2];
			*sh = num_found[num_count-1];
			FindClose(hFind);
			return TRUE;
		}
		if(!FindNextFile(hFind, &wfd))
			break;
	}

	return FALSE;
}

BOOL GetSrcDimensionFromDir(TCHAR* path, TCHAR* ext, int* sw, int *sh)
{
	BOOL ret = FALSE;

	TCHAR srchDirPath[MAX_PATH];
	lstrcpy(srchDirPath, path);
	lstrcat(srchDirPath, "\\*.*");
	
	WIN32_FIND_DATA dir_wfd;
	HANDLE hDirFind = FindFirstFile(srchDirPath, &dir_wfd);
	if(hDirFind == INVALID_HANDLE_VALUE)
		return FALSE;

	while(1) {
		TCHAR srchFilePath[MAX_PATH];
		lstrcpy(srchFilePath, path);
		lstrcat(srchFilePath, "\\");
		lstrcat(srchFilePath, dir_wfd.cFileName);

		if(lstrcmp(dir_wfd.cFileName, ".") && lstrcmp(dir_wfd.cFileName, ".") ) {
			if(GetSrcDimensionFromFile(path, dir_wfd.cFileName, ext, sw, sh)) {
				ret = TRUE;
				break;
			}
		}

		if(!FindNextFile(hDirFind, &dir_wfd))
			break;
	}
	FindClose(hDirFind);
	
	return ret;
}

void GetSizeFromName(TCHAR* raw, int* pw, int* ph)
{
	int width = 0;
	int height = 0;
	int base = 1;
	int idx = lstrlen(raw);
	// 숫자 아닌 부분 통과
	while(idx > 0 && !(raw[idx-1] >= '0' && raw[idx-1] <= '9')) idx--;
	// 숫자 부분 읽기
	base = 1;
	while(idx > 0 && (raw[idx-1] >= '0' && raw[idx-1] <= '9')) {
		height = height + (raw[idx-1]-'0') * base; base *= 10;
		idx--;
	}
	// 숫자 아닌 부분 통과
	while(idx > 0 && !(raw[idx-1] >= '0' && raw[idx-1] <= '9')) idx--;
	// 숫자 부분 읽기
	base = 1;
	while(idx > 0 && (raw[idx-1] >= '0' && raw[idx-1] <= '9')) {
		width = width + (raw[idx-1]-'0') * base; base *= 10;
		idx--;
	}

	*pw = width;
	*ph = height;
}

void ChangeSize(unsigned char* tmpbuf, int old_w, int old_h, int new_w, int new_h)
{
	RemoveSidePixels(tmpbuf, old_w, old_h, new_w, new_h);
/*
	TCHAR bmp[MAX_PATH];
	TCHAR rax[MAX_PATH];
	TCHAR old[MAX_PATH];
	lstrcpy(old, raw);
	lstrcat(old, "_old");
	MoveFile(raw, old);

	lstrcpy(bmp, raw);
	lstrcpy(bmp+lstrlen(bmp)-4, ".bmp");
	lstrcpy(old, bmp);
	lstrcat(old, "_old");
	MoveFile(raw, old);

	lstrcpy(rax, raw);
	lstrcpy(rax+lstrlen(rax)-4, ".rax");
	lstrcpy(old, rax);
	lstrcat(old, "_old");
	MoveFile(rax, old);

	char old_str[8];
	char new_str[8];
	char* old_ptr;
	sprintf_s(old_str, 8, "_%03d_", old_w);
	sprintf_s(new_str, 8, "_%03d_", new_w);
	old_ptr = strstr(raw, old_str);	if(old_ptr != NULL) memcpy_s(old_ptr, 8, new_str, 5);
	old_ptr = strstr(bmp, old_str);	if(old_ptr != NULL) memcpy_s(old_ptr, 8, new_str, 5);

	sprintf_s(old_str, 8, "_%03d.", old_h);
	sprintf_s(new_str, 8, "_%03d.", new_h);
	old_ptr = strstr(raw, old_str);	if(old_ptr != NULL) memcpy_s(old_ptr, 8, new_str, 5);
	old_ptr = strstr(bmp, old_str); if(old_ptr != NULL) memcpy_s(old_ptr, 8, new_str, 5);

	HANDLE hdst = CreateFile(raw, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hdst != INVALID_HANDLE_VALUE) {
		WriteFile(hdst, tmpbuf, new_w*new_h, &dwio, NULL);
		CloseHandle(hdst);
	}

	SaveToBitmap(bmp, tmpbuf, new_w, new_h);
*/
}


void MakeRaxFromRaw(TCHAR* raw, LPCTSTR newext, int xscale, int yscale)
{
	int width = 0;
	int height = 0;

	GetSizeFromName(raw, &width, &height);
	if(width == 0 || height == 0)
		return;

	HANDLE hsrc = CreateFile(raw, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hsrc == INVALID_HANDLE_VALUE)
		return;

	int rawsize = GetFileSize(hsrc, NULL);
	if(rawsize != width * height) {
		CloseHandle(hsrc);
		return;
	}

	unsigned char *rawbuf = (unsigned char*)malloc(rawsize);
	DWORD dwio = 0;
	ReadFile(hsrc, rawbuf, rawsize, &dwio, NULL);
	CloseHandle(hsrc);

	int raxw = width / xscale;
	int raxh = height / yscale;
	int raxsize = raxw * raxh;
	unsigned char* raxbuf = (unsigned char*)malloc(raxsize);

	ScaleImageXY(rawbuf, raxbuf, width, height, xscale, yscale);
	free(rawbuf);

	TCHAR rax[MAX_PATH];
	lstrcpy(rax, raw);
	// 확장자를 바꿔준다.
	int raxlen = lstrlen(rax);
	while(raxlen > 0) {
		raxlen--;
		if(rax[raxlen] == '.')
			break;
	}
	lstrcpy(&rax[raxlen], newext);

	TCHAR bmp[MAX_PATH];
	lstrcpy(bmp, rax);
	lstrcat(bmp, ".bmp");
	SaveToBitmap(bmp, raxbuf, raxw, raxh);

	HANDLE hdst = CreateFile(rax, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hdst != INVALID_HANDLE_VALUE) {
		DWORD dwio;
		WriteFile(hdst, raxbuf, raxsize, &dwio, NULL);
		CloseHandle(hdst);
	}

	free(raxbuf);
}

void MakeRanFromRaw(TCHAR* raw, int RaxW, int RaxH, LPCTSTR newext)
{
	int width = 0;
	int height = 0;

	GetSizeFromName(raw, &width, &height);
	if(width == 0 || height == 0)
		return;

	HANDLE hsrc = CreateFile(raw, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hsrc == INVALID_HANDLE_VALUE)
		return;

	int rawsize = GetFileSize(hsrc, NULL);
	if(rawsize != width * height) {
		CloseHandle(hsrc);
		return;
	}

	unsigned char *rawbuf = (unsigned char*)malloc(rawsize);
	DWORD dwio = 0;
	ReadFile(hsrc, rawbuf, rawsize, &dwio, NULL);
	CloseHandle(hsrc);

	int x;
	for(x = 0; x < rawsize; x++) {
		if(rawbuf[x] != 255)
			rawbuf[x] = 0;
	}

	TCHAR rax[MAX_PATH];
	lstrcpy(rax, raw);
	lstrcat(rax, newext);
	HANDLE hdst = CreateFile(rax, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hdst != INVALID_HANDLE_VALUE) {
		DWORD dwio;
		WriteFile(hdst, rawbuf, rawsize, &dwio, NULL);
		CloseHandle(hdst);
	}

	free(rawbuf);
}

void MakeRatFromRaw(TCHAR* raw, int RaxW, int RaxH, LPCTSTR newext)
{
	int width = 0;
	int height = 0;

	GetSizeFromName(raw, &width, &height);
	if(width == 0 || height == 0)
		return;

	unsigned char* xsum = (unsigned char*)malloc(width * sizeof(unsigned char));
	unsigned char* ysum = (unsigned char*)malloc(height * sizeof(unsigned char));
	
	memset(xsum, 0, width*sizeof(unsigned char));
	memset(ysum, 0, height*sizeof(unsigned char));

	HANDLE hsrc = CreateFile(raw, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hsrc == INVALID_HANDLE_VALUE)
		return;

	int rawsize = GetFileSize(hsrc, NULL);
	if(rawsize != width * height) {
		CloseHandle(hsrc);
		return;
	}

	unsigned char *rawbuf = (unsigned char*)malloc(rawsize);
	DWORD dwio = 0;
	ReadFile(hsrc, rawbuf, rawsize, &dwio, NULL);
	CloseHandle(hsrc);

	int x, y;
	for(x = 0; x < width; x++) {
		for(y = 0; y < height; y++) {
			if(rawbuf[y*width+x] != 255) xsum[x]++;
		}
	}

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			if(rawbuf[y*width+x] != 255) ysum[y]++;
		}
	}

	free(rawbuf);

	TCHAR rax[MAX_PATH];
	lstrcpy(rax, raw);
	lstrcat(rax, newext);
	HANDLE hdst = CreateFile(rax, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hdst != INVALID_HANDLE_VALUE) {
		DWORD dwio;
		WriteFile(hdst, xsum, width, &dwio, NULL);
		WriteFile(hdst, ysum, height, &dwio, NULL);
		CloseHandle(hdst);
	}

	free(xsum);
	free(ysum);
}

void CRawFormatConvertDlg::UpdateInfo()
{
	SetDlgItemInt(IDC_EDIT_SRC_WIDTH, srcWidth);
	SetDlgItemInt(IDC_EDIT_SRC_HEIGHT, srcHeight);

	SetDlgItemInt(IDC_EDIT_DST_WIDTH, srcWidth);
	SetDlgItemInt(IDC_EDIT_DST_HEIGHT, srcHeight);

	SetDlgItemInt(IDC_EDIT_NNR_WIDTH, srcWidth/m_ScaleX);
	SetDlgItemInt(IDC_EDIT_NNR_HEIGHT, srcHeight/m_ScaleY);
}

void CRawFormatConvertDlg::OnBnClickedButtonSrcFind()
{
	TCHAR ext[MAX_PATH];
	GetDlgItemText(IDC_EDIT_SRC_EXT, ext, MAX_PATH);
	ext[10] = 0; // ext 최대 길이는 10
	if(lstrlen(ext) == 0)
		return;

	TCHAR srcPath[MAX_PATH];
	GetDlgItemText(IDC_STATIC_PATH, srcPath, MAX_PATH);

	if(!GetSrcDimensionFromDir(srcPath, ext, &srcWidth, &srcHeight)) {
		MessageBox("Cannot get dimension of source image");
		return;
	}

	UpdateInfo();
}

void CRawFormatConvertDlg::OnBnClickedOk()
{
	TCHAR ext[MAX_PATH];
	GetDlgItemText(IDC_EDIT_SRC_EXT, ext, MAX_PATH);
	ext[10] = 0; // ext 최대 길이는 10
	if(ext[0] != '.') {
		MessageBox("Source extension should start with '.'");
		return;
	}

	TCHAR newext[MAX_PATH];
	GetDlgItemText(IDC_EDIT_DST_EXT, newext, MAX_PATH);
	newext[10] = 0;
	if(newext[0] != '.') {
		MessageBox("Target extension should start with '.'");
		return;
	}

	int r_width = GetDlgItemInt(IDC_EDIT_DST_WIDTH);
	int r_height = GetDlgItemInt(IDC_EDIT_DST_HEIGHT);

	TCHAR infoText[MAX_PATH];
	wsprintf(infoText, "%s\\%s to %s, size to %d,%d -> %d,%d.TXT", RegNNRFolder, ext, newext, r_width, r_height, r_width/m_ScaleX, r_height/m_ScaleY);
	HANDLE hi = CreateFile(infoText, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hi != INVALID_HANDLE_VALUE)
		CloseHandle(hi);

	TCHAR pf[MAX_PATH];
	wsprintf(pf, TEXT("%s\\*"), RegNNRFolder);
	CFileFind ff1;
	if(ff1.FindFile(pf)) {
		int benext = TRUE;
		while(benext) {
			benext = ff1.FindNextFile();
			if(!ff1.IsDots() && ff1.IsDirectory()) {
				TCHAR pf2[MAX_PATH];
				wsprintf(pf2, "%s\\%s\\*%s", RegNNRFolder, (LPCTSTR)ff1.GetFileName(), ext);

				CFileFind ff2;
				if(ff2.FindFile(pf2)) {
					int bnext2 = TRUE;
					while(bnext2){
						bnext2 = ff2.FindNextFile();
						if(ff2.IsDirectory())
							continue;
						TCHAR raw[MAX_PATH];
						wsprintf(raw, "%s\\%s\\%s", RegNNRFolder, (LPCTSTR)ff1.GetFileName(), (LPCTSTR)ff2.GetFileName());
						MakeRaxFromRaw(raw, newext, m_ScaleX, m_ScaleY);
					}
					ff2.Close();
				}
			}
		}
		ff1.Close();
	}
	MessageBox("Convert data", "Finished");
}

void CRawFormatConvertDlg::OnEnChangeEditDstWidth()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDHtmlDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	int dstWidth = GetDlgItemInt(IDC_EDIT_DST_WIDTH);
	SetDlgItemInt(IDC_EDIT_NNR_WIDTH, dstWidth/2);
}

void CRawFormatConvertDlg::OnBnClickedOk3()
{
	TCHAR ext[MAX_PATH];
	GetDlgItemText(IDC_EDIT_SRC_EXT, ext, MAX_PATH);
	ext[10] = 0; // ext 최대 길이는 10
	if(ext[0] != '.') {
		MessageBox("Source extension should start with '.'");
		return;
	}

	TCHAR newext[MAX_PATH];
	GetDlgItemText(IDC_EDIT_DST_EXT, newext, MAX_PATH);
	newext[10] = 0;
	if(newext[0] != '.') {
		MessageBox("Target extension should start with '.'");
		return;
	}
	strcat(newext, "n");

	int r_width = GetDlgItemInt(IDC_EDIT_DST_WIDTH);
	int r_height = GetDlgItemInt(IDC_EDIT_DST_HEIGHT);
	if(r_width != m_RaxW || r_height != m_RaxH) {
		m_RaxW = r_width;
		m_RaxH = r_height;
		RegSetNNRFolder();
	}

	TCHAR infoText[MAX_PATH];
	wsprintf(infoText, "%s\\%s to %s, size to %d,%d then %d,%d.TXT", RegNNRFolder, ext, newext, r_width, r_height, r_width/2, r_height);
	HANDLE hi = CreateFile(infoText, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hi != INVALID_HANDLE_VALUE)
		CloseHandle(hi);

	TCHAR pf[MAX_PATH];
	wsprintf(pf, TEXT("%s\\*"), RegNNRFolder);
	CFileFind ff1;
	if(ff1.FindFile(pf)) {
		int benext = TRUE;
		while(benext) {
			benext = ff1.FindNextFile();
			if(!ff1.IsDots() && ff1.IsDirectory()) {
				TCHAR pf2[MAX_PATH];
				wsprintf(pf2, "%s\\%s\\*%s", RegNNRFolder, (LPCTSTR)ff1.GetFileName(), ext);

				CFileFind ff2;
				if(ff2.FindFile(pf2)) {
					int bnext2 = TRUE;
					while(bnext2){
						bnext2 = ff2.FindNextFile();
						if(ff2.IsDirectory())
							continue;
						TCHAR raw[MAX_PATH];
						wsprintf(raw, "%s\\%s\\%s", RegNNRFolder, (LPCTSTR)ff1.GetFileName(), (LPCTSTR)ff2.GetFileName());
						MakeRanFromRaw(raw, m_RaxW, m_RaxH, newext);
					}
					ff2.Close();
				}
			}
		}
		ff1.Close();
	}
	MessageBox("Convert data", "Finished");
}

void CRawFormatConvertDlg::OnCbnSelchangeComboScale1()
{
	m_ScaleX = m_ComboScaleX.GetCurSel() + 1;
	UpdateInfo();
}

void CRawFormatConvertDlg::OnCbnSelchangeComboScaleY()
{
	m_ScaleY = m_ComboScaleY.GetCurSel() + 1;
	UpdateInfo();
}
