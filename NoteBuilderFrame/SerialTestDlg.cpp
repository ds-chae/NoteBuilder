// SerialTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NoteBuilderFrame.h"
#include "SerialTestDlg.h"


// CSerialTestDlg dialog

IMPLEMENT_DYNAMIC(CSerialTestDlg, CDialog)

CSerialTestDlg::CSerialTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSerialTestDlg::IDD, pParent)
{

}

CSerialTestDlg::~CSerialTestDlg()
{
}

void CSerialTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSerialTestDlg, CDialog)
	ON_WM_MEASUREITEM()
END_MESSAGE_MAP()


// CSerialTestDlg message handlers

void CSerialTestDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
