#pragma once
#include "afxwin.h"


// COCRSaveDlg dialog

class COCRSaveDlg : public CDialog
{
	DECLARE_DYNAMIC(COCRSaveDlg)

public:
	COCRSaveDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COCRSaveDlg();

public:
	void SaveFileToClass(TCHAR* SaveFolder, TCHAR* ClassFolder, int img_width, int i, int old_w, int SaveWidth, int SaveHeight);
	void SetEditWidthHeight(int CtlID, int w, int h);
	void UpdateSaveWidthHeight();
	void SetWidthHeightControls();
	void TestSaveWidthHeightInput(); // 글자의 위치, 크기가 변경되어 입력되었는지 검사한다.
	void InitialProcess();

public:
	TCHAR Serial[24];
	unsigned char ocr_data[90*1000];
	int y_pos_0;
	int v_window;
	int img_width;

	CBitmap *m_bitmap;

	// Dialog Data
	enum { IDD = IDD_OCR_SAVE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CEdit m_EditC0;
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	CListBox m_ListImage;
	CEdit m_EditSaveFolder;
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedBtnReadSize();
	afx_msg void OnBnClickedButtonTest();
protected:
	virtual void PostNcDestroy();
public:
	afx_msg void OnBnClickedCancel();
};

extern COCRSaveDlg* theOCRSaveDlg;
