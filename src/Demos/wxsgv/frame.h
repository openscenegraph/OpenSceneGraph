//
// Name:        frame.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FRAMEH
#define FRAMEH

#include "SceneGraphDlg.h"

class wxosgFrame: public wxFrame
{
public:
    wxosgFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
    ~wxosgFrame();

    // command handlers
    void OnExit(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSceneBrowse(wxCommandEvent& event);

public:
    class wxosgGLCanvas    *m_canvas;
    SceneGraphDlg        *m_pSceneGraphDlg;

protected:

DECLARE_EVENT_TABLE()
};

#endif

