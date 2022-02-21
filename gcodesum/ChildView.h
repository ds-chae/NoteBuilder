
// ChildView.h : CChildView Ŭ������ �������̽�
//


#pragma once


// CChildView â

class CChildView : public CWnd
{
// �����Դϴ�.
public:
	CChildView();

// Ư���Դϴ�.
public:
	int   x_min, x_max;
	int   y_min, y_max;
	int   x_cur, y_cur, z_cur;
	int   z_min, z_max;

// �۾��Դϴ�.
public:
	void parseLine(char* linebuffer);

// �������Դϴ�.
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// �����Դϴ�.
public:
	virtual ~CChildView();

	// ������ �޽��� �� �Լ�
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateFileOpen(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileAdd(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileNew(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileClose(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
	afx_msg void OnFileOpen();
};

