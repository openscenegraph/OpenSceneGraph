//
// Name:    SceneGraphDlg.cpp
// Author:    Ben Discoe, ben@washedashore.com
//

#ifdef __GNUG__
    #pragma implementation "SceneGraphDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#  include "wx/wx.h"
#endif
#include "wx/treectrl.h"
#include <typeinfo>

#include <osg/Group>
#include <osg/LightSource>
#include <osg/LOD>
#include <osg/Geode>
#include <osg/Transform>
#include <osg/GeoSet>
#include <osg/ImpostorSprite>

using namespace osg;

#include "app.h"
DECLARE_APP(wxosgApp)

//#include <string>

#include "SceneGraphDlg.h"

#if defined(__WXGTK__) || defined(__WXMOTIF__)
#  include "wxsgv.xpm"
#  include "icon1.xpm"
#  include "icon2.xpm"
#  include "icon3.xpm"
#  include "icon4.xpm"
#  include "icon5.xpm"
#  include "icon6.xpm"
#  include "icon7.xpm"
#  include "icon8.xpm"
#  include "icon9.xpm"
#  include "icon10.xpm"
#endif

/////////////////////////////

class MyTreeItemData : public wxTreeItemData
{
public:
    MyTreeItemData(Node *pNode)
    {
        m_pNode = pNode;
    }
    Node *m_pNode;
};


// WDR: class implementations

//----------------------------------------------------------------------------
// SceneGraphDlg
//----------------------------------------------------------------------------

// WDR: event table for SceneGraphDlg

BEGIN_EVENT_TABLE(SceneGraphDlg,wxDialog)
    EVT_INIT_DIALOG (SceneGraphDlg::OnInitDialog)
    EVT_TREE_SEL_CHANGED( ID_SCENETREE, SceneGraphDlg::OnTreeSelChanged )
    EVT_BUTTON( ID_ZOOMTO, SceneGraphDlg::OnZoomTo )
    EVT_BUTTON( ID_REFRESH, SceneGraphDlg::OnRefresh )
END_EVENT_TABLE()

SceneGraphDlg::SceneGraphDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    SceneGraphFunc( this, TRUE ); 

    m_pZoomTo = GetZoomto();
    m_pTree = GetScenetree();

    m_pZoomTo->Enable(false);

    m_imageListNormal = NULL;
    CreateImageList(16);
}

SceneGraphDlg::~SceneGraphDlg()
{
}

///////////

void SceneGraphDlg::CreateImageList(int size)
{
    delete m_imageListNormal;

    if ( size == -1 )
    {
        m_imageListNormal = NULL;
        return;
    }
    // Make an image list containing small icons
    m_imageListNormal = new wxImageList(size, size, TRUE);

    // should correspond to TreeCtrlIcon_xxx enum
    wxIcon icons[10];
    icons[0] = wxICON(icon1);
    icons[1] = wxICON(icon2);
    icons[2] = wxICON(icon3);
    icons[3] = wxICON(icon4);
    icons[4] = wxICON(icon5);
    icons[5] = wxICON(icon6);
    icons[6] = wxICON(icon7);
    icons[7] = wxICON(icon8);
    icons[8] = wxICON(icon9);
    icons[9] = wxICON(icon10);

    int sizeOrig = icons[0].GetWidth();
    for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
    {
        if ( size == sizeOrig )
            m_imageListNormal->Add(icons[i]);
        else
            m_imageListNormal->Add(wxImage(icons[i]).Rescale(size, size).
                                    ConvertToBitmap());
    }
    m_pTree->SetImageList(m_imageListNormal);
}


void SceneGraphDlg::RefreshTreeContents()
{
    // start with a blank slate
    m_pTree->DeleteAllItems();

    Node *pRoot = wxGetApp().Root();
    if (!pRoot)
        return;

    // Fill in the tree with nodes
    wxTreeItemId hRootItem = m_pTree->AddRoot("Root");
    AddNodeItemsRecursively(hRootItem, pRoot, 0);
    m_pTree->Expand(hRootItem);

    m_pSelectedNode = NULL;
}


void SceneGraphDlg::AddNodeItemsRecursively(wxTreeItemId hParentItem,
                                            Node *pNode, int depth)
{
    wxString str;
    int nImage;
    wxTreeItemId hNewItem;

    if (!pNode) return;

    else if (dynamic_cast<LightSource*>(pNode))
    {
        str = "Light";
        nImage = 4;
    }
    else if (dynamic_cast<Geode*>(pNode))
    {
        str = "Geode";
        nImage = 2;
    }
    else if (dynamic_cast<LOD*>(pNode))
    {
        str = "LOD";
        nImage = 5;
    }
    else if (dynamic_cast<Transform*>(pNode))
    {
        str = "XForm";
        nImage = 9;
    }
    else if (dynamic_cast<Group*>(pNode))
    {
        // must be just a group for grouping's sake
        str = "Group";
        nImage = 3;
    }
    else
    {
        // must be something else
        str = "Other";
        nImage = 8;
    }
    std::string name = pNode->getName();
    if (!name.empty())
    {
        const char *name2 = name.c_str();
        str += " \"";
        str += name2;
        str += "\"";
    }

    hNewItem = m_pTree->AppendItem(hParentItem, str, nImage, nImage);

    Geode *pGeode = dynamic_cast<Geode*>(pNode);
    if (pGeode)
    {
        int num_mesh = pGeode->getNumDrawables();
        wxTreeItemId    hDItem;

        for (int i = 0; i < num_mesh; i++)
        {
            Drawable *pDraw = pGeode->getDrawable(i);
            GeoSet *pGeoSet = dynamic_cast<GeoSet*>(pDraw);
            if (pGeoSet)
            {
                int iNumPrim = pGeoSet->getNumPrims();

                GeoSet::PrimitiveType pt = pGeoSet->getPrimType();
                const char *mtype;
                switch (pt)
                {
                case (GeoSet::POINTS) : mtype = "Points"; break;
                case (GeoSet::LINES) : mtype = "Lines"; break;
                case (GeoSet::LINE_LOOP) : mtype = "LineLoop"; break;
                case (GeoSet::LINE_STRIP): mtype  = "LineStrip"; break;
                case (GeoSet::FLAT_LINE_STRIP) : mtype = "FlatLineStrip"; break;
                case (GeoSet::TRIANGLES) : mtype  = "Triangles"; break;
                case (GeoSet::TRIANGLE_STRIP) : mtype = "TriStrip"; break;
                case (GeoSet::FLAT_TRIANGLE_STRIP) : mtype = "FlatTriStrip"; break;
                case (GeoSet::TRIANGLE_FAN) : mtype = "TriFan"; break;
                case (GeoSet::FLAT_TRIANGLE_FAN) : mtype = "FlatTriFan"; break;
                case (GeoSet::QUADS) : mtype  = "Quads"; break;
                case (GeoSet::QUAD_STRIP) : mtype = "QuadStrip"; break;
                case (GeoSet::POLYGON) : mtype  = "Polygon"; break;
                }
                str.Printf("GeoSet %d, %s, %d prims", i, mtype, iNumPrim);
                hDItem = m_pTree->AppendItem(hNewItem, str, 6, 6);
            }
            ImpostorSprite *pImpostorSprite = dynamic_cast<ImpostorSprite*>(pDraw);
            if (pImpostorSprite)
            {
                str.Printf("ImposterSprite");
                hDItem = m_pTree->AppendItem(hNewItem, str, 9, 9);
            }
        }
    }

    m_pTree->SetItemData(hNewItem, new MyTreeItemData(pNode));

    Group *pGroup = dynamic_cast<Group*>(pNode);
    if (pGroup)
    {
        int num_children = pGroup->getNumChildren();
        if (num_children > 200)
        {
            wxTreeItemId    hSubItem;
            str.Format("(%d children)", num_children);
            hSubItem = m_pTree->AppendItem(hNewItem, str, 8, 8);
        }
        else
        {
            for (int i = 0; i < num_children; i++)
            {
                Node *pChild = pGroup->getChild(i);
                if (!pChild) continue;

                AddNodeItemsRecursively(hNewItem, pChild, depth+1);
            }
        }
    }
    // expand a bit so that the tree is initially partially exposed
    if (depth < 2)
        m_pTree->Expand(hNewItem);
}


// WDR: handler implementations for SceneGraphDlg

void SceneGraphDlg::OnRefresh( wxCommandEvent &event )
{
    RefreshTreeContents();
}

void SceneGraphDlg::OnZoomTo( wxCommandEvent &event )
{
    if (m_pSelectedNode)
        wxGetApp().ZoomTo(m_pSelectedNode);
}

void SceneGraphDlg::OnTreeSelChanged( wxTreeEvent &event )
{
    wxTreeItemId item = event.GetItem();
    MyTreeItemData *data = (MyTreeItemData *)m_pTree->GetItemData(item);

    m_pSelectedNode = NULL;

    if (data && data->m_pNode)
    {
        m_pSelectedNode = data->m_pNode;
        m_pZoomTo->Enable(true);
    }
    else
        m_pZoomTo->Enable(false);
}

void SceneGraphDlg::OnInitDialog(wxInitDialogEvent& event) 
{
    RefreshTreeContents();

    wxWindow::OnInitDialog(event);
}

