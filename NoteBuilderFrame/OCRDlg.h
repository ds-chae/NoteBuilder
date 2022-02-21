#pragma once
#include "afxwin.h"


// COCRDlg dialog

#define	OCR_Y_MAX	90
#define	OCR_X_MAX	800

class COCRDlg : public CDialog
{
	DECLARE_DYNAMIC(COCRDlg)

public:
	COCRDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COCRDlg();
	void ClearPosInfo();

// Dialog Data
	enum { IDD = IDD_OCR };

public:
	CICUMANView* pViewWnd;
	BITMAP m_bitmap;
	CBitmap* wBitmap;
	CBitmap* irBitmap;

	int m_TotalPixels;
	int m_WThreshold;
	int m_IrThreshold;
	int m_Width;
	int m_Height;

	int m_W_ExpectedCount;
	int m_IR_ExpectedCount;
	int m_W_ResultCount;
	int m_IR_ResultCount;

#if USE_IR_Y_SUM
	int ir_y_sum[OCR_X_MAX];
#endif
#if USE_IR_X_SUM
	int ir_x_sum[OCR_Y_MAX];
#endif
	int ir_freq[256];
	int w_freq[256];

	int   NofChars;

	BOOL m_bAutoNNR;
	char Serial[24];

public:
	void DrawCharPos(CDC* pDC, RECT r);
	BOOL DoNNR(int charpos, int width, int height, int scale);
	void MakeDataFromBitmap(CBitmap *bitmap);
	void ApplyThreshold();
	void ApplyWindow();
	void MakeCopyImage();
	void ApplyPixels(int ir_count, int w_count);
	void ApplyPercent();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CEdit m_EditTotalPixels;
	CEdit m_EditWThreshold;
	CEdit m_EditIrThreshold;
	CEdit m_EditWPixels;
	CEdit m_EditIrPixels;
	CEdit m_EditHWindow;
	CEdit m_EditVWindow;
	afx_msg void OnBnClickedButtonAppThreshold();
protected:
	virtual void PostNcDestroy();
public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedOk();
	CListBox m_ListImage;
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg int OnCompareItem(int nIDCtl, LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	afx_msg void OnBnClickedButtonAppWindow();
	afx_msg void OnBnClickedButtonAppPixels();
	CEdit m_EditNnrFolder;
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonSave();
	CEdit m_EditNofChars;
	CEdit m_EditHWindow2;
	CEdit m_EditVWindow2;
	CComboBox m_ComboSource;
	afx_msg void OnCbnSelchangeComboSource();
	CEdit m_EditWPcnt;
	CEdit m_EditIrPcnt;
	CComboBox m_ComboHWindowMethod;
	afx_msg void OnCbnSelchangeComboHmethod();
	CComboBox m_comboThrType;
	afx_msg void OnCbnSelchangeComboThrType();
	afx_msg void OnBnClickedButtonDebug();
	afx_msg void OnBnClickedButtonCopyData();
	CComboBox m_ComboScaleX;
	afx_msg void OnCbnSelchangeComboScale();
	afx_msg void OnBnClickedButton1();
	CComboBox m_ComboScaleY;
};

extern COCRDlg* theOCRDlg;

