// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: ViewerEventHandlers.cpp 59 2008-05-15 20:55:31Z cubicool $

#include <osgWidget/ViewerEventHandlers>

namespace osgWidget {

MouseHandler::MouseHandler(WindowManager* wm):
_wm(wm) {
}

bool MouseHandler::handle(
	const osgGA::GUIEventAdapter& gea,
	osgGA::GUIActionAdapter&      gaa,
	osg::Object*                  obj,
	osg::NodeVisitor*             nv
) {
	osgGA::GUIEventAdapter::EventType ev = gea.getEventType();
	MouseAction                       ma = _isMouseEvent(ev);

	if(ma) {
		// If we're scrolling, we need to inform the WindowManager of that.
		_wm->setScrollingMotion(gea.getScrollingMotion());

		return (this->*ma)(gea.getX(), gea.getY(), gea.getButton());
	}
	
	return false;
}

bool MouseHandler::_handleMousePush(float x, float y, int button) {
	if(button == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) return _doMouseEvent(
		x,
		y,
		&WindowManager::mousePushedLeft
	);

	else if(button == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) return _doMouseEvent(
		x,
		y,
		&WindowManager::mousePushedRight
	);

	else if(button == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) return _doMouseEvent(
		x,
		y,
		&WindowManager::mousePushedMiddle
	);

	else return false;
}

bool MouseHandler::_handleMouseRelease(float x, float y, int button) {
	if(button == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) return _doMouseEvent(
		x,
		y,
		&WindowManager::mouseReleasedLeft
	);

	else if(button == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) return _doMouseEvent(
		x,
		y,
		&WindowManager::mouseReleasedRight
	);

	else if(button == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) return _doMouseEvent(
		x,
		y,
		&WindowManager::mouseReleasedMiddle
	);

	else return false;
}

bool MouseHandler::_handleMouseDoubleClick(float x, float y, int button) {
	return false;
}

bool MouseHandler::_handleMouseDrag(float x, float y, int button) {
	return _doMouseEvent(x, y, &WindowManager::pointerDrag);
}

bool MouseHandler::_handleMouseMove(float x, float y, int button) {
	return _doMouseEvent(x, y, &WindowManager::pointerMove);
}

bool MouseHandler::_handleMouseScroll(float x, float y, int) {
	return _doMouseEvent(x, y, &WindowManager::mouseScroll);
}

MouseHandler::MouseAction MouseHandler::_isMouseEvent(
	osgGA::GUIEventAdapter::EventType ev
) const {
	if(ev == osgGA::GUIEventAdapter::PUSH) return
		&MouseHandler::_handleMousePush
	;
		
	else if(ev == osgGA::GUIEventAdapter::RELEASE) return
		&MouseHandler::_handleMouseRelease
	;
	
	else if(ev == osgGA::GUIEventAdapter::DOUBLECLICK) return
		&MouseHandler::_handleMouseDoubleClick
	;
	
	else if(ev == osgGA::GUIEventAdapter::DRAG) return
		&MouseHandler::_handleMouseDrag
	;
	
	else if(ev == osgGA::GUIEventAdapter::MOVE) return
		&MouseHandler::_handleMouseMove
	;
	
	else if(ev == osgGA::GUIEventAdapter::SCROLL) return
		&MouseHandler::_handleMouseScroll
	;

	else return 0;
}

bool MouseHandler::_doMouseEvent(float x, float y, MouseEvent me) {
	bool handled = (_wm.get()->*me)(x, y);

	// This is called LAST for things like drag, which needs to calculate a mouse difference.
	_wm->setPointerXY(x, y);

	return handled;
}

KeyboardHandler::KeyboardHandler(WindowManager* wm):
_wm(wm) {
}

bool KeyboardHandler::handle(
	const osgGA::GUIEventAdapter& gea,
	osgGA::GUIActionAdapter&      gaa,
	osg::Object*                  obj,
	osg::NodeVisitor*             nv
) {
	osgGA::GUIEventAdapter::EventType ev = gea.getEventType();

	if(
		ev != osgGA::GUIEventAdapter::KEYDOWN &&
		ev != osgGA::GUIEventAdapter::KEYUP
	) return false;

	int key     = gea.getKey();
	int keyMask = gea.getModKeyMask();

	// -1 is the "key invalid" return code.
	if(key == -1) return false;

	if(ev == osgGA::GUIEventAdapter::KEYDOWN) return _wm->keyDown(key, keyMask);

	else if(ev == osgGA::GUIEventAdapter::KEYUP) return _wm->keyUp(key, keyMask);

	return false;
}

ResizeHandler::ResizeHandler(WindowManager* wm, osg::Camera* camera):
_wm     (wm),
_camera (camera) {
}

bool ResizeHandler::handle(
	const osgGA::GUIEventAdapter& gea,
	osgGA::GUIActionAdapter&      gaa,
	osg::Object*                  obj,
	osg::NodeVisitor*             nv
) {
	osgGA::GUIEventAdapter::EventType ev = gea.getEventType();

	if(ev != osgGA::GUIEventAdapter::RESIZE) return false;

	osg::Matrix::value_type w = gea.getWindowWidth();
	osg::Matrix::value_type h = gea.getWindowHeight();

	if(_wm->isInvertedY()) _camera->setProjectionMatrix(
		createInvertedYOrthoProjectionMatrix(w, h)
	);

	else _camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0f, w, 0.0f, h));
	
	_wm->setSize(w, h);
	_wm->resizeAllWindows();

	return true;
}

}
