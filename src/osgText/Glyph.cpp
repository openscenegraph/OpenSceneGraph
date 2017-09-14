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

#if 0
    #define TEXTURE_IMAGE_NUM_CHANNELS 1
    #define TEXTURE_IMAGE_FORMAT OSGTEXT_GLYPH_FORMAT
#else
    #define TEXTURE_IMAGE_NUM_CHANNELS 4
    #define TEXTURE_IMAGE_FORMAT GL_RGBA
#endif

GlyphTexture::GlyphTexture():
    _margin(1),
    _marginRatio(0.02f),
    _interval(1),
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
    int margin_from_ratio = (int)((float)maxAxis * _marginRatio);
    int search_distance = glyph->getFontResolution().second/8;

    int margin = _margin + osg::maximum(margin_from_ratio, search_distance);

    int width = glyph->s()+2*margin;
    int height = glyph->t()+2*margin;

    int partUsedX = ((_partUsedX % _interval) == 0) ? _partUsedX : (((_partUsedX/_interval)+1)*_interval);
    int partUsedY = ((_partUsedY % _interval) == 0) ? _partUsedY : (((_partUsedY/_interval)+1)*_interval);
    int usedY = ((_usedY % _interval) == 0) ? _usedY : (((_usedY/_interval)+1)*_interval);

    // first check box (partUsedX, usedY) to (width,height)
    if (width <= (getTextureWidth()-partUsedX) &&
        height <= (getTextureHeight()-usedY))
    {
        // can fit in existing row.

        // record the position in which the texture will be stored.
        posX = partUsedX+margin;
        posY = usedY+margin;

        // move used markers on.
        _partUsedX = posX+width;
        if (_usedY+height>_partUsedY) _partUsedY = _usedY+height;

        return true;
    }

    // start an new row.
    if (width <= getTextureWidth() &&
        height <= (getTextureHeight()-_partUsedY))
    {
        // can fit next row.
        _partUsedX = 0;
        _usedY = partUsedY;

        posX = _partUsedX+margin;
        posY = _usedY+margin;

        // move used markers on.
        _partUsedX = posX+width;
        _partUsedY = _usedY+height;

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

    copyGlyphImage(glyph);
}

void GlyphTexture::copyGlyphImage(Glyph* glyph)
{

    if (_glyphTextureFeatures==GREYSCALE)
    {
        // OSG_NOTICE<<"GlyphTexture::copyGlyphImage() greyscale copy"<<std::endl;
        _image->copySubImage(glyph->getTexturePositionX(), glyph->getTexturePositionY(), 0, glyph);
        _image->dirty();
        return;
    }

    // OSG_NOTICE<<"GlyphTexture::copyGlyphImage() generating signed distance field."<<std::endl;

    int src_columns = glyph->s();
    int src_rows = glyph->t();
    unsigned char* src_data = glyph->data();

    int dest_columns = _image->s();
    int dest_rows = _image->t();
    unsigned char* dest_data = _image->data(glyph->getTexturePositionX(),glyph->getTexturePositionY());

    int search_distance = glyph->getFontResolution().second/8;

    int left = -search_distance;
    int right = glyph->s()+search_distance;
    int lower = -search_distance;
    int upper = glyph->t()+search_distance;

    float multiplier = 1.0/255.0f;
    float max_distance = sqrtf(float(search_distance*search_distance)*2.0f);

    int num_channels = TEXTURE_IMAGE_NUM_CHANNELS;

    if ((left+glyph->getTexturePositionX())<0) left = -glyph->getTexturePositionX();
    if ((right+glyph->getTexturePositionX())>=dest_columns) right = dest_columns-glyph->getTexturePositionX()-1;

    if ((lower+glyph->getTexturePositionY())<0) lower = -glyph->getTexturePositionY();
    if ((upper+glyph->getTexturePositionY())>=dest_rows) upper = dest_rows-glyph->getTexturePositionY()-1;


    for(int dr=lower; dr<=upper; ++dr)
    {
        for(int dc=left; dc<=right; ++dc)
        {
            unsigned char value = 0;

            unsigned char center_value = 0;
            if (dr>=0 && dr<src_rows && dc>=0 && dc<src_columns) center_value = *(src_data + dr*src_columns + dc);

            float center_value_f = center_value*multiplier;
            float min_distance = FLT_MAX;

            if (center_value>0 && center_value<255)
            {
                if (center_value_f>=0.5f)
                {
                    min_distance = center_value_f-0.5f;
                    value = 128+(min_distance/max_distance)*127;
                }
                else
                {
                    min_distance = 0.5f-center_value_f;
                    value = 127-(min_distance/max_distance)*127;
                }
            }
            else
            {
                for(int radius=1; radius<search_distance; ++radius)
                {
                    for(int span=-radius; span<radius; ++span)
                    {
                        {
                            // left
                            int dx = -radius;
                            int dy = span;

                            int c = dc+dx;
                            int r = dr+dy;

                            unsigned char local_value = 0;
                            if (r>=0 && r<src_rows && c>=0 && c<src_columns) local_value = *(src_data + r*src_columns + c);
                            if (local_value!=center_value)
                            {
                                float local_value_f = float(local_value)*multiplier;

                                float D = sqrtf(float(dx*dx) + float(dy*dy));
                                float local_multiplier = (abs(dx)>abs(dy)) ? D/float(abs(dx)) : D/float(abs(dy));

                                float local_distance = sqrtf(float(radius*radius)+float(span*span));
                                if (center_value==0) local_distance += (0.5f-local_value_f)*local_multiplier;
                                else local_distance += (local_value_f - 0.5f)*local_multiplier;

                                if (local_distance<min_distance) min_distance = local_distance;
                            }
                        }

                        {
                            // top
                            int dx = radius;
                            int dy = span;

                            int c = dc+dx;
                            int r = dr+dy;

                            unsigned char local_value = 0;
                            if (r>=0 && r<src_rows && c>=0 && c<src_columns) local_value = *(src_data + r*src_columns + c);
                            if (local_value!=center_value)
                            {
                                float local_value_f = float(local_value)*multiplier;

                                float D = sqrtf(float(dx*dx) + float(dy*dy));
                                float local_multiplier = (abs(dx)>abs(dy)) ? D/float(abs(dx)) : D/float(abs(dy));

                                float local_distance = sqrtf(float(radius*radius)+float(span*span));
                                if (center_value==0) local_distance += (0.5f-local_value_f)*local_multiplier;
                                else local_distance += (local_value_f - 0.5f)*local_multiplier;

                                if (local_distance<min_distance) min_distance = local_distance;
                            }
                        }

                        {
                            // right
                            int dx = radius;
                            int dy = -span;

                            int c = dc+dx;
                            int r = dr+dy;

                            unsigned char local_value = 0;
                            if (r>=0 && r<src_rows && c>=0 && c<src_columns) local_value = *(src_data + r*src_columns + c);
                            if (local_value!=center_value)
                            {
                                float local_value_f = float(local_value)*multiplier;

                                float D = sqrtf(float(dx*dx) + float(dy*dy));
                                float local_multiplier = (abs(dx)>abs(dy)) ? D/float(abs(dx)) : D/float(abs(dy));

                                float local_distance = sqrtf(float(radius*radius)+float(span*span));
                                if (center_value==0) local_distance += (0.5f-local_value_f)*local_multiplier;
                                else local_distance += (local_value_f - 0.5f)*local_multiplier;

                                if (local_distance<min_distance) min_distance = local_distance;
                            }
                        }

                        {
                            // bottom
                            int dx = -radius;
                            int dy = -span;

                            int c = dc+dx;
                            int r = dr+dy;

                            unsigned char local_value = 0;
                            if (r>=0 && r<src_rows && c>=0 && c<src_columns) local_value = *(src_data + r*src_columns + c);
                            if (local_value!=center_value)
                            {
                                float local_value_f = float(local_value)*multiplier;

                                float D = sqrtf(float(dx*dx) + float(dy*dy));
                                float local_multiplier = (abs(dx)>abs(dy)) ? D/float(abs(dx)) : D/float(abs(dy));

                                float local_distance = sqrtf(float(radius*radius)+float(span*span));
                                if (center_value==0) local_distance += (0.5f-local_value_f)*local_multiplier;
                                else local_distance += (local_value_f - 0.5f)*local_multiplier;

                                if (local_distance<min_distance) min_distance = local_distance;
                            }
                        }
                    }
                }

                if (center_value_f>=0.5)
                {
                    value = 128+(min_distance/max_distance)*127;
                }
                else
                {
                    value = 127-(min_distance/max_distance)*127;
                }
            }


            unsigned char* dest_ptr = dest_data + (dr*dest_columns + dc)*num_channels;
            if (num_channels==4)
            {
                // signed distance field value
                *(dest_ptr++) = value;

                float outline_distance = max_distance/3.0f;

#if 0
                // compute the alpha value of outline, one texel thick
                unsigned char outline = center_value;
                if (center_value<255)
                {
                    if (min_distance<outline_distance-1.0) outline = 255;
                    else if (min_distance<outline_distance) outline = 255*(outline_distance-min_distance);
                    else outline = 0;
                }
#else
                // compute the alpha value of outline, one texel thick
                unsigned char outline = 0;
                if (center_value<255)
                {
                    float inner_outline = outline_distance-1.0f;
                    if (min_distance<inner_outline) outline = 255;
                    else if (min_distance<=outline_distance) outline = (unsigned char)(255.0*(outline_distance-min_distance)/(outline_distance-inner_outline));
                    else outline = 0;
                }
                if (outline>center_value)
                {
                    outline -= center_value;
                }
#endif
                *(dest_ptr++) = outline;


                outline_distance *= 2.0f;

#if 0
                // compute the alpha vlaue of outline two texel thick
                outline = center_value;
                if (center_value<255)
                {
                    if (min_distance<outline_distance-1.0) outline = 255;
                    else if (min_distance<outline_distance) outline = 255*(outline_distance-min_distance);
                    else outline = 0;
                }
#else
                // compute the alpha value of outline, one texel thick
                outline = 0;
                if (center_value<255)
                {
                    float inner_outline = outline_distance-1.0f;
                    if (min_distance<inner_outline) outline = 255;
                    else if (min_distance<=outline_distance) outline = (unsigned char)(255.0*(outline_distance-min_distance)/(outline_distance-inner_outline));
                    else outline = 0;
                }
                if (outline>center_value)
                {
                    outline -= center_value;
                }
#endif
                *(dest_ptr++) = outline;

                // original alpha value from glyph image
                *(dest_ptr) = center_value;
            }
            else
            {
                *(dest_ptr) = value;
            }
        }
    }
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
        OSG_NOTICE<<"GlyphTexture::createImage() : Creating image 0x"<<std::hex<<TEXTURE_IMAGE_FORMAT<<std::dec<<std::endl;

        _image = new osg::Image;

        #if defined(OSG_GL3_AVAILABLE) && !defined(OSG_GL2_AVAILABLE) && !defined(OSG_GL1_AVAILABLE)
        GLenum imageFormat = (_glyphTextureFeatures==GREYSCALE) ? GL_RED : GL_RGBA;
        #else
        GLenum imageFormat = (_glyphTextureFeatures==GREYSCALE) ? GL_ALPHA : GL_RGBA;
        #endif

        _image->allocateImage(getTextureWidth(), getTextureHeight(), 1, imageFormat, GL_UNSIGNED_BYTE);
        memset(_image->data(), 0, _image->getTotalSizeInBytes());

        for(GlyphRefList::iterator itr = _glyphs.begin();
            itr != _glyphs.end();
            ++itr)
        {
            Glyph* glyph = itr->get();
            copyGlyphImage(glyph);
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
