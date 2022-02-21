#pragma once
#include "afxcmn.h"

#define	MAX_NUM_CURRENCY		(int)80
#define MAX_NUM_SAMPLE			(int)800
#define Num_Max_Key				(int)100
#define	MAX_SIZE_X				(int)100
#define	MAX_SIZE_Y				(int)100
#define additional_array_num	(int)6
#define	merge_margin			(int)2
#define min_merged_sum			(int)2//2

struct ST_DATA {
	int		merged_count,descriptor;
	BOOL	valid;
};

// CMakeRefDataDlg dialog

class CMakeRefDataDlg : public CDialog
{
	DECLARE_DYNAMIC(CMakeRefDataDlg)

public:
	CMakeRefDataDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMakeRefDataDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	BOOL m_bContinue;
	int		m_Max_Ref_Buf_Size;
	int		m_Num_Currency;
	int		m_Img_Size_X;
	int		m_Img_Size_Y;

	int NumKeys[MAX_NUM_CURRENCY][MAX_NUM_SAMPLE];
	int sample_number[MAX_NUM_CURRENCY];

	void CopyKeyFileToRefFolder(TCHAR* RefIndex, TCHAR* tszKeyFolder, TCHAR* tszKeyFile, TCHAR* tszRefFolder);
	void CopyKeyFolderToRefFolder(TCHAR* RefIndex, TCHAR* tszKeyFolder, TCHAR* tszRefFolder);
	void GenerateSrcFromRef(TCHAR* tszRefFolder, TCHAR* tszDspFolder, TCHAR* tszPrefix, int CurrencyNum, TCHAR* CurrencyCode);
	void VariablesInitialize();
	void LoadKeyData(int CurrencyIndex, CString Dir_path);
	void Merge_1_Data(int CurrencyIndex);
	void AddReference();
	int RemoveRef(CString Dir_path);
	void CheckMaxRefBuf();
	void MakeDataFile(int CurrencyNum, TCHAR* CurrencyCode); 
	void Check_Same_And_Add(int num_files, int CurrencyIndex);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonKeyFolder();
	afx_msg void OnBnClickedButtonRefFolder3();
	afx_msg void OnBnClickedButtonDspFolder();
	afx_msg void OnBnClickedButtonRefFolder();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CListCtrl m_ListDeleted;
};
