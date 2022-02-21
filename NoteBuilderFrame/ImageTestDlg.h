#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CImageTestDlg dialog

class CImageTestDlg : public CDialog
{
	DECLARE_DYNAMIC(CImageTestDlg)

public:
	CImageTestDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImageTestDlg();

// Dialog Data
	enum { IDD = IDD_IMAGE_TEST_DLG };

public:
	CICUMANView* pWndView;

public:
	void GetDataFromControl();
	void UpdateFromView();
	void ApplyPercent();
	void ResetCoeffs();
	void SetEdgeCount(int ri, int count);
	void FillEdges(int blobCandidateCount);
	void FillTestEdgeText(CString &txt);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void PostNcDestroy();
public:
	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedClose();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonAppThreshold2();
	afx_msg void OnBnClickedButtonAppPixels2();
	afx_msg void OnBnClickedButtonAppPercent();
	CEdit m_EditTotalPixels;
	CEdit m_EditWThreshold;
	CEdit m_EditIrThreshold;
	CEdit m_EditWPixels;
	CEdit m_EditIrPixels;
	CEdit m_EditWPercent;
	CEdit m_EditIrPercent;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAppThreshold();
	CSliderCtrl m_Slider1;
	afx_msg void OnBnClickedApplyUndistort();
	afx_msg void OnBnClickedReset();
	afx_msg void OnBnClickedCheckUndistort();
	CComboBox m_PlateType;
	afx_msg void OnCbnSelchangePlateType();
	CComboBox m_BlurType;
	afx_msg void OnCbnSelchangeBlurType();
	afx_msg void OnBnClickedEnableCrop();
	afx_msg void OnBnClickedEnableScale();
	CComboBox m_imgSelect;
//	CEdit m_NNR_Folder;
	CEdit m_ImgText;
	afx_msg void OnBnClickedBrowseNnrFolder();
	afx_msg void OnCbnSelchangeImgSelect();
	afx_msg void OnBnClickedImgSave();
	afx_msg void OnBnClickedTestTree();
	CEdit m_SelectedRect;
	afx_msg void OnBnClickedTestSelection();
	CComboBox m_Edges;
	afx_msg void OnCbnSelchangeEdges();
	afx_msg void OnBnClickedTestEdges();
	CComboBox m_EquidistMethod;
	afx_msg void OnCbnSelchangeEquidistMethod();
	afx_msg void OnEnChangeHorilineMinPixels2();
	afx_msg void OnBnClickedMarkTheEdge();
	afx_msg void OnBnClickedApplyFft();
	afx_msg void OnBnClickedNewCanny();
};

extern CImageTestDlg* theImageTester;

#define	CANNY_ORIGINAL	0x01
#define	UNDISTORT_IMAGE 0x02
#define	CANNY_CROPPED	0x04

void RecognizeLicensePlate(int flags);
