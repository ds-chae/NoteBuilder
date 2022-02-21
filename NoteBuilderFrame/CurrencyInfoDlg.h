#pragma once
#include "afxwin.h"


// CCurrencyInfoDlg dialog

class CCurrencyInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CCurrencyInfoDlg)

public:
	CCurrencyInfoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCurrencyInfoDlg();

// Dialog Data
	enum { IDD = IDD_CURRENCY_INFO };

public:
	CICUMANView* pViewWnd;
	ST150Image* pImgST150;
	BOOL m_bContinue;

public:
	void FillRegionSetOrDllFunctions();

	void CopyExtractedFileToNNRFolder(TCHAR* currentRefIndex, TCHAR* subPath, TCHAR* subFile, TCHAR* tszRefFolder);
	void CopyExtractFolderToNNRFolder(TCHAR* RefIndex, TCHAR* tszExtFolder, TCHAR* tszNNRFolder);
	void UpdateCurrencyInfo();
	void SetDialogItems();
	void TestKeysOnFile(TCHAR* name);
	void TestKeysOnPath(TCHAR* path);
	void GenerateKeysOnFile(CICUMANView* pViewWnd, TCHAR* name, TCHAR* keyname);
	void GenerateKeysOnPath(CICUMANView* pViewWnd, TCHAR* path, TCHAR* keypath);
	void ExtractDataOnPath(int function, TCHAR* tszRegion, CICUMANView* pViewWnd, TCHAR* path, TCHAR* keypath, TCHAR* tszPrefix);
	void ExtractDataOnFile(int function, TCHAR* tszRegion, CICUMANView* pViewWnd, TCHAR* name, TCHAR* keyname);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboCurrencyList();
	CComboBox m_ComboCurrencyList;
	virtual BOOL OnInitDialog();
	CEdit m_EditCurCode;
	CEdit m_EditCurNumber;
	CEdit m_EditRectRegion1;
	CEdit m_EditDeciReg1;
	CEdit m_EditSumReg1;
	CEdit m_EditThresReg1;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonDefReg1();

public:
	afx_msg void OnBnClickedButtonTestref();
	CEdit m_EditDenomValues;
	CEdit m_EditCurVersions;
	CEdit m_EditNoteSize;
	afx_msg void OnBnClickedButtonLoadref();
//	CEdit m_EditImgNum;
	afx_msg void OnBnClickedButtonSel1();
	afx_msg void OnCbnSelchangeComboImageSource();
	CComboBox m_ComboReinforcement;
	afx_msg void OnCbnSelchangeComboReinforcement();
	afx_msg void OnBnClickedButtonKeyFolder();
	afx_msg void OnBnClickedButtonKeyFolder1();
	afx_msg void OnBnClickedButtonGenkeys1();
	afx_msg void OnBnClickedButtonMakeref1();
	afx_msg void OnBnClickedButtonExtFolder();
	afx_msg void OnBnClickedButtonDstFolder();
	afx_msg void OnBnClickedButtonExtractData();
	afx_msg void OnBnClickedButtonGatherData();
	afx_msg void OnBnClickedButtonExecuteNnr();
	afx_msg void OnBnClickedCheckRegionSet();
	afx_msg void OnBnClickedCheckViaDll();
	afx_msg void OnCbnSelchangeComboFunction();
	CComboBox m_test;
	CComboBox m_ComboRecogMethod;
	afx_msg void OnBnClickedButtonSaveRecogMethod();
	afx_msg void OnCbnSelchangeComboRecogMethod();
	afx_msg void OnBnClickedButtonLoadNnr();
	afx_msg void OnBnClickedCheckTestThruTestdll();
	afx_msg void OnBnClickedCancel();
};
