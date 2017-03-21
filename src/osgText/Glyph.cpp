/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osgText/Font>
#include <osgText/Text>

#include <osg/State>
#include <osg/Notify>
#include <osg/GLU>

#include <osgUtil/SmoothingVisitor>

#include <string.h>
#include <stdlib.h>

#include "GlyphGeometry.h"

using namespace osgText;
using namespace std;

GlyphTexture::GlyphTexture():
    _margin(1),
    _marginRatio(0.02f),
    _usedY(0),
    _partUsedX(0),
    _partUsedY(0)
{
    setWrap(WRAP_S, CLAMP_TO_EDGE);
    setWrap(WRAP_T, CLAMP_TO_EDGE);
}

GlyphTexture::~GlyphTexture()
{
}

// return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.
int GlyphTexture::compare(const osg::StateAttribute& rhs) const
{
    if (this<&rhs) return -1;
    else if (this>&rhs) return 1;
    return 0;
}


bool GlyphTexture::getSpaceForGlyph(Glyph* glyph, int& posX, int& posY)
{
    int maxAxis = osg::maximum(glyph->s(), glyph->t());
    int margin = _margin + (int)((float)maxAxis * _marginRatio);

    int width = glyph->s()+2*margin;
    int height = glyph->t()+2*margin;

    // first check box (_partUsedX,_usedY) to (width,height)
    if (width <= (getTextureWidth()-_partUsedX) &&
        height <= (getTextureHeight()-_usedY))
    {
        // can fit in existing row.

        // record the position in which the texture will be stored.
        posX = _partUsedX+margin;
        posY = _usedY+margin;

        // move used markers on.
        _partUsedX += width;
        if (_usedY+height>_partUsedY) _partUsedY = _usedY+height;

        return true;
    }

    // start an new row.
    if (width <= getTextureWidth() &&
        height <= (getTextureHeight()-_partUsedY))
    {
        // can fit next row.
        _partUsedX = 0;
        _usedY = _partUsedY;

        posX = _partUsedX+margin;
        posY = _usedY+margin;

        // move used markers on.
        _partUsedX += width;
        if (_usedY+height>_partUsedY) _partUsedY = _usedY+height;

        return true;
    }

    // doesn't fit into glyph.
    return false;
}

void GlyphTexture::addGlyph(Glyph* glyph, int posX, int posY)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    if (!_image.valid()) createImage();

    _glyphs.push_back(glyph);

    // set up the details of where to place glyph's image in the texture.
    glyph->setTexture(this);
    glyph->setTexturePosition(posX,posY);

    glyph->setMinTexCoord( osg::Vec2( static_cast<float>(posX)/static_cast<float>(getTextureWidth()),
                                      static_cast<float>(posY)/static_cast<float>(getTextureHeight()) ) );
    glyph->setMaxTexCoord( osg::Vec2( static_cast<float>(posX+glyph->s())/static_cast<float>(getTextureWidth()),
                                      static_cast<float>(posY+glyph->t())/static_cast<float>(getTextureHeight()) ) );

    _image->copySubImage(glyph->getTexturePositionX(), glyph->getTexturePositionY(), 0, glyph);
    _image->dirty();
}

void GlyphTexture::setThreadSafeRefUnref(bool threadSafe)
{
    osg::Texture2D::setThreadSafeRefUnref(threadSafe);
}

void GlyphTexture::resizeGLObjectBuffers(unsigned int maxSize)
{
    osg::Texture2D::resizeGLObjectBuffers(maxSize);

    unsigned int initialSize = _glyphsToSubload.size();
    _glyphsToSubload.resize(maxSize);

    for(unsigned i=initialSize; i<_glyphsToSubload.size(); ++i)
    {
        for(GlyphRefList::iterator itr = _glyphs.begin();
            itr != _glyphs.end();
            ++itr)
        {
            _glyphsToSubload[i].push_back(itr->get());
        }
    }
}

osg::Image* GlyphTexture::createImage()
{
    if (!_image)
    {
        _image = new osg::Image;
        _image->allocateImage(getTextureWidth(), getTextureHeight(), 1, OSGTEXT_GLYPH_FORMAT, GL_UNSIGNED_BYTE);
        memset(_image->data(), 0, _image->getTotalSizeInBytes());

        for(GlyphRefList::iterator itr = _glyphs.begin();
            itr != _glyphs.end();
            ++itr)
        {
            Glyph* glyph = itr->get();
            _image->copySubImage(glyph->getTexturePositionX(), glyph->getTexturePositionY(), 0, glyph);
        }
    }

    return _image.get();
}

// all the methods in Font::Glyph have been made non inline because VisualStudio6.0 is STUPID, STUPID, STUPID PILE OF JUNK.
Glyph::Glyph(Font* font, unsigned int glyphCode):
    _font(font),
    _glyphCode(glyphCode),
    _width(1.0f),
    _height(1.0f),
    _horizontalBearing(0.0f,0.f),
    _horizontalAdvance(0.f),
    _verticalBearing(0.0f,0.f),
    _verticalAdvance(0.f),
    _texture(0),
    _texturePosX(0),
    _texturePosY(0),
    _minTexCoord(0.0f,0.0f),
    _maxTexCoord(0.0f,0.0f)
{
    setThreadSafeRefUnref(true);
}

Glyph::~Glyph()
{
}

void Glyph::setHorizontalBearing(const osg::Vec2& bearing) {  _horizontalBearing=bearing; }
const osg::Vec2& Glyph::getHorizontalBearing() const { return _horizontalBearing; }

void Glyph::setHorizontalAdvance(float advance) { _horizontalAdvance=advance; }
float Glyph::getHorizontalAdvance() const { return _horizontalAdvance; }

void Glyph::setVerticalBearing(const osg::Vec2& bearing) {  _verticalBearing=bearing; }
const osg::Vec2& Glyph::getVerticalBearing() const { return _verticalBearing; }

void Glyph::setVerticalAdvance(float advance) {  _verticalAdvance=advance; }
float Glyph::getVerticalAdvance() const { return _verticalAdvance; }

void Glyph::setTexture(GlyphTexture* texture) { _texture = texture; }
GlyphTexture* Glyph::getTexture() { return _texture; }
const GlyphTexture* Glyph::getTexture() const { return _texture; }

void Glyph::setTexturePosition(int posX,int posY) { _texturePosX = posX; _texturePosY = posY; }
int Glyph::getTexturePositionX() const { return _texturePosX; }
int Glyph::getTexturePositionY() const { return _texturePosY; }

void Glyph::setMinTexCoord(const osg::Vec2& coord) { _minTexCoord=coord; }
const osg::Vec2& Glyph::getMinTexCoord() const { return _minTexCoord; }

void Glyph::setMaxTexCoord(const osg::Vec2& coord) { _maxTexCoord=coord; }
const osg::Vec2& Glyph::getMaxTexCoord() const { return _maxTexCoord; }

void Glyph::subload() const
{
    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        const GLubyte* msg = osg::gluErrorString(errorNo);
        if (msg) { OSG_WARN<<"before Glyph::subload(): detected OpenGL error: "<<msg<<std::endl; }
        else  { OSG_WARN<<"before Glyph::subload(): detected OpenGL error number: "<<errorNo<<std::endl; }
    }

    if(s() <= 0 || t() <= 0)
    {
        OSG_INFO<<"Glyph::subload(): texture sub-image width and/or height of 0, ignoring operation."<<std::endl;
        return;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT,getPacking());

    #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
    glPixelStorei(GL_UNPACK_ROW_LENGTH,getRowLength());
    #endif

    glTexSubImage2D(GL_TEXTURE_2D,0,
                    _texturePosX,_texturePosY,
                    s(),t(),
                    (GLenum)getPixelFormat(),
                    (GLenum)getDataType(),
                    data());

    errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {


        const GLubyte* msg = osg::gluErrorString(errorNo);
        if (msg) { OSG_WARN<<"after Glyph::subload() : detected OpenGL error: "<<msg<<std::endl; }
        else { OSG_WARN<<"after Glyph::subload() : detected OpenGL error number: "<<errorNo<<std::endl; }

        OSG_WARN<< "\tglTexSubImage2D(0x"<<hex<<GL_TEXTURE_2D<<dec<<" ,"<<0<<"\t"<<std::endl<<
                                 "\t                "<<_texturePosX<<" ,"<<_texturePosY<<std::endl<<
                                 "\t                "<<s()<<" ,"<<t()<<std::endl<<hex<<
                                 "\t                0x"<<(GLenum)getPixelFormat()<<std::endl<<
                                 "\t                0x"<<(GLenum)getDataType()<<std::endl<<
                                 "\t                "<<static_cast<const void*>(data())<<");"<<dec<<std::endl;
    }
}

Glyph3D::Glyph3D(Font* font, unsigned int glyphCode):
    osg::Referenced(true),
    _font(font),
    _glyphCode(glyphCode),
    _width(1.0f),
    _height(1.0f),
    _horizontalBearing(0,0),
    _horizontalAdvance(0),
    _verticalBearing(0,0),
    _verticalAdvance(0)
    {}

void Glyph3D::setThreadSafeRefUnref(bool threadSafe)
{
    for(GlyphGeometries::iterator itr = _glyphGeometries.begin();
        itr != _glyphGeometries.end();
        ++itr)
    {
        (*itr)->setThreadSafeRefUnref(threadSafe);
    }
}

GlyphGeometry* Glyph3D::getGlyphGeometry(const Style* style)
{

    for(GlyphGeometries::iterator itr = _glyphGeometries.begin();
        itr != _glyphGeometries.end();
        ++itr)
    {
        GlyphGeometry* glyphGeometry = itr->get();
        if (glyphGeometry->match(style))
        {
            OSG_INFO<<"Glyph3D::getGlyphGeometry(Style* style) found matching GlyphGeometry."<<std::endl;
            return glyphGeometry;
        }
    }

    OSG_INFO<<"Glyph3D::getGlyphGeometry(Style* style) could not find matching GlyphGeometry, creating a new one."<<std::endl;

    osg::ref_ptr<GlyphGeometry> glyphGeometry = new GlyphGeometry();
    glyphGeometry->setup(this, style);
    _glyphGeometries.push_back(glyphGeometry);

    return glyphGeometry.get();
}


GlyphGeometry::GlyphGeometry()
{
}

void GlyphGeometry::setThreadSafeRefUnref(bool threadSafe)
{
    if (_geode.valid()) _geode->setThreadSafeRefUnref(threadSafe);
}

void GlyphGeometry::setup(const Glyph3D* glyph, const Style* style)
{
    float creaseAngle = 30.0f;
    bool smooth = true;
    osg::ref_ptr<osg::Geometry> shellGeometry;

    if (!style)
    {
        OSG_INFO<<"GlyphGeometry::setup(const Glyph* glyph, NULL) creating default glyph geometry."<<std::endl;

        float width = 0.1f;

        _geometry = osgText::computeTextGeometry(glyph, width);
    }
    else
    {
        OSG_INFO<<"GlyphGeometry::setup(const Glyph* glyph, NULL) create glyph geometry with custom Style."<<std::endl;

        // record the style
        _style = osg::clone(style, osg::CopyOp::DEEP_COPY_ALL);

        const Bevel* bevel = style->getBevel();
        bool outline = style->getOutlineRatio()>0.0f;
        float width = style->getThicknessRatio();

        if (bevel)
        {
            osg::ref_ptr<osg::Geometry> glyphGeometry = osgText::computeGlyphGeometry(glyph, *bevel, width);

            _geometry = osgText::computeTextGeometry(glyphGeometry.get(), *bevel, width);
            shellGeometry = outline ? osgText::computeShellGeometry(glyphGeometry.get(), *bevel, width) : 0;
        }
        else
        {
            _geometry = osgText::computeTextGeometry(glyph, width);
        }
    }

    if (!_geometry)
    {
        OSG_INFO<<"Warning: GlyphGeometry::setup(const Glyph* glyph, const Style* style) failed."<<std::endl;
        return;
    }

    _geode = new osg::Geode;
    _geode->addDrawable(_geometry.get());
    if (shellGeometry.valid()) _geode->addDrawable(shellGeometry.get());

    // create the normals
    if (smooth)
    {
        osgUtil::SmoothingVisitor::smooth(*_geometry, osg::DegreesToRadians(creaseAngle));
    }

    _vertices = dynamic_cast<osg::Vec3Array*>(_geometry->getVertexArray());
    _normals = dynamic_cast<osg::Vec3Array*>(_geometry->getNormalArray());

    for(osg::Geometry::PrimitiveSetList::iterator itr = _geometry->getPrimitiveSetList().begin();
        itr != _geometry->getPrimitiveSetList().end();
        ++itr)
    {
        osg::PrimitiveSet* prim = itr->get();
        if (prim->getName()=="front") _frontPrimitiveSetList.push_back(prim);
        else if (prim->getName()=="back") _backPrimitiveSetList.push_back(prim);
        else if (prim->getName()=="wall") _wallPrimitiveSetList.push_back(prim);
    }
}

bool GlyphGeometry::match(const Style* style) const
{
    if (_style == style) return true;
    if (!_style || !style) return false;

    return (*_style==*style);
}
