// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

#include <osgWidget/ViewerEventHandlers>

namespace osgWidget {

MouseHandler::MouseHandler(WindowManager* wm):
_wm(wm) {
}

bool MouseHandler::handle(
    const osgGA::GUIEventAdapter& gea,
    osgGA::GUIActionAdapter&      /*gaa*/,
    osg::Object*                  /*obj*/,
    osg::NodeVisitor*             /*nv*/
) {
    osgGA::GUIEventAdapter::EventType ev = gea.getEventType();
    MouseAction                       ma = _isMouseEvent(ev);

    if(ma) {
        // If we're scrolling, we need to inform the WindowManager of that.
        _wm->setScrollingMotion(gea.getScrollingMotion());

        // osgWidget assumes origin is bottom left of window so make sure mouse coordinate are increaseing y upwards and are scaled to window size.
        float x = (gea.getX()-gea.getXmin())/(gea.getXmax()-gea.getXmin())*static_cast<float>(gea.getWindowWidth());
        float y = (gea.getY()-gea.getYmin())/(gea.getYmax()-gea.getYmin())*static_cast<float>(gea.getWindowHeight());
        if (gea.getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS) y = static_cast<float>(gea.getWindowHeight())-y;

        //OSG_NOTICE<<"MouseHandler(x="<<x<<", y="<<y<<")"<<std::endl;

        return (this->*ma)(x, y, gea.getButton());
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

bool MouseHandler::_handleMouseDoubleClick(float /*x*/, float /*y*/, int /*button*/) {
    return false;
}

bool MouseHandler::_handleMouseDrag(float x, float y, int /*button*/) {
    return _doMouseEvent(x, y, &WindowManager::pointerDrag);
}

bool MouseHandler::_handleMouseMove(float x, float y, int /*button*/) {
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
    osgGA::GUIActionAdapter&      /*gaa*/,
    osg::Object*                  /*obj*/,
    osg::NodeVisitor*             /*nv*/
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
    osgGA::GUIActionAdapter&      /*gaa*/,
    osg::Object*                  /*obj*/,
    osg::NodeVisitor*             /*nv*/
) {
    osgGA::GUIEventAdapter::EventType ev = gea.getEventType();

    if(ev != osgGA::GUIEventAdapter::RESIZE) return false;

    osg::Matrix::value_type w = gea.getWindowWidth();
    osg::Matrix::value_type h = gea.getWindowHeight();

    if(_camera.valid()) {
        _camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0f, w, 0.0f, h));

        _wm->setSize(w, h);
    }

    _wm->setWindowSize(w, h);
    _wm->resizeAllWindows();

    return true;
}

CameraSwitchHandler::CameraSwitchHandler(WindowManager* wm, osg::Camera* camera):
_wm     (wm),
_camera (camera) {
}

bool CameraSwitchHandler::handle(
    const osgGA::GUIEventAdapter& gea,
    osgGA::GUIActionAdapter&      gaa,
    osg::Object*                  /*obj*/,
    osg::NodeVisitor*             /*nv*/
) {
    if(
        gea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN ||
        gea.getKey()       != osgGA::GUIEventAdapter::KEY_F12
    ) return false;

    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&gaa);

    if(!view) return false;

    osg::Node* oldNode = view->getSceneData();

    osg::MatrixTransform* oldTrans = dynamic_cast<osg::MatrixTransform*>(oldNode);

    if(!oldTrans) {
        // Imagine this is the number of pixels...
        double scale  = 2000.0f;
        double width  = _wm->getWidth();
        double height = _wm->getHeight();

        _oldNode = oldNode;

        osg::MatrixTransform* mt = new osg::MatrixTransform();

        mt->setMatrix(
            osg::Matrix::translate(width / 2.0f, 0.0f, 0.0f) *
            osg::Matrix::scale(1.0f, 1.0f, scale) *
            osg::Matrix::rotate(osg::DegreesToRadians(45.0f), 0.0f, 1.0f, 0.0f)
        );

        mt->addChild(_wm.get());
        mt->getOrCreateStateSet()->setMode(
            GL_LIGHTING,
            osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF
        );
        mt->getOrCreateStateSet()->setMode(
            GL_SCISSOR_TEST,
            osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF
        );

        osgGA::CameraManipulator* mm = view->getCameraManipulator();

        // mm->setDistance(3000.0f);
        // mm->setMinimumZoomScale(10.0f);
        mm->setHomePosition(
            // eye
            osg::Vec3(width / 2.0f, height, 100.0f),
            // center
            osg::Vec3(0.0f, 0.0f, -(scale / 2.0f)),
            // up
            osg::Vec3(0.0f, 1.0f, 0.0f)
        );

        view->setSceneData(mt);
    }

    else view->setSceneData(_oldNode.get());

    return true;
}

}
