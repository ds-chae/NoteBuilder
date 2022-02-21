#pragma once


// CBuildLog dialog

class CBuildLog : public CDialog
{
	DECLARE_DYNAMIC(CBuildLog)

public:
	CBuildLog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBuildLog();

// Dialog Data
	TCHAR* m_Message;

	enum { IDD = IDD_BUILDLOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};
