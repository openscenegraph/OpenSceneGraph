#ifndef _WXSIMPLEVIEWERWX_H_
#define _WXSIMPLEVIEWERWX_H_

#include "wx/defs.h"
#include "wx/app.h"
#include "wx/cursor.h"
#include "wx/glcanvas.h"
#include <osgViewer/Viewer>
#include <string>

class GraphicsWindowWX : public wxGLCanvas, public osgViewer::GraphicsWindow
{
public:
    GraphicsWindowWX(wxWindow *parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxString& name = wxT("TestGLCanvas"),
        int *attributes = 0);

    ~GraphicsWindowWX();

    void init();

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);

    void OnChar(wxKeyEvent &event);
    void OnKeyUp(wxKeyEvent &event);

    void OnMouseEnter(wxMouseEvent &event);
    void OnMouseDown(wxMouseEvent &event);
    void OnMouseUp(wxMouseEvent &event);
    void OnMouseMotion(wxMouseEvent &event);


    //
    // GraphicsWindow interface
    //
    void grabFocus();
    void grabFocusIfPointerInWindow();
    void useCursor(bool cursorOn);

    bool makeCurrentImplementation();
    void swapBuffersImplementation();

    // not implemented yet...just use dummy implementation to get working.
    virtual bool valid() const { return true; }
    virtual bool realizeImplementation() { return true; }
    virtual bool isRealizedImplementation() const  { return true; }
    virtual void closeImplementation() {}
    virtual bool releaseContextImplementation() { return true; }

private:
    wxCursor _oldCursor;

    DECLARE_EVENT_TABLE()
};

class MainFrame : public wxFrame
{
public:
    MainFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

    void SetViewer(osgViewer::Viewer *viewer);
    void OnIdle(wxIdleEvent& event);

private:
    osg::ref_ptr<osgViewer::Viewer> _viewer;

    DECLARE_EVENT_TABLE()
};

/* Define a new application type */
class wxOsgApp : public wxApp
{
public:
    bool OnInit();
};

#endif // _WXSIMPLEVIEWERWX_H_
