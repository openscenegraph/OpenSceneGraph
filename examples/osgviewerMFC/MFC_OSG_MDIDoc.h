// MFC_OSG_MDIDoc.h : interface of the CMFC_OSG_MDIDoc class
//


#pragma once


class CMFC_OSG_MDIDoc : public CDocument
{
protected: // create from serialization only
    CMFC_OSG_MDIDoc();
    DECLARE_DYNCREATE(CMFC_OSG_MDIDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
    virtual void Serialize(CArchive& ar);
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    CString GetFileName() const { return m_csFileName; }

// Implementation
public:
    virtual ~CMFC_OSG_MDIDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    CString m_csFileName;

// Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
};


