
// ICUMANView.h : interface of the CICUMANView class
//

#pragma once

#define	MAX_ENTRIES	10000

using namespace cv;

class CICUMANView : public CScrollView
{
protected: // create from serialization only
	CICUMANView();
	DECLARE_DYNCREATE(CICUMANView)

// Attributes
public:
	CICUMANDoc* GetDocument() const;

	TCHAR szPathName[MAX_PATH];
	TCHAR szLeafName[MAX_PATH];
	TCHAR DirEntries[MAX_ENTRIES][MAX_PATH];
	int	  EntryCount;

//	CBitmap* m_hBitmap; - new AImage don't require this

	TCHAR tszRectAverage[128];
	int AverageW;
	int AverageIR;

	CSize ScrollSize;
	CPoint m_ViewportOrg;
	int DrawHeight;

	CPoint m_MovePoint;
	CPoint m_DownPoint;
	CPoint DownPoint; // Viewport에 따라 값이 조정된 좌표
	CPoint MovePoint;

	BOOL m_bDown;
	int DirFromDlg;
	int ThreadType;
	int ThreadWidth;
	int ThreadPos;
	int ThreadSizeX;
	int ThreadSizeY;

	int x_sum[500];
	int DecX;
	int DecY;
	int SumX;
	int SumY;

	CBitmap *MergeBitmap;
	CBitmap *ExtBitmap;
	int ExtSizeX;
	int ExtSizeY;
	int ExtAvgW;
	int ExtAvgIR;

	int ExtMinW;
	int ExtMaxW;
	int ExtMinIR;
	int ExtMaxIR;

	int ExtVarW;
	int ExtVarIR;

	int m_ImgReinforce;

	int m_WPixels;
	int m_IrPixels;

	//HDS
	CCarNumDetect carNumDetect;
	Mat CarNumPlate;

	// Operations
public:
	void GetSelectedROI(int sel, Mat& img);
	void ApplyThreshold(FILE* retf, TCHAR* path);
	void AddDebugImage(CBitmap *pBitmap);
	int ProcessOneNote(FILE* retf, TCHAR* szPathName);
	//HDS
	int ProcessOneNote_HDS(FILE* retf, TCHAR* szPathName, Mat* imgmat);
	int ProcessOneNote_HDS2(FILE* retf, TCHAR* szPathName, Mat* imgmat);
	int ProcessOneNote_Fill(FILE* retf, TCHAR* szPathName, Mat* imgmat);
	void SelectImageOnPlate(int sel);
	void DrawEdge(int sel);
	void MarkEdge(int edge_to_mark);
	void GridTest(FILE* retf, TCHAR* szPathName);
	void VarAnalysisGrid(int x, int y);

	void ToggleImgReinforce();
	int CopyIfDstIsOld(TCHAR* src, TCHAR* dst);
//	BOOL Conv_TestDLL_vcproj(TCHAR* dst);
//	BOOL Conv_dllmain_cpp(TCHAR* dst);
//	void TestProjectTestDLL();

	void SaveThreadRegion(TCHAR* name);
	void GenerateRegionRawOnFile(TCHAR* name);
	void GenerateRegionRawOnPath(TCHAR* path);
//	void OnGenerateRegionRaw(int tType, int tWidth);

	void DrawMainImage(CDC* pDC);
	void FindThread(int tType, int tWidth);
	void DisplayRegionFromSelf(CDC* pDC);
	void DisplayRegionFromDlg(CDC* pDC);
	void MergeIrAndW();

	void TestTree();
	void TestTree(FILE *f, TCHAR* path);
	void TestFile(FILE *f, TCHAR* path);
	void TestSelection();
	void TestEdges(TCHAR* tszbuf);

	void ExtractRegion(BOOL bInvalidate, BOOL bGetNewBitmap);
//	void SetCurrentLocalByPath(TCHAR* path);
	int  SetPathName(TCHAR* path);
	void ClearMem();
	void GetWidthHeight(LPCTSTR str);
	int  FillListFromDirectoryEntries();
//	void MakeImageFromFiles();
//	void RemoveCurrencyInfo();
//	void MakeRegionImage(int RegionNumber); 20120820, dschae 필요 없어서 삭제함.

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CICUMANView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnViewDebugimage();
//	afx_msg void OnOcr();
	afx_msg void OnViewImageTester();
	afx_msg void OnUpdateOcr(CCmdUI *pCmdUI);
	afx_msg void OnCurrencySetting(); //ryuhs74
//	afx_msg void OnViewMgUvCIS();

private:
	sSetting m_sSetting;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

#ifndef _DEBUG  // debug version in ICUMANView.cpp
inline CICUMANDoc* CICUMANView::GetDocument() const
   { return reinterpret_cast<CICUMANDoc*>(m_pDocument); }
#endif

extern CICUMANView* pMainView;
