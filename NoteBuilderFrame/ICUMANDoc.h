
// ICUMANDoc.h : interface of the CICUMANDoc class
//


#pragma once


class CICUMANDoc : public CDocument
{
protected: // create from serialization only
	CICUMANDoc();
	DECLARE_DYNCREATE(CICUMANDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CICUMANDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
};


