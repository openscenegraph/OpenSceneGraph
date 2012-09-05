#include <iostream>

#include "FOX_OSG.h"

// Map
FXDEFMAP(GraphicsWindowFOX) GraphicsWindowFOX_Map[] = {
	//________Message_Type_________			___ID___		________Message_Handler________
	FXMAPFUNC(SEL_CONFIGURE,				0,				GraphicsWindowFOX::onConfigure),
	FXMAPFUNC(SEL_KEYPRESS,					0,				GraphicsWindowFOX::onKeyPress),
	FXMAPFUNC(SEL_KEYRELEASE,				0,				GraphicsWindowFOX::onKeyRelease),
	FXMAPFUNC(SEL_LEFTBUTTONPRESS,			0,				GraphicsWindowFOX::onLeftBtnPress),
	FXMAPFUNC(SEL_LEFTBUTTONRELEASE,		0,				GraphicsWindowFOX::onLeftBtnRelease),
	FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,		0,				GraphicsWindowFOX::onMiddleBtnPress),
	FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,		0,				GraphicsWindowFOX::onMiddleBtnRelease),
	FXMAPFUNC(SEL_RIGHTBUTTONPRESS,			0,				GraphicsWindowFOX::onRightBtnPress),
	FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,		0,				GraphicsWindowFOX::onRightBtnRelease),
	FXMAPFUNC(SEL_MOTION,					0,				GraphicsWindowFOX::onMotion)
};

FXIMPLEMENT(GraphicsWindowFOX, FXGLCanvas, GraphicsWindowFOX_Map, ARRAYNUMBER(GraphicsWindowFOX_Map))

GraphicsWindowFOX::GraphicsWindowFOX(FXComposite *parent, FXGLVisual *vis,
									 FXObject *tgt, FXSelector sel,
									 FXuint opts, FXint x, FXint y,
									 FXint w, FXint h)
									 : FXGLCanvas(parent, vis, tgt, sel, opts, x, y, w, h)
{
	// default cursor to standard
	_oldCursor = new FXCursor(parent->getApp(),CURSOR_CROSS);

	_traits = new GraphicsContext::Traits;
	_traits->x = x;
	_traits->y = y;
	_traits->width = w;
	_traits->height = h;
	_traits->windowDecoration = false;
	_traits->doubleBuffer = true;
	_traits->sharedContext = 0;

	init();

}

void GraphicsWindowFOX::init()
{
	if (valid())
	{
		setState( new osg::State );
		getState()->setGraphicsContext(this);

		if (_traits.valid() && _traits->sharedContext.valid())
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

GraphicsWindowFOX::~GraphicsWindowFOX()
{
}

void GraphicsWindowFOX::grabFocus()
{
	// focus this window
	setFocus();
}

void GraphicsWindowFOX::grabFocusIfPointerInWindow()
{
	// do nothing
}

void GraphicsWindowFOX::useCursor(bool cursorOn)
{
	if (cursorOn) {
		// show the old cursor
		setDefaultCursor(_oldCursor);
	}
	else {
		setDefaultCursor(NULL);
	}
}

bool GraphicsWindowFOX::makeCurrentImplementation()
{
	FXGLCanvas::makeCurrent();
	return true;
}

bool GraphicsWindowFOX::releaseContext()
{
	FXGLCanvas::makeNonCurrent();
	return true;
}

void GraphicsWindowFOX::swapBuffersImplementation()
{
	FXGLCanvas::swapBuffers();
}


long GraphicsWindowFOX::onConfigure(FXObject *sender, FXSelector sel, void* ptr)
{
	// set GL viewport (not called by     FXGLCanvas::onConfigure on all platforms...) 
	// update the window dimensions, in case the window has been resized.
	getEventQueue()->windowResize(0, 0, getWidth(), getHeight());
	resized(0, 0, getWidth(), getHeight());
	
	return FXGLCanvas::onConfigure(sender, sel, ptr);
}

long GraphicsWindowFOX::onKeyPress(FXObject *sender, FXSelector sel, void* ptr)
{
	int key = ((FXEvent*)ptr)->code;
	getEventQueue()->keyPress(key);

	return FXGLCanvas::onKeyPress(sender, sel, ptr);
}

long GraphicsWindowFOX::onKeyRelease(FXObject *sender, FXSelector sel, void* ptr)
{
	int key = ((FXEvent*)ptr)->code;
	getEventQueue()->keyRelease(key);

	return FXGLCanvas::onKeyRelease(sender, sel, ptr);
}

long GraphicsWindowFOX::onLeftBtnPress(FXObject *sender, FXSelector sel, void* ptr)
{
	handle(this,FXSEL(SEL_FOCUS_SELF,0),ptr);

	FXEvent* event=(FXEvent*)ptr;
	getEventQueue()->mouseButtonPress(event->click_x, event->click_y, 1);

	return FXGLCanvas::onLeftBtnPress(sender, sel, ptr);
}

long GraphicsWindowFOX::onLeftBtnRelease(FXObject *sender, FXSelector sel, void* ptr)
{
	FXEvent* event=(FXEvent*)ptr;
	getEventQueue()->mouseButtonRelease(event->click_x, event->click_y, 1);

	return FXGLCanvas::onLeftBtnRelease(sender, sel, ptr);
}

long GraphicsWindowFOX::onMiddleBtnPress(FXObject *sender, FXSelector sel, void* ptr)
{
	handle(this,FXSEL(SEL_FOCUS_SELF,0),ptr);
	
	FXEvent* event=(FXEvent*)ptr;
	getEventQueue()->mouseButtonPress(event->click_x, event->click_y, 2);

	return FXGLCanvas::onMiddleBtnPress(sender, sel, ptr);
}

long GraphicsWindowFOX::onMiddleBtnRelease(FXObject *sender, FXSelector sel, void* ptr)
{
	FXEvent* event=(FXEvent*)ptr;
	getEventQueue()->mouseButtonRelease(event->click_x, event->click_y, 2);

	return FXGLCanvas::onMiddleBtnRelease(sender, sel, ptr);
}

long GraphicsWindowFOX::onRightBtnPress(FXObject *sender, FXSelector sel, void* ptr)
{
	handle(this,FXSEL(SEL_FOCUS_SELF,0),ptr);
	
	FXEvent* event=(FXEvent*)ptr;
	getEventQueue()->mouseButtonPress(event->click_x, event->click_y, 3);

	return FXGLCanvas::onRightBtnPress(sender, sel, ptr);
}

long GraphicsWindowFOX::onRightBtnRelease(FXObject *sender, FXSelector sel, void* ptr)
{
	FXEvent* event=(FXEvent*)ptr;
	getEventQueue()->mouseButtonRelease(event->click_x, event->click_y, 3);

	return FXGLCanvas::onRightBtnRelease(sender, sel, ptr);
}

long GraphicsWindowFOX::onMotion(FXObject *sender, FXSelector sel, void* ptr)
{
	FXEvent* event=(FXEvent*)ptr;
	getEventQueue()->mouseMotion(event->win_x, event->win_y);

	return FXGLCanvas::onMotion(sender, sel, ptr);
}
