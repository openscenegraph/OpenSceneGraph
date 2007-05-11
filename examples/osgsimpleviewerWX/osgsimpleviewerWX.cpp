// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxOsgSample.h"
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <wx/image.h>
#include <wx/menu.h>

// `Main program' equivalent, creating windows and returning main app frame
bool wxOsgApp::OnInit()
{
    // Create the main frame window
	MainFrame *frame = new MainFrame(NULL, wxT("wxWidgets OSG Sample"),
		wxDefaultPosition, wxDefaultSize);

	// create osg canvas
	//	- initialize
	SimpleViewerWX *viewerWindow = new SimpleViewerWX(frame, wxID_ANY, wxDefaultPosition,
        wxSize(200, 200), wxSUNKEN_BORDER);
		
	// load the scene.
	osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile("cow.osg");
	if (!loadedModel)
	{
		return false;
	}
	viewerWindow->setSceneData(loadedModel.get());
	viewerWindow->setCameraManipulator(new osgGA::TrackballManipulator);

	frame->SetSimpleViewer(viewerWindow);

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
	_viewerWindow = NULL;
}

void MainFrame::SetSimpleViewer(SimpleViewerWX *viewer)
{
	_viewerWindow = viewer;
}

void MainFrame::OnIdle(wxIdleEvent &event)
{
	_viewerWindow->render();
	event.RequestMore();
}

BEGIN_EVENT_TABLE(GraphicsWindowWX, wxGLCanvas)
	EVT_SIZE			(GraphicsWindowWX::OnSize			)
	EVT_PAINT			(GraphicsWindowWX::OnPaint			)
	EVT_ERASE_BACKGROUND(GraphicsWindowWX::OnEraseBackground)
	EVT_KEY_DOWN		(GraphicsWindowWX::OnKeyDown		)
	EVT_KEY_UP			(GraphicsWindowWX::OnKeyUp			)
	EVT_MOUSE_EVENTS	(GraphicsWindowWX::OnMouse			)
END_EVENT_TABLE()

GraphicsWindowWX::GraphicsWindowWX(wxWindow *parent, wxWindowID id,
	const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: wxGLCanvas(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name)
{
	// default cursor to standard
	_oldCursor = *wxSTANDARD_CURSOR;
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
}

void GraphicsWindowWX::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
	/* Do nothing, to avoid flashing on MSW */
}

void GraphicsWindowWX::OnKeyDown(wxKeyEvent &event)
{
	int key = event.GetKeyCode();
	getEventQueue()->keyPress(key);

	// propagate event
	event.Skip();
}

void GraphicsWindowWX::OnKeyUp(wxKeyEvent &event)
{
	int key = event.GetKeyCode();
	getEventQueue()->keyRelease(key);	

	// propagate event
	event.Skip();
}

void GraphicsWindowWX::OnMouse(wxMouseEvent& event)
{
	if (event.ButtonDown()) {
		int button = event.GetButton();
		getEventQueue()->mouseButtonPress(event.GetX(), event.GetY(), button);
	}
	else if (event.ButtonUp()) {
		int button = event.GetButton();
		getEventQueue()->mouseButtonRelease(event.GetX(), event.GetY(), button);
	}
	else if (event.Dragging()) {
		getEventQueue()->mouseMotion(event.GetX(), event.GetY());
	}
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
		//	- can't find a way to do this neatly, so create a 1x1, transparent image
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
