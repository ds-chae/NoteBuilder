
// ICUMANDoc.cpp : implementation of the CICUMANDoc class
//

#include "stdafx.h"
#include "NoteBuilderFrame.h"

#include "ICUMANDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CICUMANDoc

IMPLEMENT_DYNCREATE(CICUMANDoc, CDocument)

BEGIN_MESSAGE_MAP(CICUMANDoc, CDocument)
END_MESSAGE_MAP()


// CICUMANDoc construction/destruction

CICUMANDoc::CICUMANDoc()
{
	// TODO: add one-time construction code here

}

CICUMANDoc::~CICUMANDoc()
{
}

BOOL CICUMANDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CICUMANDoc serialization

void CICUMANDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CICUMANDoc diagnostics

#ifdef _DEBUG
void CICUMANDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CICUMANDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CICUMANDoc commands

BOOL CICUMANDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  Add your specialized creation code here

	return TRUE;
}
