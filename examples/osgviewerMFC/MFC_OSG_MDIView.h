// MFC_OSG_MDIView.h : interface of the CMFC_OSG_MDIView class
//
#pragma once

#include "MFC_OSG.h"

class CMFC_OSG_MDIView : public CView
{
protected: // create from serialization only
    CMFC_OSG_MDIView();
    DECLARE_DYNCREATE(CMFC_OSG_MDIView)

// Attributes
public:
    CMFC_OSG_MDIDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
    virtual void OnDraw(CDC* pDC);  // overridden to draw this view
    virtual void OnInitialUpdate();
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
    virtual ~CMFC_OSG_MDIView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    cOSG* mOSG;
    HANDLE mThreadHandle;

// Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()

    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#ifndef _DEBUG  // debug version in MFC_OSG_MDIView.cpp
inline CMFC_OSG_MDIDoc* CMFC_OSG_MDIView::GetDocument() const
   { return reinterpret_cast<CMFC_OSG_MDIDoc*>(m_pDocument); }
#endif

