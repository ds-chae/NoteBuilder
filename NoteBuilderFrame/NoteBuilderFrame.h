
// NoteBuilderFrame.h : main header file for the NoteBuilderFrame application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CNoteBuilderFrameApp:
// See NoteBuilderFrame.cpp for the implementation of this class
//

class CNoteBuilderFrameApp : public CWinAppEx
{
public:
	CNoteBuilderFrameApp();

protected:
	CMultiDocTemplate* m_pDocTemplate;
public:

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnFileNewFrame();
	afx_msg void OnFileNew();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CNoteBuilderFrameApp theApp;

extern	TCHAR RegNNRFolder[MAX_PATH];
extern	TCHAR m_tszKeyFolder1[MAX_PATH];
extern	TCHAR m_tszKeyFolder2[MAX_PATH];
extern	TCHAR m_tszRefFolder[MAX_PATH];
extern	TCHAR m_tszExtFolder[MAX_PATH];
extern	TCHAR m_tszDstFolder[MAX_PATH];
extern	TCHAR m_tszNNRExeFile[MAX_PATH];

extern int canny_th_min[];
extern int canny_th_max[];
extern int canny_th_cnt;

#define	MAX_OCR_LIMIT	16

#if CURRENCY
extern "C" int OCR_pos_cnt;
#endif

extern  int bTestThruTestDLL;
extern	int	bNNRLoaded;

extern BOOL bDebugEnabled;
extern int  nPlateType;
extern int	nBlurType;
extern int  equitest_method;
extern int  bTestHoughLine;
extern int  bApplyFFT;
extern int  bNewCanny;

#define	BLUR_NORMAL		0
#define	BLUR_GAUSSIAN	1

extern TCHAR *tszPlateOptions[];

void RegSetNNRFolder();
void RegGetNNRFolder();
void RegSetOptions();
void RegGetOptions();

#define	AXIAL_LPR TEXT("SOFTWARE\\AxialSpace\\LPR")

void CannyEdge(BYTE *pImage, int step, int width, int height, int th_high, int th_low, BYTE *pEdge);

extern int var_limit;
extern int grid_cell_height;
extern int grid_cell_width;
