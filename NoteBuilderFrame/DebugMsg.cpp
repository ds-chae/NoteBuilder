// DebugMsg.cpp : implementation file
//

#include "stdafx.h"
#include "NoteBuilderFrame.h"
#include "DebugMsg.h"


// CDebugMsg dialog

IMPLEMENT_DYNAMIC(CDebugMsg, CDialog)

CDebugMsg::CDebugMsg(CWnd* pParent /*=NULL*/)
	: CDialog(CDebugMsg::IDD, pParent)
{

}

CDebugMsg::~CDebugMsg()
{
}

void CDebugMsg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDebugMsg, CDialog)
	ON_BN_CLICKED(IDOK, &CDebugMsg::OnBnClickedOk)
END_MESSAGE_MAP()


// CDebugMsg message handlers

void CDebugMsg::OnBnClickedOk()
{
	CWnd * pWnd = GetDlgItem(IDC_EDIT_MSG);
	pWnd->SendMessage(EM_SETSEL, 0, (LPARAM)-1);
    pWnd->SendMessage(WM_COPY, 0, 0);

	OnOK();
}

BOOL CDebugMsg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_EDIT_MSG, msg);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
