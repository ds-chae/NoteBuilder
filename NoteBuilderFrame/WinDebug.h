#pragma once


// CWinDebug

class CWinDebug : public CWnd
{
	DECLARE_DYNAMIC(CWinDebug)

public:
	CWinDebug();
	virtual ~CWinDebug();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


