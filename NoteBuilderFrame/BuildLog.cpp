// BuildLog.cpp : implementation file
//

#include "stdafx.h"
#include "NoteBuilderFrame.h"
#include "BuildLog.h"


// CBuildLog dialog

IMPLEMENT_DYNAMIC(CBuildLog, CDialog)

CBuildLog::CBuildLog(CWnd* pParent /*=NULL*/)
	: CDialog(CBuildLog::IDD, pParent)
{
	m_Message = NULL;
}

CBuildLog::~CBuildLog()
{
}

void CBuildLog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBuildLog, CDialog)
END_MESSAGE_MAP()


// CBuildLog message handlers

BOOL CBuildLog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(m_Message != NULL)
		SetDlgItemText(IDC_EDIT_MSG, m_Message);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
