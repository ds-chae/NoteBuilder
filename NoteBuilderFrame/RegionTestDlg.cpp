// RegionTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mainfrm.h"
#include "NoteBuilderFrame.h"

#include "B_Common.h"

#include "ST150Image.h"
#include "ICUMANDoc.h"
#include "ICUMANView.h"

#include "CurrencyInfo.h"
#include "RegionTable.h"
#include "SelectRegionDlg.h"
#include "RegionTestDlg.h"
#include "OCRDlg.h"

CRegionTestDlg *pRegionTestDlg = NULL;

// CRegionTestDlg dialog

IMPLEMENT_DYNCREATE(CRegionTestDlg, CDHtmlDialog)

CRegionTestDlg::CRegionTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegionTestDlg::IDD, pParent)
{
	pViewWnd = (CICUMANView*)pParent;
	m_AddIndex = 0;
	m_bAutoExtract = FALSE;
	m_bByDenom = FALSE;
}

CRegionTestDlg::~CRegionTestDlg()
{
}

void CRegionTestDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_EDIT_SX, m_EditSx);
	DDX_Control(pDX, IDC_EDIT_EX, m_EditEx);
	DDX_Control(pDX, IDC_EDIT_DECX, m_EditDecX);
	DDX_Control(pDX, IDC_EDIT_SUMX, m_EditSumX);
	DDX_Control(pDX, IDC_EDIT_SY, m_EditSy);
	DDX_Control(pDX, IDC_EDIT_EY, m_EditEy);
	DDX_Control(pDX, IDC_EDIT_DECY, m_EditDecY);
	DDX_Control(pDX, IDC_EDIT_SUMY, m_EditSumY);
	DDX_Control(pDX, IDC_EDIT_DEGREE, m_EditDegree);
	DDX_Control(pDX, IDC_EDIT_IR_AVG, m_EditIRAvg);
	DDX_Control(pDX, IDC_EDIT_IR_AVG2, m_EditRegionSize);
	DDX_Control(pDX, IDC_EDIT_THREAD_WIDTH, m_EditThreadWidth);
	DDX_Control(pDX, IDC_EDIT_THREAD_POS, m_EditThreadPos);
	DDX_Control(pDX, IDC_EDIT_THREAD_AVG, m_EditThreadAvg);
	DDX_Control(pDX, IDC_COMBO_DIRECTION, m_ComboDirection);
	DDX_Control(pDX, IDC_EDIT_THREAD_SIZE, m_EditThreadSize);
	DDX_Control(pDX, IDC_LIST_REGION, m_ListRegion);
	DDX_Control(pDX, IDC_COMBO_REGION_TYPE, m_ComboRegionType);
	DDX_Control(pDX, IDC_EDIT_REG_NAME, m_EditRegName);
	DDX_Control(pDX, IDC_EDIT_REG_IDX, m_EditRegIdx);

	DDX_Control(pDX, IDC_COMBO_R00, m_ComboR_0_L);
	DDX_Control(pDX, IDC_COMBO_R10, m_ComboR_1_L);
	DDX_Control(pDX, IDC_COMBO_R20, m_ComboR_2_L);
	DDX_Control(pDX, IDC_COMBO_R30, m_ComboR_3_L);
	DDX_Control(pDX, IDC_COMBO_R40, m_ComboR_4_L);
	DDX_Control(pDX, IDC_COMBO_R50, m_ComboR_5_L);
	DDX_Control(pDX, IDC_COMBO_R60, m_ComboR_6_L);
	DDX_Control(pDX, IDC_COMBO_R70, m_ComboR_7_L);

	DDX_Control(pDX, IDC_COMBO_R01, m_ComboR_0_R);
	DDX_Control(pDX, IDC_COMBO_R11, m_ComboR_1_R);
	DDX_Control(pDX, IDC_COMBO_R21, m_ComboR_2_R);
	DDX_Control(pDX, IDC_COMBO_R31, m_ComboR_3_R);
	DDX_Control(pDX, IDC_COMBO_R41, m_ComboR_4_R);
	DDX_Control(pDX, IDC_COMBO_R51, m_ComboR_5_R);
	DDX_Control(pDX, IDC_COMBO_R61, m_ComboR_6_R);
	DDX_Control(pDX, IDC_COMBO_R71, m_ComboR_7_R);

	DDX_Control(pDX, IDC_LABEL_RESULT0, m_LabelResult0);
	DDX_Control(pDX, IDC_LABEL_RESULT1, m_LabelResult1);
	DDX_Control(pDX, IDC_LABEL_RESULT2, m_LabelResult2);
	DDX_Control(pDX, IDC_LABEL_RESULT3, m_LabelResult3);
	DDX_Control(pDX, IDC_LABEL_RESULT4, m_LabelResult4);
	DDX_Control(pDX, IDC_LABEL_RESULT5, m_LabelResult5);
	DDX_Control(pDX, IDC_LABEL_RESULT6, m_LabelResult6);
	DDX_Control(pDX, IDC_LABEL_RESULT7, m_LabelResult7);
	DDX_Control(pDX, IDC_EDIT_W_AVG, m_EditAvgW);
	DDX_Control(pDX, IDC_EDIT_W_MIN, m_EditMinW);
	DDX_Control(pDX, IDC_EDIT_W_MAX, m_EditMaxW);
	DDX_Control(pDX, IDC_EDIT_W_VAR, m_EditVarW);
	DDX_Control(pDX, IDC_EDIT_IR_MIN, m_EditMinIR);
	DDX_Control(pDX, IDC_EDIT_IR_MAX, m_EditMaxIR);
	DDX_Control(pDX, IDC_EDIT_IR_VAR, m_EditVarIR);
	DDX_Control(pDX, IDC_LIST_REGION, m_ListRegion);
	DDX_Control(pDX, IDC_COMBO_CIS_ENC, m_ComboCisEnc);
	DDX_Control(pDX, IDC_CHECK_AUTO_EXTRACT, m_CheckAutoExtract);
}

BEGIN_MESSAGE_MAP(CRegionTestDlg, CDialog)
	ON_BN_CLICKED(IDC_CLOSE, &CRegionTestDlg::OnBnClickedClose)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_COMBO_DIRECTION, &CRegionTestDlg::OnCbnSelchangeComboDirection)
	ON_BN_CLICKED(IDC_BUTTON_EXTRACT, &CRegionTestDlg::OnBnClickedButtonExtract)
	ON_BN_CLICKED(IDC_BUTTON_FIND_THREAD, &CRegionTestDlg::OnBnClickedButtonFindThread)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE, &CRegionTestDlg::OnBnClickedButtonGenerate)

	ON_CBN_SELCHANGE(IDC_COMBO_R00, &CRegionTestDlg::OnCbnSelchangeComboR00)
	ON_CBN_SELCHANGE(IDC_COMBO_R01, &CRegionTestDlg::OnCbnSelchangeComboR01)
	ON_CBN_SELCHANGE(IDC_COMBO_R10, &CRegionTestDlg::OnCbnSelchangeComboR10)
	ON_CBN_SELCHANGE(IDC_COMBO_R11, &CRegionTestDlg::OnCbnSelchangeComboR11)
	ON_CBN_SELCHANGE(IDC_COMBO_R20, &CRegionTestDlg::OnCbnSelchangeComboR20)
	ON_CBN_SELCHANGE(IDC_COMBO_R21, &CRegionTestDlg::OnCbnSelchangeComboR21)
	ON_CBN_SELCHANGE(IDC_COMBO_R30, &CRegionTestDlg::OnCbnSelchangeComboR30)
	ON_CBN_SELCHANGE(IDC_COMBO_R31, &CRegionTestDlg::OnCbnSelchangeComboR31)
	ON_CBN_SELCHANGE(IDC_COMBO_R40, &CRegionTestDlg::OnCbnSelchangeComboR40)
	ON_CBN_SELCHANGE(IDC_COMBO_R41, &CRegionTestDlg::OnCbnSelchangeComboR41)
	ON_CBN_SELCHANGE(IDC_COMBO_R50, &CRegionTestDlg::OnCbnSelchangeComboR50)
	ON_CBN_SELCHANGE(IDC_COMBO_R51, &CRegionTestDlg::OnCbnSelchangeComboR51)
	ON_CBN_SELCHANGE(IDC_COMBO_R60, &CRegionTestDlg::OnCbnSelchangeComboR60)
	ON_CBN_SELCHANGE(IDC_COMBO_R61, &CRegionTestDlg::OnCbnSelchangeComboR61)
	ON_CBN_SELCHANGE(IDC_COMBO_R70, &CRegionTestDlg::OnCbnSelchangeComboR70)
	ON_CBN_SELCHANGE(IDC_COMBO_R71, &CRegionTestDlg::OnCbnSelchangeComboR71)
	ON_BN_CLICKED(IDC_BUTTON_SELECT, &CRegionTestDlg::OnBnClickedButtonSelect)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CRegionTestDlg::OnBnClickedButtonDelete)
	ON_LBN_SELCHANGE(IDC_LIST_REGION, &CRegionTestDlg::OnLbnSelchangeListRegion)
	ON_BN_CLICKED(IDC_BUTTON_NO_DRAW, &CRegionTestDlg::OnBnClickedButtonNoDraw)
	ON_WM_DESTROY()
//	ON_WM_CHAR()
ON_BN_CLICKED(IDC_BUTTON_IR_AND_W, &CRegionTestDlg::OnBnClickedButtonIrAndW)
ON_CBN_SELCHANGE(IDC_COMBO_CIS_ENC, &CRegionTestDlg::OnCbnSelchangeComboCisEnc)
ON_BN_CLICKED(IDC_BUTTON_SAVE, &CRegionTestDlg::OnBnClickedButtonSave)
ON_BN_CLICKED(IDC_CHECK_AUTO_EXTRACT, &CRegionTestDlg::OnBnClickedCheckAutoExtract)
ON_BN_CLICKED(IDC_BUTTON_COPY_REGION, &CRegionTestDlg::OnBnClickedButtonCopyRegion)
ON_BN_CLICKED(IDC_CHECK_SERIAL_IMAGE, &CRegionTestDlg::OnBnClickedCheckSerialImage)
ON_EN_CHANGE(IDC_EDIT_DECY, &CRegionTestDlg::OnEnChangeEditDecy)
ON_BN_CLICKED(IDC_BUTTON_SAVE_SER_IMG, &CRegionTestDlg::OnBnClickedButtonSaveSerImg)
ON_BN_CLICKED(IDC_CHECK_BY_DENOM, &CRegionTestDlg::OnBnClickedCheckByDenom)
END_MESSAGE_MAP()

// CRegionTestDlg message handlers

void CRegionTestDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	DisplayRegion.RegionCount = 0;

	delete pRegionTestDlg;
	pRegionTestDlg = NULL;
}


void CRegionTestDlg::OnBnClickedClose()
{
	DestroyWindow();
}

void CRegionTestDlg::OnClose()
{
	CDialog::OnClose();

	DestroyWindow();
}

BOOL CRegionTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ComboRegionType.AddString( L"Denomination");
	m_ComboRegionType.AddString( L"Vertical");
	m_ComboRegionType.AddString( L"Horizontal");
	m_ComboRegionType.AddString( L"Normal");

	m_ComboDirection.AddString(L"FF");
	m_ComboDirection.AddString(L"FB");
	m_ComboDirection.AddString(L"BF");
	m_ComboDirection.AddString(L"BB");
	m_ComboDirection.AddString(L"UNKN");
	m_ComboDirection.SetCurSel(0);

	SetDlgItemInt(m_EditDecX.GetDlgCtrlID(), 1);
	SetDlgItemInt(m_EditSumX.GetDlgCtrlID(), 1);

	SetDlgItemInt(m_EditDecY.GetDlgCtrlID(), 1);
	SetDlgItemInt(m_EditSumY.GetDlgCtrlID(), 1);

	UpdateFromView();

	m_ComboListL[0] = &m_ComboR_0_L;
	m_ComboListL[1] = &m_ComboR_1_L;
	m_ComboListL[2] = &m_ComboR_2_L;
	m_ComboListL[3] = &m_ComboR_3_L;
	m_ComboListL[4] = &m_ComboR_4_L;
	m_ComboListL[5] = &m_ComboR_5_L;
	m_ComboListL[6] = &m_ComboR_6_L;
	m_ComboListL[7] = &m_ComboR_7_L;

	m_ComboListR[0] = &m_ComboR_0_R;
	m_ComboListR[1] = &m_ComboR_1_R;
	m_ComboListR[2] = &m_ComboR_2_R;
	m_ComboListR[3] = &m_ComboR_3_R;
	m_ComboListR[4] = &m_ComboR_4_R;
	m_ComboListR[5] = &m_ComboR_5_R;
	m_ComboListR[6] = &m_ComboR_6_R;
	m_ComboListR[7] = &m_ComboR_7_R;

	m_LabelResult[0] = &m_LabelResult0;
	m_LabelResult[1] = &m_LabelResult1;
	m_LabelResult[2] = &m_LabelResult2;
	m_LabelResult[3] = &m_LabelResult3;
	m_LabelResult[4] = &m_LabelResult4;
	m_LabelResult[5] = &m_LabelResult5;
	m_LabelResult[6] = &m_LabelResult6;
	m_LabelResult[7] = &m_LabelResult7;

	for(int i = 0; i < 8; i++) {
		m_LabelResult[i]->SetWindowTextW(L"");
		m_ComboListL[i]->AddString(L"-");
		m_ComboListR[i]->AddString(L"-");
	}

	m_ComboCisEnc.AddString(L"1");
	m_ComboCisEnc.AddString(L"2");
	m_ComboCisEnc.SetCurSel(gst_State.CisEnc-1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRegionTestDlg::UpdateFromView()
{
	if(pViewWnd->imgST150.OneNote.Dir >= 0 && pViewWnd->imgST150.OneNote.Dir <= 3)
		m_ComboDirection.SetCurSel(pViewWnd->imgST150.OneNote.Dir);
	else
		m_ComboDirection.SetCurSel(4);

	if(pViewWnd == NULL)
		return;

/*
	SetDlgItemInt(m_EditThreadWidth.GetDlgCtrlID(), pViewWnd->ThreadWidth);
	CPoint p1 = pViewWnd->m_DownPoint;
	if(p1.x && p1.y) {
		if(p1.y > imgST150.m_WImageSize.cy)
			p1.y -= imgST150.m_WImageSize.cy;
		p1.x -= OneNote.CIS.Center_X;
		p1.y -= OneNote.CIS.Center_Y;
	} else {
		p1.x = -164;
		p1.y = -55;
	}
	SetDlgItemInt(m_EditSx.GetDlgCtrlID(), p1.x);
	SetDlgItemInt(m_EditSy.GetDlgCtrlID(), p1.y);

	CPoint p2 = pViewWnd->m_MovePoint;
	if(p2.x && p2.y) {
		if(p2.y > imgST150.m_WImageSize.cy)
			p2.y -= imgST150.m_WImageSize.cy;
		p2.x -= OneNote.CIS.Center_X;
		p2.y -= OneNote.CIS.Center_Y;
	} else {
		p2.x = 284;
		p2.y = 45;
	}
	SetDlgItemInt(m_EditEx.GetDlgCtrlID(), p2.x);
	SetDlgItemInt(m_EditEy.GetDlgCtrlID(), p2.y);
	SetDlgItemInt(m_EditRegionSize.GetDlgCtrlID(), abs(p2.x-p1.x)*abs(p2.y-p1.y));
*/

	TCHAR buf[16];
	swprintf(buf, 16, L"%5.3f", (float)pViewWnd->imgST150.OneNote.IAR_A.Degree);
	m_EditDegree.SetWindowTextW(buf);

	SetDlgItemInt(m_EditAvgW.GetDlgCtrlID(), pViewWnd->AverageW);
	SetDlgItemInt(m_EditIRAvg.GetDlgCtrlID(), pViewWnd->AverageIR);

	m_ComboCisEnc.SetCurSel(gst_State.CisEnc - 1);

	if(m_bByDenom) {
		// 액면에 따른 영역을 가져와서 설정해 준다.
		m_bAutoExtract = TRUE; // 자동으로 뜨도록 해 주고.
		SetRegionByDenom();
	}
	
	if(pViewWnd->imgST150.m_bLoadedFromJSN && pViewWnd->imgST150.extr.Region.Sx != 0) {
		SetItemByRegion(&pViewWnd->imgST150.extr);
	}

	if(m_bAutoExtract)
		OnBnClickedButtonExtract();
}

void CRegionTestDlg::UpdateRectFromView()
{
	if(pViewWnd->imgST150.OneNote.Dir >= 0 && pViewWnd->imgST150.OneNote.Dir <= 3)
		m_ComboDirection.SetCurSel(pViewWnd->imgST150.OneNote.Dir);
	else
		m_ComboDirection.SetCurSel(4);
	if(pViewWnd == NULL)
		return;
	
	SetDlgItemInt(m_EditThreadWidth.GetDlgCtrlID(), pViewWnd->ThreadWidth);
	ST_SQUARE ViewSquare;
	ViewSquare.Sx = (Int16)pViewWnd->m_DownPoint.x;
	ViewSquare.Sy = (Int16)pViewWnd->m_DownPoint.y;
	ViewSquare.Ex = (Int16)pViewWnd->m_MovePoint.x;
	ViewSquare.Ey = (Int16)pViewWnd->m_MovePoint.y;

	if(ViewSquare.Sy > pViewWnd->imgST150.m_W_ImageSize.cy)
		ViewSquare.Sy -= (Int16)pViewWnd->imgST150.m_W_ImageSize.cy;
	if(ViewSquare.Ey > pViewWnd->imgST150.m_W_ImageSize.cy)
		ViewSquare.Ey -= (Int16)pViewWnd->imgST150.m_W_ImageSize.cy;

	ViewSquare.Sx -= pViewWnd->imgST150.OneNote.IAR_A.Center_X;
	ViewSquare.Ex -= pViewWnd->imgST150.OneNote.IAR_A.Center_X;
	ViewSquare.Sy -= pViewWnd->imgST150.OneNote.IAR_A.Center_Y;
	ViewSquare.Ey -= pViewWnd->imgST150.OneNote.IAR_A.Center_Y;

	int dir = m_ComboDirection.GetCurSel();
	switch(dir) {
	case 0 :
		break;
	case DIR_FB :
		ViewSquare.Sx = -ViewSquare.Sx;
		ViewSquare.Ex = -ViewSquare.Ex;
		ViewSquare.Sy = -ViewSquare.Sy;
		ViewSquare.Ey = -ViewSquare.Ey;
		break;
	case DIR_BF :
		ViewSquare.Sx = -ViewSquare.Sx;
		ViewSquare.Ex = -ViewSquare.Ex;
		break;
	case DIR_BB:
		ViewSquare.Sy = -ViewSquare.Sy;
		ViewSquare.Ey = -ViewSquare.Ey;
	}

	if(ViewSquare.Sx > ViewSquare.Ex) {
		int t = ViewSquare.Sx;
		ViewSquare.Sx = ViewSquare.Ex;
		ViewSquare.Ex = t;
	}
	if(ViewSquare.Sy > ViewSquare.Ey) {
		int t = ViewSquare.Sy;
		ViewSquare.Sy = ViewSquare.Ey;
		ViewSquare.Ey = t;
	}

	if(gst_State.CisEnc == 1) { // Serial Image 이면, y 크기는 1/2이 되어야 한다.
		ViewSquare.Sy /= 2;
		ViewSquare.Ey /= 2;
	}
	
	SetDlgItemInt(m_EditSx.GetDlgCtrlID(), ViewSquare.Sx);
	SetDlgItemInt(m_EditSy.GetDlgCtrlID(), ViewSquare.Sy);
	SetDlgItemInt(m_EditEx.GetDlgCtrlID(), ViewSquare.Ex);
	SetDlgItemInt(m_EditEy.GetDlgCtrlID(), ViewSquare.Ey);

	SetDlgItemInt(m_EditRegionSize.GetDlgCtrlID(), abs(ViewSquare.Ex-ViewSquare.Sx)*abs(ViewSquare.Ey-ViewSquare.Sy));
}

void CRegionTestDlg::OnCbnSelchangeComboDirection()
{
	int dir = m_ComboDirection.GetCurSel();
	if(dir < 4) {
		pViewWnd->DirFromDlg = dir;
		pViewWnd->Invalidate();
	}
}

void CRegionTestDlg::GetImgRegion(ST_IMG_REGION *preg)
{
	preg->Region.Sx = GetDlgItemInt(m_EditSx.GetDlgCtrlID());
	preg->Region.Sy = GetDlgItemInt(m_EditSy.GetDlgCtrlID());
	preg->Region.Ex = GetDlgItemInt(m_EditEx.GetDlgCtrlID());
	preg->Region.Ey = GetDlgItemInt(m_EditEy.GetDlgCtrlID());

	preg->Shift_X = GetDlgItemInt(m_EditDecX.GetDlgCtrlID());
	preg->Shift_Y = GetDlgItemInt(m_EditDecY.GetDlgCtrlID());

	preg->Sum_X = GetDlgItemInt(m_EditSumX.GetDlgCtrlID());
	preg->Sum_Y = GetDlgItemInt(m_EditSumY.GetDlgCtrlID());
}

void CRegionTestDlg::PutRegionParametersToView(ST_IMG_REGION& ExtRegion)
{
	pViewWnd->DownPoint.x = ExtRegion.Region.Sx;
	pViewWnd->DownPoint.y = ExtRegion.Region.Sy;
	pViewWnd->MovePoint.x = ExtRegion.Region.Ex;
	pViewWnd->MovePoint.y = ExtRegion.Region.Ey;

	pViewWnd->DecX = ExtRegion.Shift_X;
	pViewWnd->DecY = ExtRegion.Shift_Y;

	pViewWnd->SumX = ExtRegion.Sum_X;
	pViewWnd->SumY = ExtRegion.Sum_Y;
}

ST_IMG_REGION* CRegionTestDlg::AddRegionToList(ST_IMG_REGION *exRegion, TCHAR* name)
{
	ST_IMG_REGION* newRegion = NULL;
	for(int i = 0; i < m_ListRegion.GetCount(); i++) {
		newRegion = (ST_IMG_REGION*)m_ListRegion.GetItemData(i);
		if(newRegion != NULL) {
			if(IsSameRegion(newRegion, exRegion)) {
				return newRegion;
			}
		}
	}

	AddIndexToCombo();

	TCHAR text[128];
	swprintf_s(text, 128, L"%d: %s", m_AddIndex++, name);
	int index = m_ListRegion.AddString(text);
	newRegion = (ST_IMG_REGION *)malloc(sizeof(ST_IMG_REGION));
	*newRegion = *exRegion;
	m_ListRegion.SetItemData(index, (DWORD_PTR)newRegion);
	m_ListRegion.SetSel(index);

	return newRegion;
}

void CRegionTestDlg::OnBnClickedButtonExtract()
{
	extern CICUMANView* pMainView;
	if(pMainView == NULL)
		return;

	if(pMainView->m_MainImageSize.cx == 0) // Image is not loaded yet
		return;

	TCHAR txtbuf[128];
	m_EditRegName.GetWindowTextW(txtbuf, 128);
	StrTrim(txtbuf, L" ,.");
	// 영역 이름이 없으면 Added로 표시한다.
	if(lstrlen(txtbuf) == 0)
		lstrcpy(txtbuf, L" Added");
	TCHAR indx[64];
	m_EditRegIdx.GetWindowTextW(indx, 64);
	indx[1] = 0;
	if(indx[0] < '0' || indx[0] > '9') indx[0] = '0';

	ST_IMG_REGION ExtRegion;
	GetImgRegion(&ExtRegion);
	ST_IMG_REGION *newRegion = AddRegionToList(&ExtRegion, txtbuf);

	PutRegionParametersToView(ExtRegion);

	REGION_RECORD *prec = DisplayRegion.FindRegion(newRegion);
	if(prec == NULL)
		prec = DisplayRegion.AddRegion(newRegion);


	pViewWnd->ExtractRegion(TRUE, TRUE);
	
	SetDlgItemInt(m_EditRegionSize.GetDlgCtrlID(), prec->ExtSizeX * prec->ExtSizeY);
	SetDlgItemInt(m_EditAvgW.GetDlgCtrlID(), prec->ExtAvgW);
	SetDlgItemInt(m_EditIRAvg.GetDlgCtrlID(), prec->ExtAvgIR);

	SetDlgItemInt(m_EditAvgW.GetDlgCtrlID(), prec->ExtAvgW);
	SetDlgItemInt(m_EditMinW.GetDlgCtrlID(), prec->ExtMinW);
	SetDlgItemInt(m_EditMaxW.GetDlgCtrlID(), prec->ExtMaxW);
	SetDlgItemInt(m_EditVarW.GetDlgCtrlID(), prec->ExtVarW);
	SetDlgItemInt(m_EditMinIR.GetDlgCtrlID(), prec->ExtMinIR);
	SetDlgItemInt(m_EditMaxIR.GetDlgCtrlID(), prec->ExtMaxIR);
	SetDlgItemInt(m_EditVarIR.GetDlgCtrlID(), prec->ExtVarIR);

	UpdateResult();

	if(theOCRDlg != NULL) {
		theOCRDlg->MakeDataFromBitmap(pViewWnd->ExtBitmap);
		if(pViewWnd != NULL) {
			strcpy_s(theOCRDlg->Serial, pViewWnd->imgST150.OneNote.Serial);
		}
	}
}

void CRegionTestDlg::OnBnClickedButtonFindThread()
{
	pViewWnd->FindThread(m_ComboThreadType.GetCurSel(), GetDlgItemInt(m_EditThreadWidth.GetDlgCtrlID()));
	SetDlgItemInt(m_EditThreadSize.GetDlgCtrlID(), pViewWnd->ThreadSizeX * pViewWnd->ThreadSizeY);
}

void CRegionTestDlg::OnBnClickedButtonGenerate()
{
	ST_IMG_REGION imgRegion;
	GetImgRegion(&imgRegion);
	PutRegionParametersToView(imgRegion);

	pViewWnd->OnGenerateRegionRaw(m_ComboThreadType.GetCurSel(), GetDlgItemInt(m_EditThreadWidth.GetDlgCtrlID()));
}

void CRegionTestDlg::OnCbnSelchangeComboR00()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR01()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR10()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR11()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR20()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR21()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR30()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR31()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR40()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR41()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR50()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR51()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR60()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR61()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR70()
{
	UpdateResult();
}

void CRegionTestDlg::OnCbnSelchangeComboR71()
{
	UpdateResult();
}

void CRegionTestDlg::UpdateResult()
{
	for(int li = 0; li < 8; li++) {
		int lindex = m_ComboListL[li]->GetCurSel() - 1;
		int rindex = m_ComboListR[li]->GetCurSel() - 1;
		if(lindex >= 0 && rindex >= 0) {
			ST_IMG_REGION *RegLeft = (ST_IMG_REGION*)m_ListRegion.GetItemData(lindex);
			ST_IMG_REGION *RegRight= (ST_IMG_REGION*)m_ListRegion.GetItemData(rindex);
			if(RegLeft != NULL && RegRight != NULL) {
				lindex = DisplayRegion.FindRegion(RegLeft->Region.Sx, RegLeft->Region.Sy, RegLeft->Region.Ex, RegLeft->Region.Ey);
				rindex = DisplayRegion.FindRegion(RegRight->Region.Sx, RegRight->Region.Sy, RegRight->Region.Ex, RegRight->Region.Ey);
				if(lindex >= 0 && rindex >= 0) {
					if(DisplayRegion.Data[rindex].ExtAvgIR != 0) {
						int v = (DisplayRegion.Data[lindex].ExtAvgIR * 100) / DisplayRegion.Data[rindex].ExtAvgIR;
						SetDlgItemInt(m_LabelResult[li]->GetDlgCtrlID(), v);
					} else {
						m_LabelResult[li]->SetWindowTextW(L"NaN");
					}
				}
			} else {
				m_LabelResult[li]->SetWindowTextW(L"");
			}
		}
	}
}

void CRegionTestDlg::AddIndexToCombo()
{
	for(int i = 0; i < 8; i++) {
		TCHAR num[16];
		swprintf_s(num, 16, L"%d", m_ListRegion.GetCount());
		m_ComboListL[i]->AddString(num);
		m_ComboListR[i]->AddString(num);
	}
}

void CRegionTestDlg::OnBnClickedButtonSelect()
{
	CSelectRegionDlg dlg;
	if(dlg.DoModal() == IDOK) {
		ST_IMG_REGION *pRegion = currencyInfo.FindRegionByIndex(dlg.m_SelectedRegion);
		if(pRegion != NULL) {
			AddRegionToList(pRegion, dlg.m_SelectedName);
		}
	}
}

void CRegionTestDlg::OnBnClickedButtonDelete()
{
	int selected = m_ListRegion.GetCurSel();
	if(selected < 0)
		return;
	
	ST_IMG_REGION *pRegion = (ST_IMG_REGION*)m_ListRegion.GetItemData(selected);
	DisplayRegion.DelRegion(pRegion->Region.Sx, pRegion->Region.Sy, pRegion->Region.Ex, pRegion->Region.Ey,
		pRegion->Shift_X, pRegion->Shift_Y, pRegion->Sum_X, pRegion->Sum_Y);
	free(pRegion);

	m_ListRegion.DeleteString(selected);
	for(int i = 0; i < 8; i++) {
		m_ComboListL[i]->DeleteString(m_ListRegion.GetCount()-1);
		m_ComboListR[i]->DeleteString(m_ListRegion.GetCount()-1);
	}
	UpdateResult();

	if(pViewWnd != NULL)
		pViewWnd->Invalidate();
}

void CRegionTestDlg::OnLbnSelchangeListRegion()
{
	int selected = m_ListRegion.GetCurSel();
	if(selected < 0)
		return;
	
	ST_IMG_REGION *pRegion = (ST_IMG_REGION*)m_ListRegion.GetItemData(selected);
	if(pRegion == NULL)
		return;

	SetItemByRegion(pRegion);
}

void CRegionTestDlg::SetItemByRegion(ST_IMG_REGION* pRegion)
{
	SetDlgItemInt(m_EditSx.GetDlgCtrlID(), pRegion->Region.Sx);
	SetDlgItemInt(m_EditSy.GetDlgCtrlID(), pRegion->Region.Sy);
	SetDlgItemInt(m_EditEx.GetDlgCtrlID(), pRegion->Region.Ex);
	SetDlgItemInt(m_EditEy.GetDlgCtrlID(), pRegion->Region.Ey);

	SetDlgItemInt(m_EditDecX.GetDlgCtrlID(), pRegion->Shift_X);
	SetDlgItemInt(m_EditSumX.GetDlgCtrlID(), pRegion->Sum_X);

	SetDlgItemInt(m_EditDecY.GetDlgCtrlID(), pRegion->Shift_Y);
	if(pRegion->Shift_Y == -1)
		GetDlgItem(IDC_CHECK_SERIAL_IMAGE)->SendMessage(BM_SETCHECK, BST_CHECKED, 0);
	else
		GetDlgItem(IDC_CHECK_SERIAL_IMAGE)->SendMessage(BM_SETCHECK, 0, 0);

	SetDlgItemInt(m_EditSumY.GetDlgCtrlID(), pRegion->Sum_Y);
}

void CRegionTestDlg::OnBnClickedButtonNoDraw()
{
	int selected = m_ListRegion.GetCurSel();
	if(selected < 0)
		return;
	
	ST_IMG_REGION *pRegion = (ST_IMG_REGION*)m_ListRegion.GetItemData(selected);
	DisplayRegion.DelRegion(pRegion->Region.Sx, pRegion->Region.Sy, pRegion->Region.Ex, pRegion->Region.Ey,
		pRegion->Shift_X, pRegion->Shift_Y, pRegion->Sum_X, pRegion->Sum_Y);

	UpdateResult();

	if(pViewWnd != NULL)
		pViewWnd->Invalidate();
}

void CRegionTestDlg::OnDestroy()
{
	CDialog::OnDestroy();
	
	for(int i = 0; i < m_ListRegion.GetCount(); i++) {
		ST_IMG_REGION *pRegion = (ST_IMG_REGION *)m_ListRegion.GetItemData(i);
		free(pRegion);
	}
}

void CRegionTestDlg::OnBnClickedButtonIrAndW()
{
	pViewWnd->MergeIrAndW();
	pViewWnd->Invalidate();
}

void CRegionTestDlg::OnCbnSelchangeComboCisEnc()
{
/*
	SetCisEnc(m_ComboCisEnc.GetCurSel() + 1);

	CBitmap* pBitmap = NULL;
	CBitmap* pBitmapY = NULL;

	GetRecogKeysFromLowImage(this, &pBitmap, &pViewWnd->imgST150, pViewWnd->m_ImageSource);
	if(pBitmap != NULL)
		pViewWnd->AddDebugImage(pBitmap);

	GetRecogKeysFromUpImage(this, &pBitmapY, &pViewWnd->imgST150, pViewWnd->m_ImageSource);
	if(pBitmapY != NULL)
		pViewWnd->AddDebugImage(pBitmapY);

	pViewWnd->Invalidate();
*/
}

void CRegionTestDlg::OnBnClickedButtonSave()
{
	GetImgRegion(&pViewWnd->imgST150.extr);
	pViewWnd->imgST150.SaveNoteInfo();
}

void CRegionTestDlg::OnBnClickedCheckAutoExtract()
{
	m_bAutoExtract = m_CheckAutoExtract.GetCheck() == BST_CHECKED;
}

void CRegionTestDlg::OnBnClickedButtonCopyRegion()
{
	ST_IMG_REGION region;
	GetImgRegion(&region);

//	sx, sy, ex, ey, state, shift x, shift y, sum x, sum y
	TCHAR text[128];
	wsprintf(text, L"{{ %d, %d, %d, %d, 1 }, %d, %d, %d, %d},",
		region.Region.Sx, region.Region.Sy, region.Region.Ex, region.Region.Ey,
		region.Shift_X, region.Shift_Y, region.Sum_X, region.Sum_Y);

	CWnd* pWnd = GetDlgItem(IDC_EDIT_COPY);
	pWnd->SetWindowTextW(text);
	pWnd->SendMessage(EM_SETSEL, 0, (LPARAM)-1);
    pWnd->SendMessage(WM_COPY, 0, 0);
}

void CRegionTestDlg::OnBnClickedCheckSerialImage()
{
	CWnd* pWnd = GetDlgItem(IDC_CHECK_SERIAL_IMAGE);
	if(pWnd->SendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED) {
		SetDlgItemInt(IDC_EDIT_DECY, -1);
	} else {
		if(GetDlgItemInt(IDC_EDIT_DECY) == -1)
			SetDlgItemInt(IDC_EDIT_DECY, 1);
	}
}

void CRegionTestDlg::OnEnChangeEditDecy()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CWnd* pWnd = GetDlgItem(IDC_CHECK_SERIAL_IMAGE);
	if(GetDlgItemInt(IDC_EDIT_DECY) == -1) {
		pWnd->SendMessage(BM_SETCHECK, BST_CHECKED, 0);
	} else {
		pWnd->SendMessage(BM_SETCHECK, 0, 0);
	}
}

void CRegionTestDlg::OnBnClickedButtonSaveSerImg()
{
	CBitmap *bitmap = pViewWnd->ExtBitmap;
	if(bitmap == NULL)
		return;

	BITMAP m_bitmap;
	bitmap->GetBitmap(&m_bitmap);
	int m_Width = (m_bitmap.bmWidth-5) / 2; // 비트맵 이미지는 두 개이고, 중간에 5픽셀 공간이 있으므로 이렇게 한다.
	int m_Height = m_bitmap.bmHeight;

	int textSize = (m_Width * m_Height * 4) * 2 + 128;
	TCHAR *pImgText = (TCHAR*)malloc(textSize * sizeof(TCHAR));
	if(pImgText == NULL)
		return;

	CDC *pDC = GetDC();
	CDC dc;
	dc.CreateCompatibleDC(pDC);
	ReleaseDC(pDC);
	CBitmap *obit = dc.SelectObject(bitmap);

	int textLen = 0;
	textLen += 	wsprintf(pImgText, L"STCSCopyImage\r\n%d,%d,\r\n", m_Width, m_Height);

	// copy white data first
	for(int y = 0; y < m_Height && y < OCR_Y_MAX; y++) {
		for(int x = 0; x < m_Width && x < OCR_X_MAX; x++) {
			if(textLen < textSize-5) {
				textLen += wsprintf(&pImgText[textLen], L"%d,", (unsigned char*)(dc.GetPixel(x+m_Width+5, y) & 0x0FF));
			}
		}
		if(textLen < textSize-5) {
			textLen += wsprintf(&pImgText[textLen], L"\r\n");
		}
	}

	// copy IR data next
	for(int y = 0; y < m_Height && y < OCR_Y_MAX; y++) {
		for(int x = 0; x < m_Width && x < OCR_X_MAX; x++) {
			if(textLen < textSize-5) {
				textLen += wsprintf(&pImgText[textLen], L"%d,", (unsigned char*)(dc.GetPixel(x, y) & 0x0FF));
			}
		}
		if(textLen < textSize-5) {
			textLen += wsprintf(&pImgText[textLen], L"\r\n");
		}
	}
	pImgText[textLen] = 0;

	dc.SelectObject(obit);
	dc.DeleteDC();

	CWnd* pWnd = GetDlgItem(IDC_EDIT_COPY);
	pWnd->SetWindowTextW(pImgText);
	free(pImgText);
	pWnd->SendMessage(EM_SETSEL, 0, (LPARAM)-1);
    pWnd->SendMessage(WM_COPY, 0, 0);

	HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"STCSCopyImage");
	if(hEvent != NULL) {
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}
}

void CRegionTestDlg::OnBnClickedCheckByDenom()
{
	m_bByDenom = GetDlgItem(IDC_CHECK_BY_DENOM)->SendMessage(BM_GETCHECK) == BST_CHECKED;
}

void CRegionTestDlg::SetRegionByDenom()
{
	if(pViewWnd == NULL)
		return;

	ST_IMG_REGION region;
	char* regName = NULL;

	if(!currencyInfo.GetRegionByIndex(0, pViewWnd->imgST150.OneNote.IAR_A.Max_First_Denom / 4, &region, &regName))
		return;

	if(regName != NULL) {
		TCHAR tstr[MAX_PATH];
		MultiByteToWideChar(CP_ACP, 0, regName, strlen(regName)+1, tstr, MAX_PATH);
		m_EditRegName.SetWindowTextW(tstr);
	}
	SetItemByRegion(&region);
}
