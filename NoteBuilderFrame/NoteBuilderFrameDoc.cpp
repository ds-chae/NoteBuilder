
// NoteBuilderFrameDoc.cpp : implementation of the CNoteBuilderFrameDoc class
//

#include "stdafx.h"
#include "NoteBuilderFrame.h"

#include "NoteBuilderFrameDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNoteBuilderFrameDoc

IMPLEMENT_DYNCREATE(CNoteBuilderFrameDoc, CDocument)

BEGIN_MESSAGE_MAP(CNoteBuilderFrameDoc, CDocument)
END_MESSAGE_MAP()


// CNoteBuilderFrameDoc construction/destruction

CNoteBuilderFrameDoc::CNoteBuilderFrameDoc()
{
	// TODO: add one-time construction code here

}

CNoteBuilderFrameDoc::~CNoteBuilderFrameDoc()
{
}

BOOL CNoteBuilderFrameDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CNoteBuilderFrameDoc serialization

void CNoteBuilderFrameDoc::Serialize(CArchive& ar)
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


// CNoteBuilderFrameDoc diagnostics

#ifdef _DEBUG
void CNoteBuilderFrameDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CNoteBuilderFrameDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CNoteBuilderFrameDoc commands
