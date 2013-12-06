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

#include "FreeTypeFont.h"
#include "FreeTypeLibrary.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_BBOX_H

#include <osg/Notify>
#include <osg/io_utils>
#include <osgDB/WriteFile>

namespace FreeType
{

struct Char3DInfo
{
    Char3DInfo(int numSteps):
        _verts( new osg::Vec3Array ),
        _geometry( new osg::Geometry ),
        _numSteps(numSteps),
        _maxY(-FLT_MAX),
        _maxX(-FLT_MAX),
        _minX(FLT_MAX),
        _minY(FLT_MAX),
        _coord_scale(1.0/64.0)
    {
        _geometry->setVertexArray(_verts.get());
    }

    ~Char3DInfo()
    {
    }

    void completeCurrentPrimitiveSet()
    {
        if (_currentPrimitiveSet.valid() && _currentPrimitiveSet->size()>1)
        {
            _geometry->addPrimitiveSet( _currentPrimitiveSet.get() );
        }
        _currentPrimitiveSet = 0;
    }

    osg::Geometry* get()
    {
        completeCurrentPrimitiveSet();

        return _geometry.get();
    }

    void addVertex(osg::Vec3 pos)
    {
        _previous = pos;

        pos *= _coord_scale;

        if (!_verts->empty() && _verts->back()==pos)
        {
            // OSG_NOTICE<<"addVertex("<<pos<<") duplicate, ignoring"<<std::endl;
            return;
        }

        if (!_currentPrimitiveSet)
        {
            _currentPrimitiveSet = new osg::DrawElementsUShort( osg::PrimitiveSet::POLYGON);
            _currentPrimitiveSet->setName("boundary");
        }

        if (!(_currentPrimitiveSet->empty()) &&
            (*_verts)[(*_currentPrimitiveSet)[0]] == pos)
        {
            _currentPrimitiveSet->push_back( (*_currentPrimitiveSet)[0] );
        }
        else
        {
            _currentPrimitiveSet->push_back( _verts->size() );

            _verts->push_back( pos );

            setMinMax(pos);
        }
    }

    void moveTo(const osg::Vec2& pos)
    {
        completeCurrentPrimitiveSet();

        addVertex( osg::Vec3(pos.x(),pos.y(),0) );

    }
    void lineTo(const osg::Vec2& pos)
    {
        addVertex( osg::Vec3(pos.x(),pos.y(),0) );
    }

    void conicTo(const osg::Vec2& control, const osg::Vec2& pos)
    {
        osg::Vec3 p0 = _previous;
        osg::Vec3 p1 = osg::Vec3(control.x(),control.y(),0);
        osg::Vec3 p2 = osg::Vec3(pos.x(),pos.y(),0);

        double dt = 1.0/_numSteps;
        double u=0;
        for (int i=0; i<=_numSteps; ++i)
        {
            double w = 1;
            double bs = 1.0/( (1-u)*(1-u)+2*(1-u)*u*w +u*u );
            osg::Vec3 p = (p0*((1-u)*(1-u)) + p1*(2*(1-u)*u*w) + p2*(u*u))*bs;
            addVertex( p );

            u += dt;
        }
    }

    void cubicTo(const osg::Vec2& control1, const osg::Vec2& control2, const osg::Vec2& pos)
    {
        osg::Vec3 p0 = _previous;
        osg::Vec3 p1 = osg::Vec3(control1.x(),control1.y(),0);
        osg::Vec3 p2 = osg::Vec3(control2.x(),control2.y(),0);
        osg::Vec3 p3 = osg::Vec3(pos.x(),pos.y(),0);

        double cx = 3*(p1.x() - p0.x());
        double bx = 3*(p2.x() - p1.x()) - cx;
        double ax = p3.x() - p0.x() - cx - bx;
        double cy = 3*(p1.y() - p0.y());
        double by = 3*(p2.y() - p1.y()) - cy;
        double ay = p3.y() - p0.y() - cy - by;

        double dt = 1.0/_numSteps;
        double u=0;
        for (int i=0; i<=_numSteps; ++i)
        {
            osg::Vec3 p = osg::Vec3( ax*u*u*u + bx*u*u  + cx*u + p0.x(),ay*u*u*u + by*u*u  + cy*u + p0.y(),0 );
            addVertex( p );

            u += dt;
        }
    }

    void setMinMax(const osg::Vec3& pos)
    {
        _maxY = std::max(_maxY, (double) pos.y());
        _minY = std::min(_minY, (double) pos.y());
        _maxX = std::max(_maxX, (double) pos.x());
        _minX = std::min(_minX, (double) pos.x());
    }

    osg::ref_ptr<osg::Vec3Array>    _verts;
    osg::ref_ptr<osg::DrawElementsUShort> _currentPrimitiveSet;
    osg::ref_ptr<osg::Geometry>     _geometry;
    osg::Vec3                       _previous;
    int                             _numSteps;
    double                          _maxY;
    double                          _maxX;
    double                          _minX;
    double                          _minY;
    double                          _coord_scale;

};


int moveTo( const FT_Vector* to, void* user )
{
    Char3DInfo* char3d = (Char3DInfo*)user;
    char3d->moveTo( osg::Vec2(to->x,to->y) );
    return 0;
}
int lineTo( const FT_Vector* to, void* user )
{
    Char3DInfo* char3d = (Char3DInfo*)user;
    char3d->lineTo( osg::Vec2(to->x,to->y) );
    return 0;
}
int conicTo( const FT_Vector* control,const FT_Vector* to, void* user )
{
    Char3DInfo* char3d = (Char3DInfo*)user;
    char3d->conicTo( osg::Vec2(control->x,control->y), osg::Vec2(to->x,to->y) );
    return 0;
}
int cubicTo( const FT_Vector* control1,const FT_Vector* control2,const FT_Vector* to, void* user )
{
    Char3DInfo* char3d = (Char3DInfo*)user;
    char3d->cubicTo(
        osg::Vec2(control1->x,control1->y),
        osg::Vec2(control2->x,control2->y),
        osg::Vec2(to->x,to->y) );
    return 0;
}

}

FreeTypeFont::FreeTypeFont(const std::string& filename, FT_Face face, unsigned int flags):
    _currentRes(osgText::FontResolution(0,0)),
    _filename(filename),
    _buffer(0),
    _face(face),
    _flags(flags)
{
    init();
}

FreeTypeFont::FreeTypeFont(FT_Byte* buffer, FT_Face face, unsigned int flags):
    _currentRes(osgText::FontResolution(0,0)),
    _filename(""),
    _buffer(buffer),
    _face(face),
    _flags(flags)
{
    init();
}

FreeTypeFont::~FreeTypeFont()
{
    if(_face)
    {
        FreeTypeLibrary* freeTypeLibrary = FreeTypeLibrary::instance();
        if (freeTypeLibrary)
        {
            // remove myself from the local registry to ensure that
            // not dangling pointers remain
            freeTypeLibrary->removeFontImplmentation(this);

            // free the freetype font face itself
            FT_Done_Face(_face);
            _face = 0;

            // release memory held for FT_Face to work
            if (_buffer)
            {
                delete [] _buffer;
                _buffer = 0;
            }
        }
    }
}

void FreeTypeFont::init()
{
    FT_Error _error;
    _error = FT_Set_Pixel_Sizes(_face, 32, 32);
    if (_error)
    {
        OSG_NOTICE << "FreeTypeFont3D: set pixel sizes failed ..." << std::endl;
        return;
    }
    _currentRes.first = 32;
    _currentRes.second = 32;
}

void FreeTypeFont::setFontResolution(const osgText::FontResolution& fontSize)
{
    if (fontSize==_currentRes) return;

    int width = fontSize.first;
    int height = fontSize.second;
    int maxAxis = std::max(width, height);
    int margin = _facade->getGlyphImageMargin() + (int)((float)maxAxis * _facade->getGlyphImageMarginRatio());

    if ((unsigned int)(width+2*margin) > _facade->getTextureWidthHint() ||
        (unsigned int)(width+2*margin) > _facade->getTextureHeightHint())
    {
        OSG_WARN<<"Warning: FreeTypeFont::setSize("<<width<<","<<height<<") sizes too large,"<<std::endl;

        width = _facade->getTextureWidthHint()-2*margin;
        height = _facade->getTextureHeightHint()-2*margin;

        OSG_WARN<<"         sizes capped ("<<width<<","<<height<<") to fit int current glyph texture size."<<std::endl;
    }

    FT_Error error = FT_Set_Pixel_Sizes( _face,      /* handle to face object  */
                                         width,      /* pixel_width            */
                                         height );   /* pixel_height            */

    if (error)
    {
        OSG_WARN<<"FT_Set_Pixel_Sizes() - error 0x"<<std::hex<<error<<std::dec<<std::endl;
    }
    else
    {
        _currentRes = fontSize;
    }

}

osgText::Glyph* FreeTypeFont::getGlyph(const osgText::FontResolution& fontRes, unsigned int charcode)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(FreeTypeLibrary::instance()->getMutex());

    setFontResolution(fontRes);

    float coord_scale = getCoordScale();

    //
    // GT: fix for symbol fonts (i.e. the Webdings font) as the wrong character are being
    // returned, for symbol fonts in windows (FT_ENCONDING_MS_SYMBOL in freetype) the correct
    // values are from 0xF000 to 0xF0FF not from 0x000 to 0x00FF (0 to 255) as you would expect.
    // Microsoft uses a private field for its symbol fonts
    //
    unsigned int charindex = charcode;
    if (_face->charmap != NULL)
    {
        if (_face->charmap->encoding == FT_ENCODING_MS_SYMBOL)
        {
            charindex |= 0xF000;
        }
    }

    FT_Error error = FT_Load_Char( _face, charindex, FT_LOAD_RENDER|FT_LOAD_NO_BITMAP|_flags );
    if (error)
    {
        OSG_WARN << "FT_Load_Char(...) error 0x"<<std::hex<<error<<std::dec<<std::endl;
        return 0;
    }


    FT_GlyphSlot glyphslot = _face->glyph;

    int pitch = glyphslot->bitmap.pitch;
    unsigned char* buffer = glyphslot->bitmap.buffer;

    unsigned int sourceWidth = glyphslot->bitmap.width;;
    unsigned int sourceHeight = glyphslot->bitmap.rows;

    unsigned int width = sourceWidth;
    unsigned int height = sourceHeight;

    osg::ref_ptr<osgText::Glyph> glyph = new osgText::Glyph(_facade, charcode);

    unsigned int dataSize = width*height;
    unsigned char* data = new unsigned char[dataSize];


    // clear the image to zeros.
    for(unsigned char* p=data;p<data+dataSize;) { *p++ = 0; }

    glyph->setImage(width,height,1,
                    GL_ALPHA,
                    GL_ALPHA,GL_UNSIGNED_BYTE,
                    data,
                    osg::Image::USE_NEW_DELETE,
                    1);

    glyph->setInternalTextureFormat(GL_ALPHA);

    // copy image across to osgText::Glyph image.
    switch(glyphslot->bitmap.pixel_mode)
    {
        case FT_PIXEL_MODE_MONO:
            for(int r=sourceHeight-1;r>=0;--r)
            {
                unsigned char* ptr = buffer+r*pitch;
                for(unsigned int c=0;c<sourceWidth;++c)
                {
                    (*data++)= (ptr[c >> 3] & (1 << (~c & 7))) ? 255 : 0;
                }
            }
            break;


        case FT_PIXEL_MODE_GRAY:
            for(int r=sourceHeight-1;r>=0;--r)
            {
                unsigned char* ptr = buffer+r*pitch;
                for(unsigned int c=0;c<sourceWidth;++c,++ptr)
                {
                    (*data++)=*ptr;
                }
            }
            break;

        default:
            OSG_WARN << "FT_Load_Char(...) returned bitmap with unknown pixel_mode " << glyphslot->bitmap.pixel_mode << std::endl;
    }


    FT_Glyph_Metrics* metrics = &(_face->glyph->metrics);

    glyph->setWidth((float)metrics->width * coord_scale);
    glyph->setHeight((float)metrics->height * coord_scale);
    glyph->setHorizontalBearing(osg::Vec2((float)metrics->horiBearingX * coord_scale,(float)(metrics->horiBearingY-metrics->height) * coord_scale)); // bottom left.
    glyph->setHorizontalAdvance((float)metrics->horiAdvance * coord_scale);
    glyph->setVerticalBearing(osg::Vec2((float)metrics->vertBearingX * coord_scale,(float)(metrics->vertBearingY-metrics->height) * coord_scale)); // top middle.
    glyph->setVerticalAdvance((float)metrics->vertAdvance * coord_scale);

#if 0
    OSG_NOTICE<<"getGlyph("<<charcode<<", "<<char(charcode)<<") _face="<<_face<<", _filename="<<_filename<<std::endl;
    OSG_NOTICE<<"   height="<<glyph->getHeight()<<std::endl;
    OSG_NOTICE<<"   width="<<glyph->getWidth()<<std::endl;
    OSG_NOTICE<<"   horizontalBearing="<<glyph->getHorizontalBearing()<<std::endl;
    OSG_NOTICE<<"   horizontalAdvance="<<glyph->getHorizontalAdvance()<<std::endl;
    OSG_NOTICE<<"   verticalBearing="<<glyph->getHorizontalBearing()<<std::endl;
    OSG_NOTICE<<"   verticalAdvance="<<glyph->getVerticalAdvance()<<std::endl;
    OSG_NOTICE<<"   coord_scale = "<<coord_scale<<std::endl;
    OSG_NOTICE<<"   _face->units_per_EM = "<<_face->units_per_EM<<", scale="<<1.0f/float(_face->units_per_EM)<<std::endl;
#endif

//    cout << "      in getGlyph() implementation="<<this<<"  "<<_filename<<"  facade="<<_facade<<endl;

    return glyph.release();

}

osgText::Glyph3D * FreeTypeFont::getGlyph3D(unsigned int charcode)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(FreeTypeLibrary::instance()->getMutex());

    //
    // GT: fix for symbol fonts (i.e. the Webdings font) as the wrong character are being
    // returned, for symbol fonts in windows (FT_ENCONDING_MS_SYMBOL in freetype) the correct
    // values are from 0xF000 to 0xF0FF not from 0x000 to 0x00FF (0 to 255) as you would expect.
    // Microsoft uses a private field for its symbol fonts
    //
    unsigned int charindex = charcode;
    if (_face->charmap != NULL)
    {
        if (_face->charmap->encoding == FT_ENCODING_MS_SYMBOL)
        {
            charindex |= 0xF000;
        }
    }

    FT_Error error = FT_Load_Char( _face, charindex, FT_LOAD_DEFAULT|_flags );
    if (error)
    {
        OSG_WARN << "FT_Load_Char(...) error 0x"<<std::hex<<error<<std::dec<<std::endl;
        return 0;
    }
    if (_face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
    {
        OSG_WARN << "FreeTypeFont3D::getGlyph : not a vector font" << std::endl;
        return 0;
    }

    float coord_scale = getCoordScale();

    // ** init FreeType to describe the glyph
    FreeType::Char3DInfo char3d(_facade->getNumberCurveSamples());
    char3d._coord_scale = coord_scale;

    FT_Outline outline = _face->glyph->outline;
    FT_Outline_Funcs funcs;
    funcs.conic_to = (FT_Outline_ConicToFunc)&FreeType::conicTo;
    funcs.line_to = (FT_Outline_LineToFunc)&FreeType::lineTo;
    funcs.cubic_to = (FT_Outline_CubicToFunc)&FreeType::cubicTo;
    funcs.move_to = (FT_Outline_MoveToFunc)&FreeType::moveTo;
    funcs.shift = 0;
    funcs.delta = 0;

    // ** record description
    FT_Error _error = FT_Outline_Decompose(&outline, &funcs, &char3d);
    if (_error)
    {
        OSG_WARN << "FreeTypeFont3D::getGlyph : - outline decompose failed ..." << std::endl;
        return 0;
    }

    // ** create geometry for each part of the glyph
    osg::ref_ptr<osg::Geometry> frontGeo(new osg::Geometry);

    osg::ref_ptr<osg::Vec3Array> rawVertices = new osg::Vec3Array(*(char3d._verts));
    osg::Geometry::PrimitiveSetList rawPrimitives;
    for(osg::Geometry::PrimitiveSetList::iterator itr = char3d.get()->getPrimitiveSetList().begin();
        itr != char3d.get()->getPrimitiveSetList().end();
        ++itr)
    {
        rawPrimitives.push_back(dynamic_cast<osg::PrimitiveSet*>((*itr)->clone(osg::CopyOp::DEEP_COPY_ALL)));
    }

    // ** save vertices and PrimitiveSetList of each face in the Glyph3D PrimitiveSet face list
    osg::ref_ptr<osgText::Glyph3D> glyph = new osgText::Glyph3D(_facade, charcode);

    // copy the raw primitive set list before we tessellate it.
    glyph->getRawFacePrimitiveSetList() = rawPrimitives;
    glyph->setRawVertexArray(rawVertices.get());


    FT_Glyph_Metrics* metrics = &(_face->glyph->metrics);

    glyph->setWidth((float)metrics->width * coord_scale);
    glyph->setHeight((float)metrics->height * coord_scale);
    glyph->setHorizontalBearing(osg::Vec2((float)metrics->horiBearingX * coord_scale,(float)(metrics->horiBearingY-metrics->height) * coord_scale)); // bottom left.
    glyph->setHorizontalAdvance((float)metrics->horiAdvance * coord_scale);
    glyph->setVerticalBearing(osg::Vec2((float)metrics->vertBearingX * coord_scale,(float)(metrics->vertBearingY-metrics->height) * coord_scale)); // top middle.
    glyph->setVerticalAdvance((float)metrics->vertAdvance * coord_scale);

#if 0
    OSG_NOTICE<<"getGlyph3D("<<charcode<<", "<<char(charcode)<<")"<<std::endl;
    OSG_NOTICE<<"   height="<<glyph->getHeight()<<std::endl;
    OSG_NOTICE<<"   width="<<glyph->getWidth()<<std::endl;
    OSG_NOTICE<<"   horizontalBearing="<<glyph->getHorizontalBearing()<<std::endl;
    OSG_NOTICE<<"   horizontalAdvance="<<glyph->getHorizontalAdvance()<<std::endl;
    OSG_NOTICE<<"   verticalBearing="<<glyph->getHorizontalBearing()<<std::endl;
    OSG_NOTICE<<"   verticalAdvance="<<glyph->getVerticalAdvance()<<std::endl;
#endif

    FT_BBox ftbb;
    FT_Outline_Get_BBox(&outline, &ftbb);
    osg::BoundingBox bb(float(ftbb.xMin) * coord_scale, float(ftbb.yMin) * coord_scale, 0.0f, float(ftbb.xMax) * coord_scale, float(ftbb.yMax) * coord_scale, 0.0f);

    glyph->setBoundingBox(bb);

    return glyph.release();
}

osg::Vec2 FreeTypeFont::getKerning(unsigned int leftcharcode,unsigned int rightcharcode, osgText::KerningType kerningType)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(FreeTypeLibrary::instance()->getMutex());

    if (!FT_HAS_KERNING(_face) || (kerningType == osgText::KERNING_NONE)) return osg::Vec2(0.0f,0.0f);

    FT_Kerning_Mode mode = (kerningType==osgText::KERNING_DEFAULT) ? ft_kerning_default : ft_kerning_unfitted;

    // convert character code to glyph index
    FT_UInt left = FT_Get_Char_Index( _face, leftcharcode );
    FT_UInt right = FT_Get_Char_Index( _face, rightcharcode );

    // get the kerning distances.
    FT_Vector  kerning;

    FT_Error error = FT_Get_Kerning( _face,                     // handle to face object
                                     left,                      // left glyph index
                                     right,                     // right glyph index
                                     mode,                      // kerning mode
                                     &kerning );                // target vector

    if (error)
    {
        OSG_WARN << "FT_Get_Kerning(...) returned error code " <<std::hex<<error<<std::dec<< std::endl;
        return osg::Vec2(0.0f,0.0f);
    }

    float coord_scale = getCoordScale();

    return osg::Vec2((float)kerning.x*coord_scale,(float)kerning.y*coord_scale);
}

bool FreeTypeFont::hasVertical() const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(FreeTypeLibrary::instance()->getMutex());
    return FT_HAS_VERTICAL(_face)!=0;
}

bool FreeTypeFont::getVerticalSize(float & ascender, float & descender) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(FreeTypeLibrary::instance()->getMutex());
#if 0
    if(_face->units_per_EM != 0)
    {
        float coord_scale = 1.0f/static_cast<float>(_face->units_per_EM);
        ascender = static_cast<float>(_face->ascender) * coord_scale;
        descender = static_cast<float>(_face->descender) * coord_scale;
        return true;
    }
    else
    {
        return false;
    }
#else
    float coord_scale = getCoordScale();
    ascender = static_cast<float>(_face->ascender) * coord_scale;
    descender = static_cast<float>(_face->descender) * coord_scale;
    return true;
#endif
}

float FreeTypeFont::getCoordScale() const
{
    //float coord_scale = _freetype_scale/64.0f;
    //float coord_scale = 1.0f/64.0f;
    float coord_scale = 1.0f/(float(_currentRes.second)*64.0f);
    return coord_scale;
}
