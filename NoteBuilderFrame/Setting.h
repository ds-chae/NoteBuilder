#pragma once
#include "Define.h"
#include "afxwin.h"

// CSetting dialog
/**************************************************************
Date		: 2012/03/13
Author		: ryuhs74
Descriptor 
Level 값들을 Setting해주는 다이알로그 박스
****************************************************************/


class CSetting : public CDialog
{
	DECLARE_DYNAMIC(CSetting)

public:
	CSetting(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSetting();

// Dialog Data
	enum { IDD = IDD_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double GetDlgItemDouble(int id);
	void SetDlgItemDouble(int id, double value);
	afx_msg void OnBnClickedOk();

private:

public:	
	CEdit m_Rho;
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
};

extern CSetting *theHoughSetting;