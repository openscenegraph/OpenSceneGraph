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

#include <freetype/ftoutln.h>
#include <freetype/ftbbox.h>

#include <osg/Notify>  
#include <osgDB/WriteFile>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Tessellator>

namespace FreeType
{

struct Char3DInfo
{
    Char3DInfo(int numSteps):
        _verts( new osg::Vec3Array ),
        _geometry( new osg::Geometry ),
        _idx(0),
        _numSteps(numSteps),
        _maxY(-FLT_MAX),
        _maxX(-FLT_MAX),
        _minX(FLT_MAX),
        _minY(FLT_MAX)
    {
    }
    ~Char3DInfo()
    {
    }

    osg::Geometry* get()
    {
        int len = _verts->size()-_idx;
        if (len)
        {
            _geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::POLYGON, _idx, len ) );
            _idx = _verts->size();
        }

        _geometry->setVertexArray(_verts.get());
        return _geometry.get();
    }

    void addVertex(const osg::Vec3& pos)
    {
        if (!_verts->empty() && _verts->back()==pos)
        {
            // OSG_NOTICE<<"addVertex("<<pos<<") duplicate, ignoring"<<std::endl;
            return;
        }

        _verts->push_back( pos );
        setMinMax(pos);
    }

    void moveTo(const osg::Vec2& pos)
    {
        if (_verts->size())
        {
            int len = _verts->size()-_idx;
            _geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::POLYGON, _idx, len ) );
        }
        _idx = _verts->size();
        addVertex( osg::Vec3(pos.x(),pos.y(),0) );

    }
    void lineTo(const osg::Vec2& pos)
    {
        addVertex( osg::Vec3(pos.x(),pos.y(),0) );
    }
    void conicTo(const osg::Vec2& control, const osg::Vec2& pos)
    {
        osg::Vec3 p0 = _verts->back();
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
        osg::Vec3 p0 = _verts->back();
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
    osg::ref_ptr<osg::Geometry>     _geometry;
    int                             _idx;
    int                             _numSteps;
    double                          _maxY;
    double                          _maxX;
    double                          _minX;
    double                          _minY;
};


#define FT_NUM(x) (x/64.0)
int moveTo( const FT_Vector* to, void* user )
{
    Char3DInfo* char3d = (Char3DInfo*)user;
    char3d->moveTo( osg::Vec2(FT_NUM(to->x),FT_NUM(to->y)) );
    return 0;
}
int lineTo( const FT_Vector* to, void* user )
{
    Char3DInfo* char3d = (Char3DInfo*)user;
    char3d->lineTo( osg::Vec2(FT_NUM(to->x),FT_NUM(to->y)) );
    return 0;
}
int conicTo( const FT_Vector* control,const FT_Vector* to, void* user )
{
    Char3DInfo* char3d = (Char3DInfo*)user;
    char3d->conicTo( osg::Vec2(FT_NUM(control->x),FT_NUM(control->y)), osg::Vec2(FT_NUM(to->x),FT_NUM(to->y)) );
    return 0;
}
int cubicTo( const FT_Vector* control1,const FT_Vector* control2,const FT_Vector* to, void* user )
{
    Char3DInfo* char3d = (Char3DInfo*)user;
    char3d->cubicTo(
        osg::Vec2(FT_NUM(control1->x),FT_NUM(control1->y)),
        osg::Vec2(FT_NUM(control2->x),FT_NUM(control2->y)),
        osg::Vec2(FT_NUM(to->x),FT_NUM(to->y)) );
    return 0;
}
#undef FT_NUM

}

FreeTypeFont::FreeTypeFont(const std::string& filename, FT_Face face, unsigned int flags):
    _currentRes(osgText::FontResolution(0,0)),
    _filename(filename),
    _buffer(0),
    _face(face),
    _flags(flags),
    _scale(1.0f)
{
    init();
}

FreeTypeFont::FreeTypeFont(FT_Byte* buffer, FT_Face face, unsigned int flags):
    _currentRes(osgText::FontResolution(0,0)),
    _filename(""),
    _buffer(buffer),
    _face(face),
    _flags(flags),
    _scale(1.0f)
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

    FT_Error _error = FT_Set_Pixel_Sizes(_face, 32, 32);
    if (_error)
    {
        OSG_NOTICE << "FreeTypeFont3D: set pixel sizes failed ..." << std::endl;
        return;
    }

    FT_Set_Char_Size( _face, 64*64, 64*64, 600, 600);

    int glyphIndex = FT_Get_Char_Index( _face, 'M' );
    _error = FT_Load_Glyph( _face, glyphIndex, FT_LOAD_DEFAULT );
    if (_error)
    {
        OSG_NOTICE << "FreeTypeFont3D: initial glyph load failed ..." << std::endl;
        return;
    }

    if (_face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
    {
        OSG_NOTICE << "FreeTypeFont3D: not a vector font" << std::endl;
        return;
    }

    {
        FreeType::Char3DInfo char3d(10);

        FT_Outline outline = _face->glyph->outline;
        FT_Outline_Funcs funcs;
        funcs.conic_to = (FT_Outline_ConicToFunc)&FreeType::conicTo;
        funcs.line_to = (FT_Outline_LineToFunc)&FreeType::lineTo;
        funcs.cubic_to = (FT_Outline_CubicToFunc)&FreeType::cubicTo;
        funcs.move_to = (FT_Outline_MoveToFunc)&FreeType::moveTo;
        funcs.shift = 0;
        funcs.delta = 0;
        _error = FT_Outline_Decompose(&outline,&funcs,&char3d);
        if (_error)
        {
            OSG_NOTICE << "FreeTypeFont3D: - outline decompose failed ..." << std::endl;
            return;
        }

        FT_BBox bb;
        FT_Outline_Get_BBox(&outline,&bb);

        long ymin = ft_floor( bb.yMin );
        long ymax = ft_ceiling( bb.yMax );
        double height = double(ymax - ymin)/64.0;

        // long xmin = ft_floor( bb.xMin );
        // long xmax = ft_ceiling( bb.xMax );
        // double width = (xmax - xmin)/64.0;

        _scale = 1.0/height;
    }
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

    osg::ref_ptr<osgText::Glyph> glyph = new osgText::Glyph(charcode);
    
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


    FT_Glyph_Metrics* metrics = &(glyphslot->metrics);

    glyph->setHorizontalBearing(osg::Vec2((float)metrics->horiBearingX/64.0f,(float)(metrics->horiBearingY-metrics->height)/64.0f)); // bottom left.
    glyph->setHorizontalAdvance((float)metrics->horiAdvance/64.0f);
    glyph->setVerticalBearing(osg::Vec2((float)metrics->vertBearingX/64.0f,(float)(metrics->vertBearingY-metrics->height)/64.0f)); // top middle.
    glyph->setVerticalAdvance((float)metrics->vertAdvance/64.0f);

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

    // ** init FreeType to describe the glyph
    FreeType::Char3DInfo char3d(_facade->getNumberCurveSamples());

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

    frontGeo->setVertexArray(char3d.get()->getVertexArray());
    frontGeo->setPrimitiveSetList(char3d.get()->getPrimitiveSetList());

    osg::ref_ptr<osg::Geometry> wallGeo(new osg::Geometry);
    wallGeo->setVertexArray(frontGeo->getVertexArray());

    osg::ref_ptr<osg::Geometry> backGeo(new osg::Geometry);
    backGeo->setVertexArray(frontGeo->getVertexArray());



    // ** for convenience.
    osg::Vec3Array * vertices = char3d._verts.get();



    // ** duplicate the vertex for the back face
    // ** with a depth equal to the font depth
    std::size_t len = vertices->size();
    std::size_t dlen = len * 2;

    vertices->reserve(dlen);

    osg::Vec3Array::iterator begin = vertices->begin();
    osg::Vec3Array::iterator it = vertices->begin();

    for (std::size_t i = 0; i != len; ++i, ++it)
        vertices->push_back(*it);
//    std::copy(begin, begin + len, begin + len + 1); TOCHECK


    // ** and decal new vertices
    unsigned int depth = _facade->getFontDepth();
    for (std::size_t i = len; i != dlen; ++i)
    {
        (*vertices)[i].z() -= depth;
    }

    osg::Vec3Array::iterator end;

    // ** create wall and back face from the front polygon
    // **  then accumulate them in the appropriate geometry wallGeo and backGeo
    for (std::size_t i=0; i < frontGeo->getNumPrimitiveSets(); ++i)
    {
        // ** get the front polygon
        osg::ref_ptr<osg::DrawArrays> daFront(dynamic_cast<osg::DrawArrays*>(frontGeo->getPrimitiveSet(i)));
        unsigned int idx = daFront->getFirst();
        unsigned int cnt = daFront->getCount();

        // ** reverse vertices to draw the front face in the CCW
        std::reverse(begin + idx, begin + idx + cnt);

        // ** create the back polygon
        osg::ref_ptr<osg::DrawArrays> daBack(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, idx + len, cnt));
        backGeo->addPrimitiveSet(daBack.get());


        // ** create the wall triangle strip
        osg::ref_ptr<osg::DrawElementsUInt> deWall(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP));
        wallGeo->addPrimitiveSet(deWall.get());

        // **  link triangle strip
        deWall->push_back(idx + len);
        for (unsigned int j = 1; j < cnt; ++j)
        {
            deWall->push_back(idx + cnt - j);
            deWall->push_back(idx + len + j);
        }
        deWall->push_back(idx);
        deWall->push_back(idx + len);
        deWall->push_back(idx + cnt - 1);
    }

    // ** tesselate front and back face
    {
        osgUtil::Tessellator ts;
        ts.setWindingType(osgUtil::Tessellator::TESS_WINDING_POSITIVE);
        ts.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        ts.retessellatePolygons(*frontGeo);
    }

    {
        osgUtil::Tessellator ts;
        ts.setWindingType(osgUtil::Tessellator::TESS_WINDING_POSITIVE);
        ts.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        ts.retessellatePolygons(*backGeo);
    }

    // ** generate normal
    {
        osgUtil::SmoothingVisitor sm;
        osg::ref_ptr<osg::Geode> geode(new osg::Geode);
        geode->addDrawable(wallGeo.get());
        geode->accept(sm);
    }

    // ** save vertices and PrimitiveSetList of each face in the Glyph3D PrimitiveSet face list
    osg::ref_ptr<osgText::Glyph3D> glyph3D = new osgText::Glyph3D(charcode);

    // copy the raw primitive set list before we tessellate it.
    glyph3D->getRawFacePrimitiveSetList() = rawPrimitives;
    glyph3D->setRawVertexArray(rawVertices.get());

    glyph3D->setVertexArray(dynamic_cast<osg::Vec3Array*>(frontGeo->getVertexArray()));
    glyph3D->setNormalArray(dynamic_cast<osg::Vec3Array*>(wallGeo->getNormalArray()));

    glyph3D->getFrontPrimitiveSetList() = frontGeo->getPrimitiveSetList();
    glyph3D->getWallPrimitiveSetList() = wallGeo->getPrimitiveSetList();
    glyph3D->getBackPrimitiveSetList() = backGeo->getPrimitiveSetList();


    FT_Glyph_Metrics* metrics = &(_face->glyph->metrics);

    glyph3D->setHorizontalBearing(osg::Vec2((float)metrics->horiBearingX/64.0f,(float)(metrics->horiBearingY-metrics->height)/64.0f)); // bottom left.
    glyph3D->setHorizontalAdvance((float)metrics->horiAdvance/64.0f);
    glyph3D->setVerticalBearing(osg::Vec2((float)metrics->vertBearingX/64.0f,(float)(metrics->vertBearingY-metrics->height)/64.0f)); // top middle.
    glyph3D->setVerticalAdvance((float)metrics->vertAdvance/64.0f);

    glyph3D->setWidth((float)metrics->width / 64.0f);
    glyph3D->setHeight((float)metrics->height / 64.0f);


    FT_BBox ftbb;
    FT_Outline_Get_BBox(&outline, &ftbb);

    long xmin = ft_floor( ftbb.xMin );
    long xmax = ft_ceiling( ftbb.xMax );
    long ymin = ft_floor( ftbb.yMin );
    long ymax = ft_ceiling( ftbb.yMax );

    osg::BoundingBox bb(xmin / 64.0f, ymin / 64.0f, 0.0f, xmax / 64.0f, ymax / 64.0f, 0.0f);

    glyph3D->setBoundingBox(bb);

    return glyph3D.release();
}

osg::Vec2 FreeTypeFont::getKerning(const osgText::FontResolution& fontRes, unsigned int leftcharcode,unsigned int rightcharcode, osgText::KerningType kerningType)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(FreeTypeLibrary::instance()->getMutex());

    if (!FT_HAS_KERNING(_face) || (kerningType == osgText::KERNING_NONE)) return osg::Vec2(0.0f,0.0f);

    setFontResolution(fontRes);

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

    return osg::Vec2((float)kerning.x/64.0f,(float)kerning.y/64.0f);
}

bool FreeTypeFont::hasVertical() const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(FreeTypeLibrary::instance()->getMutex());
    return FT_HAS_VERTICAL(_face)!=0;
}
