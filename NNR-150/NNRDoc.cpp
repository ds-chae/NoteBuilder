// NNRDoc.cpp : implementation of the CNNRDoc class
//

#include "stdafx.h"
#include "NNR.h"

#include "NNRDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNNRDoc

IMPLEMENT_DYNCREATE(CNNRDoc, CDocument)

BEGIN_MESSAGE_MAP(CNNRDoc, CDocument)
	//{{AFX_MSG_MAP(CNNRDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNNRDoc construction/destruction

CNNRDoc::CNNRDoc()
{
	// TODO: add one-time construction code here

}

CNNRDoc::~CNNRDoc()
{
}

BOOL CNNRDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CNNRDoc serialization

void CNNRDoc::Serialize(CArchive& ar)
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

/////////////////////////////////////////////////////////////////////////////
// CNNRDoc diagnostics

#ifdef _DEBUG
void CNNRDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CNNRDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CNNRDoc commands
