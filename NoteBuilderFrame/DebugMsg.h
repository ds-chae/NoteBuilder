#pragma once


// CDebugMsg dialog

class CDebugMsg : public CDialog
{
	DECLARE_DYNAMIC(CDebugMsg)

public:
	CDebugMsg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDebugMsg();

// Dialog Data
	enum { IDD = IDD_DEBUG_MSG };

public:
	TCHAR msg[1024];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
