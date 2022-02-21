
// NoteBuilderFrameDoc.h : interface of the CNoteBuilderFrameDoc class
//


#pragma once


class CNoteBuilderFrameDoc : public CDocument
{
protected: // create from serialization only
	CNoteBuilderFrameDoc();
	DECLARE_DYNCREATE(CNoteBuilderFrameDoc)

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
	virtual ~CNoteBuilderFrameDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


