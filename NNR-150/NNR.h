// NNR.h : main header file for the NNR application
//

#if !defined(AFX_NNR_H__8921B114_8ADB_45D7_A2AF_55E5C07B8798__INCLUDED_)
#define AFX_NNR_H__8921B114_8ADB_45D7_A2AF_55E5C07B8798__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CNNRApp:
// See NNR.cpp for the implementation of this class
//

class CNNRApp : public CWinApp
{
public:
	CNNRApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNNRApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CNNRApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NNR_H__8921B114_8ADB_45D7_A2AF_55E5C07B8798__INCLUDED_)
