// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: Window.cpp 66 2008-07-14 21:54:09Z cubicool $

#include <algorithm>
#include <osgGA/GUIEventAdapter>
#include <osgWidget/WindowManager>

namespace osgWidget {

bool callbackWindowMove(Event& ev) {
    if(!ev.getWindow() || !ev.getWindowManager()->isLeftMouseButtonDown()) return false;

    ev.getWindow()->addOrigin(ev.x, ev.y);
    ev.getWindow()->update();

    return true;
}

bool callbackWindowRotate(Event& ev) {
    if(!ev.getWindow() || !ev.getWindowManager()->isRightMouseButtonDown()) return false;

    ev.getWindow()->addRotate(ev.y);
    ev.getWindow()->update();

    return true;
}

bool callbackWindowScale(Event& ev) {
    if(!ev.getWindow() || !ev.getWindowManager()->isMiddleMouseButtonDown()) return false;

    ev.getWindow()->addScale(ev.y);
    ev.getWindow()->update();

    return true;
}

bool callbackWindowTabFocus(Event& ev) {
    if(!ev.getWindow() || ev.key != osgGA::GUIEventAdapter::KEY_Tab) return false;

    return ev.getWindow()->setNextFocusable();
}

Window::EmbeddedWindow::EmbeddedWindow(const std::string& name, point_type w, point_type h):
Widget(name, w, h) {
}

Window::EmbeddedWindow::EmbeddedWindow(const EmbeddedWindow& wiw, const osg::CopyOp& co):
Widget(wiw, co) {
    // TODO: Get this!
    // _window = 0;
}

void Window::EmbeddedWindow::parented(Window* parent) {
    if(!_window.valid()) return;

    if(!_window->_parent) {
        _window->_parent = parent;

        // Add this Window to the Window, on the same level as a Window's
        // internal Geode. This will require special handling of events!
        parent->addChild(_window.get());
    }

    else warn()
        << "EmbeddedWindow Widget [" << _name
        << "] cannot embed itself in Window [" << _window->getName()
        << "], since it is already a child of [" << _window->_parent->getName()
        << "]" << std::endl
    ;
}

void Window::EmbeddedWindow::unparented(Window*) {
    if(_window.valid()) {
        _window->_parent = 0;

        if(_parent) _parent->removeChild(_window.get());
    }
}

void Window::EmbeddedWindow::managed(WindowManager* wm) {
    if(!_window.valid()) return;

    _window->setNodeMask(wm->getNodeMask());
    _window->managed(wm);
}

void Window::EmbeddedWindow::unmanaged(WindowManager* wm) {
    _window->unmanaged(wm);
}

void Window::EmbeddedWindow::positioned() {
    if(!_window.valid()) return;

    point_type x = getX();
    point_type y = getY();
    point_type w = getWidth();
    point_type h = getHeight();

    // If the widget is fillable, ask the internal Window to resize itself.
    // Whether or not the Window honors this reqest will be up to it.
    _window->setOrigin(x, y);
    _window->setZ(_calculateZ(getLayer() + 1));
    _window->setZRange(_calculateZ(LAYER_TOP - (getLayer() + 1)));
    _window->setVisibleArea(0, 0, static_cast<int>(w), static_cast<int>(h));
    _window->resize(w, h);
}

bool Window::EmbeddedWindow::setWindow(Window* win) {
    if(!win) {
        warn()
            << "EmbeddedWindow [" << _name
            << "] attempted to set a NULL Window." << std::endl
        ;

        return false;
    }

    if (_window.valid() && _parent)
        unparented(_parent);

    _window = win;

    _window->resize();
    _window->setVisibilityMode(VM_PARTIAL);

    if(_parent) parented(_parent);

    WindowManager* wm = _getWindowManager();

    if(wm) managed(wm);

    return true;
}

void Window::EmbeddedWindow::updateSizeFromWindow() {
    setSize(_window->getSize());

    if(_parent) _parent->resize();
}

Window::Window(const std::string& name):
    _parent     (0),
    _wm         (0),
    _index      (0),
    _x          (0.0f),
    _y          (0.0f),
    _z          (0.0f),
    _zRange     (0.0f),
    _strata     (STRATA_NONE),
    _vis        (VM_FULL),
    _r          (0.0f),
    _s          (1.0f),
    _scaleDenom (100.0f),
    _vAnchor    (VA_NONE),
    _hAnchor    (HA_NONE)
{
    _name = name.size() ? name : generateRandomName("Window");

    // TODO: Fix the "bg" name.
    osg::Geode* geode = new osg::Geode();
    Widget*     bg    = new Widget(name + "bg", 0.0f, 0.0f);

    bg->setLayer(Widget::LAYER_BG);
    bg->setColor(1.0f, 1.0f, 1.0f, 1.0f);

    _setParented(bg);

    geode->addDrawable(bg);

    addChild(geode);
    setDataVariance(osg::Object::DYNAMIC);
    setEventMask(EVENT_ALL);

    getOrCreateStateSet()->setAttributeAndModes(
        new osg::Scissor(0, 0, 0, 0),
        osg::StateAttribute::ON
    );
}

Window::Window(const Window& window, const osg::CopyOp& co):
    MatrixTransform (window, co),
    EventInterface  (window),
    StyleInterface  (window),
    _parent         (0),
    _wm             (0),
    _index          (0),
    _x              (window._x),
    _y              (window._y),
    _z              (window._z),
    _zRange         (window._zRange),
    _strata         (window._strata),
    _vis            (window._vis),
    _r              (window._r),
    _s              (window._s),
    _scaleDenom     (window._scaleDenom),
    _width          (window._width),
    _height         (window._height),
    _vAnchor        (window._vAnchor),
    _hAnchor        (window._hAnchor),
    _visibleArea    (window._visibleArea)
{
    // Construct our vector of Widgets for easier use. :)
    // TODO: I almost certainly will need to use the getPosition() thing here eventually
    // for things to work 100% properly. For example, some Geodes may contain labels,
    // etc. Also, any widget that doesn't support simple addWidget probably won't
    // work (Table?)
    osg::Geode* geode = _geode();

    Widget* bg = dynamic_cast<Widget*>(geode->getDrawable(0));

    if(bg) {
        _setParented(bg);

        // TODO: This is silly...
        bg->setName(_name + "bg");
    }

    for(unsigned int i = 1; i < geode->getNumDrawables(); i++) {
        Widget* widget = dynamic_cast<Widget*>(geode->getDrawable(i));

        if(!widget) continue;

        // TODO: Properly test this...
        if(!widget->canClone()) {
            // geode->removeDrawable(widget);

            continue;
        }

        _setParented(widget);

        _objects.push_back(widget);
    }

    geode->setName(_name);
}

// This is the method by which all Windows are redrawn/resized. Keep in mind that not all
// Windows are required to absolutely honor a resize request, which is why we call the
// _getWidthImplementation() and _getHeightImplementation() functions instead of using the
// values passed in.
bool Window::resize(point_type width, point_type height) {
    // First, we query and store what sizes the Window currently is.
    _setWidthAndHeight();

    // Second, we determine if there is a difference between what the size currently
    // is and what the user has requested.
    point_type diffWidth  = width > 0.0f ? width - _width.current : 0.0f;
    point_type diffHeight = height > 0.0f ? height - _height.current : 0.0f;

    return resizeAdd(diffWidth, diffHeight);
}

bool Window::resizeAdd(point_type diffWidth, point_type diffHeight) {
    if(
        _width.current + diffWidth < _width.minimum ||
        _height.current + diffHeight < _height.minimum
    ) {
        warn()
            << "Window [" << _name << "] can't call resizeAdd() with the "
            << "values " << diffWidth << " and " << diffHeight << std::endl
        ;

        return false;
    }

    // Now we initiate the resize, which may or may not succeed.
    _resizeImplementation(diffWidth, diffHeight);

    // Inform each widget that it has been positioned.
    for(Iterator i = begin(); i != end(); i++) if(i->valid()) {
        i->get()->dirtyBound();
        i->get()->setDimensions();
        i->get()->positioned();
    }

    _setWidthAndHeight();

    Widget* bg = _bg();

    bg->setSize(_width.current, _height.current);
    bg->dirtyBound();
    bg->positioned();

    update();

    return true;
}

bool Window::resizePercent(point_type width, point_type height) {
    if(!_parent && !_wm) {
        warn()
            << "Window [" << _name
            << "] cannot resizePercent without being managed or parented."
            << std::endl
        ;

        return false;
    }

    if(!_parent) return resize(
        _wm->getWidth() * (width / 100.0f),
        _wm->getHeight() * (height / 100.0f)
    );

    else return resize(
        _parent->getWidth() * (width / 100.0f),
        _parent->getHeight() * (height / 100.0f)
    );
}

void Window::update() {
    WindowList wl;

    getEmbeddedList(wl);

    for(WindowList::iterator w = wl.begin(); w != wl.end(); w++) w->get()->update();

    matrix_type x  = _x;
    matrix_type y  = _y;
    XYCoord     xy = getAbsoluteOrigin();

    // We only honor ANCHOR requests on topmost Windows, not embedded ones.
    if((_vAnchor != VA_NONE || _hAnchor != HA_NONE) && !_parent && _wm) {
        if(_vAnchor == VA_TOP) y = _wm->getHeight() - _height.current;
        else if(_vAnchor == VA_CENTER) y = osg::round(_wm->getHeight() / 2.0f);
        else if(_vAnchor == VA_BOTTOM) y = 0.0f;

        if(_hAnchor == HA_LEFT) x = 0.0f;
        else if(_hAnchor == HA_CENTER) x = osg::round((_wm->getWidth() - _width.current)/ 2.0f);
        else if(_hAnchor == HA_RIGHT) x = _wm->getWidth() - _width.current + _visibleArea[2];

        xy.set(x, y);
    }

    matrix_type z = _z;

    // We can't do proper scissoring until we have access to our parent WindowManager, and
    // we need to determine the sorting method we want to use.
    if(_wm) {
        if(_wm->isUsingRenderBins()) {
            getOrCreateStateSet()->setRenderBinDetails(
                static_cast<int>((1.0f - fabs(_z)) * OSGWIDGET_RENDERBIN_MOD),
                "RenderBin"
            );

            z = 0.0f;
        }

        int sx = static_cast<int>(xy.x());
        int sy = static_cast<int>(xy.y());
        int sw = static_cast<int>(_width.current);
        int sh = static_cast<int>(_height.current);

        // This sets the Scissor area to some offset defined by the user.
        if(_vis == VM_PARTIAL) {
            sw = static_cast<int>(_visibleArea[2]);
            sh = static_cast<int>(_visibleArea[3]);
        }

        // Otherwise, use the size of the WindowManager itself.
        else if(_vis == VM_ENTIRE) {
            sx = 0;
            sy = 0;
            sw = static_cast<int>(_wm->getWidth());
            sh = static_cast<int>(_wm->getHeight());
        }

        _scissor()->setScissor(sx, sy, sw, sh);
    }

    // Update the Window itself, setting it's matrix according to translate, rotate, and
    // scale values.
    osg::Matrix r = osg::Matrix::rotate(
        osg::DegreesToRadians(_r),
        osg::Vec3d(0.0f, 0.0f, 1.0f)
    );

    osg::Matrix s = osg::Matrix::scale(_s, _s, 1.0f);
    osg::Matrix t = osg::Matrix::translate(x - _visibleArea[0], y - _visibleArea[1], z);

    setMatrix(r * s * t);
}

void Window::_setWidthAndHeightUnknownSizeError(const std::string& size, point_type val) {
    warn()
        << "Window [" << _name << "] doesn't know its " << size
        << " (" << val << ")." << std::endl
    ;
}

void Window::_setWidthAndHeightNotPAError(const std::string& size, point_type val) {
    warn()
        << "Window [" << _name
        << "] should be pixel-aligned, but a remainder was detected for it's "
        << size << " (" << val << ")." << std::endl
    ;
}

// Since there is so much error-checking associated with setting the width and height properly
// of a Window, this function attempts to abstract some of that tedium.
void Window::_setWidthAndHeight() {
    _width  = _getWidthImplementation();
    _height = _getHeightImplementation();

    if(_width.current < 0.0f) _setWidthAndHeightUnknownSizeError("current width", _width.current);

    if(_width.minimum < 0.0f) _setWidthAndHeightUnknownSizeError("minimum width", _width.minimum);

    if(_height.current < 0.0f) _setWidthAndHeightUnknownSizeError("current height", _height.current);

    if(_height.minimum < 0.0f) _setWidthAndHeightUnknownSizeError("minimum height", _height.minimum);

    if(hasDecimal(_width.current)) _setWidthAndHeightNotPAError("current width", _width.current);

    if(hasDecimal(_width.minimum)) _setWidthAndHeightNotPAError("minimum width", _width.minimum);

    if(hasDecimal(_height.current)) _setWidthAndHeightNotPAError("current height", _height.current);

    if(hasDecimal(_height.minimum)) _setWidthAndHeightNotPAError("minimum height", _height.minimum);
}

void Window::_removeFromGeode(Widget* widget) {
    if(!widget) return;

    widget->_index = 0;

    _setParented(widget, true);

    _geode()->removeDrawable(widget);
}

// This is a somewhat complicated function designed to only be called by derived classes,
// allowing them to insert Widgets (which can be added to the Window in any way the derived
// class sees fit) into the REAL internal _objects container.
// TODO: This doesn't handle insertion properly!!!
bool Window::_setWidget(Widget* widget, int index) {
    if(!widget) {
        warn() << "Window [" << _name << "] called addWidget with NULL." << std::endl;

        return false;
    }

    if(widget->_parent) {
        warn()
            << "Window [" << _name
            << "] attempted to parent Widget [" << widget->getName()
            << "], which is already parented by [" << widget->_parent->getName()
            << "]." << std::endl
        ;

        return false;
    }

    if(index >= 0 && index >= static_cast<int>(size())) {
        warn()
            << "Window [" << _name
            << "] attempted to manually insert the Widget [" << widget->getName()
            << "] at position " << index
            << ", but there is not enough space available."
            << std::endl
        ;

        return false;
    }

    // If we're just appending another widget...
    if(index < 0) _objects.push_back(widget);

    // Otherwise, we're inserting and need to call removeWidget on the old
    // one (if valid)...
    else {
        if(_objects[index].valid()) _removeFromGeode(_objects[index].get());

        _objects[index] = widget;
    }

    osg::Geode* geode = _geode();

    widget->_index = geode->getNumDrawables();

    geode->addDrawable(widget);

    _setParented(widget);
    _setManaged(widget);
    _setStyled(widget);

    // We make sure and resize after every added Widget. This ensures the most
    // accurate geometry...
    resize();

    return true;
}

bool Window::_setVisible(bool visible) {
    if(!_wm) return false;

    _wm->setValue(_index, visible);

    return true;
}

void Window::_setFocused(Widget* widget) {
    if(widget && _wm) {
        Event ev(_wm);

        ev._window = this;

        if(_focused.valid()) {
            ev._widget = _focused.get();

            _focused->callMethodAndCallbacks(ev.makeType(EVENT_UNFOCUS));
        }

        _focused   = widget;
        ev._widget = widget;

        _focused->callMethodAndCallbacks(ev.makeType(EVENT_FOCUS));
    }
}

void Window::_setStyled(Widget* widget) {
    if(!widget || !_wm) return;

    if(!widget->_isStyled) return;

    widget->_isStyled = true;

    _wm->getStyleManager()->applyStyles(widget);
}

void Window::_setParented(Widget* widget, bool setUnparented) {
    if(!widget) return;

    if(!setUnparented) {
        widget->_parent = this;

        widget->parented(this);
    }

    else {
        widget->unparented(this);

        widget->_parent = 0;
    }
}

void Window::_setManaged(Widget* widget, bool setUnmanaged) {
    if(!widget || !_wm) return;

    // Tell the widget it's managed if it isn't already...
    if(!setUnmanaged) {
        if(widget->_isManaged) return;

        widget->_isManaged = true;

        widget->managed(_wm);
    }

    // Otherwise, make sure it IS managed and tell it that it no longer will be. :)
    else {
        if(!widget->_isManaged) return;

        widget->_isManaged = false;

        widget->unmanaged(_wm);
    }
}

Widget* Window::_getBackground() const {
    const osg::Geode* geode = _geode();

    // lol...
    if(geode) return dynamic_cast<Widget*>(const_cast<osg::Drawable*>(geode->getDrawable(0)));

    return 0;
}

Window* Window::_getTopmostParent() const {
    WindowList windowList;

    getParentList(windowList);

    return windowList.back().get();
}

// This will position a widget based on the amount of width and height it has
// to fill. The x/y values should already be set, since we will be adding here.
// However, the width and height can be anything and will be adjusted accordingly.
void Window::_positionWidget(Widget* widget, point_type width, point_type height) {
    point_type w  = widget->getWidth();
    point_type h  = widget->getHeight();
    point_type pl = widget->getPadLeft();
    point_type pr = widget->getPadRight();
    point_type pt = widget->getPadTop();
    point_type pb = widget->getPadBottom();

    if(widget->canFill()) {
        point_type nw = osg::round(width - pr - pl);
        point_type nh = osg::round(height - pt - pb);

        widget->addOrigin(pl, pb);

        if(w != nw) widget->setWidth(nw);
        if(h != nh) widget->setHeight(nh);

        return;
    }

    point_type ha = osg::round((width - w - pl - pr) / 2.0f);
    point_type va = osg::round((height - h - pt - pb) / 2.0f);

    // Handle HORIZONTAL alignment.
    if(widget->getAlignHorizontal() == Widget::HA_LEFT) widget->addX(pl);

    else if(widget->getAlignHorizontal() == Widget::HA_RIGHT) widget->addX(width - w - pr);

    else widget->addX(ha + pl);

    // Handle VERTICAL alignment.
    if(widget->getAlignVertical() == Widget::VA_BOTTOM) widget->addY(height - h - pt);

    else if(widget->getAlignVertical() == Widget::VA_TOP) widget->addY(pb);

    else widget->addY(va + pb);
}

bool Window::isVisible() const {
    if(!_wm) return false;

    return _wm->getValue(_index);
}

bool Window::isXYWithinVisible(float x, float y) const {
    return
        (x >= _visibleArea[0] && x <= (_visibleArea[0] + _visibleArea[2])) &&
        (y >= _visibleArea[1] && y <= (_visibleArea[1] + _visibleArea[3]))
    ;
}

void Window::setVisibleArea(int x, int y, int w, int h) {
    _visibleArea[0] = x;
    _visibleArea[1] = y;
    _visibleArea[2] = w;
    _visibleArea[3] = h;
}

void Window::addVisibleArea(int x, int y, int w, int h) {
    _visibleArea[0] += x;
    _visibleArea[1] += y;
    _visibleArea[2] += w;
    _visibleArea[3] += h;
}

// The topmost Window always has this method called, instead of the embedded window directly.
bool Window::setFocused(const Widget* widget) {
    // TODO: I've turned on the warn() here, but perhaps I shouldn't? I need to define
    // the conditions under which it's okay to call setFocus() with a NULL widget.
    if(!widget) {
        warn() << "Window [" << _name << "] can't focus a NULL Widget." << std::endl;

        return false;
    }

    ConstIterator i = std::find(begin(), end(), widget);

    bool found = false;

    if(i == end()) {
        // We couldn't find the widget in the toplevel, so lets see if one of our
        // EmbeddedWindow objects has it.
        WindowList wl;

        getEmbeddedList(wl);

        for(WindowList::iterator w = wl.begin(); w != wl.end(); w++) {
            ConstIterator ii = std::find(w->get()->begin(), w->get()->end(), widget);

            if(ii != w->get()->end()) {
                found = true;
                i     = ii;
            }
        }
    }

    else found = true;

    if(!found) {
        warn()
            << "Window [" << _name
            << "] couldn't find the Widget [" << widget->getName()
            << "] in it's object list." << std::endl
        ;

        return false;
    }

    _setFocused(i->get());

    return true;
}

bool Window::setFocused(const std::string& name) {
    Widget* w1 = getByName(name);

    bool found = false;

    if(!w1) {
        // Just like above, we couldn't find the widget in the toplevel, so lets see if
        // one of our EmbeddedWindow objects has it. The difference here is that we
        // search by name.
        WindowList wl;

        getEmbeddedList(wl);

        for(WindowList::iterator w = wl.begin(); w != wl.end(); w++) {
            Widget* w2 = w->get()->getByName(name);

            if(w2) {
                found = true;
                w1    = w2;
            }
        }
    }

    else found = true;

    if(!found) {
        warn()
            << "Window [" << _name
            << "] couldn't find a Widget named [" << name
            << "] to set as it's focus." << std::endl
        ;

        return false;
    }

    _setFocused(w1);

    return true;
}

bool Window::grabFocus() {
    if(!_wm) return false;

    return _wm->setFocused(this);
}

bool Window::setFirstFocusable() {
    WidgetList focusList;

    if(getFocusList(focusList)) {
        _setFocused(focusList.front().get());

        return true;
    }

    return false;
}

bool Window::setNextFocusable() {
    WidgetList focusList;

    if(!getFocusList(focusList)) return false;

    WidgetList::iterator w = focusList.begin();

    // TODO: This needs to be a more complicated object, since the focus may be
    // in a child Window instead of a Widget.
    unsigned int focusedIndex = 0;

    for(unsigned int i = 0; w != focusList.end(); w++, i++) if(*w == _focused) {
        focusedIndex = i;

        break;
    }

    if(focusedIndex < focusList.size() - 1) _setFocused((++w)->get());

    else _setFocused(focusList.front().get());

    return true;
}

XYCoord Window::localXY(double absx, double absy) const {
    XYCoord xy = getAbsoluteOrigin();
    double  x  = absx - xy.x();
    double  y  = absy - xy.y();

    return XYCoord(x + _visibleArea[0], y + _visibleArea[1]);
}

XYCoord Window::getAbsoluteOrigin() const {
    XYCoord xy(0, 0);

    WindowList windowList;

    getParentList(windowList);

    for(WindowList::iterator i = windowList.begin(); i != windowList.end(); i++) {
        if(!i->valid()) continue;

        xy.x() += static_cast<int>(i->get()->getX());
        xy.y() += static_cast<int>(i->get()->getY());
    }

    return xy;
}

Window::EmbeddedWindow* Window::embed(
    const std::string& newName,
    Widget::Layer      layer,
    unsigned int       layerOffset
) {
    EmbeddedWindow* ew = new EmbeddedWindow(
        newName.size() > 0 ? newName : _name + "Embedded",
        getWidth(),
        getHeight()
    );

    ew->setWindow(this);
    ew->setSize(getWidth(), getHeight());
    ew->setCanFill(true);
    ew->setLayer(layer, layerOffset);

    return ew;
}

bool Window::getFocusList(WidgetList& wl) const {
    for(ConstIterator i = begin(); i != end(); i++) if(i->valid()) {
        EmbeddedWindow* ew = dynamic_cast<EmbeddedWindow*>(i->get());

        if(!ew) {
            if(i->get()->canFocus()) wl.push_back(i->get());
        }

        else {
            if(ew->getWindow()) ew->getWindow()->getFocusList(wl);
        }
    }

    return wl.size() != 0;
}

bool Window::getEmbeddedList(WindowList& wl) const {
    for(ConstIterator i = begin(); i != end(); i++) if(i->valid()) {
        EmbeddedWindow* ew = dynamic_cast<EmbeddedWindow*>(i->get());

        if(!ew || !ew->getWindow()) continue;

        else {
            wl.push_back(ew->getWindow());

            ew->getWindow()->getEmbeddedList(wl);
        }
    }

    return wl.size() != 0;
}

void Window::getParentList(WindowList& wl) const {
    const Window* current = this;

    while(current) {
        wl.push_back(const_cast<Window*>(current));

        if(current->_parent) current = current->_parent;

        else current = 0;
    }
}

void Window::managed(WindowManager* wm) {
    _wm = wm;

    for(Iterator i = begin(); i != end(); i++) {
        _setManaged(i->get());
        _setStyled(i->get());
    }

    setFirstFocusable();
    resize();
    update();
}

void Window::unmanaged(WindowManager* /*wm*/) {
    for(Iterator i = begin(); i != end(); i++) _setManaged(i->get(), true);

    _wm = 0;
}

bool Window::addWidget(Widget* widget) {
    return _setWidget(widget);
}

bool Window::insertWidget(Widget* widget, unsigned int pos) {
    return _setWidget(widget, pos);
}

bool Window::removeWidget(Widget* widget) {
    if(!widget) return false;

    if(_remove(widget)) {
        _removeFromGeode(widget);

        resize();

        return true;
    }

    return false;
}

bool Window::replaceWidget(Widget* /*oldWidget*/, Widget* /*newWidget*/) {
    return false;
}

unsigned int Window::addDrawableAndGetIndex(osg::Drawable* drawable) {
    osg::Geode* geode = _geode();

    if(geode->addDrawable(drawable)) return geode->getDrawableIndex(drawable);

    // 0 is a valid error return code here, since our background widget should be
    // the first child.
    return 0;
}

unsigned int Window::addChildAndGetIndex(osg::Node* node) {
    if(addChild(node)) return getChildIndex(node);

    return 0;
}

// All of the subsequent functions are very boring and uninteresting, although hopefully
// self-explanatory. They simply wrap calls to _compare<>() with the proper templates, and
// forward the optional iteration ranges...

point_type Window::_getMinWidgetWidth(int begin, int end, int add) const {
    return _compare<Less>(&Widget::getWidth, begin, end, add);
}

point_type Window::_getMinWidgetHeight(int begin, int end, int add) const {
    return _compare<Less>(&Widget::getHeight, begin, end, add);
}

point_type Window::_getMaxWidgetWidth(int begin, int end, int add) const {
    return _compare<Greater>(&Widget::getWidth, begin, end, add);
}

point_type Window::_getMaxWidgetHeight(int begin, int end, int add) const {
    return _compare<Greater>(&Widget::getHeight, begin, end, add);
}

point_type Window::_getMinWidgetMinWidth(int begin, int end, int add) const {
    return _compare<Less>(&Widget::getMinWidth, begin, end, add);
}

point_type Window::_getMinWidgetMinHeight(int begin, int end, int add) const {
    return _compare<Less>(&Widget::getMinHeight, begin, end, add);
}

point_type Window::_getMaxWidgetMinWidth(int begin, int end, int add) const {
    return _compare<Greater>(&Widget::getMinWidth, begin, end, add);
}

point_type Window::_getMaxWidgetMinHeight(int begin, int end, int add) const {
    return _compare<Greater>(&Widget::getMinHeight, begin, end, add);
}

point_type Window::_getMinWidgetWidthTotal(int begin, int end, int add) const {
    return _compare<Less>(&Widget::getWidthTotal, begin, end, add);
}

point_type Window::_getMinWidgetHeightTotal(int begin, int end, int add) const {
    return _compare<Less>(&Widget::getHeightTotal, begin, end, add);
}

point_type Window::_getMaxWidgetWidthTotal(int begin, int end, int add) const {
    return _compare<Greater>(&Widget::getWidthTotal, begin, end, add);
}

point_type Window::_getMaxWidgetHeightTotal(int begin, int end, int add) const {
    return _compare<Greater>(&Widget::getHeightTotal, begin, end, add);
}

point_type Window::_getMinWidgetMinWidthTotal(int begin, int end, int add) const {
    return _compare<Less>(&Widget::getMinWidthTotal, begin, end, add);
}

point_type Window::_getMinWidgetMinHeightTotal(int begin, int end, int add) const {
    return _compare<Less>(&Widget::getMinHeightTotal, begin, end, add);
}

point_type Window::_getMaxWidgetMinWidthTotal(int begin, int end, int add) const {
    return _compare<Greater>(&Widget::getMinWidthTotal, begin, end, add);
}

point_type Window::_getMaxWidgetMinHeightTotal(int begin, int end, int add) const {
    return _compare<Greater>(&Widget::getMinHeightTotal, begin, end, add);
}

point_type Window::_getMinWidgetPadHorizontal(int begin, int end, int add) const {
    return _compare<Less>(&Widget::getPadHorizontal, begin, end, add);
}

point_type Window::_getMinWidgetPadVertical(int begin, int end, int add) const {
    return _compare<Less>(&Widget::getPadVertical, begin, end, add);
}

point_type Window::_getMaxWidgetPadHorizontal(int begin, int end, int add) const {
    return _compare<Greater>(&Widget::getPadHorizontal, begin, end, add);
}

point_type Window::_getMaxWidgetPadVertical(int begin, int end, int add) const {
    return _compare<Greater>(&Widget::getPadVertical, begin, end, add);
}

point_type Window::_getNumFill(int begin, int end, int add) const {
    return _accumulate<Plus>(&Widget::getFillAsNumeric, begin, end, add);
}

Window::Sizes Window::_getWidthImplementation() const {
    osg::BoundingBox bb = getGeode()->getBoundingBox();

    point_type w = osg::round(bb.xMax() - bb.xMin());

    return Sizes(w, 0.0f);
}

Window::Sizes Window::_getHeightImplementation() const {
    osg::BoundingBox bb = getGeode()->getBoundingBox();

    point_type h = osg::round(bb.yMax() - bb.yMin());

    return Sizes(h, 0.0f);
}

}
