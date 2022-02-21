// OCRSaveDlg.cpp : implementation file
//

#include "stdafx.h"

#include "B_Common.h"

#include "ST150Image.h"
#include "NoteBuilderFrame.h"
#include "OCRSaveDlg.h"
#include "CodeParser.h"
#include "CurrencyInfo.h"
#include "ICUMANDoc.h"
#include "ICUMANView.h"
#include "OCRDlg.h"

#include "OCRStuff.h"
#include "MacroValue.h"

COCRSaveDlg* theOCRSaveDlg = NULL;

// COCRSaveDlg dialog

IMPLEMENT_DYNAMIC(COCRSaveDlg, CDialog)

COCRSaveDlg::COCRSaveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COCRSaveDlg::IDD, pParent)
{
	m_bitmap = NULL;
}

COCRSaveDlg::~COCRSaveDlg()
{
	if(m_bitmap != NULL)
		delete m_bitmap;
}

void COCRSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_C0, m_EditC0);
	DDX_Control(pDX, IDC_LIST_IMAGE, m_ListImage);
	DDX_Control(pDX, IDC_EDIT_SAVE_FOLDER, m_EditSaveFolder);
}


BEGIN_MESSAGE_MAP(COCRSaveDlg, CDialog)
	ON_BN_CLICKED(IDOK, &COCRSaveDlg::OnBnClickedOk)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BTN_BROWSE, &COCRSaveDlg::OnBnClickedBtnBrowse)
	ON_BN_CLICKED(IDC_BTN_READ_SIZE, &COCRSaveDlg::OnBnClickedBtnReadSize)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &COCRSaveDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDCANCEL, &COCRSaveDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


int SaveWidthHeightCtlID[] = {
	IDC_EDIT_C16, IDC_EDIT_C17, IDC_EDIT_C18, IDC_EDIT_C19, IDC_EDIT_C20,
	IDC_EDIT_C21, IDC_EDIT_C22, IDC_EDIT_C23, IDC_EDIT_C24, IDC_EDIT_C25,
	IDC_EDIT_C26, IDC_EDIT_C27, IDC_EDIT_C28, IDC_EDIT_C29, IDC_EDIT_C30,
	IDC_EDIT_C31, -1 };
int SaveXPosCtlID[] = {
	IDC_EDIT_C32, IDC_EDIT_C33, IDC_EDIT_C34, IDC_EDIT_C35, IDC_EDIT_C36,
	IDC_EDIT_C37, IDC_EDIT_C38, IDC_EDIT_C39, IDC_EDIT_C40, IDC_EDIT_C41,
	IDC_EDIT_C42, IDC_EDIT_C43, IDC_EDIT_C44, IDC_EDIT_C45, IDC_EDIT_C46,
	IDC_EDIT_C47, -1 };


// COCRSaveDlg message handlers

BOOL COCRSaveDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitialProcess();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

extern CICUMANView* pMainView;

void COCRSaveDlg::InitialProcess()
{
	SetWidthHeightControls();

#if CURRENCY
	int sx = OCR_x_pos[0];
	int ex = OCR_x_pos[OCR_pos_cnt-1] + OCR_x_len[OCR_pos_cnt-1];
	int bm_width = ex - sx;

	CDC *pDC = GetDC();
	if(m_bitmap != NULL) {
		delete m_bitmap;
		m_ListImage.DeleteString(0);
	}
	m_ListImage.AddString(L"0");

	m_bitmap = new CBitmap();
	m_bitmap->CreateCompatibleBitmap(pDC, bm_width, v_window);
	CDC dc;
	dc.CreateCompatibleDC(pDC);
	ReleaseDC(pDC);

	CBitmap *obit = dc.SelectObject(m_bitmap);
	for(int y = 0; y < v_window; y++) {
		for(int x = 0; x < bm_width; x++) {
			int yy = y+y_pos_0;
			int xx = x+sx;
			unsigned char c = ocr_data[yy*img_width + xx];
			dc.SetPixel(x, y, RGB(c,c,c));
		}
	}
	dc.SelectObject(obit);
	dc.DeleteDC();

	m_EditSaveFolder.SetWindowTextW(NNRFolder);
	if(pMainView != NULL) {
		MultiByteToWideChar(CP_ACP, 0, pMainView->imgST150.OneNote.Serial, strlen(pMainView->imgST150.OneNote.Serial)+1, Serial, 23);
		SetDlgItemText(IDC_EDIT_READ, Serial);
	}
#endif
}

void COCRSaveDlg::SetWidthHeightControls()
{
#if CURRENCY
	for(int si = 0; si < OCR_save_width_height_count.Get(); si++) {
		if(SaveWidthHeightCtlID[si] == -1)
			break;
		SetEditWidthHeight(SaveWidthHeightCtlID[si], ORG_x_len[si], ORG_y_len[si]);
		SetDlgItemInt(SaveXPosCtlID[si], ORG_x_pos[si]);
	}
#endif
}

void GetClassFolder(TCHAR* SaveFolder, TCHAR* ClassFolder, TCHAR* Char)
{
	ClassFolder[0] = 0;

	TCHAR SearchPath[MAX_PATH];
	swprintf(SearchPath, MAX_PATH, L"%s*", SaveFolder);
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(SearchPath, &wfd);
	if(hFind == INVALID_HANDLE_VALUE)
		return;
	
	int MaxIndex = 0;
	while(1) {
		if(lstrcmp(wfd.cFileName, L".") == 0 || lstrcmp(wfd.cFileName, L"..") == 0) {
		} else if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if(lstrcmp(wfd.cFileName + lstrlen(wfd.cFileName) - lstrlen(Char), Char) == 0) {
				lstrcpy(ClassFolder, wfd.cFileName);
				break;
			}
			int index = StrToInt(wfd.cFileName);
			if(index > MaxIndex)
				MaxIndex = index;
		}
		if(!FindNextFile(hFind, &wfd))
			break;
	}
	FindClose(hFind);
	if(ClassFolder[0])
		return;

	wsprintf(ClassFolder, L"%03d_%s", MaxIndex+1, Char);
	//wsprintf(ClassFolder, L"%s", Char);
	TCHAR FullPath[MAX_PATH];
	wsprintf(FullPath, L"%s%s", SaveFolder, ClassFolder);
	CreateDirectory(FullPath, NULL);
}

void SaveToBitmap(TCHAR* path, unsigned char* buf, int x_size, int y_size)
{
	BITMAPINFOHEADER bi;

	//*------------------------- 비트멥 헤더를 기록함 -------------------------
	int xbytes = (((x_size * 3)+3)/4)*4;

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = x_size;
	bi.biHeight = -y_size;
	bi.biPlanes = 1;
	bi.biBitCount = 24;       
	bi.biCompression = BI_RGB;
	bi.biSizeImage = y_size * xbytes;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	BITMAPFILEHEADER hdr;

	hdr.bfType = ((WORD)('M' << 8) | 'B');        
	hdr.bfSize = bi.biSizeImage + sizeof(bi) + sizeof(hdr);   
	hdr.bfReserved1 = 0;                                 
	hdr.bfReserved2 = 0;                                 
	int nColors = 0; // Color Index 없음
	hdr.bfOffBits=(DWORD)(sizeof(hdr) + bi.biSize + nColors * sizeof(RGBQUAD));

	/*--------------------- 실제 파일에 기록함 --------------------------*/
	HANDLE pFile = NULL;

	TCHAR bmpFileName[MAX_PATH];
	lstrcpy(bmpFileName, path);
	lstrcpy(bmpFileName+lstrlen(bmpFileName)-3, L"bmp");
	pFile = CreateFile(bmpFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	bool bResult = false;
	if (pFile == INVALID_HANDLE_VALUE)
		return;

	DWORD dwio = 0;
	WriteFile(pFile, &hdr, sizeof(hdr), &dwio, NULL);
	WriteFile(pFile, &bi, sizeof(bi), &dwio, NULL);
	unsigned char *rowBuffer = (unsigned char*)malloc(xbytes);
	int x, y;

	for(y = 0; y < y_size; y++) {
		unsigned char* dp = rowBuffer;
		for(x = 0; x < x_size; x++) {
			*dp++ = buf[y*x_size + x];
			*dp++ = buf[y*x_size + x];
			*dp++ = buf[y*x_size + x];
		}
		WriteFile(pFile, rowBuffer, xbytes, &dwio, NULL);
	}
	free(rowBuffer);

	CloseHandle(pFile);
}

extern "C" {
int GetLengthInBytesForNNROCR(int width, int height);
int MakeFeatureForNNROCR(unsigned char* srcbuf, unsigned char* dstbuf, int width, int height);
void RemoveUpperBlank(unsigned char* image, int width, int height);
void RemoveSidePixels(unsigned char* srcbuf, int old_w, int old_h, int new_w, int new_h);
};

int TestSameData(TCHAR *SaveFolder, TCHAR* ClassFolder, unsigned char* buf, int rawsize, TCHAR* rfname)
{
	int MaxIndex = 0;
	WIN32_FIND_DATA wfd;
	TCHAR SrchPath[MAX_PATH];
	wsprintf(SrchPath, L"%s%s\\*", SaveFolder, ClassFolder);
	HANDLE hFind = FindFirstFile(SrchPath, &wfd);
	if(hFind == INVALID_HANDLE_VALUE) {
		free(buf);
		return -1;
	}

	while(1) {
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		} else if(lstrlen(wfd.cFileName) > 10) {
			if(lstrcmp(wfd.cFileName+lstrlen(wfd.cFileName)-4, L".raw") == 0) {
				int index = StrToInt(wfd.cFileName);
				if(index >= MaxIndex)
					MaxIndex = index;
				// 같은 내용인지 검사한다.
				TCHAR fname[MAX_PATH];
				wsprintf(fname, TEXT("%s%s\\%s"), SaveFolder, ClassFolder, wfd.cFileName);
				HANDLE hr = CreateFile(fname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if(hr != INVALID_HANDLE_VALUE) {
					if(GetFileSize(hr, NULL) == rawsize) {
						unsigned char* rbuf = (unsigned char*)malloc(rawsize);
						DWORD dwio = 0;
						ReadFile(hr, rbuf, rawsize, &dwio, NULL);
						if(memcmp(rbuf, buf, rawsize) == 0) {
							lstrcpy(rfname, fname);
							MaxIndex = -1; // 중복파일 발견
						}
						free(rbuf);
					}
					CloseHandle(hr);
				}
			}
		}
		if(MaxIndex < 0)
			break;

		if(!FindNextFile(hFind, &wfd))
			break;
	}
	FindClose(hFind);

	return MaxIndex;
}

void COCRSaveDlg::SaveFileToClass(TCHAR* SaveFolder, TCHAR* ClassFolder, int img_width, int i, int old_w, int SaveWidth, int SaveHeight)
{
#if CURRENCY
	int rawsize = SaveWidth * SaveHeight;
	unsigned char *buf = (unsigned char*)malloc(rawsize);

	MakeOneImage(buf, ocr_data, i, OCR_x_pos, img_width, y_pos_0, SaveWidth, v_window, SaveHeight, old_w);

	TCHAR SrchPath[MAX_PATH];
	wsprintf(SrchPath, L"%s%s", SaveFolder, ClassFolder);
	CreateDirectory(SrchPath, NULL);

	TCHAR fname[MAX_PATH];
	int MaxIndex = TestSameData(SaveFolder, ClassFolder, buf, rawsize, fname);
	if(MaxIndex < 0) {
		MessageBox(fname, TEXT("Same data found"));
		free(buf);
		return;
	}

	TCHAR FullPath[MAX_PATH];
	wsprintf(FullPath, L"%s%s\\%03d_%03d_%03d.raw", SaveFolder, ClassFolder, MaxIndex+1, SaveWidth, SaveHeight);

	DWORD dwio = 0;
	HANDLE h = CreateFile(FullPath, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(h, buf, rawsize, &dwio, NULL);
	CloseHandle(h);

	SaveToBitmap(FullPath, buf, SaveWidth, SaveHeight);

	free(buf);
#endif
}

TCHAR OCR_ConstFilePath[MAX_PATH] = TEXT("");

void ReplaceTagText(char* dstbuf, int bufsize, char* tag, char* txt)
{
	char* srcbuf = (char*)malloc(bufsize);
	strcpy_s(srcbuf, bufsize, dstbuf);

	char* sp = srcbuf;
	char* dp = dstbuf;
	*dp = 0;

	char* tp1 = strstr(srcbuf, tag);
	if(tp1 == NULL) {
		// tag이 없으면, 마지막에 txt를 그냥 덧붙여 준다.
		while(*sp) *dp++ = *sp++;
		if(*(dp-1) != '\n') {
			*dp++ = '\r';
			*dp++ = '\n';
		}
		*dp = 0;
		strcat_s(dstbuf, bufsize, txt);
		free(srcbuf);
		return;
	}

	// tag까지 복사
	while(*sp && sp != tp1) *dp++ = *sp++;
	*dp = 0;
	// 이전의 #define 통과
	while(*sp && *sp != '\n') sp++;
	if(*sp == '\n') sp++;

	strcat_s(dp, 1024, txt);
	dp += strlen(txt);
	*dp = 0;

	// 끝까지 복사
	while(*sp) *dp++ = *sp++;
	*dp = 0;

	free(srcbuf);
}

int LoadFile(TCHAR* tfn, char* buf, int bufsize)
{
	HANDLE h = CreateFile(tfn, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return -1;
	
	int fsize = GetFileSize(h, NULL);
	if(bufsize < fsize) {
		CloseHandle(h);
		return -1;
	}

	DWORD dwio = 0;
	ReadFile(h, buf, fsize, &dwio, NULL);
	CloseHandle(h);

	buf[fsize] = 0;
	return fsize;
}

char dstbuf[2048];
char srcbuf[2048];
char txt2[2048];
char txt3[2048];
char txt4[2048];

void WriteSaveWidthHeightToFile()
{
#if CURRENCY
	sprintf_s(txt2, sizeof(txt2),
			"const int %s_OCR_X_LEN[%s_OCR_SAVE_WIDTH_HEIGHT_COUNT] = {",
			pWinCurrentLocal->szCurrency, pWinCurrentLocal->szCurrency);
	for(int si = 0; si < OCR_save_width_height_count.Get(); si++) {
		char tmp[16];
		sprintf_s(tmp, 16, "%d", ORG_x_len[si]);
		if(si == OCR_save_width_height_count.Get()-1)
			strcat_s(tmp, 16, "};\r\n");
		else
			strcat_s(tmp, 16, ", ");
		strcat_s(txt2, sizeof(txt2), tmp);
	}

	sprintf_s(txt3, sizeof(txt3),
			"const int %s_OCR_Y_LEN[%s_OCR_SAVE_WIDTH_HEIGHT_COUNT] = {",
			pWinCurrentLocal->szCurrency, pWinCurrentLocal->szCurrency);
	for(int si = 0; si < OCR_save_width_height_count.Get(); si++) {
		char tmp[16];
		sprintf_s(tmp, 16, "%d", ORG_y_len[si]);
		if(si == OCR_save_width_height_count.Get()-1)
			strcat_s(tmp, 16, "};\r\n");
		else
			strcat_s(tmp, 16, ", ");
		strcat_s(txt3, sizeof(txt3), tmp);
	}

	sprintf_s(txt4, sizeof(txt4),
		"const int %s_OCR_X_POS[%s_OCR_SAVE_WIDTH_HEIGHT_COUNT] = {",
		pWinCurrentLocal->szCurrency, pWinCurrentLocal->szCurrency);
	for(int si = 0; si < OCR_save_width_height_count.Get(); si++) {
		char tmp[16];
		sprintf_s(tmp, 16, "%d", ORG_x_pos[si]);
		if(si == OCR_save_width_height_count.Get()-1)
			strcat_s(tmp, 16, "};\r\n");
		else
			strcat_s(tmp, 16, ", ");
		strcat_s(txt4, sizeof(txt4), tmp);
	}

	int fsize = LoadFile(OCR_ConstFilePath, srcbuf, sizeof(srcbuf));
	if(fsize <= 0) {
		// 기존 파일의 내용이 없으므로 그냥 기록하기만 하면 된다.
		HANDLE h = CreateFile(OCR_ConstFilePath, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h == INVALID_HANDLE_VALUE) {
			MessageBox(NULL, OCR_ConstFilePath, TEXT("Cannot create"), MB_OK);
			goto _return;
		}
		DWORD dwio = 0;
		WriteFile(h, txt2, strlen(txt2), &dwio, NULL);
		WriteFile(h, txt3, strlen(txt3), &dwio, NULL);
		WriteFile(h, txt4, strlen(txt4), &dwio, NULL);
		CloseHandle(h);
		goto _return;
	}

	strcpy_s(dstbuf, sizeof(dstbuf), srcbuf);

	char tag2[128];
	char tag3[128];
	char tag4[128];

	sprintf_s(tag2, 128, "const int %s_OCR_X_LEN", pWinCurrentLocal->szCurrency);
	sprintf_s(tag3, 128, "const int %s_OCR_Y_LEN", pWinCurrentLocal->szCurrency);
	sprintf_s(tag4, 128, "const int %s_OCR_X_POS", pWinCurrentLocal->szCurrency);

	ReplaceTagText(dstbuf, sizeof(dstbuf), tag2, txt2);
	ReplaceTagText(dstbuf, sizeof(dstbuf), tag3, txt3);
	ReplaceTagText(dstbuf, sizeof(dstbuf), tag4, txt4);

	TCHAR backFile[MAX_PATH];
	wsprintf(backFile, TEXT("%s.bak"), OCR_ConstFilePath);
	MoveFile(OCR_ConstFilePath, backFile);
	
	HANDLE h = CreateFile(OCR_ConstFilePath, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h != INVALID_HANDLE_VALUE) {
		DWORD dwio = 0;
		WriteFile(h, dstbuf, strlen(dstbuf), &dwio, NULL);
		CloseHandle(h);
		MessageBox(NULL, OCR_ConstFilePath, TEXT("Width and Height were saved to..."), MB_OK);

		goto _return;
	}

	MessageBox(NULL, OCR_ConstFilePath, TEXT("Failed to write to..."), MB_OK);

_return:
	;
#endif
}

void ReadSaveWidthHeightFromFile()
{
#if CURRENCY
	CCodeParser* cp = new CCodeParser();
	if(!cp->LoadFile(OCR_ConstFilePath)) {
		MessageBox(NULL, OCR_ConstFilePath, L"해당파일을 찾을수 없습니다.", MB_OK);
		return;
	}

	for(int i = 0; i < cp->assign_count; i++) {
		if(strstr(cp->assign[i].name, "OCR_X_LEN")) {
			int pos = cp->assign[i].start_pos;
			char* data_pos = cp->assign_buf;
			for(int si = 0; si < OCR_save_width_height_count.Get(); si++) {
				ORG_x_len[si] = atoi(data_pos+pos);
				pos += strlen(data_pos+pos) + 1;
			}
		}
		if(strstr(cp->assign[i].name, "OCR_Y_LEN")) {
			int pos = cp->assign[i].start_pos;
			char* data_pos = cp->assign_buf;
			for(int si = 0; si < OCR_save_width_height_count.Get(); si++) {
				ORG_y_len[si] = atoi(data_pos+pos);
				pos += strlen(data_pos+pos) + 1;
			}
		}
		if(strstr(cp->assign[i].name, "OCR_X_POS")) {
			int pos = cp->assign[i].start_pos;
			char* data_pos = cp->assign_buf;
			for(int si = 0; si < OCR_save_width_height_count.Get(); si++) {
				ORG_x_pos[si] = atoi(data_pos+pos);
				pos += strlen(data_pos+pos) + 1;
			}
		}
	}

	delete cp;
#endif
}

int dimension_modified = 0;

// 입력창의 값들이 변동이 있는지 검사한다.
void COCRSaveDlg::TestSaveWidthHeightInput()
{
	int count = 0;

#if CURRENCY
	for(int si = 0; si < MAX_OCR_LIMIT; si++) {
		TCHAR tbuf[16];
		GetDlgItemText(SaveWidthHeightCtlID[si], tbuf, 16);
		int index = 0;
		int w = 0;
		while(tbuf[index] && tbuf[index] != ',') {
			if(tbuf[index] >= '0' && tbuf[index] <= '9') w = w * 10 + (tbuf[index]-'0');
			index++;
		}
		int h = 0;
		if(tbuf[index] == ',') {
			index++;
			while(tbuf[index]) {
				if(tbuf[index] >= '0' && tbuf[index] <= '9') h = h * 10 + (tbuf[index] - '0');
				index++;
			}
		}
		if(w == 0 || h == 0)
			break;
		if(ORG_x_len[si] != w || ORG_y_len[si] != h) {
			ORG_x_len[si] = w;
			ORG_y_len[si] = h;
			dimension_modified = 1;
		}

		int xpos = GetDlgItemInt(SaveXPosCtlID[si]);
		if(ORG_x_pos[si] != xpos) {
			dimension_modified = 1;
			ORG_x_pos[si] = xpos;
		}

		count++;
	}

	if(OCR_save_width_height_count.Get() != count) {
		OCR_save_width_height_count.Set(count);
		dimension_modified = 1;
	}
#endif
}


void COCRSaveDlg::UpdateSaveWidthHeight()
{
	TestSaveWidthHeightInput();

	if(dimension_modified || OCR_ConstFilePath[0] == 0) {
		// 수정되었거나 파일에서 읽어온 게 아니면 저장해야 한다.
		WriteSaveWidthHeightToFile();
		dimension_modified = 0;
	}
}

void COCRSaveDlg::OnBnClickedOk()
{
	UpdateSaveWidthHeight();

	TCHAR SaveFolder[MAX_PATH];
	lstrcpy(SaveFolder, NNRFolder);
	if(SaveFolder[lstrlen(SaveFolder)-1] != '\\') lstrcat(SaveFolder, TEXT("\\"));

	TCHAR txtbuf[24];
	m_EditC0.GetWindowText(txtbuf, 23);
	m_EditC0.SetWindowText(L"");

	TCHAR tdbuf[24];
	memset(tdbuf, 0, sizeof(tdbuf));
	GetDlgItemText(IDC_EDIT_READ, tdbuf, 23);
	SetDlgItemText(IDC_EDIT_READ, L"");

#if CURRENCY
	for(int i = 0; i < OCR_pos_cnt; i++) {
		if(i >= wcslen(txtbuf))
			break;
		if( (txtbuf[i] == ' ') || (txtbuf[i] == '_') || (txtbuf[i] == tdbuf[i]))
			continue;

		TCHAR classchar[2];
		classchar[0] = txtbuf[i];
		classchar[1] = 0;

		TCHAR ClassFolder[MAX_PATH];
		GetClassFolder(SaveFolder, ClassFolder, classchar); // 입력받은 글자를 가지고 클래스 폴더의 이름을 생성한다.
		if(ClassFolder[0]) {
			if(OCR_save_width_height_count.Get() <= 1) {
				// 갯수가 한개라면(고정 크기라면) 0번째 크기를 사용하여 기록한다.
				SaveFileToClass(SaveFolder, ClassFolder, img_width, i, OCR_x_len[i], OCR_HWindow.Get(), OCR_f_height.Get());
			} else {
				// 가변 크기를 가지는 글자라면 ocr_y_len 중 선택한 크기로 기록한다.
				SaveFileToClass(SaveFolder, ClassFolder, img_width, i, OCR_x_len[i], OCR_x_len[i], ORG_y_len[i]);
			}
		}
	}
#endif
//	OnOK();
//	DestroyWindow();
}

void COCRSaveDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
#if CURRENCY
	if(nIDCtl == IDC_LIST_IMAGE) {
		lpMeasureItemStruct->itemWidth = (OCR_x_pos[OCR_pos_cnt-1]+OCR_x_len[OCR_pos_cnt-1]+1) * 2;
		lpMeasureItemStruct->itemHeight = (v_window+2) * 2;
		return;
	}
#endif
	CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

void COCRSaveDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if(nIDCtl == IDC_LIST_IMAGE) {
		int left = lpDrawItemStruct->rcItem.left;
		int top = lpDrawItemStruct->rcItem.top;
		CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

		BITMAP bm;
		m_bitmap->GetBitmap(&bm);
		CDC dc;
		dc.CreateCompatibleDC(pDC);
		CBitmap *obit = dc.SelectObject(m_bitmap);
		pDC->StretchBlt(1, 1, bm.bmWidth*2, bm.bmHeight*2, &dc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		dc.SelectObject(obit);

		COLORREF b = RGB(0,0,255);
		CPen p(PS_SOLID, 1, RGB(0,0,255));
		CPen *op = pDC->SelectObject(&p);
#if CURRENCY
		for(int i = 0; i < OCR_pos_cnt; i++) {
			int sx = OCR_x_pos[i] - OCR_x_pos[0];
			int ex = sx + OCR_x_len[i];
			pDC->MoveTo(left + sx*2    , top);
			pDC->LineTo(left + ex*2 + 1, top);
			pDC->LineTo(left + ex*2 + 1, top + v_window*2 + 2);
			pDC->LineTo(left + sx*2    , top + v_window*2 + 2);
			pDC->LineTo(left + sx*2    , top);
		}
#endif
		pDC->SelectObject(op);
		return;
	}

	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

BOOL BrowseFolder(HWND hWnd, TCHAR *pszPathname, LPARAM lParam, DWORD flag);

void COCRSaveDlg::SetEditWidthHeight(int CtlID, int w, int h)
{
	TCHAR buf[32];
	wsprintf(buf, TEXT("%d,%d"), w, h);
	SetDlgItemText(CtlID, buf);
}

void COCRSaveDlg::OnBnClickedBtnBrowse()
{
	if(BrowseFolder(GetSafeHwnd(), NNRFolder, (LPARAM)NNRFolder, BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_NEWDIALOGSTYLE)) {
		m_EditSaveFolder.SetWindowTextW(NNRFolder);
		RegSetNNRFolder();
	}
}

void COCRSaveDlg::OnBnClickedBtnReadSize()
{
	ReadSaveWidthHeightFromFile();
	SetWidthHeightControls();
}

void COCRSaveDlg::OnBnClickedButtonTest()
{
	// 일단 Hwidnow Method가 Given Width인 경우만 코딩해 보자.
	int max_len = 0;
	TestSaveWidthHeightInput();
	if(theOCRDlg != NULL)
		theOCRDlg->OnBnClickedButtonAppWindow();
}

void COCRSaveDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	delete theOCRSaveDlg;
	theOCRSaveDlg = NULL;
}

void COCRSaveDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();

	DestroyWindow();
}
