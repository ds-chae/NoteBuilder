// NNRDoc.h : interface of the CNNRDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_NNRDOC_H__9F86F8DA_01FA_4C58_ACA7_759551C04DC1__INCLUDED_)
#define AFX_NNRDOC_H__9F86F8DA_01FA_4C58_ACA7_759551C04DC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CNNRDoc : public CDocument
{
protected: // create from serialization only
	CNNRDoc();
	DECLARE_DYNCREATE(CNNRDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNNRDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNNRDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CNNRDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NNRDOC_H__9F86F8DA_01FA_4C58_ACA7_759551C04DC1__INCLUDED_)
