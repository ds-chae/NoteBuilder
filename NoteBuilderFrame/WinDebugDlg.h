#pragma once


// WinDebugDlg dialog

class WinDebugDlg : public CDialog
{
	DECLARE_DYNAMIC(WinDebugDlg)

public:
	WinDebugDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~WinDebugDlg();

// Dialog Data
	enum { IDD = IDD_WINDEBUG };

public:
	unsigned char* debugdata;
	int clen, t, w, h;
	int sh; // h <= 1 이면, sh == 256 이다 - 선 그래프 그리기
	char caption[128];
	CBitmap *pBitmap;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
};
