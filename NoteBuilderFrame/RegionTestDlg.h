#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#if 0
// CRegionTestDlg dialog

class CRegionTestDlg : public CDialog
{
	DECLARE_DYNCREATE(CRegionTestDlg)

public:
	CRegionTestDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegionTestDlg();
// Overrides

// Dialog Data
	enum { IDD = IDD_REGION_TEST };

public:
	CICUMANView* pViewWnd;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
//	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_EditSx;
	CEdit m_EditEx;
	CEdit m_EditDecX;
	CEdit m_EditSumX;
	CEdit m_EditSy;
	CEdit m_EditEy;
	CEdit m_EditDecY;
	CEdit m_EditSumY;
	CEdit m_EditDegree;
	CEdit m_EditIRAvg;
	CEdit m_EditRegionSize;
	CComboBox m_ComboThreadType;
	CEdit m_EditThreadWidth;
	CEdit m_EditThreadPos;
	CEdit m_EditThreadAvg;
	CComboBox m_ComboDirection;

	CComboBox* m_ComboListL[8];
	CComboBox* m_ComboListR[8];
	CStatic* m_LabelResult[8];

	ST_IMG_REGION ExtRegion;
	int m_AddIndex;
	BOOL m_bAutoExtract;
	BOOL m_bByDenom;

public:
	void AddIndexToCombo();
	void GetImgRegion(ST_IMG_REGION *preg);
	void UpdateFromView();
	void UpdateRectFromView();
	void PutRegionParametersToView(ST_IMG_REGION& ExtRegion);
	void UpdateResult();
	ST_IMG_REGION* AddRegionToList(ST_IMG_REGION *exRegion, TCHAR* name);
	void SetRegionByDenom();
	void SetItemByRegion(ST_IMG_REGION* pRegion);

protected:
	virtual void PostNcDestroy();
public:

	afx_msg void OnBnClickedClose();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeComboDirection();
	afx_msg void OnBnClickedButtonExtract();
	afx_msg void OnBnClickedButtonFindThread();
	CEdit m_EditThreadSize;
	afx_msg void OnBnClickedButtonGenerate();
	CComboBox m_ComboRegionType;
	afx_msg void OnBnClickedButtonReload();
	CEdit m_EditRegName;
	CEdit m_EditRegIdx;

	CComboBox m_ComboR_0_L;
	CComboBox m_ComboR_1_L;
	CComboBox m_ComboR_2_L;
	CComboBox m_ComboR_3_L;
	CComboBox m_ComboR_4_L;
	CComboBox m_ComboR_5_L;
	CComboBox m_ComboR_6_L;
	CComboBox m_ComboR_7_L;

	CComboBox m_ComboR_0_R;
	CComboBox m_ComboR_1_R;
	CComboBox m_ComboR_2_R;
	CComboBox m_ComboR_3_R;
	CComboBox m_ComboR_4_R;
	CComboBox m_ComboR_5_R;
	CComboBox m_ComboR_6_R;
	CComboBox m_ComboR_7_R;

	CStatic m_LabelResult0;
	CStatic m_LabelResult1;
	CStatic m_LabelResult2;
	CStatic m_LabelResult3;
	CStatic m_LabelResult4;
	CStatic m_LabelResult5;
	CStatic m_LabelResult6;
	CStatic m_LabelResult7;

	afx_msg void OnCbnSelchangeComboR00();
	afx_msg void OnCbnSelchangeComboR01();
	afx_msg void OnCbnSelchangeComboR10();
	afx_msg void OnCbnSelchangeComboR11();
	afx_msg void OnCbnSelchangeComboR20();
	afx_msg void OnCbnSelchangeComboR21();
	afx_msg void OnCbnSelchangeComboR30();
	afx_msg void OnCbnSelchangeComboR31();
	afx_msg void OnCbnSelchangeComboR40();
	afx_msg void OnCbnSelchangeComboR41();
	afx_msg void OnCbnSelchangeComboR50();
	afx_msg void OnCbnSelchangeComboR51();
	afx_msg void OnCbnSelchangeComboR60();
	afx_msg void OnCbnSelchangeComboR61();
	afx_msg void OnCbnSelchangeComboR70();
	afx_msg void OnCbnSelchangeComboR71();
	afx_msg void OnBnClickedButtonAdd();
	CEdit m_EditAvgW;
	CEdit m_EditMinW;
	CEdit m_EditMaxW;
	CEdit m_EditVarW;
	CEdit m_EditMinIR;
	CEdit m_EditMaxIR;
	CEdit m_EditVarIR;
	afx_msg void OnBnClickedButtonSelect();
	CListBox m_ListRegion;
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnLbnSelchangeListRegion();
	afx_msg void OnBnClickedButtonNoDraw();
	afx_msg void OnDestroy();
//	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnBnClickedButtonIrAndW();
	CComboBox m_ComboCisEnc;
	afx_msg void OnCbnSelchangeComboCisEnc();
	afx_msg void OnBnClickedButtonSave();
	CButton m_CheckAutoExtract;
	afx_msg void OnBnClickedCheckAutoExtract();
	afx_msg void OnBnClickedButtonCopyRegion();
	afx_msg void OnBnClickedCheckSerialImage();
	afx_msg void OnEnChangeEditDecy();
	afx_msg void OnBnClickedButtonSaveSerImg();
	afx_msg void OnBnClickedCheckByDenom();
};

extern CRegionTestDlg *pRegionTestDlg;

#endif
