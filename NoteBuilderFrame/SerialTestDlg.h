#pragma once


// CSerialTestDlg dialog

class CSerialTestDlg : public CDialog
{
	DECLARE_DYNAMIC(CSerialTestDlg)

public:
	CSerialTestDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSerialTestDlg();

// Dialog Data
	enum { IDD = IDD_SERIAL_TEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
};
