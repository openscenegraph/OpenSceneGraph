// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

#include <osg/io_utils>
#include <osgDB/ReadFile>
#include <osgWidget/WindowManager>
#include <osgWidget/Frame>
#include <cassert>

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
}

Frame::Corner::Corner(const Corner& corner, const osg::CopyOp& co):
Widget  (corner, co),
_corner (corner._corner)
{
}

void Frame::Corner::parented(Window* window) {
    Frame* parent = dynamic_cast<Frame*>(getParent());

    if(!parent) return;

    if(parent->canResize()) setEventMask(EVENT_MASK_MOUSE_DRAG);
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
}

Frame::Border::Border(const Border& border, const osg::CopyOp& co):
Widget  (border, co),
_border (border._border) 
{
}

void Frame::Border::parented(Window* window) {
    Frame* parent = dynamic_cast<Frame*>(getParent());

    if(!parent) return;

    if(parent->canResize()) setEventMask(EVENT_MASK_MOUSE_DRAG);
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
    unsigned int       flags,
    Frame*             exFrame
) {
    Frame* frame = 0;
    
    // Use an "existing frame" if we have it (for example, if you've in inherited from
    // Frame and want to use this stuff.
    if(!exFrame) frame = new Frame(name, flags);

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
    unsigned int       flags,
    Frame*             exFrame
) {
    Frame* frame = 0;

    double w = width;
    double h = height;

    if (image)
    {
        w = image->s() / 8.0f;
        h = image->t();
    }

    // The same as above...
    if(!exFrame) frame = createSimpleFrame(name, w, h, width, height, flags);

    else frame = createSimpleFrame(name, w, h, width, height, 0, exFrame);

    if (image)
    {

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
    }
    else 
    {
        osg::notify(osg::WARN) << "createSimpleFrameWithSingleTexture with a null image, the frame " << name << " will be use texture" << std::endl;
    }

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


osg::Image* createNatifEdgeImageFromTheme(osg::Image* theme);

Frame* Frame::createSimpleFrameFromTheme(
    const std::string& name,
    osg::Image*        image,
    point_type         width,
    point_type         height,
    unsigned int       flags,
    Frame*             exFrame
) {

    osg::ref_ptr<osg::Image> natifImage = createNatifEdgeImageFromTheme(image);
    Frame* frame;

    frame = createSimpleFrameWithSingleTexture(name, natifImage.get(), width, height, flags, exFrame);

    if (frame && image && natifImage.valid()) 
    {
        const unsigned int bpps = image->getPixelSizeInBits() / 8;
        const unsigned int one_third_s = image->s()/3;
        unsigned char* srcdata = (unsigned char*)image->data();
        osg::Vec4 color(0,0,0,1);
        for (unsigned int d = 0; d < bpps; d++)
        {
            color[d] = srcdata[one_third_s * image->s() * bpps + (one_third_s) * bpps + d] * 1.0/255.0;
        }
        frame->getEmbeddedWindow()->setColor(color);
    }
    return frame;
}







// (c) 2006-2008   Jean-Sébastien Guay
// adapted by Cedric Pinson

/** Implementation of copyImage. */
template<typename T>
void copyDataImpl(const osg::Image* source,
                  const unsigned int x1, const unsigned int y1,
                  const unsigned int x2, const unsigned int y2,
                  osg::Image* destination, 
                  const unsigned int xd = 0, const unsigned int yd = 0)
{
    if ((unsigned int)destination->s() >= xd + (x2 - x1) && 
        (unsigned int)destination->t() >= yd + (y2 - y1))
    {
        const unsigned int bpps =      source->getPixelSizeInBits() / (8 * sizeof(T));

        T* srcdata = (T*)source->data();
        T* dstdata = (T*)destination->data();

        for (unsigned int y = 0; y < y2 - y1; ++y)
        {
            for (unsigned int x = 0; x < x2 - x1; ++x)
            {
                for (unsigned int d = 0; d < bpps; d++) 
                {
                    T v = srcdata[(y + y1) * source->s() * bpps + (x + x1) * bpps + d];
                    dstdata[(yd + y) * destination->s() * bpps + (xd + x) * bpps + d] = v;
                }
            }
        }
    }
    else
        assert(false && "copyDataImpl: Incorrect image dimensions.");
}

/** Copies a rectangle of corners (x1, y1), (x2, y2) from an image into 
    another image starting at position (xd, yd). No scaling is done, the
    pixels are just copied, so the destination image must be at least 
    (xd + (x2 - x1)) by (yd + (y2 - y1)) pixels. */
void copyData(const osg::Image* source,
              const unsigned int x1, const unsigned int y1, 
              const unsigned int x2, const unsigned int y2,
              osg::Image* destination, 
              const unsigned int xd, const unsigned int yd)
{
    if (source->getDataType() == destination->getDataType())
    {
        if (source->getDataType() == GL_UNSIGNED_BYTE)
        {
            copyDataImpl<unsigned char>(source, x1, y1, x2, y2, 
                                        destination, xd, yd);
        }
        else
        {
            assert(false && "copyData not implemented for this data type");
        }
    }
    else
    {
        assert(false && "source and destination images must be of the same type.");
        return;
    }
}


/** Implementation of rotateImage. */
template<typename T>
osg::Image* rotateImageImpl(osg::Image* image)
{
    if (image->s() == image->t())
    {
        const unsigned int s = image->s();
        const unsigned int bpp = image->getPixelSizeInBits() / (8 * sizeof(T));

        osg::ref_ptr<osg::Image> destination  = new osg::Image;
        destination->allocateImage(s, s, 1,
                                   image->getPixelFormat(), image->getDataType(),
                                   image->getPacking());
        destination->setInternalTextureFormat(image->getInternalTextureFormat());

        T* srcdata = (T*)image->data();
        T* dstdata = (T*)destination->data();

        for (unsigned int y = 0; y < s; ++y)
        {
            for (unsigned int x = 0; x < s; ++x)
            {
                for (unsigned int p = 0; p < bpp; p++)
                    dstdata[y * s * bpp + x * bpp + p] = srcdata[x * s * bpp + y * bpp + p];
            }
        }

        return destination.release();
    }
    else
    {
        assert(false && "rotateImageImpl: Image must be square.");
        return 0;
    }
}

/** Rotates an osg::Image by 90 degrees. Returns a new osg::Image, be sure to
    store it in a ref_ptr so it will be freed correctly. */
osg::Image* rotateImage(osg::Image* image)
{
    if (image->getDataType() == GL_UNSIGNED_BYTE)
    {
        return rotateImageImpl<unsigned char>(image);
    }
    else
    {
        assert(false && "rotateImage not implemented for this data type");
        return 0;
    }
}



// SOURCE
//          +---+---+---+
//          | 1 | 2 | 3 |
//          +---+---+---+
//          | 4 |   | 5 |
//          +---+---+---+
//          | 6 | 7 | 8 |
//          +---+---+---+


// FINAL
//         +---+---+---+---+---+---+---+---+
//         | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
//         +---+---+---+---+---+---+---+---+

//         1. Upper-Left corner.
//         2. Top border (rotated 90 degrees CCW).
//         3. Upper-Right corner.
//         4. Left border.
//         5. Right border.
//         6. Bottom-Left corner.
//         7. Bottom border (rotated 90 degrees CCW).
//         8. Bottom-Right corner.

osg::Image* createNatifEdgeImageFromTheme(osg::Image* theme)
{
    if (!theme) {
        osg::notify(osg::WARN) << "can't create a natif edge image from null image theme as argument" << std::endl;
        return 0;
    }
    osg::ref_ptr<osg::Image> final = new osg::Image;
    const int s = theme->s();
    const int t = theme->t();
    const GLenum pixelFormat   = theme->getPixelFormat();
    const GLenum dataType      = theme->getDataType();
    const GLint internalFormat = theme->getInternalTextureFormat();
    unsigned int packing       = theme->getPacking();

    if (s != t)
    {
        osg::notify(osg::WARN) << "width and height are different, bad format theme image " << theme->getFileName() << std::endl;
        return 0;
    }
    
    // check size
    int ceilvalue = static_cast<int>(ceil(s * 1.0 / 3));
    int intvalue = s/3;
    if (intvalue != ceilvalue)
    {
        osg::notify(osg::WARN) << "the size of theme file " << theme->getFileName() << " can not be divided by 3, check the documentation about theme format" << std::endl;
        return 0;
    }

    const unsigned int one_third_s = s/3;
    const unsigned int one_third_t = t/3;

    final->allocateImage(8 * one_third_s , one_third_t, 1, pixelFormat, dataType, packing);
    final->setInternalTextureFormat(internalFormat);

    // copy 1 (6 in source)
    copyData(theme, 0, 2 * one_third_s, one_third_s, 3 * one_third_s, final.get(), 0, 0);

    // rotate and copy 2
    osg::ref_ptr<osg::Image> rotateandcopy2  = new osg::Image;
    rotateandcopy2->allocateImage(one_third_s , one_third_t, 1, pixelFormat, dataType, packing);
    rotateandcopy2->setInternalTextureFormat(internalFormat);
    copyData(theme, one_third_s, 0, 2 * one_third_s , one_third_s, rotateandcopy2.get(), 0, 0);
    rotateandcopy2 = rotateImage(rotateandcopy2.get());
    rotateandcopy2->flipHorizontal();
    copyData(rotateandcopy2.get(), 0, 0, one_third_s , one_third_s, final.get(), 6*one_third_s, 0);

    // copy 3 (8 in source)
    copyData(theme, 2*one_third_s , 2 *one_third_s, 3*one_third_s , 3 * one_third_s, final.get(), 2 * one_third_s, 0);

    // copy 4
    copyData(theme, 0, one_third_s, one_third_s , 2 * one_third_s, final.get(), 3 * one_third_s, 0);

    // copy 5
    copyData(theme, 2*one_third_s , one_third_s, 3 * one_third_s , 2 * one_third_s, final.get(), 4 * one_third_s, 0);

    // copy 6 (1 in source)
    copyData(theme, 0 , 0, one_third_s, one_third_s, final.get(), 5 * one_third_s, 0);

    // rotate and copy 7
    osg::ref_ptr<osg::Image> rotateandcopy7  = new osg::Image;
    rotateandcopy7->allocateImage(one_third_s , one_third_t, 1, pixelFormat, dataType, packing);
    rotateandcopy7->setInternalTextureFormat(internalFormat);
    copyData(theme, one_third_s, 2*one_third_s, 2 * one_third_s , 3 * one_third_s, rotateandcopy7.get(), 0, 0);
    rotateandcopy7 = rotateImage(rotateandcopy7.get());
    rotateandcopy7->flipHorizontal();
    copyData(rotateandcopy7.get(), 0, 0, one_third_s , one_third_s, final.get(), one_third_s, 0);

    // copy 8 (3 in source)
    copyData(theme, 2 * one_third_s, 0, 3 * one_third_s , one_third_s , final.get(), 7 * one_third_s, 0);

    return final.release();
}

}
