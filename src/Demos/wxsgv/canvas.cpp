//
// Name:    canvas.cpp
// Purpose:    Implements the canvas class for a wxWindows application.
// Author:    Ben Discoe, ben@washedashore.com
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <osgWX/WXEventAdapter>
#include <osgUtil/CameraManipulator>

using namespace osg;
using namespace osgWX;

#include "canvas.h"
#include "frame.h"
#include "app.h"

DECLARE_APP(wxosgApp)

/*
 * wxosgGLCanvas implementation
 */
BEGIN_EVENT_TABLE(wxosgGLCanvas, wxGLCanvas)
    EVT_CLOSE(wxosgGLCanvas::OnClose)
    EVT_SIZE(wxosgGLCanvas::OnSize)
    EVT_PAINT(wxosgGLCanvas::OnPaint)
    EVT_CHAR(wxosgGLCanvas::OnChar)
    EVT_MOUSE_EVENTS(wxosgGLCanvas::OnMouseEvent)
    EVT_ERASE_BACKGROUND(wxosgGLCanvas::OnEraseBackground)
END_EVENT_TABLE()

wxosgGLCanvas::wxosgGLCanvas(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name, int* gl_attrib):
  wxGLCanvas(parent, id, pos, size, style, name, gl_attrib)
{
    parent->Show(TRUE);
    SetCurrent();

    m_bPainting = false;
    m_bRunning = true;
    QueueRefresh(FALSE);

    m_initialTick = m_timer.tick();
}


wxosgGLCanvas::~wxosgGLCanvas(void)
{
}

void wxosgGLCanvas::QueueRefresh(bool eraseBackground)
    // A Refresh routine we can call from inside OnPaint.
    //   (queues the events rather than dispatching them immediately).
{
    // With wxGTK, you can't do a Refresh() in OnPaint because it doesn't
    //   queue (post) a Refresh event for later.  Rather it dispatches
    //   (processes) the underlying events immediately via ProcessEvent
    //   (read, recursive call).  See the wxPostEvent docs and Refresh code
    //   for more details.
    if ( eraseBackground )
    {
        wxEraseEvent eevent( GetId() );
        eevent.SetEventObject( this );
        wxPostEvent( GetEventHandler(), eevent );
    }

    wxPaintEvent event( GetId() );
    event.SetEventObject( this );
    wxPostEvent( GetEventHandler(), event );
}


void wxosgGLCanvas::OnPaint( wxPaintEvent& event )
{
    // place the dc inside a scope, to delete it before the end of function
    if (1)
    {
        // This is a dummy, to avoid an endless succession of paint messages.
        // OnPaint handlers must always create a wxPaintDC.
        wxPaintDC dc(this);
#ifdef __WXMSW__
        if (!GetContext()) return;
#endif

        if (m_bPainting || !m_bRunning) return;

        m_bPainting = true;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update the camera manipulator.
        if (wxGetApp().Manip())
        {
            ref_ptr<WXEventAdapter> ea = new WXEventAdapter;
            ea->adaptFrame(clockSeconds());
            wxGetApp().Manip()->handle(*ea, wxGetApp());
        }

        // Render the OSG scene
        wxGetApp().DoUpdate();

        SwapBuffers();

#ifdef WIN32
        // Call Refresh again for continuous rendering,
        if (m_bRunning)
            Refresh(FALSE);
#else
        // Queue another refresh for continuous rendering.
        //   (Yield first so we don't starve out keyboard & mouse events.)
        //
        // FIXME: We may want to use a frame timer instead of immediate-
        //   redraw so we don't eat so much CPU on machines that can
        //   easily handle the frame rate.
        wxYield();
        QueueRefresh(FALSE);
#endif

        m_bPainting = false;
    }

    // Must allow some idle processing to occur - or the toolbars will not
    // update, and the close box will not respond!
    wxGetApp().ProcessIdle();
}

static void Reshape(int width, int height)
{
    glViewport(0, 0, (GLint)width, (GLint)height);
}

void wxosgGLCanvas::OnClose(wxCloseEvent& event)
{
    m_bRunning = false;
}

void wxosgGLCanvas::OnSize(wxSizeEvent& event)
{
  // Presumably this is a wxMSWism.
  // For wxGTK & wxMotif, all canvas resize events occur before the context
  //   is set.  So ignore this context check and grab the window width/height
  //   when we get it so it (and derived values such as aspect ratio and
  //   viewport parms) are computed correctly.
#ifdef __WXMSW__
    if (!GetContext()) return;
#endif

    SetCurrent();
    int width, height;
    GetClientSize(& width, & height);
    Reshape(width, height);

    if (wxGetApp().Manip())
    {
        ref_ptr<WXEventAdapter> ea = new WXEventAdapter;
        ea->adaptResize(clockSeconds(), 0, 0, width, height);
        wxGetApp().Manip()->handle(*ea, wxGetApp());
    }

    wxGetApp().SetWindowSize(width, height);
}

void wxosgGLCanvas::OnChar(wxKeyEvent& event)
{
    ref_ptr<WXEventAdapter> ea = new WXEventAdapter;
    ea->adaptKeyboard(clockSeconds(), event.KeyCode(), event.GetX(), event.GetY());
    wxGetApp().Manip()->handle(*ea, wxGetApp());
}

void wxosgGLCanvas::OnMouseEvent(wxMouseEvent& event)
{
    // turn WX mouse event into a OSG mouse event
    ref_ptr<WXEventAdapter> ea = new WXEventAdapter;
    ea->adaptMouse(clockSeconds(), &event);
    wxGetApp().Manip()->handle(*ea, wxGetApp());
}

void wxosgGLCanvas::OnEraseBackground(wxEraseEvent& event)
{
    // Do nothing, to avoid flashing.
}

