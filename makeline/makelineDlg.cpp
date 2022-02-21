
// makelineDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "makeline.h"
#include "makelineDlg.h"
#include "afxdialogex.h"

using namespace cv;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CmakelineDlg ��ȭ ����




CmakelineDlg::CmakelineDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CmakelineDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CmakelineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CmakelineDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CmakelineDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CmakelineDlg �޽��� ó����

BOOL CmakelineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CmakelineDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CmakelineDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CmakelineDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int main(char *fname);

void CmakelineDlg::OnBnClickedButton1()
{
	int ith = GetDlgItemInt(IDC_THRESHOLD);
	
	CString str;
	CString pathname;
	CFileDialog Dlg(FALSE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, str, NULL);
	if(Dlg.DoModal() != IDOK)
		return;

	pathname = Dlg.GetPathName();
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	char szpath[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)pathname, pathname.GetLength()+1, szpath, MAX_PATH, NULL, NULL);
#if 0
	Mat mat_src = imread(szpath, CV_LOAD_IMAGE_GRAYSCALE);
	if(mat_src.empty())
		return;
	Mat mat_dst;
	double dth = (double)ith;
	double dmx = 120;
	Canny(mat_src, mat_dst, dth, dmx, 3);

	threshold(mat_dst, mat_dst, dth, dmx, THRESH_BINARY_INV);
	char bwpath[MAX_PATH];
	strcpy_s(bwpath, MAX_PATH, szpath);
	strcat_s(bwpath, MAX_PATH, ".bw.jpg");
	imwrite(bwpath, mat_dst);
#else
	main(szpath);
#endif
}

int main(char *fname)
{
	double alpha = 0.5, beta;
	Mat src, gauss;
	src = imread(fname);

	Mat src_gray;
	cvtColor( src, src_gray, CV_BGR2GRAY );
	//   randn(gauss, 0, 0.005);
	gauss = src_gray.clone();
	randn(gauss, 0, 5);
	Mat gaussian_noise;
	src_gray.copyTo(gaussian_noise);

	beta = (1.0 - alpha);
//	addWeighted(src_gray, alpha, gauss, beta, 0.0, gaussian_noise);
	gaussian_noise += gauss;

	cvNamedWindow("src", CV_WINDOW_AUTOSIZE);//CV_WINDOW_FREERATIO);
	imshow("src", src_gray);

	cvNamedWindow("gaussian_noise", CV_WINDOW_AUTOSIZE);
	imshow("gaussian_noise", gaussian_noise);

	waitKey(0);
	return 0;

#if 0
	Mat src, gauss;
//	src = imread("20140221_114914.jpg");
	src = imread(fname);
	imshow("src", src);
	gauss= src.clone();
	randn(gauss, 0, 0.005);

	cvNamedWindow("gaussian_noise", CV_WINDOW_AUTOSIZE);
	imshow("gaussian_noise", gauss);

	waitKey(0);
	return 0;   
#endif
}
