#pragma once
#include "afxwin.h"


// CSelectRegionDlg dialog

class CSelectRegionDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelectRegionDlg)

public:
	CSelectRegionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectRegionDlg();

// Dialog Data
	enum { IDD = IDD_SELECT_REGION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	int m_SelectedRegion;
	TCHAR m_SelectedName[128];

public:
	CListBox m_ListRegion;
	afx_msg void OnLbnDblclkListRegion();
	afx_msg void OnLbnSelchangeListRegion();
	virtual BOOL OnInitDialog();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
};
