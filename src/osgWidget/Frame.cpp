// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: Frame.cpp 59 2008-05-15 20:55:31Z cubicool $

#include <osgDB/ReadFile>
#include <osgWidget/WindowManager>
#include <osgWidget/Frame>

namespace osgWidget {

std::string Frame::cornerToString(CORNER c) {
    if(c == CORNER_LOWER_LEFT) return "CornerLowerLeft";

    else if(c == CORNER_LOWER_RIGHT) return "CornerLowerRight";

    else if(c == CORNER_UPPER_RIGHT) return "CornerUpperRight";

    else return "CornerUpperLeft";
}

std::string Frame::borderToString(BORDER b) {
    if(b == BORDER_LEFT) return "BorderLeft";

    else if(b == BORDER_RIGHT) return "BorderRight";

    else if(b == BORDER_TOP) return "BorderTop";

    else return "BorderBottom";
}

Frame::Corner::Corner(CORNER corner, point_type width, point_type height):
Widget  (cornerToString(corner), width, height),
_corner (corner) {
    setEventMask(EVENT_MASK_MOUSE_DRAG);
}

Frame::Corner::Corner(const Corner& corner, const osg::CopyOp& co):
Widget  (corner, co),
_corner (corner._corner) {
}

bool Frame::Corner::mouseDrag(double x, double y, WindowManager* wm) {
    Window* parent = getParent();

    if(!parent) return false;

    if(wm->isInvertedY()) {
        if(_corner == CORNER_UPPER_LEFT) {
            if(parent->resizeAdd(-x, -y)) parent->addOrigin(x, y);
        }

        else if(_corner == CORNER_UPPER_RIGHT) {
            if(parent->resizeAdd(x, -y)) parent->addY(y);
        }

        else if(_corner == CORNER_LOWER_RIGHT) parent->resizeAdd(x, y);

        else {
            if(parent->resizeAdd(-x, y)) parent->addX(x);
        }
    }

    // These are basically flipped-around versions of the above routines; we
    // do it this way to avoid lots of uncessary if tests.
    else {
        if(_corner == CORNER_UPPER_LEFT) {
            if(parent->resizeAdd(-x, y)) parent->addX(x);
        }

        else if(_corner == CORNER_UPPER_RIGHT) parent->resizeAdd(x, y);

        else if(_corner == CORNER_LOWER_RIGHT) {
            if(parent->resizeAdd(x, -y)) parent->addY(y);
        }

        else {
            if(parent->resizeAdd(-x, -y)) parent->addOrigin(x, y);
        }
    }

    parent->update();
    
    return true;
}

Frame::Border::Border(BORDER border, point_type width, point_type height):
Widget  (borderToString(border), width, height),
_border (border) {
    setCanFill(true);
    setEventMask(EVENT_MASK_MOUSE_DRAG);
}

Frame::Border::Border(const Border& border, const osg::CopyOp& co):
Widget  (border, co),
_border (border._border) {
}

bool Frame::Border::mouseDrag(double x, double y, WindowManager* wm) {
    Window* parent = getParent();

    if(!parent) return false;

    if(_border == BORDER_LEFT) {
        if(parent->resizeAdd(-x, 0.0f)) parent->addX(x);
    }

    else if(_border == BORDER_RIGHT) parent->resizeAdd(x, 0.0f);

    else if(_border == BORDER_TOP) parent->addOrigin(x, y);

    else {
        // The only BORDER that inverted-Y affects is this...
        if(wm->isInvertedY()) parent->resizeAdd(0.0f, y);

        else {
            if(parent->resizeAdd(0.0f, -y)) parent->addY(y);
        }
    }

    parent->update();

    return true;
}

Frame::Frame(const std::string& name):
Table(name, 3, 3) {
}

Frame::Frame(const Frame& frame, const osg::CopyOp& co):
Table(frame, co) {
}

Widget* Frame::_getCorner(CORNER c) const {
    return const_cast<Widget*>(getByName(cornerToString(c)));
}

Widget* Frame::_getBorder(BORDER b) const {
    return const_cast<Widget*>(getByName(borderToString(b)));
}

void Frame::managed(WindowManager* wm) {
    Window::managed(wm);

    // Our Frame is created in an inverted-Y environment, so if this is the case
    // just return here.
    if(wm->isInvertedY()) return;

    Corner* ll = getCorner(CORNER_LOWER_LEFT);
    Corner* lr = getCorner(CORNER_LOWER_RIGHT);
    Corner* ul = getCorner(CORNER_UPPER_LEFT);
    Corner* ur = getCorner(CORNER_UPPER_RIGHT);
    Border* t  = getBorder(BORDER_TOP);
    Border* b  = getBorder(BORDER_BOTTOM);

    if(!ll || !lr || !ul || !ur || !t || !b) {
        warn()
            << "One or more of your Corner/Border objects in the Frame [" 
            << _name << "] are invalid; cannot invert orientation." << std::endl
        ;

        return;
    }

    ll->setCornerAndName(CORNER_UPPER_LEFT);
    lr->setCornerAndName(CORNER_UPPER_RIGHT);
    ul->setCornerAndName(CORNER_LOWER_LEFT);
    ur->setCornerAndName(CORNER_LOWER_RIGHT);
    t->setBorderAndName(BORDER_BOTTOM);
    b->setBorderAndName(BORDER_TOP);
}

bool Frame::setWindow(Window* window) {
    if(!window) return false;

    EmbeddedWindow* ew = getEmbeddedWindow();

    // If it's the first time setting the Window...
    // if(!ew || !ew->getWindow()) return addWidget(window->embed(), 1, 1);
    if(!ew) return addWidget(window->embed(), 1, 1);

    else return ew->setWindow(window);
}

Frame* Frame::createSimpleFrame(
    const std::string& name,
    point_type         cw,
    point_type         ch,
    point_type         w,
    point_type         h,
    Frame*             exFrame
) {
    Frame* frame = 0;
    
    // Use an "existing frame" if we have it (for example, if you've in inherited from
    // Frame and want to use this stuff.
    if(!exFrame) frame = new Frame(name);

    else frame = exFrame;
    
    frame->addWidget(new Corner(CORNER_UPPER_LEFT, cw,  ch), 0, 0);
    frame->addWidget(new Border(BORDER_TOP, w, ch), 0, 1);
    frame->addWidget(new Corner(CORNER_UPPER_RIGHT, cw, ch), 0, 2);
    frame->addWidget(new Border(BORDER_LEFT, cw, h), 1, 0);
    frame->addWidget(new Border(BORDER_RIGHT, cw, h), 1, 2);
    frame->addWidget(new Corner(CORNER_LOWER_LEFT, cw, ch), 2, 0);
    frame->addWidget(new Border(BORDER_BOTTOM, w, ch), 2, 1);
    frame->addWidget(new Corner(CORNER_LOWER_RIGHT, cw, ch), 2, 2);

    EmbeddedWindow* ew = new EmbeddedWindow(name, w, h);

    ew->setCanFill(true);

    frame->addWidget(ew, 1, 1);

    return frame;
}

Frame* Frame::createSimpleFrameWithSingleTexture(
    const std::string& name,
    const std::string& texture,
    point_type         tw,
    point_type         th,
    point_type         cw,
    point_type         ch,
    point_type         w,
    point_type         h,
    Frame*             exFrame
) {
    Frame* frame = 0;

    // The same as above...
    if(!exFrame) frame = createSimpleFrame(name, cw, ch, w, h);

    else frame = createSimpleFrame(name, cw, ch, w, h, exFrame);

    for(unsigned int i = 0; i < 9; i++) frame->getObjects()[i]->setImage(texture);

    frame->getByRowCol(0, 0)->setTexCoordRegion(0.0f, th - ch, cw, ch);
    frame->getByRowCol(0, 1)->setTexCoordRegion(cw, th - ch, tw - (cw * 2.0f), ch);
    frame->getByRowCol(0, 2)->setTexCoordRegion(tw - cw, th - ch, cw, ch);
    frame->getByRowCol(1, 0)->setTexCoordRegion(0.0f, ch, cw, th - (ch * 2.0f));
    frame->getByRowCol(1, 2)->setTexCoordRegion(tw - cw, ch, cw, th - (ch * 2.0f));
    frame->getByRowCol(2, 0)->setTexCoordRegion(0.0f, 0.0f, cw, ch);
    frame->getByRowCol(2, 1)->setTexCoordRegion(cw, 0.0f, tw - (cw * 2.0f), ch);
    frame->getByRowCol(2, 2)->setTexCoordRegion(tw - cw, 0.0f, cw, ch);

    frame->getEmbeddedWindow()->setTexCoordRegion(cw, ch, tw - (cw * 2.0f), th - (ch * 2.0f));

    return frame;
}

}
