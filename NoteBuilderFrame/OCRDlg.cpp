#include "stdafx.h"
#include "NoteBuilderFrame.h"

#include "B_Common.h"

#include "ST150Image.h"
#include "CurrencyInfo.h"
#include "ICUMANDoc.h"
#include "ICUMANView.h"
#include "DebugMsg.h"

#include "OCRDlg.h"
#include "OCRSaveDlg.h"
#include "CodeParser.h"
#include "MacroValue.h"
#include "OCRStuff.h"

COCRDlg* theOCRDlg = NULL;

#define	MAX_SER_NUMBERS	16

/*
long HiddenWeight[MAX_SER_NUMBERS][MAX_WEIGHT_SIZE];
int  HiddenWeightCount[MAX_SER_NUMBERS] = {0};
long OutputWeight[MAX_SER_NUMBERS][MAX_WEIGHT_SIZE];
int  OutputWeightCount[MAX_SER_NUMBERS] = {0};
int  ClassNameCount[MAX_SER_NUMBERS] = {0};
char ClassName[MAX_SER_NUMBERS][MAX_CLASS][NAME_SIZE];

int  m_HiddenNodeSize[MAX_SER_NUMBERS];
int  m_InputNodeSize[MAX_SER_NUMBERS];
int  m_ClassSize[MAX_SER_NUMBERS];
*/

void ReadSaveWidthHeightFromFile();

// COCRDlg dialog

IMPLEMENT_DYNAMIC(COCRDlg, CDialog)

COCRDlg::COCRDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COCRDlg::IDD, pParent)
{
	pViewWnd = (CICUMANView*)pParent;
	wBitmap = NULL;
	irBitmap = NULL;

	ClearPosInfo();

	if(pWinCurrentLocal != NULL) {
		switch(pWinCurrentLocal->CurrencyNum) {
		case EUR :
			NofChars = 12;
			break;
		default :
			NofChars = 12;
		}
	}
}

int sumarr[600];

void COCRDlg::ClearPosInfo()
{
#if CURRENCY
	OCR_y_pos_0 = 0;
	OCR_y_pos_1 = 0;
	OCR_pos_cnt = 0;
#endif
}

COCRDlg::~COCRDlg()
{
	if(wBitmap != NULL) delete wBitmap;
	if(irBitmap != NULL) delete irBitmap;
}

void COCRDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TOT_PIXELS, m_EditTotalPixels);
	DDX_Control(pDX, IDC_EDIT_W_THRESHOLD, m_EditWThreshold);
	DDX_Control(pDX, IDC_EDIT_IR_THRESHOLD, m_EditIrThreshold);
	DDX_Control(pDX, IDC_EDIT_W_PIXELS, m_EditWPixels);
	DDX_Control(pDX, IDC_EDIT_IR_PIXELS, m_EditIrPixels);
	DDX_Control(pDX, IDC_EDIT_H_WINDOW, m_EditHWindow);
	DDX_Control(pDX, IDC_EDIT_V_WINDOW, m_EditVWindow);
	DDX_Control(pDX, IDC_LIST_IMAGE, m_ListImage);
	DDX_Control(pDX, IDC_EDIT_NNR_FOLDER, m_EditNnrFolder);
	DDX_Control(pDX, IDC_EDIT_NOF_CHARS, m_EditNofChars);
	DDX_Control(pDX, IDC_EDIT_H_WINDOW2, m_EditHWindow2);
	DDX_Control(pDX, IDC_EDIT_V_WINDOW2, m_EditVWindow2);
	DDX_Control(pDX, IDC_COMBO_SOURCE, m_ComboSource);
	DDX_Control(pDX, IDC_EDIT_W_PCENT, m_EditWPcnt);
	DDX_Control(pDX, IDC_EDIT_IR_PCENT, m_EditIrPcnt);
	DDX_Control(pDX, IDC_COMBO_HMETHOD, m_ComboHWindowMethod);
	DDX_Control(pDX, IDC_COMBO_THR_TYPE, m_comboThrType);
	DDX_Control(pDX, IDC_COMBO_SCALEX, m_ComboScaleX);
	DDX_Control(pDX, IDC_COMBO_SCALEY, m_ComboScaleY);
}


BEGIN_MESSAGE_MAP(COCRDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_APP_THRESHOLD, &COCRDlg::OnBnClickedButtonAppThreshold)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, &COCRDlg::OnBnClickedOk)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_COMPAREITEM()
	ON_BN_CLICKED(IDC_BUTTON_APP_WINDOW, &COCRDlg::OnBnClickedButtonAppWindow)
	ON_BN_CLICKED(IDC_BUTTON_APP_PIXELS, &COCRDlg::OnBnClickedButtonAppPixels)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &COCRDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &COCRDlg::OnBnClickedButtonSave)
	ON_CBN_SELCHANGE(IDC_COMBO_SOURCE, &COCRDlg::OnCbnSelchangeComboSource)
	ON_CBN_SELCHANGE(IDC_COMBO_HMETHOD, &COCRDlg::OnCbnSelchangeComboHmethod)
	ON_CBN_SELCHANGE(IDC_COMBO_THR_TYPE, &COCRDlg::OnCbnSelchangeComboThrType)
	ON_BN_CLICKED(IDC_BUTTON_DEBUG, &COCRDlg::OnBnClickedButtonDebug)
	ON_BN_CLICKED(IDC_BUTTON_COPY_DATA, &COCRDlg::OnBnClickedButtonCopyData)
	ON_CBN_SELCHANGE(IDC_COMBO_SCALEX, &COCRDlg::OnCbnSelchangeComboScale)
	ON_BN_CLICKED(IDC_BUTTON1, &COCRDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

// COCRDlg message handlers

BOOL COCRDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	lstrcpy(NNRFolder, RegNNRFolder);
	m_EditNnrFolder.SetWindowTextW(RegNNRFolder);
	
	CCodeParser *cp = new CCodeParser();

	OCR_VWindow_2.Set(0); // 이 값이 나중에도 0이면, 1하고 같은 값을 가지게 해준다.
	delete cp;

	if(OCR_VWindow_2.Get() == 0)
		OCR_VWindow_2.Set(OCR_VWindow.Get());

	SetDlgItemInt(m_EditNofChars.GetDlgCtrlID(), NofChars);

	m_ComboSource.AddString(L"IR Transmit");
	m_ComboSource.AddString(L"W Reflection");
	m_ComboSource.AddString(L"Combination");
	m_ComboSource.SetCurSel(OCR_ImageSource.Get());

	m_ComboHWindowMethod.AddString(L"Count");
	m_ComboHWindowMethod.AddString(L"Sum of Value");
	m_ComboHWindowMethod.AddString(L"Given Width");

	m_ComboHWindowMethod.SetCurSel(OCR_xwindow_method.Get());

	m_comboThrType.AddString(L"Full");
	m_comboThrType.AddString(L"Vert window");
	m_comboThrType.SetCurSel(OCR_ThrType.Get());

	SetDlgItemInt(m_EditIrPcnt.GetDlgCtrlID(), OCR_IR_Percent.Get());
	SetDlgItemInt(m_EditWPcnt.GetDlgCtrlID(), OCR_W_Percent.Get());

	SetDlgItemInt(m_EditHWindow.GetDlgCtrlID(), OCR_HWindow.Get());// - OCR_HWINDOW에서 OCR_X_WINDOW로 변경함 dschae 20121222
	SetDlgItemInt(m_EditVWindow.GetDlgCtrlID(), OCR_VWindow.Get());
	SetDlgItemInt(IDC_EDIT_V_WIN2, OCR_VWindow_2.Get());

	SetDlgItemInt(IDC_EDIT_LEAST_PIXEL, least_pixel.Get());

	SetDlgItemInt(IDC_EDIT_F_HEIGHT, OCR_f_height.Get());

	if(pViewWnd->ExtBitmap) {
		MakeDataFromBitmap(pViewWnd->ExtBitmap);
	}

	m_ComboScaleX.AddString(L"1");
	m_ComboScaleX.AddString(L"2");
	m_ComboScaleX.AddString(L"3");
	m_ComboScaleX.SetCurSel(OCR_scaleX.Get());
	
	m_ComboScaleY.AddString(L"1");
	m_ComboScaleY.SetCurSel(OCR_scaleY.Get());

	ReadSaveWidthHeightFromFile();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

int ApplySelected = 2;

unsigned char w_data[OCR_Y_MAX * OCR_X_MAX];
unsigned char ir_data[OCR_Y_MAX * OCR_X_MAX];
unsigned char copy_w_data[OCR_Y_MAX * OCR_X_MAX];
unsigned char copy_ir_data[OCR_Y_MAX * OCR_X_MAX];


extern "C" void AdjustImageDirection(unsigned short Dir, unsigned char* w_data, unsigned char* ir_data, int width, int height);

void COCRDlg::MakeDataFromBitmap(CBitmap *bitmap)
{
	if(bitmap == NULL)
		return;

	ClearPosInfo();

	bitmap->GetBitmap(&m_bitmap);
	m_Width = (m_bitmap.bmWidth-5) / 2; // 비트맵 이미지는 두 개이고, 중간에 5픽셀 공간이 있으므로 이렇게 한다.
	m_Height = m_bitmap.bmHeight;

	CDC *pDC = GetDC();
	CDC dc;
	dc.CreateCompatibleDC(pDC);
	ReleaseDC(pDC);
	CBitmap *obit = dc.SelectObject(bitmap);

	if(m_Height > m_Width) {
		// 세로 시리얼 이다. 이런 경우에는 이미지를 돌려야 한다.
		int temp = m_Height;
		m_Height = m_Width;
		m_Width = temp;

		for(int y = 0; y < m_Height && y < OCR_Y_MAX; y++) {
			int ypos = y * m_Width;
			for(int x = 0; x < m_Width && x < OCR_X_MAX; x++) {
				int xx = m_Width-x-1;
				unsigned int ui = dc.GetPixel(y, x);
				w_data [ypos + xx] = ui;
				ir_data[ypos + xx] = dc.GetPixel(y+m_Height+5, x);
			}
		}
	} else {
		for(int y = 0; y < m_Height && y < OCR_Y_MAX; y++) {
			for(int x = 0; x < m_Width && x < OCR_X_MAX; x++) {
				unsigned int ui = dc.GetPixel(x, y);
				w_data [y*m_Width + x] = ui;
				ir_data[y*m_Width + x] = dc.GetPixel(x+m_Width+5, y);
			}
		}
	}

	dc.SelectObject(obit);
	dc.DeleteDC();

	m_TotalPixels = m_Width * m_Height;

	SetDlgItemInt(m_EditTotalPixels.GetDlgCtrlID(), m_TotalPixels);

#if CURRENCY // license plate is always recognizable, so do not check it
	RemoveMetalThread(ir_data, m_Width, m_Height, 45);
	// 방향을 정방향으로 바꾼다.
	AdjustImageDirection(pViewWnd->imgST150.OneNote.Dir, w_data, ir_data, m_Width, m_Height);
#endif

#if USE_IR_Y_SUM
	for(int x = 0; x < m_Width; x++) {
		ir_y_sum[x] = 0;
		for(int y = 0; y < m_Height; y++)
			ir_y_sum[x] += ir_data[y][x];
	}
#endif
#if USR_IR_X_SUM
	for(int y = 0; y < m_Height; y++) {
		ir_x_sum[y] = 0;
		for(int x = 0; x < m_Width; x++)
			ir_x_sum[y] += ir_data[y][x];
	}
#endif
	ApplyPercent();

	OnBnClickedButtonAppWindow();
}

void COCRDlg::OnBnClickedButtonAppThreshold()
{
	m_WThreshold = GetDlgItemInt(m_EditWThreshold.GetDlgCtrlID());
	m_IrThreshold = GetDlgItemInt(m_EditIrThreshold.GetDlgCtrlID());

	ApplyThreshold();
}

extern "C" {
	int CountUnderThreshold(int threshold, unsigned char* w_data, unsigned char* copy_w_data, int width, int height);
}

void COCRDlg::ApplyThreshold()
{
#if CURRENCY // license plate is always recognizable, so do not check it
	// CountUnderThreshold 함수는 threshold 값 보다 큰 것은 255(최대치)로 채운다. 그 이하의 것은 갯수를 센다.
	m_W_ResultCount = CountUnderThreshold(m_WThreshold, w_data, copy_w_data, m_Width, m_Height);
	m_IR_ResultCount = CountUnderThreshold(m_IrThreshold, ir_data, copy_ir_data, m_Width, m_Height);

	SetDlgItemInt(IDC_EDIT_W_PIXELS_R,  m_W_ResultCount);
	SetDlgItemInt(IDC_EDIT_IR_PIXELS_R, m_IR_ResultCount);
	
	MakeCopyImage();
#endif
}

void COCRDlg::MakeCopyImage()
{
	CDC *pDC = GetDC();
	CDC dc;
	dc.CreateCompatibleDC(pDC);

	if(wBitmap != NULL) delete wBitmap;
	wBitmap = new CBitmap();
	wBitmap->CreateCompatibleBitmap(pDC, m_Width, m_Height);

	if(irBitmap != NULL) delete irBitmap;
	irBitmap = new CBitmap();
	irBitmap->CreateCompatibleBitmap(pDC, m_Width, m_Height);
	
	ReleaseDC(pDC);

	CBitmap* obitmap = dc.SelectObject(wBitmap);
	for(int y = 0; y < m_Height; y++) {
		for(int x = 0; x < m_Width; x++) {
			unsigned char c = copy_w_data[y*m_Width + x];
			COLORREF rgb = RGB(c,c,c);
			dc.SetPixel(x, y, rgb);
		}
	}
	dc.SelectObject(obitmap);

	obitmap = dc.SelectObject(irBitmap);
	for(int y = 0; y < m_Height; y++) {
		for(int x = 0; x < m_Width; x++) {
			unsigned char c = copy_ir_data[y*m_Width + x];
			COLORREF rgb = RGB(c,c,c);
			if(c != 255)
				rgb = RGB(c,c,c);
			dc.SetPixel(x, y, rgb);
		}
	}
	dc.SelectObject(obitmap);

	while(m_ListImage.GetCount() > 0)
		m_ListImage.DeleteString(0);
	while(m_ListImage.GetCount() < 3)
		m_ListImage.AddString(L"EMPTY IMAGE");
	m_ListImage.Invalidate();
}

void COCRDlg::ApplyPixels(int ir_count, int w_count)
{
#if CURRENCY // license plate is always recognizable, so do not check it
	if(OCR_ThrType.Get() != 0) {
		if(OCR_ImageSource.Get() == 0)
			FindVerticalWindow(ir_data, m_Width, m_Height, OCR_VWindow.Get());
		else
			FindVerticalWindow(w_data, m_Width, m_Height, OCR_VWindow.Get());
		m_IrThreshold = GetThresholdByPixelsYY(ir_freq, ir_data, m_Width, m_Height, ir_count);
		m_WThreshold = GetThresholdByPixelsYY(w_freq, w_data, m_Width, m_Height, w_count);
	} else {
		m_IrThreshold = GetThresholdByPixels(ir_freq, ir_data, m_Width, m_Height, ir_count);
		m_WThreshold = GetThresholdByPixels(w_freq, w_data, m_Width, m_Height, w_count);
	}

	ApplyThreshold();

	SetDlgItemInt(m_EditIrThreshold.GetDlgCtrlID(), m_IrThreshold);
	SetDlgItemInt(m_EditWThreshold.GetDlgCtrlID(), m_WThreshold);
#endif
}

void COCRDlg::ApplyWindow()
{
}


void COCRDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	
	delete theOCRDlg;
	theOCRDlg = NULL;
}

void COCRDlg::OnClose()
{
	CDialog::OnClose();

	DestroyWindow();
}

void COCRDlg::OnBnClickedOk()
{
	DestroyWindow();
}

void COCRDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if(nIDCtl != IDC_LIST_IMAGE) {
		CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
		return;
	}

	lpMeasureItemStruct->itemWidth = m_Width * 2 + 2;
	switch(lpMeasureItemStruct->itemID){
	case 0 :
	case 1 :
		lpMeasureItemStruct->itemHeight = m_Height * 2 + 2;
		break;
	case 2 :
		lpMeasureItemStruct->itemHeight = m_Height * 4 + 2;
	}
}

#define	DRAWITEM_SCALE	2

void DrawFreqGraph(CDC* pDC, int* ir_freq, RECT r, int Threshold)
{
	pDC->MoveTo(r.left, ir_freq[0] + r.top);
	for(int x = 1; x < 256; x++)
		pDC->LineTo(r.left+x, ir_freq[x] + r.top);
	{
		CPen pen;
		pen.CreatePen(PS_SOLID, 1, RGB(128,0,255));
		CPen *open = pDC->SelectObject(&pen);
		pDC->MoveTo(r.left+Threshold, r.top);
		pDC->LineTo(r.left+Threshold, r.top+100);
		pDC->SelectObject(open);
		pen.DeleteObject();
	}
}

void COCRDlg::DrawCharPos(CDC* pDC, RECT r)
{
#if CURRENCY
	for(int pc = 0; pc < OCR_pos_cnt; pc++) {
		CPen p;
		p.CreatePen(PS_SOLID, 1, RGB(0,128,255));
		CPen *open = pDC->SelectObject(&p);
		pDC->MoveTo(r.left+OCR_x_pos[pc]*DRAWITEM_SCALE, r.top+0);
		pDC->LineTo(r.left+OCR_x_pos[pc]*DRAWITEM_SCALE, r.top+8);
		pDC->MoveTo(r.left+(OCR_x_pos[pc]+OCR_x_len[pc])*DRAWITEM_SCALE, r.top+0);
		pDC->LineTo(r.left+(OCR_x_pos[pc]+OCR_x_len[pc])*DRAWITEM_SCALE, r.top+8);
		pDC->SelectObject(open);
		p.DeleteObject();
	}
	if(OCR_y_pos_0 > 0) {
		CPen p;
		p.CreatePen(PS_SOLID, 1, RGB(255,255,0));
		CPen *open = pDC->SelectObject(&p);
		pDC->MoveTo(r.left, r.top+(OCR_y_pos_0-1)*DRAWITEM_SCALE);
		pDC->LineTo(r.right, r.top+(OCR_y_pos_0-1)*DRAWITEM_SCALE);
		pDC->SelectObject(open);
		p.DeleteObject();
	}
	if(OCR_y_pos_1 > 0) {
		CPen p;
		p.CreatePen(PS_SOLID, 1, RGB(255,255,0));
		CPen *open = pDC->SelectObject(&p);
		pDC->MoveTo(r.left, r.top+(OCR_y_pos_1)*DRAWITEM_SCALE);
		pDC->LineTo(r.right, r.top+(OCR_y_pos_1)*DRAWITEM_SCALE);
		pDC->SelectObject(open);
		p.DeleteObject();
	}
#endif
}

void COCRDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if(nIDCtl != IDC_LIST_IMAGE) {
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
		return;
	}

	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CDC srcdc;
	srcdc.CreateCompatibleDC(pDC);

	CBitmap* obitmap = NULL;

	int left = lpDrawItemStruct->rcItem.left;
	int top = lpDrawItemStruct->rcItem.top;
		
	switch(lpDrawItemStruct->itemID){
	case 0 :
		obitmap = srcdc.SelectObject(wBitmap);
		pDC->StretchBlt(left, top, m_Width*2, m_Height*2, &srcdc, 0, 0, m_Width, m_Height, SRCCOPY);
		if(OCR_ImageSource.Get() != 0)
			DrawCharPos(pDC, lpDrawItemStruct->rcItem);
		break;
	case 1 :
		obitmap = srcdc.SelectObject(irBitmap);
		pDC->StretchBlt(left, top, m_Width*DRAWITEM_SCALE, m_Height*DRAWITEM_SCALE, &srcdc, 0, 0, m_Width, m_Height, SRCCOPY);
		if(OCR_ImageSource.Get() == 0)
			DrawCharPos(pDC, lpDrawItemStruct->rcItem);
		break;
	case 2 :
		if(OCR_ImageSource.Get() == 0)
			DrawFreqGraph(pDC, ir_freq, lpDrawItemStruct->rcItem, m_IrThreshold);
		else
			DrawFreqGraph(pDC, w_freq, lpDrawItemStruct->rcItem, m_WThreshold);
		break;
	}

	if(obitmap != NULL)
		srcdc.SelectObject(obitmap);
	srcdc.DeleteDC();
}

int COCRDlg::OnCompareItem(int nIDCtl, LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	if(nIDCtl == IDC_LIST_IMAGE)
		return lpCompareItemStruct->itemID2 - lpCompareItemStruct->itemID1;

	return CDialog::OnCompareItem(nIDCtl, lpCompareItemStruct);
}

static char srcbuf[2048];

void ReplaceTagTextInFile(TCHAR* OCR_ConstFileName, char* tag1, char* txt1)
{
	int fsize = LoadFile(OCR_ConstFileName, srcbuf, sizeof(srcbuf));
	if(fsize <= 0) {
		HANDLE h = CreateFile(OCR_ConstFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h != INVALID_HANDLE_VALUE) {
			DWORD dwio = 0;
			WriteFile(h, txt1, strlen(txt1), &dwio, NULL);
			CloseHandle(h);
		}
		return;
	}

	int dstsize = fsize + 1024;
	char* dstbuf = (char*)malloc(dstsize);
	strcpy_s(dstbuf, dstsize, srcbuf);

	ReplaceTagText(dstbuf, dstsize, tag1, txt1);

	HANDLE h = CreateFile(OCR_ConstFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h != INVALID_HANDLE_VALUE) {
		DWORD dwio = 0;
		WriteFile(h, dstbuf, strlen(dstbuf), &dwio, NULL);
		CloseHandle(h);
	}
	free(dstbuf);
}

void SaveWindowValues()
{
}

void FindHorizontalIndonesia(int img_width, int ys, int ye, unsigned char* copied_data)
{
	int x, y;
	int pmin, minx, minv;
	// 일단 수직 합을 OCR_VWindow 크기로 구한다.
	for(x = 0; x < img_width; x++) {
		sumarr[x] = 0;
		for(y = ys; y < ye; y++) {
			sumarr[x] += copied_data[y*img_width + x];
		}
	}
#define	IDR_SERIMG_WIDTH	222
	// IDR_SERIMG_WIDTH 단위의 합을 구한다
	pmin = 0;
	for(x = 0; x < IDR_SERIMG_WIDTH; x++) pmin += sumarr[x];
	minx = 0;
	minv = pmin;
	for(x = 1; x < img_width-IDR_SERIMG_WIDTH; x++) {
		int tmin = pmin - sumarr[x-1] + sumarr[x+IDR_SERIMG_WIDTH-1];
		if(tmin < minv) {
			minv = tmin;
			minx = x;
		}
		pmin = tmin;
	}

	// 맨 왼쪽 글자의 좌측에 여백이 많고, 맨 우측 글자의 좌측에도 여백이 많으면
	// '1'때문에 좌측으로 밀린 거니까, 우측으로 이동
	int mpos = 0;
	int white_limit = 255 * (OCR_VWindow.Get() - 1);
	if(sumarr[minx+0] >= white_limit && sumarr[minx+1] >= white_limit)
		mpos = 2;
	else if(sumarr[minx+1] >= white_limit && sumarr[minx+2] >= white_limit)
		mpos = 3;
	else if(sumarr[minx+2] >= white_limit && sumarr[minx+3] >= white_limit)
		mpos = 4;
	if(mpos > 1) {
		while(sumarr[minx+mpos] >= white_limit) {
			mpos++;
		}
	}
	if(mpos > 1)
		minx += mpos;

#if CURRENCY
// 20 44 66 88 112 135 162 189 220
	OCR_pos_cnt = 9;
	OCR_x_pos[0] = minx;
	OCR_x_pos[1] = minx + 24;
	OCR_x_pos[2] = minx + 46;
	OCR_x_pos[3] = minx + 68;
	OCR_x_pos[4] = minx + 92;
	OCR_x_pos[5] = minx + 115;
	OCR_x_pos[6] = minx + 142;
	OCR_x_pos[7] = minx + 169;
	OCR_x_pos[8] = minx + 199;

	OCR_x_len[0] = 16;
	OCR_x_len[1] = 16;
	OCR_x_len[2] = 16;
	OCR_x_len[3] = 18;
	OCR_x_len[4] = 19;
	OCR_x_len[5] = 20;
	OCR_x_len[6] = 21;
	OCR_x_len[7] = 22;
	OCR_x_len[8] = 23;

	// 각각의 글자에 대해서 좌우 흔들어 보기, 좌우 끝단은 고려하지 않고 중간에 있는 글자들만 처리한다.
	while(1) {
		int move_to_right = 0;
		int moved[10];
		moved[8] = 0;
		for(x = 0; x < 8; x++) {
			moved[x] = 0;
			if(sumarr[OCR_x_pos[x]-1] < sumarr[OCR_x_pos[x]+OCR_x_len[x]-1]) { // 좌측에 작은 게 있으면 좌로 이동
				OCR_x_pos[x] -= 1;
				moved[x] = -1; // 좌측으로 이동했다는 표시
			} else if(sumarr[OCR_x_pos[x]] > sumarr[OCR_x_pos[x]+OCR_x_len[x]]) {
				// 우측에 작은 게 있으면 우로 이동
				OCR_x_pos[x] += 1;
				move_to_right++;
				moved[x] = 1; // 우측으로 이동했다는 표시
			}
		}
/* 이렇게 해도 붉은 글씨 오른쪽 끝이 '1'이면 노이즈 때문에 붙들려서 안 끌려온다.
		if(move_to_right > 4) { // 오른 쪽 끝의 '1' 때문에 심하게 좌로 치우친 상태이므로 우측으로 전체가 이동한다.
			for( x = 0; x < 9; x++) {
				// 왼쪽에 있는 글자가 7인 경우 7의 우측 음영에 붙들려서 못 오는 경우가 있는데, 이 경우도 끌어당긴다.
				OCR_x_pos[x] += (1 - moved[x]); // 만약에 왼쪽으로 움직였던 것이라면 두칸 우측으로 움직이게 된다.
			}
		} else {
			break;
		}
*/
		break; // 한 칸만 좌우측으로 이동하게 했다.
	}
	// 맨 마지막 글자는 좌측으로만 이동한다.
	if(sumarr[OCR_x_pos[8]-1] < sumarr[OCR_x_pos[8]+OCR_x_len[8]-1]) { // 좌측에 작은 게 있으면 좌로 이동
		OCR_x_pos[8] -= 1;
	}
#endif
}

int l_move[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int r_move[16] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void COCRDlg::OnBnClickedButtonAppWindow()
{
	int modified = 0;

	int hwindow  = GetDlgItemInt(m_EditHWindow.GetDlgCtrlID());
	int vwindow = GetDlgItemInt(m_EditVWindow.GetDlgCtrlID());
	int vw2 = GetDlgItemInt(IDC_EDIT_V_WIN2);

	OCR_f_height.Set(GetDlgItemInt(IDC_EDIT_F_HEIGHT));

	OCR_HWindow.Set(hwindow);
	OCR_VWindow.Set(vwindow);

	least_pixel.Set(GetDlgItemInt(IDC_EDIT_LEAST_PIXEL));
	if(vw2 != 0 && vw2 != OCR_VWindow_2.Get()) {
		OCR_VWindow_2.Set(vw2);
		modified = 1;
		if(OCR_ThrType.Get() > 0)
			ApplyPercent(); // 임계값을 재계산하도록 한다.
	}

	SaveWindowValues();

#if CURRENCY
	int ys, ye;
	unsigned char* copied_data = OCR_ImageSource.Get() == 0 ? copy_ir_data : copy_w_data;
	// 상하 찾기 먼저 수행한다.
	if(OCR_VWindow.Get() != OCR_VWindow_2.Get()) {
		// 인도네시아 테스트, 일단 상하 방향 윈도우 14 기준으로 구한다
		FindVerticalWindowSE(copied_data, m_Width, m_Height, OCR_VWindow.Get(), &ys, &ye);
		// 이걸로 20짜리를 만든다
		ye += 1;
		OCR_y_pos_1 = ye;
		OCR_y_pos_0 = ye - OCR_VWindow_2.Get();

		SetDlgItemInt(m_EditVWindow2.GetDlgCtrlID(), OCR_VWindow_2.Get());
	} else {
		int length = FindVerticalWindow(copied_data, m_Width, m_Height, OCR_VWindow.Get());
		SetDlgItemInt(m_EditVWindow2.GetDlgCtrlID(), length);
	}
#endif

#if CURRENCY
	int y_size = OCR_y_pos_1 - OCR_y_pos_0;
	if(OCR_HWindow.Get() && y_size > 0) {
		int max_length = 0;
		if(OCR_xwindow_method.Get() == 2) {
			OCR_pos_cnt = OCR_save_width_height_count.Get();
			FindHorizontalWindowsEx(copied_data, m_Width, OCR_pos_cnt, ORG_x_pos, ORG_x_len, ORG_y_len, l_move, r_move);
			int last_count = 0;
			for(int y1 = OCR_y_pos_0; y1 < OCR_y_pos_1; y1++) {
				int yyx = y1 * m_Width;
				for(int x1 = OCR_x_pos[9]; x1 < OCR_x_pos[9]+OCR_x_len[9]; x1++) {
					if(copied_data[yyx + x1] < 255)
						last_count++;
				}
			}
			if(last_count < 20) {
				// 10자리 시리얼인줄 알았는데 9자리인 경우 - GBP가 이렇다.
				int ORA_x_pos[MAX_CHAR_X_POS];
				OCR_pos_cnt = OCR_save_width_height_count.Get()-1;
				for(int i = 0; i < OCR_pos_cnt; i++) // 글자의 위치가 두번째부터 0기반으로 수정된 것을 사용해야 한다.
					ORA_x_pos[i] = ORG_x_pos[i+1] - ORG_x_pos[1];
				FindHorizontalWindowsEx(copied_data, m_Width, OCR_pos_cnt, ORA_x_pos, &ORG_x_len[1], &ORG_y_len[1], l_move, r_move);
			}
		} else {
			if(OCR_VWindow.Get() == OCR_VWindow_2.Get()) {
				OCR_pos_cnt = FindHorizontalWindows(copied_data, m_Width, OCR_HWindow.Get(), &max_length, OCR_maxchar.Get()+1,
					least_pixel.Get(), 0, OCR_xwindow_method.Get());
				while(OCR_pos_cnt > OCR_maxchar.Get()) {
					int ci;
					if(OCR_x_pos[1] - OCR_x_pos[0] > OCR_x_pos[OCR_pos_cnt-1]-OCR_x_pos[OCR_pos_cnt-2]) {
						// 첫번째가 노이즈이다
						for(ci = 1; ci < OCR_pos_cnt; ci++) {
							OCR_x_pos[ci-1] = OCR_x_pos[ci];
							OCR_x_len[ci-1] = OCR_x_len[ci];
						}
					}
					OCR_pos_cnt--;
				}
			} else {
				// 인도네시아를 위해서 임시로 만든 기능이다.
				FindHorizontalIndonesia(m_Width, OCR_y_pos_0, OCR_y_pos_1, copied_data);
			}
		}

		// m_HWindow = max_length;
		int x;
		for(x = 0; x < OCR_pos_cnt; x++) {
			if(max_length < OCR_x_len[x])
				max_length = OCR_x_len[x];
		}
		SetDlgItemInt(m_EditHWindow2.GetDlgCtrlID(), max_length);
	}
#endif
	m_ListImage.Invalidate();

	if(theOCRSaveDlg != NULL)
		OnBnClickedButtonSave();
}

void COCRDlg::OnBnClickedButtonAppPixels()
{
	int tircount = GetDlgItemInt(m_EditIrPixels.GetDlgCtrlID());
	int twcount = GetDlgItemInt(m_EditWPixels.GetDlgCtrlID());
	int tirpcent = GetDlgItemInt(m_EditIrPcnt.GetDlgCtrlID());
	int twpcent = GetDlgItemInt(m_EditWPcnt.GetDlgCtrlID());

	if(tircount != m_IR_ExpectedCount || twcount != m_W_ExpectedCount) {
		m_IR_ExpectedCount = tircount;
		m_W_ExpectedCount = twcount;
		ApplySelected = 1;
		ApplyPixels(m_IR_ExpectedCount, m_W_ExpectedCount);
		return;
	}

	if(OCR_IR_Percent.Set(tirpcent) || OCR_W_Percent.Set(twpcent)) {
		ApplySelected = 2;
		ApplyPercent();
		SaveWindowValues();
		return;
	}

	switch(ApplySelected) {
	case 1 :
		ApplyPixels(m_IR_ExpectedCount, m_W_ExpectedCount);
		break;
	case 2 :
		ApplyPercent();
	}
}

// 초기 셋팅 시 '경로'를 무조건 나오게 할려면
int CALLBACK OCRDLG_BICallBackFunc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
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

void COCRDlg::OnBnClickedButtonBrowse()
{
	ITEMIDLIST*  pildBrowse;
	TCHAR   pszPathname[MAX_PATH];
	BROWSEINFO  bInfo;

	memset(&bInfo, 0, sizeof(bInfo));

	bInfo.hwndOwner   = GetSafeHwnd();
	bInfo.pidlRoot   = NULL;
	bInfo.pszDisplayName = pszPathname;
	bInfo.lpszTitle   = _T("디렉토리를 선택하세요");             // dialog title
	bInfo.ulFlags   = BIF_RETURNONLYFSDIRS|BIF_VALIDATE | BIF_NEWDIALOGSTYLE;                 // 여러가지 옵션이 존재합니다.. msdn을 참고해보세요.
	bInfo.lpfn    = OCRDLG_BICallBackFunc;                        // 옵션? 입니다.. 초기 경로 설정등을 할때는 여기에 추가하세요.
	bInfo.lParam  = (LPARAM)(LPCTSTR)RegNNRFolder;

	pildBrowse    = ::SHBrowseForFolder(&bInfo);

	if(!pildBrowse)
		return;

	SHGetPathFromIDList(pildBrowse, pszPathname);

	lstrcpy(NNRFolder, (LPCTSTR)pszPathname);                                // => 실제 선택한 폴더가 전체 경로로 나옵니다.
	m_EditNnrFolder.SetWindowTextW(NNRFolder);
	RegSetNNRFolder();
}


void COCRDlg::OnBnClickedButtonSave()
{
	int create_save_dlg = 0;
	
#if CURRENCY
	if(OCR_pos_cnt == 0 || OCR_y_pos_1 == 0) {
		OCR_pos_cnt = 1;
		return;
	}

	if(theOCRSaveDlg == NULL) {
		theOCRSaveDlg = new COCRSaveDlg();
		create_save_dlg = 1;
	}

	theOCRSaveDlg->y_pos_0 = OCR_y_pos_0;
	theOCRSaveDlg->img_width = m_Width;

	for(int y = 0; y < m_Height; y++) {
		for(int x = 0; x < m_Width; x++) {
			int flat_index = y*m_Width + x;
			if(OCR_ImageSource.Get() == 0)
				theOCRSaveDlg->ocr_data[flat_index] = copy_ir_data[flat_index];
			else
				theOCRSaveDlg->ocr_data[flat_index] = copy_w_data[flat_index];
		}
	}

	theOCRSaveDlg->v_window = OCR_VWindow_2.Get();

	if(create_save_dlg) {
		theOCRSaveDlg->Create(IDD_OCR_SAVE, this); // Create를 호출하면 OnInitDialog();가 호출된다.
		theOCRSaveDlg->ShowWindow(SW_SHOW);
	}
#endif
	theOCRSaveDlg->InitialProcess();
}

unsigned char OneLineImage[900];

unsigned char FeatureInBytes[900];

unsigned char test_char[] = {
	0x38,0x3F,0xAF,0xFF,0xFF,0xFF,0x9E,0x18, 0x9F,0x34,0x24,0xBD,0xFF,0xFF,0xFF,0x9C,
	0x04,0x98,0x3A,0x23,0xBB,0xFF,0xFF,0xFF, 0xA3,0x0F,0x50,0x34,0x30,0xB5,0xFF,0xFF,
	0xFF,0xA4,0x02,0x93,0x8C,0x0F,0x23,0x1D, 0x17,0x83,0xA9,0x0A,0x4F,0x45,0x20,0x5C,
	0x65,0x6F,0x4A,0x15,0x01,0x4E,0x52,0x1A, 0xAC,0xFF,0xFF,0xB9,0x6B,0x00,0x41,0x1A,
	0x35,0xFF,0xFF,0xFF,0xFF,0x35,0x0D,0xBB, 0x0F,0x32,0xFF,0xFF,0xFF,0xFF,0x40,0x06,
	0xAE,0x0E,0x27,0xFF,0xFF,0xFF,0xFF,0x3B, 0x06,0xAC,0x18,0x2A,0xFF,0xFF,0xFF,0xFF,
	0x44,0x00,0xA3,0x0E,0x23,0xBE,0xFF,0xFF, 0xFF,0x3F,0x00,0x98,0x6C,0x73,0xFF,0xFF,
	0xFF,0xFF,0x3A,0x04,0x54,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x33,0xC9,
	0x26,0x8A,0x09,0x43,0x26,0x3A,0x01,0x74, 0x04,0xE2,0xF8,0x3C,0x0D,0x59,0x5B,0xC3,
	0x53,0x51,0x2E,0xC6,0x06,0xFF,0xB7,0x20, 0x2E,0x80,0x26,0x05,0xB8,0xDF,0x3C,0x20,
	0x74,0x36,0x3C,0x09,0x74,0x32,0x3C,0x2C, 0x74,0x31,0x3C,0xA1,0x75,0x0C,0x80,0x3C,
	0xA1,0x75,0x07,0xB0,0x20,0x46,0x3A,0xC0, 0xEB,0x1E,0x26,0x80,0x7D,0x02,0x01,0x72,
	0x17,0x33,0xC9,0x26,0x8A,0x4D,0x03,0x0B, 0xC9,0x74,0x0B,0xBB,0x03,0x00,0x43,0x26,
	0x3A,0x01,0x74,0x07,0xE2,0xF8,0x3C,0x20, 0x59,0x5B,0xC3,0x2E,0xA2,0xFF,0xB7,0x2E,
	0xF6,0x06,0x05,0xB8,0x01,0x75,0x06,0x2E, 0x80,0x0E,0x05,0xB8,0x20,0x3A,0xC0,0xEB,
	0xE7,0x8D,0x2E,0x0E,0xB8,0x3B,0xDD,0x74, 0x1A,0x3C,0x2F,0x75,0x13,0xF9,0x50,0x2E,
};

extern "C" struct ST_NNR_RESULT gst_NNR_Result;

void SaveToBitmap(TCHAR* path, unsigned char* buf, int x_size, int y_size);

void COCRDlg::OnCbnSelchangeComboSource()
{
	OCR_ImageSource.Set(m_ComboSource.GetCurSel());
}

void COCRDlg::ApplyPercent()
{
	if(OCR_ThrType.Get()) { // Vertical Window
		m_IR_ExpectedCount = ((m_Width * OCR_VWindow_2.Get()) * OCR_IR_Percent.Get()) / 1000;
		m_W_ExpectedCount  = ((m_Width * OCR_VWindow_2.Get()) * OCR_W_Percent.Get()) / 1000;
	} else {
		m_IR_ExpectedCount = ((m_Width * m_Height) * OCR_IR_Percent.Get()) / 1000;
		m_W_ExpectedCount  = ((m_Width * m_Height) * OCR_W_Percent.Get()) / 1000;
	}

	SetDlgItemInt(IDC_EDIT_IR_PIXELS, m_IR_ExpectedCount);
	SetDlgItemInt(IDC_EDIT_W_PIXELS,  m_W_ExpectedCount);

	ApplyPixels(m_IR_ExpectedCount, m_W_ExpectedCount);
}

void COCRDlg::OnCbnSelchangeComboHmethod()
{
	OCR_xwindow_method.Set(m_ComboHWindowMethod.GetCurSel());
}

void COCRDlg::OnCbnSelchangeComboThrType()
{
	OCR_ThrType.Set(m_comboThrType.GetCurSel());
}

void COCRDlg::OnBnClickedButtonDebug()
{
	TCHAR tmp[16];
	CDebugMsg* pdlg = new CDebugMsg();
	pdlg->msg[0] = 0;

#if CURRENCY
	wsprintf(tmp, L"CNT=%d\r\n", OCR_pos_cnt);
	lstrcat(pdlg->msg, tmp);

	lstrcat(pdlg->msg, L"POS=");
	for(int si = 0; si < OCR_pos_cnt; si++) {
		if(si == OCR_pos_cnt-1)
			wsprintf(tmp, L"%d\r\n", OCR_x_pos[si]);
		else
			wsprintf(tmp, L"%d,", OCR_x_pos[si]);
		lstrcat(pdlg->msg, tmp);
	}

	lstrcat(pdlg->msg, L"OFT=");
	for(int si = 0; si < OCR_pos_cnt; si++) {
		if(si == OCR_pos_cnt-1)
			wsprintf(tmp, L"%d\r\n", OCR_x_pos[si] - OCR_x_pos[0]);
		else
			wsprintf(tmp, L"%d,", OCR_x_pos[si] - OCR_x_pos[0]);
		lstrcat(pdlg->msg, tmp);
	}

	lstrcat(pdlg->msg, L"SIZ=");
	for(int si = 0; si < OCR_pos_cnt; si++) {
		if(si == OCR_pos_cnt-1)
			wsprintf(tmp, L"%d\r\n", OCR_x_len[si]);
		else
			wsprintf(tmp, L"%d,", OCR_x_len[si]);
		lstrcat(pdlg->msg, tmp);
	}

	if(OCR_pos_cnt > 0) {
		wsprintf(tmp, L"LEN=%d\r\n", OCR_x_pos[OCR_pos_cnt-1] - OCR_x_pos[0] + OCR_x_len[OCR_pos_cnt-1]);
		lstrcat(pdlg->msg, tmp);
	}
	pdlg->DoModal();
	delete pdlg;
#endif
}

void COCRDlg::OnBnClickedButtonCopyData()
{
	HANDLE h = CreateFile(L"C:\\OCR_IMG_DATA.c", GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return;
	
	DWORD dwio = 0;
	unsigned char* img_data = (OCR_ImageSource.Get() == 0) ? ir_data : w_data;
	char buf[128];
	sprintf_s(buf, 128, "unsigned char test_data[%d][%d] = {\r\n", m_Width, m_Height);
	WriteFile(h, buf, strlen(buf), &dwio, NULL);
	for(int y = 0; y < m_Height; y++) {
		strcpy_s(buf, 128, "{ ");
		WriteFile(h, buf, strlen(buf), &dwio, NULL);
		for(int x = 0; x < m_Width; x++) {
			sprintf(buf, "%d", img_data[y*m_Width + x]);
			if(x == m_Width-1)
				strcat_s(buf, 128, "}");
			else
				strcat_s(buf, 128, ", ");
			WriteFile(h, buf, strlen(buf), &dwio, NULL);
		}
		if(y == m_Height-1)
			strcpy_s(buf, 128, "} };\r\n");
		else
			strcpy_s(buf, 128, "},\r\n");
		WriteFile(h, buf, strlen(buf), &dwio, NULL);
	}
	CloseHandle(h);
}

void COCRDlg::OnCbnSelchangeComboScale()
{
	OCR_scaleX.Set(m_ComboScaleX.GetCurSel());
	SaveWindowValues();
}

#define W	30
#define	H	20

static void SaveImageToTempBitmap(unsigned char* buf, int x_size, int y_size)
{
	TCHAR path[MAX_PATH];
	BITMAPINFOHEADER bi;
	BITMAPFILEHEADER hdr;
	BOOL bResult = FALSE;
	HANDLE pFile = NULL;

	DWORD dwio = 0;
	unsigned char *rowBuffer = NULL;
	int x, y;

	int x_byte_len = ((x_size * 3 + 3)/4)*4;
	int nColors = 0; // Color Index 없음

	CreateDirectory(TEXT("C:\\temp"), NULL);
	GetTempFileName(TEXT("C:\\temp"), TEXT("ocr_"), 0, path);

	//*------------------------- 비트멥 헤더를 기록함 -------------------------
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = x_size;
	bi.biHeight = -y_size;
	bi.biPlanes = 1;
	bi.biBitCount = 24;       
	bi.biCompression = BI_RGB;
	bi.biSizeImage = y_size * x_byte_len;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	hdr.bfType = ((WORD)('M' << 8) | 'B');        
	hdr.bfSize = bi.biSizeImage + sizeof(bi) + sizeof(hdr);   
	hdr.bfReserved1 = 0;                                 
	hdr.bfReserved2 = 0;                                 
	hdr.bfOffBits=(DWORD)(sizeof(hdr) + bi.biSize + nColors * sizeof(RGBQUAD));

	/*--------------------- 실제 파일에 기록함 --------------------------*/
	DeleteFile(path);
	lstrcat(path, TEXT(".bmp"));
	pFile = CreateFile(path, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (pFile == INVALID_HANDLE_VALUE)
		return;

	WriteFile(pFile, &hdr, sizeof(hdr), &dwio, NULL);
	WriteFile(pFile, &bi, sizeof(bi), &dwio, NULL);

	rowBuffer = (unsigned char*)malloc(x_byte_len);

	for(y = 0; y < y_size; y++) {
		unsigned char* dp = rowBuffer;
		for(x = 0; x < x_size; x++) {
			*dp++ = buf[y*x_size + x];
			*dp++ = buf[y*x_size + x];
			*dp++ = buf[y*x_size + x];
		}
		WriteFile(pFile, rowBuffer, x_byte_len, &dwio, NULL);
	}
	free(rowBuffer);

	CloseHandle(pFile);
}

void MakeH(unsigned char* buf)
{
	memset(buf, 255, W*H);
	for(int y = 2; y < H-2; y++) {
		for(int x = 2; x < 4; x++) {
			buf[W*y + x] = 0;
		}
		for(int x = W-4; x < W-2; x++) {
			buf[W*y + x] = 0;
		}
	}

	for(int y = 8; y < 12; y++) {
		for(int x = 2; x < W-2; x++) {
			buf[W*y + x] = 0;
		}
	}
}

void GiveNoise(unsigned char*buf)
{
	buf[3*W + 0] = 0;
	buf[4*W + 0] = 0;
	buf[3*W + W-1] = 0;
	buf[4*W + W-1] = 0;
	buf[(H-1)*W + 2] = 0;
}

void COCRDlg::OnBnClickedButton1()
{
#if CURRENCY // license plate is always recognizable, so do not check it
	unsigned char buf[1000];
	// H 자를 만들자
	MakeH(buf);
	SaveImageToTempBitmap(buf, W, H);

	MakeH(buf);
	GiveNoise(buf);
	SaveImageToTempBitmap(buf, W, H);

	RemoveHSingleDots(buf, W, H);
	SaveImageToTempBitmap(buf, W, H);

	MakeH(buf);
	GiveNoise(buf);
	SaveImageToTempBitmap(buf, W, H);
	RemoveSidePixels(buf, W, H, W+1, H+1);
	SaveImageToTempBitmap(buf, W+1, H+1);

	MakeH(buf);
	GiveNoise(buf);
	SaveImageToTempBitmap(buf, W, H);
	RemoveSidePixels(buf, W, H, W-2, H-2);
	SaveImageToTempBitmap(buf, W-2, H-2);

	MakeH(buf);
	GiveNoise(buf);
	RemoveHSingleDots(buf, W, H);
	RemoveSidePixels(buf, W, H, W-3, H-3);
	SaveImageToTempBitmap(buf, W-3, H-3);
#endif
}
