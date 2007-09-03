// MFC_OSG_MDIView.cpp : implementation of the CMFC_OSG_MDIView class
//

#include "stdafx.h"
#include "MFC_OSG_MDI.h"
#include "MFC_OSG_MDIDoc.h"
#include "MFC_OSG_MDIView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CMFC_OSG_MDIView, CView)

BEGIN_MESSAGE_MAP(CMFC_OSG_MDIView, CView)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_KEYDOWN()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CMFC_OSG_MDIView::CMFC_OSG_MDIView() :
   mOSG(0L)
{
}

CMFC_OSG_MDIView::~CMFC_OSG_MDIView()
{
}

BOOL CMFC_OSG_MDIView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

void CMFC_OSG_MDIView::OnDraw(CDC* /*pDC*/)
{
    CMFC_OSG_MDIDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;
}

#ifdef _DEBUG
void CMFC_OSG_MDIView::AssertValid() const
{
    CView::AssertValid();
}

void CMFC_OSG_MDIView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CMFC_OSG_MDIDoc* CMFC_OSG_MDIView::GetDocument() const // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFC_OSG_MDIDoc)));
    return (CMFC_OSG_MDIDoc*)m_pDocument;
}
#endif //_DEBUG


int CMFC_OSG_MDIView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    // Let MFC create the window before OSG
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Now that the window is created setup OSG
    mOSG = new cOSG(m_hWnd);

    return 1;
}

void CMFC_OSG_MDIView::OnDestroy()
{
    if(mOSG != 0) delete mOSG;

    WaitForSingleObject(mThreadHandle, 1000);

    CView::OnDestroy();
}

void CMFC_OSG_MDIView::OnInitialUpdate()
{
    CView::OnInitialUpdate();

    // Get Filename from DocumentOpen Dialog
    CString csFileName = GetDocument()->GetFileName();

    // Init the osg class
    mOSG->InitOSG(csFileName.GetString());

    // Start the thread to do OSG Rendering
    mThreadHandle = (HANDLE)_beginthread(&cOSG::Render, 0, mOSG); 
}

void CMFC_OSG_MDIView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // Pass Key Presses into OSG
    mOSG->getViewer()->getEventQueue()->keyPress(nChar);

    // Close Window on Escape Key
    if(nChar == VK_ESCAPE)
    {
        GetParent()->SendMessage(WM_CLOSE);
    }
}


BOOL CMFC_OSG_MDIView::OnEraseBkgnd(CDC* pDC)
{
    /* Do nothing, to avoid flashing on MSW */
    return true;
}
