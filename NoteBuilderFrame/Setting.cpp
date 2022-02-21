// Setting.cpp : implementation file
//

#include "stdafx.h"
#include "NoteBuilderFrame.h"

#include "ICUMANDoc.h"
#include "Define.h"
#include "opencv_use.h"
#include "CarNumDetect.h"
#include "ICUMANView.h"

#include "Setting.h"

CSetting *theHoughSetting = NULL;

// CSetting dialog

IMPLEMENT_DYNAMIC(CSetting, CDialog)

CSetting::CSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CSetting::IDD, pParent)
{

}

CSetting::~CSetting()
{
}

void CSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ROH, m_Rho);
}


BEGIN_MESSAGE_MAP(CSetting, CDialog)
		ON_BN_CLICKED(IDOK, &CSetting::OnBnClickedOk)
END_MESSAGE_MAP()


extern double hough_rho;
extern double hough_theta;
extern int hough_threshold;
extern double hough_minLineLength;
extern double hough_maxLineGap;

// CSetting message handlers
void CSetting::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	hough_rho = GetDlgItemDouble(IDC_ROH);
	hough_theta = GetDlgItemDouble(IDC_THETA);
	hough_threshold = GetDlgItemInt(IDC_THRESHOLD);
	hough_minLineLength = GetDlgItemDouble(IDC_MIN_LINE_LENGTH);
	hough_maxLineGap = GetDlgItemDouble(IDC_MAX_LINE_GAP);

	EdgeLinePoints = GetDlgItemInt(IDC_EDGE_POINTS);
	EdgeLineMinX = GetDlgItemInt(IDC_EDGE_LINE_MIN_X);
	EdgeLineMaxY = GetDlgItemInt(IDC_EDGE_LINE_MAX_Y);
	EdgeLineMaxX = GetDlgItemInt(IDC_EDGE_LINE_MAX_X);
	EdgeLineMinY = GetDlgItemInt(IDC_EDGE_LINE_MIN_Y);

	EdgeSlopeMinX = GetDlgItemDouble(IDC_EDGE_SLOPE_MIN_X);
	EdgeSlopeMaxX = GetDlgItemDouble(IDC_EDGE_SLOPE_MAX_X);
	EdgeSlopeMinY = GetDlgItemDouble(IDC_EDGE_SLOPE_MIN_Y);
	EdgeSlopeMaxY = GetDlgItemDouble(IDC_EDGE_SLOPE_MAX_Y);
	EdgeMaxVarX = GetDlgItemDouble(IDC_EDGE_VAR_X);
	EdgeMaxVarY = GetDlgItemDouble(IDC_EDGE_VAR_Y);

	if(pMainView != NULL)
		pMainView ->ApplyThreshold(NULL, NULL);
}

double CSetting::GetDlgItemDouble(int id)
{
	TCHAR tbuf[64];
	GetDlgItemText(id, tbuf, 64);
	return wcstod(tbuf, NULL);
}

void CSetting::SetDlgItemDouble(int id, double value)
{
	TCHAR tbuf[16];
	float f = value;
	swprintf(tbuf, L"%f", f);
	SetDlgItemText(id, tbuf);
}


BOOL CSetting::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemDouble(IDC_ROH, hough_rho);
	SetDlgItemDouble(IDC_THETA, hough_theta);
	SetDlgItemInt(IDC_THRESHOLD, hough_threshold);
	SetDlgItemDouble(IDC_MIN_LINE_LENGTH, hough_minLineLength);
	SetDlgItemDouble(IDC_MAX_LINE_GAP, hough_maxLineGap);

	SetDlgItemInt(IDC_EDGE_POINTS, EdgeLinePoints);
	SetDlgItemInt(IDC_EDGE_LINE_MIN_X, EdgeLineMinX);
	SetDlgItemInt(IDC_EDGE_LINE_MAX_Y, EdgeLineMaxY);
	SetDlgItemInt(IDC_EDGE_LINE_MAX_X, EdgeLineMaxX);
	SetDlgItemInt(IDC_EDGE_LINE_MIN_Y, EdgeLineMinY);
	SetDlgItemDouble(IDC_EDGE_SLOPE_MIN_X, EdgeSlopeMinX);
	SetDlgItemDouble(IDC_EDGE_SLOPE_MAX_X, EdgeSlopeMaxX);
	SetDlgItemDouble(IDC_EDGE_SLOPE_MIN_Y, EdgeSlopeMinY);
	SetDlgItemDouble(IDC_EDGE_SLOPE_MAX_Y, EdgeSlopeMaxY);
	SetDlgItemDouble(IDC_EDGE_VAR_X, EdgeMaxVarX);
	SetDlgItemDouble(IDC_EDGE_VAR_Y, EdgeMaxVarY);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CSetting::PostNcDestroy()
{
	delete this;
	theHoughSetting = NULL;

	CDialog::PostNcDestroy();
}
