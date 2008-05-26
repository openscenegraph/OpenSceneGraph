// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "osgviewerWX.h"


#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <wx/image.h>
#include <wx/menu.h>

#include <iostream>

// `Main program' equivalent, creating windows and returning main app frame
bool wxOsgApp::OnInit()
{
    if (argc<2)
    {
        std::cout << wxString(argv[0]).mb_str() <<": requires filename argument." << std::endl;
        return false;
    }

    int width = 800;
    int height = 600;

    // Create the main frame window

    MainFrame *frame = new MainFrame(NULL, wxT("wxWidgets OSG Sample"),
        wxDefaultPosition, wxSize(width, height));

    // create osg canvas
    //    - initialize

    int *attributes = new int[6];
    attributes[0] = int(WX_GL_DOUBLEBUFFER);
    attributes[1] = WX_GL_RGBA;
    attributes[2] = WX_GL_DEPTH_SIZE;
    attributes[3] = 8;
    attributes[4] = WX_GL_STENCIL_SIZE;
    attributes[5] = 8;

    GraphicsWindowWX* gw = new GraphicsWindowWX(frame, wxID_ANY, wxDefaultPosition,
                                                wxSize(width, height), wxSUNKEN_BORDER, wxT("osgviewerWX"), attributes);

    osgViewer::Viewer *viewer = new osgViewer::Viewer;
    viewer->getCamera()->setGraphicsContext(gw);
    viewer->getCamera()->setViewport(0,0,width,height);
    viewer->addEventHandler(new osgViewer::StatsHandler);
    viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // load the scene.
    wxString fname(argv[1]);
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(std::string(fname.mb_str()));
    if (!loadedModel)
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return false;
    }

    viewer->setSceneData(loadedModel.get());
    viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    frame->SetViewer(viewer);

    /* Show the frame */
    frame->Show(true);

    return true;
}

IMPLEMENT_APP(wxOsgApp)

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_IDLE(MainFrame::OnIdle)
END_EVENT_TABLE()

/* My frame constructor */
MainFrame::MainFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
    const wxSize& size, long style)
    : wxFrame(frame, wxID_ANY, title, pos, size, style)
{
}

void MainFrame::SetViewer(osgViewer::Viewer *viewer)
{
    _viewer = viewer;
}

void MainFrame::OnIdle(wxIdleEvent &event)
{
    _viewer->frame();

    event.RequestMore();
}

BEGIN_EVENT_TABLE(GraphicsWindowWX, wxGLCanvas)
    EVT_SIZE                (GraphicsWindowWX::OnSize)
    EVT_PAINT               (GraphicsWindowWX::OnPaint)
    EVT_ERASE_BACKGROUND    (GraphicsWindowWX::OnEraseBackground)

    EVT_CHAR                (GraphicsWindowWX::OnChar)
    EVT_KEY_UP              (GraphicsWindowWX::OnKeyUp)

    EVT_ENTER_WINDOW    (GraphicsWindowWX::OnMouseEnter)
    EVT_LEFT_DOWN       (GraphicsWindowWX::OnMouseDown)
    EVT_MIDDLE_DOWN     (GraphicsWindowWX::OnMouseDown)
    EVT_RIGHT_DOWN      (GraphicsWindowWX::OnMouseDown)
    EVT_LEFT_UP         (GraphicsWindowWX::OnMouseUp)
    EVT_MIDDLE_UP       (GraphicsWindowWX::OnMouseUp)
    EVT_RIGHT_UP        (GraphicsWindowWX::OnMouseUp)
    EVT_MOTION          (GraphicsWindowWX::OnMouseMotion)
END_EVENT_TABLE()

GraphicsWindowWX::GraphicsWindowWX(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name, int *attributes)
    : wxGLCanvas(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name, attributes)
{
    // default cursor to standard
    _oldCursor = *wxSTANDARD_CURSOR;

    _traits = new GraphicsContext::Traits;
    _traits->x = pos.x;
    _traits->y = pos.y;
    _traits->width = size.x;
    _traits->height = size.y;

    init();

}

void GraphicsWindowWX::init()
{
    if (valid())
    {
        setState( new osg::State );
        getState()->setGraphicsContext(this);

        if (_traits.valid() && _traits->sharedContext)
        {
            getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
            incrementContextIDUsageCount( getState()->getContextID() );
        }
        else
        {
            getState()->setContextID( osg::GraphicsContext::createNewContextID() );
        }
    }
}

GraphicsWindowWX::~GraphicsWindowWX()
{
}

void GraphicsWindowWX::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    /* must always be here */
    wxPaintDC dc(this);
}

void GraphicsWindowWX::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);

    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    int width, height;
    GetClientSize(&width, &height);

    // update the window dimensions, in case the window has been resized.
    getEventQueue()->windowResize(0, 0, width, height);
    resized(0,0,width,height);
}

void GraphicsWindowWX::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
    /* Do nothing, to avoid flashing on MSW */
}

void GraphicsWindowWX::OnChar(wxKeyEvent &event)
{
#if wxUSE_UNICODE
    int key = event.GetUnicodeKey();
#else
    int key = event.GetKeyCode();
#endif
    getEventQueue()->keyPress(key);

    // If this key event is not processed here, we should call
    // event.Skip() to allow processing to continue.
}

void GraphicsWindowWX::OnKeyUp(wxKeyEvent &event)
{
#if wxUSE_UNICODE
    int key = event.GetUnicodeKey();
#else
    int key = event.GetKeyCode();
#endif
    getEventQueue()->keyRelease(key);

    // If this key event is not processed here, we should call
    // event.Skip() to allow processing to continue.
}

void GraphicsWindowWX::OnMouseEnter(wxMouseEvent &event)
{
    // Set focus to ourselves, so keyboard events get directed to us
    SetFocus();
}

void GraphicsWindowWX::OnMouseDown(wxMouseEvent &event)
{
    getEventQueue()->mouseButtonPress(event.GetX(), event.GetY(),
        event.GetButton());
}

void GraphicsWindowWX::OnMouseUp(wxMouseEvent &event)
{
    getEventQueue()->mouseButtonRelease(event.GetX(), event.GetY(),
        event.GetButton());
}

void GraphicsWindowWX::OnMouseMotion(wxMouseEvent &event)
{
    getEventQueue()->mouseMotion(event.GetX(), event.GetY());
}


void GraphicsWindowWX::grabFocus()
{
    // focus this window
    SetFocus();
}

void GraphicsWindowWX::grabFocusIfPointerInWindow()
{
    // focus this window, if the pointer is in the window
    wxPoint pos = wxGetMousePosition();
    if (this == wxFindWindowAtPoint(pos)) {
        SetFocus();
    }
}

void GraphicsWindowWX::useCursor(bool cursorOn)
{
    if (cursorOn) {

        // show the old cursor
        SetCursor(_oldCursor);
    }
    else {

        // remember the old cursor
        _oldCursor = GetCursor();

        // hide the cursor
        //    - can't find a way to do this neatly, so create a 1x1, transparent image
        wxImage image(1,1);
        image.SetMask(true);
        image.SetMaskColour(0, 0, 0);
        wxCursor cursor(image);
        SetCursor(cursor);
    }
}

bool GraphicsWindowWX::makeCurrentImplementation()
{
    SetCurrent();
    return true;
}

void GraphicsWindowWX::swapBuffersImplementation()
{
    SwapBuffers();
}
