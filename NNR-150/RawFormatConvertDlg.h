#pragma once
#include "afxwin.h"

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CRawFormatConvertDlg dialog

class CRawFormatConvertDlg : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CRawFormatConvertDlg)

public:
	CRawFormatConvertDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRawFormatConvertDlg();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_DLG_RAWFMT_CONVERT, IDH = IDR_HTML_RAWFORMATCONVERTDLG };

public:
	int srcWidth;
	int srcHeight;
	int m_ScaleX;
	int m_ScaleY;

public:
	void UpdateInfo();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	afx_msg void OnBnClickedButtonPathSelect();
	afx_msg void OnBnClickedButtonPathSelect2();
	afx_msg void OnBnClickedButtonSrcFind();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeEditDstWidth();
	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedOk3();
	CComboBox m_ComboScaleX;
	CComboBox m_ComboScaleY;
	afx_msg void OnCbnSelchangeComboScale1();
	afx_msg void OnCbnSelchangeComboScaleY();
};
