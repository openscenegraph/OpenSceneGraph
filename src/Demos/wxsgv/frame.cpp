//
// Name:    frame.cpp
// Purpose:    The frame class for a wxWindows application.
// Author:    Ben Discoe, ben@washedashore.com
//

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "app.h"
#include "frame.h"
#include "canvas.h"

// IDs for the menu commands
enum
{
    ID_FILE_OPEN,
    ID_SCENE_BROWSE
};

DECLARE_APP(wxosgApp)

BEGIN_EVENT_TABLE(wxosgFrame, wxFrame)
    EVT_MENU(wxID_EXIT, wxosgFrame::OnExit)
    EVT_MENU(ID_FILE_OPEN, wxosgFrame::OnOpen)
    EVT_MENU(ID_SCENE_BROWSE, wxosgFrame::OnSceneBrowse)
END_EVENT_TABLE()

// My frame constructor
wxosgFrame::wxosgFrame(wxFrame *parent, const wxString& title, const wxPoint& pos,
    const wxSize& size, long style):
    wxFrame(parent, -1, title, pos, size, style)
{
    // Make a wxosgGLCanvas
  //   FIXME:  Can remove this special case once wxMotif 2.3 is released
#ifdef __WXMOTIF__
    int gl_attrib[20] = { GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1, GLX_DEPTH_SIZE, 1,
            GLX_DOUBLEBUFFER, None };
#else
    int *gl_attrib = NULL;
#endif

    m_canvas = new wxosgGLCanvas(this, -1, wxPoint(0, 0), wxSize(-1, -1), 0,
        "wxosgGLCanvas", gl_attrib);

    // File (project) menu
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(ID_FILE_OPEN, "&Open\tCtrl+O", "Open OSG File");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "&Exit\tEsc", "Exit Viewer");

    // Scene menu
    wxMenu *sceneMenu = new wxMenu;
    sceneMenu->Append(ID_SCENE_BROWSE, "&Browse Scene Graph\tCtrl+G", "Browse Scene Graph");

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&Project");
    menuBar->Append(sceneMenu, "&Scene");
    SetMenuBar(menuBar);

    // Show the frame
    Show(TRUE);

#if 1
    m_pSceneGraphDlg = new SceneGraphDlg(this, -1, "Scene Graph",
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    m_pSceneGraphDlg->SetSize(250, 350);
#endif

    m_canvas->SetCurrent();
}

wxosgFrame::~wxosgFrame()
{
    delete m_canvas;
}


//
// Handle menu commands
//

void wxosgFrame::OnExit(wxCommandEvent& event)
{
    m_canvas->m_bRunning = false;
    Destroy();
}

void wxosgFrame::OnOpen(wxCommandEvent& event)
{
    wxFileDialog loadFile(NULL, "Load Project", "", "",
        "OSG Files (*.osg)|*.osg|", wxOPEN);
    if (loadFile.ShowModal() == wxID_OK)
        wxGetApp().LoadFile(loadFile.GetPath());
}

void wxosgFrame::OnSceneBrowse(wxCommandEvent& event)
{
    m_pSceneGraphDlg->Show(TRUE);
}

