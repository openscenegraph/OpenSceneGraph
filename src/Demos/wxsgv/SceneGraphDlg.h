//
// Name:    SceneGraphDlg.h
// Author:    Ben Discoe, ben@washedashore.com
//

#ifndef __SceneGraphDlg_H__
#define __SceneGraphDlg_H__

#ifdef __GNUG__
    #pragma interface "SceneGraphDlg.cpp"
#endif

#include "wx/imaglist.h"
#include "wxsgv_wdr.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// SceneGraphDlg
//----------------------------------------------------------------------------

class SceneGraphDlg: public wxDialog
{
public:
    // constructors and destructors
    SceneGraphDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    ~SceneGraphDlg();

    void OnInitDialog(wxInitDialogEvent& event);
    wxButton    *m_pZoomTo;
    wxCheckBox  *m_pEnabled;
    wxTreeCtrl  *m_pTree;

    osg::Node *m_pSelectedNode;

    void CreateImageList(int size = 16);
    void RefreshTreeContents();
    void AddNodeItemsRecursively(wxTreeItemId hParentItem,
                                 osg::Node *pNode, int depth);

    // WDR: method declarations for SceneGraphDlg
    wxButton* GetZoomto()  { return (wxButton*) FindWindow( ID_ZOOMTO ); }
    wxTreeCtrl* GetScenetree()  { return (wxTreeCtrl*) FindWindow( ID_SCENETREE ); }

private:
    // WDR: member variable declarations for SceneGraphDlg
    wxImageList *m_imageListNormal;

private:
    // WDR: handler declarations for SceneGraphDlg
    void OnRefresh( wxCommandEvent &event );
    void OnZoomTo( wxCommandEvent &event );
    void OnTreeSelChanged( wxTreeEvent &event );

private:
    DECLARE_EVENT_TABLE()
};




#endif
