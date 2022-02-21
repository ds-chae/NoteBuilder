// NNRView.h : interface of the CNNRView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_NNRVIEW_H__2B890592_D429_447E_9B2D_3329703D61C8__INCLUDED_)
#define AFX_NNRVIEW_H__2B890592_D429_447E_9B2D_3329703D61C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Neural.h"
#include "afxwin.h"


#define SELECTSINGLE		1
#define SELECTONEHIDDEN		2
#define SELECTTWOHIDDEN		3


class CNNRView : public CFormView
{
protected: // create from serialization only
	CNNRView();
	virtual ~CNNRView();

	DECLARE_DYNCREATE(CNNRView)

public:

public:
	//{{AFX_DATA(CNNRView)
	enum { IDD = IDD_NNR_FORM };
	CListCtrl	m_List_Result;
	UINT	m_Edit_EpochNum;
	double	m_Edit_AverageError;
	CString	m_Combo_SelectLayer;
	int		m_Edit_SecondHiddenNode;
	int		m_Edit_InputNodeNum;
	double	m_Edit_ConversionErrorRate;
	double	m_Edit_Momentum;
	double	m_Edit_ETA;
	int		m_Edit_GroupNum;
	int		m_Edit_ClassNum;

	UINT	m_Edit_Sel_GroupNum;
	int		m_Edit_MatchMax;
	int		m_Edit_MatchMin;
	double	m_Edit_MaximumError;
	//}}AFX_DATA

// Attributes
public:
	CNNRDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNNRView)
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
//	static UINT StartNetThread(LPVOID);
//	void InitThreadNNR();

	void InitNNR();
	void LoadSingleWeight(UINT Char_Tile, UINT nCount_Tile);
	void LoadTwoHiddenWeight(UINT Char_Tile, UINT nCount_Tile);
	void LoadOneHiddenWeight(UINT Char_Tile, UINT nCount_Tile);
	void InitTwoHiddenLayer();
	void InitOneHiddenLayer();
	void DeleteTrainData();
	void LoadTrainData();
	void SingleWeightSave();
	void TwoHiddenWeightSave();
	void OneHiddenWeightSave();
	void WeightLoad();
	UINT FileDataLoad(CString FileFormat);
	void SaveIqFile(CString& FileName);
	int CountFiles(CString& pattern);

	unsigned char * m_Load_Buffer;

	double ** m_pLoad_FirstHiddenWeight;
	double ** m_pLoad_SecondHiddenWeight;
	double ** m_pLoad_OutputWeight;

	double m_InputMax;
	double m_InputMin;

	int m_GroupNum;
	int m_SelectLayer;
	int m_ActiveFlag;
	int m_WeightLoadFlag;

	int m_MaxErrorGroup;
	int m_MaxErrorClass;

	SYSTEMTIME m_startTime;

//	CWinThread *net;
//	int m_bStop;

	int m_SelectActivation;

	CNeuralTwoHidden * m_pNeuralTwoHidden;
	CNeuralOneHidden * m_pNeuralOneHidden;
//	double (*m_TrainData)[INPUTSAMPLE][INPUTNODE];
	double ***m_pTrainData;

	BOOL m_bPathSet;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CEdit m_EditExt;
	int m_foundGroups;
	int m_foundFiles;
	int m_elapsedSeconds;

	CString m_Ext;
// Generated message map functions
protected:
	//{{AFX_MSG(CNNRView)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButtonSampleload();
	afx_msg void OnButtonWeighthoad();
	afx_msg void OnButtonRun();
	afx_msg void OnButtonWeightsave();
	afx_msg void OnButtonConviq();
	afx_msg void OnButtonPathSelect();
	afx_msg void OnButtonMakeNtf();
	afx_msg void OnButtonPathResult();
	afx_msg void OnRadioLogistic();
	afx_msg void OnRadioHyperbolic();
	afx_msg void OnButtonStop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEditAverageLimit();
	afx_msg void OnEnChangeEditExt();
	afx_msg void OnBnClickedButtonRemake();
	afx_msg void OnBnClickedButtonMakeNtf2();
	afx_msg void OnBnClickedButtonFind();
	int m_EditInputNode;
};

#ifndef _DEBUG  // debug version in NNRView.cpp
inline CNNRDoc* CNNRView::GetDocument()
   { return (CNNRDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

void RegSetNNRFolder();
void RegGetNNRFolder();
BOOL BrowseFolder(HWND hWnd, TCHAR *pszPathname);

extern TCHAR RegNNRFolder[MAX_PATH];
extern CString NNRFolder;

extern double m_averageLimit;
extern double RegAverageLimit;

extern int m_RaxW;
extern int m_RaxH;

#endif // !defined(AFX_NNRVIEW_H__2B890592_D429_447E_9B2D_3329703D61C8__INCLUDED_)
