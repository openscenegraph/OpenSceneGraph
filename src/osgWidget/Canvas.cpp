// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

#include <osgWidget/Canvas>

namespace osgWidget {

Canvas::Canvas(const std::string& name):
Window(name) {
}

Canvas::Canvas(const Canvas& canvas, const osg::CopyOp& co):
Window(canvas, co) {
}

void Canvas::_resizeImplementation(point_type /*w*/, point_type /*h*/) {
    // A Canvas has no layout, so it doesn't really know how to honor a resize
    // request. :) The best I could do here is store the differences and add them
    // later to the calls to getWidth/getHeight.
}

bool Canvas::addWidget(Widget* widget, point_type x, point_type y) {
    if(!widget) return false;

    widget->setOrigin(x, y);

    return Window::addWidget(widget);
}

}
