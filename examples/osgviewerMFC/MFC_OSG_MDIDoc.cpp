// MFC_OSG_MDIDoc.cpp : implementation of the CMFC_OSG_MDIDoc class
//

#include "stdafx.h"
#include "MFC_OSG_MDI.h"

#include "MFC_OSG_MDIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFC_OSG_MDIDoc

IMPLEMENT_DYNCREATE(CMFC_OSG_MDIDoc, CDocument)

BEGIN_MESSAGE_MAP(CMFC_OSG_MDIDoc, CDocument)
END_MESSAGE_MAP()


// CMFC_OSG_MDIDoc construction/destruction

CMFC_OSG_MDIDoc::CMFC_OSG_MDIDoc()
{
}

CMFC_OSG_MDIDoc::~CMFC_OSG_MDIDoc()
{
}

BOOL CMFC_OSG_MDIDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    m_csFileName = lpszPathName;

    if (!CDocument::OnOpenDocument(lpszPathName))
      return FALSE;

    return TRUE;
}


// CMFC_OSG_MDIDoc serialization

void CMFC_OSG_MDIDoc::Serialize(CArchive& ar)
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


// CMFC_OSG_MDIDoc diagnostics

#ifdef _DEBUG
void CMFC_OSG_MDIDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CMFC_OSG_MDIDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG


// CMFC_OSG_MDIDoc commands
