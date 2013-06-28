// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: WindowManager.cpp 66 2008-07-14 21:54:09Z cubicool $

#include <iostream>
#include <algorithm>
#include <osg/io_utils>
#include <osgWidget/Types>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Lua>
#include <osgWidget/Python>
#include <osgWidget/Box>
#include <osgWidget/Label>

namespace osgWidget {

WindowManager::WindowManager(
    osgViewer::View* view,
    point_type       width,
    point_type       height,
    unsigned int     nodeMask,
    unsigned int     flags
):
_width          (width),
_height         (height),
_windowWidth    (width),
_windowHeight   (height),
_flags          (flags),
_nodeMask       (nodeMask),
_view           (view),
_lastX          (0.0f),
_lastY          (0.0f),
_lastEvent      (0),
_lastPush       (0),
_lastVertical   (PD_NONE),
_lastHorizontal (PD_NONE),
_focusMode      (PFM_FOCUS),
_leftDown       (false),
_middleDown     (false),
_rightDown      (false),
_scrolling      (osgGA::GUIEventAdapter::SCROLL_NONE),
_styleManager   (new StyleManager()) {
    _name = generateRandomName("WindowManager");

    if(_flags & WM_USE_LUA) {
        _lua = new LuaEngine(this);

        if(!_lua->initialize()) warn() << "Error creating LuaEngine." << std::endl;
    }

    if(_flags & WM_USE_PYTHON) {
        _python = new PythonEngine(this);

        if(!_python->initialize()) warn() << "Error creating PythonEngine." << std::endl;
    }

    if(_flags & WM_USE_RENDERBINS) getOrCreateStateSet()->setMode(GL_DEPTH_TEST, false);

    // Setup our picking debug (is debug the right word here?) Window...
    if(_flags & WM_PICK_DEBUG) {
        _pickWindow = new Box("PickWindow", Box::VERTICAL);

        Label* label = new Label("PickLabel");

        label->setFontSize(13);
        label->setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
        label->setFont("fonts/VeraMono.ttf");
        label->setPadding(5.0f);
        label->setCanFill(true);

        _pickWindow->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.85f);
        _pickWindow->addWidget(label);
        _pickWindow->setNodeMask(~_nodeMask);
        _pickWindow->removeEventMask(EVENT_MASK_FOCUS);
        _pickWindow->setStrata(Window::STRATA_FOREGROUND);

        addChild(_pickWindow.get());

        _updatePickWindow(0, 0, 0);
    }

    getOrCreateStateSet()->setMode(
        GL_BLEND,
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
    );
    getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
}

WindowManager::WindowManager(const WindowManager& wm, const osg::CopyOp& co):
    osg::Switch(wm, co),
    _width          (wm._width),
    _height         (wm._height),
    _windowWidth    (wm._width),
    _windowHeight   (wm._height),
    _flags          (wm._flags),
    _nodeMask       (wm._nodeMask),
    _view           (wm._view),
    _lastX          (0.0f),
    _lastY          (0.0f),
    _lastEvent      (0),
    _lastPush       (0),
    _lastVertical   (PD_NONE),
    _lastHorizontal (PD_NONE),
    _focusMode      (PFM_FOCUS),
    _leftDown       (false),
    _middleDown     (false),
    _rightDown      (false),
    _scrolling      (osgGA::GUIEventAdapter::SCROLL_NONE),
    _styleManager   (new StyleManager())
{
}

WindowManager::~WindowManager()
{
    if(_flags & WM_USE_LUA) _lua->close();

    if(_flags & WM_USE_PYTHON) _python->close();
}

void WindowManager::setEventFromInterface(Event& ev, EventInterface* ei) {
    Widget* widget = dynamic_cast<Widget*>(ei);
    Window* window = dynamic_cast<Window*>(ei);

    if(widget) {
        ev._window = widget->getParent();
        ev._widget = widget;
    }

    else if(window) ev._window = window;
}

bool WindowManager::_handleMousePushed(float x, float y, bool& down) {
    down = true;

    Event ev(this, EVENT_MOUSE_PUSH);

    WidgetList widgetList;

    if(!pickAtXY(x, y, widgetList)) return false;

    ev.makeMouse(x, y);

    _lastPush = getFirstEventInterface(widgetList, ev);

    if(!_lastPush) return false;

    // TODO: This is the old way; it didn't allow Event handler code to call grabFocus().
    // bool handled = _lastPush->callMethodAndCallbacks(ev);

    if(_focusMode != PFM_SLOPPY) {
        if(ev._window) {
            Window* topmostWindow = ev._window->getTopmostParent();

            setFocused(topmostWindow);

            if(ev._widget) topmostWindow->setFocused(ev._widget);
        }

        // If the user wants to be able to "unfocus" the last Window.
        else if(_focusMode == PFM_UNFOCUS) setFocused(0);
    }

    return _lastPush->callMethodAndCallbacks(ev);
}

bool WindowManager::_handleMouseReleased(float /*x*/, float /*y*/, bool& down) {
    down = false;

    // If were were in a drag state, reset our boolean flag.
    // if(_lastDrag) _lastDrag = 0;

    if(!_lastPush) return false;

    // By design, we can only release an EventInterface we previously pressed.
    // Whether or not we're ON the EventInterface when the release occurs isn't important.
    Event ev(this, EVENT_MOUSE_RELEASE);

    setEventFromInterface(ev, _lastPush);

    bool handled = _lastPush->callMethodAndCallbacks(ev);

    _lastPush = 0;

    return handled;
}

void WindowManager::_getPointerXYDiff(float& x, float& y) {
    x -= _lastX;
    y -= _lastY;
}

void WindowManager::_updatePickWindow(const WidgetList* wl, point_type x, point_type y) {
    Label* label = dynamic_cast<Label*>(_pickWindow->getByName("PickLabel"));

    if(!wl) {
        setValue(0, false);

        return;
    }

    setValue(0, true);

    std::stringstream ss;

    point_type xdiff = x;
    point_type ydiff = y;

    _getPointerXYDiff(xdiff, ydiff);

    ss
        << "At XY Coords: " << x << ", " << y
        << " ( diff " << xdiff << ", " << ydiff << " )"
        << std::endl
    ;

    const Window* parent = wl->back()->getParent();

    ss
        << "Window: " << parent->getName()
        << " ( xyz " << parent->getPosition() << " )"
        << " { zRange " << parent->getZRange() << " }"
        << " < size " << parent->getSize() << " >"
        << " EventMask: " << std::hex << parent->getEventMask()
        << std::endl
    ;

    for(WidgetList::const_iterator i = wl->begin(); i != wl->end(); i++) {
        Widget* widget = i->get();

        ss
            << "   - " << widget->getName()
            << " ( xyz " << widget->getPosition() << " )"
            << " [ XYZ " << widget->getPosition() * parent->getMatrix()
            << " ] < size " << widget->getSize() << " >"
            << " EventMask: " << std::hex << widget->getEventMask()
            << std::endl
        ;
    }

    label->setLabel(ss.str());

    XYCoord size = label->getTextSize();

    _pickWindow->resize(size.x() + 10.0f, size.y() + 10.0f);
    _pickWindow->setOrigin(5.0f, _height - _pickWindow->getHeight() - 5.0f);
    _pickWindow->update();
}

void WindowManager::childInserted(unsigned int i) {
    Window* window = dynamic_cast<Window*>(getChild(i));

    if(!window) return;

    // Update Window's index
    for(Iterator w = begin(); w != end(); w++) {
        if(w->get()->_index >= i) w->get()->_index++;
    }

    _objects.push_back(window);

    window->_index = i;

    setFocused(window);

    window->setNodeMask(_nodeMask);
    window->managed(this);

    for(Window::Iterator w = window->begin(); w != window->end(); w++) if(w->valid()) {
        _styleManager->applyStyles(w->get());
    }

    _styleManager->applyStyles(window);
}

void WindowManager::childRemoved(unsigned int start, unsigned int numChildren) {
    for (unsigned int i = start; i < start+numChildren; i++)
    {
        Window* window = getByIndex(i);

        if(!window) continue;

        if(_remove(window)) {

            window->_index = 0;
            window->unmanaged(this);
        }
    }

    // Update Window's index
    for(Iterator w = begin(); w != end(); w++) {
        if(w->get()->_index >= start) w->get()->_index -= numChildren;
    }

}

// This method performs intersection testing at the given XY coords, and returns true if
// any intersections were found. It will break after processing the first pickable Window
// it finds.
bool WindowManager::pickAtXY(float x, float y, WidgetList& wl)
{
    Intersections intr;


    osg::Camera* camera = _view->getCamera();
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(camera->getGraphicsContext());
    if (gw)
    {
        _view->computeIntersections(camera, osgUtil::Intersector::WINDOW, x, y, intr, _nodeMask);
    }

    if (!intr.empty())
    {
        // Get the first Window at the XY coordinates; if you want a Window to be
        // non-pickable, set the NodeMask to something else.
        Window* activeWin = 0;

        // Iterate over every picked result and create a list of Widgets that belong
        // to that Window.
        for(Intersections::iterator i = intr.begin(); i != intr.end(); i++) {
            Window* win = dynamic_cast<Window*>(i->nodePath.back()->getParent(0));

            // Make sure that our window is valid, and that our pick is within the
            // "visible area" of the Window.
            if(
                !win ||
                (win->getVisibilityMode() == Window::VM_PARTIAL && !win->isPointerXYWithinVisible(x, y))
            ) {
                continue;
            }

            // Set our activeWin, so that we know when we've got all the Widgets
            // that belong to it.
            if(!activeWin) activeWin = win;

            // If we've found a new Widnow, break out!
            else if(activeWin != win) break;

            Widget* widget = dynamic_cast<Widget*>(i->drawable.get());

            if(!widget) continue;

            // We need to return a list of every Widget that was picked, so
            // that the handler can operate on it accordingly.
            else wl.push_back(widget);
        }

        if(wl.size()) {
            // Potentially VERY expensive; only to be used for debugging. :)
            if(_flags & WM_PICK_DEBUG) _updatePickWindow(&wl, x, y);

            return true;
        }
    }

    if(_flags & WM_PICK_DEBUG) _updatePickWindow(0, x, y);

    return false;
}


bool WindowManager::setFocused(Window* window) {
    Event ev(this);

    ev._window = window;

    // Inform the previously focused Window that it is going to be unfocused.
    if(_focused.valid()) _focused->callMethodAndCallbacks(ev.makeType(EVENT_UNFOCUS));

    _focused = window;

    if(!window || !window->canFocus()) return false;

    // Build a vector of every Window that is focusable, in the foreground, and in the
    // background. All these Windows are handled differently.
    Vector focusable;
    Vector bg;
    Vector fg;

    for(ConstIterator it = begin(); it != end(); it++) if(it->valid()) {
        Window* w = it->get();

        if(w->getStrata() == Window::STRATA_FOREGROUND) fg.push_back(w);

        else if(w->getStrata() == Window::STRATA_BACKGROUND) bg.push_back(w);

        else focusable.push_back(w);
    }

    // After this call to sort, the internal objects will be arranged such that the
    // previously focused window is the first, followed by all other Windows in
    // descending order.
    std::sort(focusable.begin(), focusable.end(), WindowZCompare());

    // This is the depth range for each Window. Each Window object must be informed of
    // the Z space allocated to it so that it can properly arrange it's children. We
    // add 2 additional Windows here for anything that should appear in the background
    // and foreground areas.
    matrix_type zRange = 1.0f / (focusable.size() + 2.0f);

    // Our offset for the following for() loop.
    unsigned int i = 3;

    // Handle all of our focusable Windows.
    for(Iterator w = focusable.begin(); w != focusable.end(); w++) {
        Window* win = w->get();

        // Set our newly focused Window as the topmost element.
        if(*w == window) win->_z = -zRange * 2.0f;

        // Set the current Z of the remaining Windows and set their zRange so that
        // they can update their own children.
        else {
            win->_z = -zRange * i;

            i++;
        }
    }

    // Handled our special BACKGROUND Windows.
    for(Iterator w = bg.begin(); w != bg.end(); w++) w->get()->_z = -zRange * i;

    // Handle our special FOREGOUND Windows.
    for(Iterator w = fg.begin(); w != fg.end(); w++) w->get()->_z = -zRange;

    // Update every window, regardless.
    for(Iterator w = begin(); w != end(); w++) {
        Window* win = w->get();

        win->_zRange = zRange;

        win->update();
    }

    _focused->callMethodAndCallbacks(ev.makeType(EVENT_FOCUS));

    return true;
}

void WindowManager::setPointerXY(float x, float y) {
    float xdiff = x;
    float ydiff = y;

    _getPointerXYDiff(xdiff, ydiff);

    // If ydiff isn't NEAR 0 (floating point booleans aren't 100% reliable, but that
    // doesn't matter in our case), assume we have either up or down movement.
    if(ydiff != 0.0f) _lastVertical = ydiff > 0.0f ? PD_UP : PD_DOWN;

    else _lastVertical = PD_NONE;

    // If xdiff isn't 0, assume we have either left or right movement.
    if(xdiff != 0.0f) _lastHorizontal = xdiff > 0.0f ? PD_RIGHT : PD_LEFT;

    else _lastHorizontal = PD_NONE;

    _lastX = x;
    _lastY = y;
}

void WindowManager::setStyleManager(StyleManager* sm) {
    _styleManager = sm;

    for(Iterator i = begin(); i != end(); i++) if(i->valid()) {
        Window* window = i->get();

        for(Window::Iterator w = window->begin(); w != window->end(); w++) {
            if(!w->valid()) continue;

            _styleManager->applyStyles(w->get());
        }

        _styleManager->applyStyles(window);
    }
}

void WindowManager::resizeAllWindows(bool visible) {
    for(Iterator i = begin(); i != end(); i++) if(i->valid()) {
        if(visible && !getValue(i->get()->_index)) continue;

        i->get()->resize();
    }
}

// Returns the application window coordinates of the WindowManager XY position.
XYCoord WindowManager::windowXY(double x, double y) const {
    return XYCoord((_windowWidth / _width) * x, (_windowHeight / _height) * y);
}

// Returns the WindowManager coordinates of the application window XY position.
XYCoord WindowManager::localXY(double x, double y) const {
    return XYCoord((_width / _windowWidth) * x, (_height / _windowHeight) * y);
}

// This is called by a ViewerEventHandler/MouseHandler (or whatever) as the pointer moves
// around and intersects with objects. It also resets our state data (_widget, _leftDown,
// etc.) The return value of this method is mostly useless.
bool WindowManager::pointerMove(float x, float y) {
    WidgetList wl;
    Event      ev(this);

    if(!pickAtXY(x, y, wl)) {
        if(_lastEvent) {
            setEventFromInterface(ev.makeMouse(x, y, EVENT_MOUSE_LEAVE), _lastEvent);

            _lastEvent->callMethodAndCallbacks(ev);
        }

        if(_focusMode == PFM_SLOPPY) setFocused(0);

        _lastEvent  = 0;
        _leftDown   = 0;
        _middleDown = 0;
        _rightDown  = 0;

        return false;
    }

    EventInterface* ei = getFirstEventInterface(wl, ev.makeMouse(x, y, EVENT_MOUSE_OVER));

    if(!ei) return false;

    if(_lastEvent != ei) {
        if(_lastEvent) {
            Event evLeave(this);

            evLeave.makeMouse(x, y, EVENT_MOUSE_LEAVE);

            setEventFromInterface(evLeave, _lastEvent);

            _lastEvent->callMethodAndCallbacks(evLeave);
        }

        _lastEvent = ei;

        if(_focusMode == PFM_SLOPPY && ev._window) setFocused(ev._window);

        _lastEvent->callMethodAndCallbacks(ev.makeMouse(x, y, EVENT_MOUSE_ENTER));
    }

    ei->callMethodAndCallbacks(ev.makeMouse(x, y, EVENT_MOUSE_OVER));

    return true;
}

bool WindowManager::pointerDrag(float x, float y) {
    WidgetList widgetList;
    Event      ev(this);

    float xdiff = x;
    float ydiff = y;

    _getPointerXYDiff(xdiff, ydiff);

    ev.makeMouse(xdiff, ydiff, EVENT_MOUSE_DRAG);

    // If we're still in the drag state...
    if(_lastPush) {
        setEventFromInterface(ev, _lastPush);

        return _lastPush->callMethodAndCallbacks(ev);
    }

    return false;
}

bool WindowManager::mouseScroll(float x, float y) {
    WidgetList wl;

    if(!pickAtXY(x, y, wl)) return false;

    Event ev(this, EVENT_MOUSE_SCROLL);

    EventInterface* ei = getFirstEventInterface(wl, ev);

    if(!ei) return false;

    return ei->callMethodAndCallbacks(ev);
}

// Keypresses only go the focused Window.
bool WindowManager::keyDown(int key, int mask) {
    if(_focused.valid()) {
        Event ev(this, EVENT_KEY_DOWN);

        ev.makeKey(key, mask);

        Widget* focusedWidget = _focused->getFocused();

        ev._window = _focused.get();
        ev._widget = focusedWidget;

        bool handled = false;

        if(focusedWidget) handled = focusedWidget->callMethodAndCallbacks(ev);

        if(!handled) return _focused->callMethodAndCallbacks(ev);

        else return true;
    }

    return false;
}

bool WindowManager::keyUp(int key, int mask) {
    if(_focused.valid()) {
        Event ev(this, EVENT_KEY_UP);

        ev.makeKey(key, mask);

        Widget* focusedWidget = _focused->getFocused();

        ev._window = _focused.get();
        ev._widget = focusedWidget;

        bool handled = false;

        if(focusedWidget) handled = focusedWidget->callMethodAndCallbacks(ev);

        if(!handled) return _focused->callMethodAndCallbacks(ev);

        else return true;
    }

    return false;
}

// A convenience wrapper for creating a proper orthographic camera using the current
// width and height.
osg::Camera* WindowManager::createParentOrthoCamera() {
    osg::Camera* camera = createOrthoCamera(_width, _height);

    camera->addChild(this);

    return camera;
}

}
