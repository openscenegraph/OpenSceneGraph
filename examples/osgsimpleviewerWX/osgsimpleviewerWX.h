#ifndef _WXSIMPLEVIEWERWX_H_
#define _WXSIMPLEVIEWERWX_H_

#include "wx/defs.h"
#include "wx/app.h"
#include "wx/cursor.h"
#include "wx/glcanvas.h"
#include <osgViewer/SimpleViewer>
#include <string>

class GraphicsWindowWX: public wxGLCanvas, virtual osgViewer::GraphicsWindow
{
public:
	GraphicsWindowWX(wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = 0,
		const wxString& name = wxT("TestGLCanvas"));

	~GraphicsWindowWX();

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnKeyDown(wxKeyEvent &event);
	void OnKeyUp(wxKeyEvent &event);
	void OnMouse(wxMouseEvent &event);

	//
	// GraphicsWindow interface
	//

	void grabFocus();
    void grabFocusIfPointerInWindow();
    void useCursor(bool cursorOn);

	bool makeCurrentImplementation();
	void swapBuffersImplementation();

private:
	wxCursor _oldCursor;

	DECLARE_EVENT_TABLE()
};

class SimpleViewerWX : public osgViewer::SimpleViewer, public GraphicsWindowWX
{
public:
    SimpleViewerWX(wxWindow *parent, wxWindowID id = wxID_ANY,
					const wxPoint& pos = wxDefaultPosition,
					const wxSize& size = wxDefaultSize, long style = 0,
					const wxString& name = wxT("TestGLCanvas")) :
		GraphicsWindowWX(parent, id, pos, size, style, name)
	{
	}

	void render()
	{ 
		makeCurrentImplementation();

		// update and render the scene graph
		frame();

		swapBuffersImplementation();
	}
};

class MainFrame : public wxFrame
{
public:
	MainFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, 
		const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

	void SetSimpleViewer(SimpleViewerWX *viewer);
	void OnIdle(wxIdleEvent& event);

private:
	SimpleViewerWX *_viewerWindow;

	DECLARE_EVENT_TABLE()
};

/* Define a new application type */
class wxOsgApp : public wxApp
{
public:
	bool OnInit();
};

#endif // _WXSIMPLEVIEWERWX_H_
