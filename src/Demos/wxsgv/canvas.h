//
// Name:        canvas.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CANVASH
#define CANVASH

#if !wxUSE_GLCANVAS
#error Please set wxUSE_GLCANVAS to 1 in setup.h.
#endif
#include "wx/glcanvas.h"

#include <osg/Timer>

//
// A Canvas for the main view area.
//
class wxosgGLCanvas: public wxGLCanvas
{
public:
    wxosgGLCanvas(wxWindow *parent, const wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = "wxosgGLCanvas",
      int* gl_attrib = NULL);
    ~wxosgGLCanvas(void);

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void OnClose(wxCloseEvent& event);
    void QueueRefresh(bool eraseBackground);

    bool m_bPainting;
    bool m_bRunning;

    // time since initClock() in seconds.
    float clockSeconds() { return m_timer.delta_s(m_initialTick, m_timer.tick()); }
    osg::Timer        m_timer;
    osg::Timer_t    clockTick() { return m_timer.tick(); }
    osg::Timer_t    m_initialTick;

protected:
    DECLARE_EVENT_TABLE()
};

#endif
