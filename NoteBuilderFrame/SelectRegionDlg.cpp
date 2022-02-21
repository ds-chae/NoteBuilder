// SelectRegionDlg.cpp : implementation file
//

#include "stdafx.h"

#include "B_Common.h"

#include "NoteBuilderFrame.h"
#include "SelectRegionDlg.h"
#include "CurrencyInfo.h"

// CSelectRegionDlg dialog

IMPLEMENT_DYNAMIC(CSelectRegionDlg, CDialog)

CSelectRegionDlg::CSelectRegionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectRegionDlg::IDD, pParent)
{
	m_SelectedRegion = -1;
	m_SelectedName[0] = 0;
}

CSelectRegionDlg::~CSelectRegionDlg()
{
}

void CSelectRegionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_REGION, m_ListRegion);
}


BEGIN_MESSAGE_MAP(CSelectRegionDlg, CDialog)
	ON_LBN_DBLCLK(IDC_LIST_REGION, &CSelectRegionDlg::OnLbnDblclkListRegion)
	ON_LBN_SELCHANGE(IDC_LIST_REGION, &CSelectRegionDlg::OnLbnSelchangeListRegion)
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()


// CSelectRegionDlg message handlers

void CSelectRegionDlg::OnLbnDblclkListRegion()
{
	OnOK();
}

void CSelectRegionDlg::OnLbnSelchangeListRegion()
{
	m_SelectedRegion = m_ListRegion.GetCurSel();
	m_ListRegion.GetText(m_SelectedRegion, m_SelectedName);
}

BOOL CSelectRegionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for(int ri = 0; ri < currencyInfo.RegionSetCount; ri++) {
		REGION_SET *pSet = &currencyInfo.RegionSet[ri];
		int rc = pSet->intRegionsCount;
		for(int ii = 0; ii < rc; ii++) {
			char szBuf[128];
			sprintf_s(szBuf, 128, "%s [%d]", pSet->ImgRegionName, ii);
			TCHAR txtbuf[128];
			MultiByteToWideChar(CP_ACP, 0, szBuf, strlen(szBuf)+1, txtbuf, 128);
			m_ListRegion.AddString(txtbuf);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectRegionDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}
