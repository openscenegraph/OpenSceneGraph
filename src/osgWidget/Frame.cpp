// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

#include <osg/io_utils>
#include <osgDB/ReadFile>
#include <osgWidget/WindowManager>
#include <osgWidget/Frame>

namespace osgWidget {

std::string Frame::cornerTypeToString(CornerType c)
{
    if(c == CORNER_LOWER_LEFT) return "CornerLowerLeft";

    else if(c == CORNER_LOWER_RIGHT) return "CornerLowerRight";

    else if(c == CORNER_UPPER_RIGHT) return "CornerUpperRight";

    else return "CornerUpperLeft";
}

std::string Frame::borderTypeToString(BorderType b)
{
    if(b == BORDER_LEFT) return "BorderLeft";

    else if(b == BORDER_RIGHT) return "BorderRight";

    else if(b == BORDER_TOP) return "BorderTop";

    else return "BorderBottom";
}

Frame::Corner::Corner(CornerType corner, point_type width, point_type height):
Widget  (cornerTypeToString(corner), width, height),
_corner (corner)
{
    setEventMask(EVENT_MASK_MOUSE_DRAG);
}

Frame::Corner::Corner(const Corner& corner, const osg::CopyOp& co):
Widget  (corner, co),
_corner (corner._corner)
{
}

bool Frame::Corner::mouseDrag(double x, double y, WindowManager* wm)
{
    Frame* parent = dynamic_cast<Frame*>(getParent());

    if(!parent || !parent->canResize()) return false;

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

    parent->update();
    
    return true;
}

Frame::Border::Border(BorderType border, point_type width, point_type height):
Widget  (borderTypeToString(border), width, height),
_border (border)
{
    setCanFill(true);
    setEventMask(EVENT_MASK_MOUSE_DRAG);
}

Frame::Border::Border(const Border& border, const osg::CopyOp& co):
Widget  (border, co),
_border (border._border) 
{
}

void Frame::Border::positioned()
{
    osg::Image* image = _image();

    if(!image) return;

    Frame* parent = dynamic_cast<Frame*>(getParent());

    if(!parent || !parent->canTexture()) return;

    point_type w = image->s() / 8.0f;
    point_type h = getHeight();

    if(_border == BORDER_LEFT) setTexCoordRegion(w * 3, 0.0f, w, h);
    
    else if(_border == BORDER_RIGHT) setTexCoordRegion(w * 4, 0.0f, w, h);

    else if(_border == BORDER_TOP) {
        // TODO: Temporary; fix this.
        point_type tx1 = (w * 2) / image->s();
        point_type tx2 = w / image->s();
        point_type tx3 = getWidth() / w;

        setTexCoord(tx1, tx3,  LL);
        setTexCoord(tx1, 0.0f, LR);
        setTexCoord(tx2, 0.0f, UR);
        setTexCoord(tx2, tx3,  UL);
    }

    else {
        point_type tx1 = (w * 7) / image->s();
        point_type tx2 = (w * 6) / image->s();
        point_type tx3 = getWidth() / w;

        setTexCoord(tx1, tx3,  LL);
        setTexCoord(tx1, 0.0f, LR);
        setTexCoord(tx2, 0.0f, UR);
        setTexCoord(tx2, tx3,  UL);
    }
}

bool Frame::Border::mouseDrag(double x, double y, WindowManager* wm)
{
    Frame* parent = dynamic_cast<Frame*>(getParent());
    
    if(!parent) return false;

    if(_border == BORDER_TOP && parent->canMove()) parent->addOrigin(x, y);

    else {
        if(!parent->canResize()) return false;

        if(_border == BORDER_LEFT) {
            if(parent->resizeAdd(-x, 0.0f)) parent->addX(x);
        }

        else if(_border == BORDER_RIGHT) parent->resizeAdd(x, 0.0f);

        else {
            if(parent->resizeAdd(0.0f, -y)) parent->addY(y);
        }
    }

    parent->update();

    return true;
}

Frame::Frame(const std::string& name, unsigned int flags):
Table  (name, 3, 3),
_flags (flags)
{
}

Frame::Frame(const Frame& frame, const osg::CopyOp& co):
Table(frame, co)
{
}

Widget* Frame::_getCorner(CornerType c) const
{
    return const_cast<Widget*>(getByName(cornerTypeToString(c)));
}

Widget* Frame::_getBorder(BorderType b) const
{
    return const_cast<Widget*>(getByName(borderTypeToString(b)));
}

bool Frame::setWindow(Window* window)
{
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
    
    frame->addWidget(new Corner(CORNER_LOWER_LEFT,  cw, ch), 0, 0);
    frame->addWidget(new Border(BORDER_BOTTOM,      w,  ch), 0, 1);
    frame->addWidget(new Corner(CORNER_LOWER_RIGHT, cw, ch), 0, 2);
    frame->addWidget(new Border(BORDER_LEFT,        cw, h),  1, 0);
    frame->addWidget(new Border(BORDER_RIGHT,       cw, h),  1, 2);
    frame->addWidget(new Corner(CORNER_UPPER_LEFT,  cw, ch), 2, 0);
    frame->addWidget(new Border(BORDER_TOP,         w,  ch), 2, 1);
    frame->addWidget(new Corner(CORNER_UPPER_RIGHT, cw, ch), 2, 2);

    EmbeddedWindow* ew = new EmbeddedWindow(name, w, h);

    ew->setCanFill(true);

    frame->addWidget(ew, 1, 1);

    return frame;
}

/*
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
*/

// Inspired by: http://www.wowwiki.com/EdgeFiles
Frame* Frame::createSimpleFrameWithSingleTexture(
    const std::string& name,
    osg::Image*        image,
    point_type         width,
    point_type         height,
    Frame*             exFrame
) {
    Frame* frame = 0;

    double w = image->s() / 8.0f;
    double h = image->t();

    // The same as above...
    if(!exFrame) frame = createSimpleFrame(name, w, h, width, height);

    else frame = createSimpleFrame(name, w, h, width, height, exFrame);

    for(unsigned int i = 0; i < 9; i++) frame->getObjects()[i]->setImage(image);

    XYCoord twh(w, h);

    frame->getCorner(CORNER_UPPER_LEFT )->setTexCoordRegion(0.0f,  0.0f, twh);
    frame->getBorder(BORDER_TOP        )->setTexCoordRegion(w,     0.0f, twh);
    frame->getCorner(CORNER_UPPER_RIGHT)->setTexCoordRegion(w * 2, 0.0f, twh);
    frame->getBorder(BORDER_LEFT       )->setTexCoordRegion(w * 3, 0.0f, twh);
    frame->getBorder(BORDER_RIGHT      )->setTexCoordRegion(w * 4, 0.0f, twh);
    frame->getCorner(CORNER_LOWER_LEFT )->setTexCoordRegion(w * 5, 0.0f, twh);
    frame->getBorder(BORDER_BOTTOM     )->setTexCoordRegion(w * 6, 0.0f, twh);
    frame->getCorner(CORNER_LOWER_RIGHT)->setTexCoordRegion(w * 7, 0.0f, twh);

    // We set all of these to wrap vertically, but the REAL texture coordinates will
    // be generated properly in the positioned() method.
    frame->getByRowCol(0, 1)->setTexCoordWrapVertical();
    frame->getByRowCol(1, 0)->setTexCoordWrapVertical();
    frame->getByRowCol(1, 2)->setTexCoordWrapVertical();
    frame->getByRowCol(2, 1)->setTexCoordWrapVertical();

    // frame->getEmbeddedWindow()->setTexCoordRegion(cw, ch, tw - (cw * 2.0f), th - (ch * 2.0f));

    return frame;
}

bool Frame::resizeFrame(point_type w, point_type h) {
    Border* left   = getBorder(BORDER_LEFT);
    Border* right  = getBorder(BORDER_RIGHT);
    Border* top    = getBorder(BORDER_TOP);
    Border* bottom = getBorder(BORDER_BOTTOM);

    if(!left || !right || !top || !bottom) return false;

    return resize(
        left->getWidth() + right->getWidth() + w,
        top->getHeight() + bottom->getHeight() + h
    ); 
}

}
