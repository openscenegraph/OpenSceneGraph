/* --------------------------------------------------------------------------
 *
 *    openscenegraph textLib / FTGL
 *
 * --------------------------------------------------------------------------
 *    
 *    prog:    max rheiner;mrn@paus.ch
 *    date:    4/25/2001    (m/d/y)
 *
 * ----------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------
 */


#include <osgText/Font>

#include <osg/Notify>
#include <osgDB/FileUtils>

#include "FTFace.h"
#include "FTGLBitmapFont.h"
#include "FTGLPixmapFont.h"
#include "FTGLOutlineFont.h"
#include "FTGLPolygonFont.h"
#include "FTGLTextureFont.h"


using namespace osg;
using namespace osgText;

std::string findFontFile(const std::string& str)
{
    // try looking in OSGFILEPATH etc first for fonts.
    std::string filename = osgDB::findDataFile(str);
    if (!filename.empty()) return std::string(filename);


    static osgDB::FilePathList s_FontFilePath;
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
    #if defined(WIN32)
        osgDB::Registry::convertStringPathIntoFilePathList(
            ".;C:/winnt/fonts;C:/windows/fonts",
            s_FontFilePath);

        char *ptr;
        if ((ptr = getenv( "windir" )))
        {
            s_FontFilePath.push_back(ptr);
        }
    #else
        osgDB::Registry::convertStringPathIntoFilePathList(
            ".:/usr/share/fonts/ttf:/usr/share/fonts/ttf/western:/usr/share/fonts/ttf/decoratives",
            s_FontFilePath);
    #endif
    }

    filename = osgDB::findFileInPath(str,s_FontFilePath);
    if (!filename.empty()) return filename;

    osg::notify(osg::WARN)<<"Warning: font file \""<<str<<"\" not found."<<std::endl;    
    return std::string();
}



///////////////////////////////////////////////////////////////////////////////
// Font
Font::
Font()
{
    _init=false;
    _font=NULL;
    _created=false;

    _pointSize=14;
    _textureSize=0;
    _res=72;
}

bool Font::
init(const std::string& font)
{
    _font=NULL;
    _created=false;

    open(font);

    if(_font!=NULL)
        return true; 
    else
        return false;
}

Font::
~Font()
{
    clear();
}

void Font::copyAndInvalidate(Font &dest)
{
    // delete destination's font object
    delete dest._font;
        
    // copy local data to destination object
    dest._init = _init;
    dest._created = _created;
    dest._font = _font;
    dest._fontName = _fontName;
    dest._pointSize = _pointSize;
    dest._res = _res;
    dest._textureSize = _textureSize;

    // invalidate this object
    _init = false;
    _created = false;
    _font = 0;
    _fontName = std::string();
}

bool Font::
open(const std::string& font)
{
    clear();
        
    std::string filename = findFontFile(font);
    if (filename.empty()) return false;

    _font=createFontObj();
    if( _font!=NULL && _font->Open(filename.c_str()) )
    {
        _init=true;
        _fontName=font;
        return true;
    }
    else
        return false;
}

bool  Font::open(const char* font)
{
    return open(std::string(font));
}

bool Font::
create(osg::State& state,int pointSize,unsigned int res)
{
    _pointSize=pointSize;
    _res=res;

    return create(state);
}

bool  Font::create(osg::State& state)
{
    if(_init)
    {
        if(_font->Created(state.getContextID()))
            return true;

        if(_font->FaceSize(_pointSize,_res,state.getContextID()))
        {
            _created=true;
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

void Font::output(osg::State& state,const char* text) const
{
    if(_created)
        _font->render(text,state.getContextID());
    else
    {
        // ahhhh, this is bit doddy, the draw is potentially
        // modifying the text object, this isn't thread safe.
        Font* this_non_const = const_cast<Font*>(this);
        this_non_const->create(state,_pointSize);
    }
}

void  Font::clear()
{
    _init=false;
    
    if(_font)
    {
        delete _font;
        _font=NULL;
    }

    _fontName="";
}

float Font::
getWidth(const char* text)  const
{
    if(_init && _created)
        return _font->Advance(text);
    else
        return -1;
}

int Font::
getHeight()  const
{
    if(_init && _created)
        return _pointSize;
    else
        return -1;
}

int Font::
getDescender() const
{
    if(_init && _created)
        return _font->Descender();
    else
        return -1;
}

int Font::
getAscender() const
{
    if(_init && _created)
        return _font->Ascender();
    else
        return -1;
}


// Font
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// BitmapFont

BitmapFont::
BitmapFont(const std::string&    font, 
           int                    point_size):
RasterFont()
{
    if(init(font))
    {
    }
    _pointSize=point_size;
}

FTFont*  BitmapFont::
createFontObj(void)
{
    return (FTFont*)(new FTGLBitmapFont);
}
    
// BitmapFont
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PixmapFont

PixmapFont::
PixmapFont(const std::string&    font, 
            int                    point_size):
RasterFont(font)
{
    if(init(font))
    {
    }
    _pointSize=point_size;
}
    

FTFont*  PixmapFont::
createFontObj(void)
{
    return (FTFont*)(new FTGLPixmapFont);
}
    
// PixmapFont
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// TextureFont

TextureFont::
TextureFont(const std::string&    font, 
            int                    point_size):
RasterFont(font)
{
    _textureSize=0;
    if(init(font))
    {
    }
    _pointSize=point_size;
}


TextureFont::
TextureFont(const std::string&    font, 
            int                    point_size,
            int textureSize ):
RasterFont(font)
{
    _textureSize=textureSize;
    if(init(font))
    {
    }
    _pointSize=point_size;
}
    


FTFont*  TextureFont::
createFontObj(void)
{
    return (FTFont*)(new FTGLTextureFont(_textureSize));
}
    
// TextureFont
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// _FTGLOutlineFont

OutlineFont::
OutlineFont(const std::string&    font, 
            int                    point_size,
            double                precision):
VectorFont(font)
{
    if(init(font))
    {
    }
    _pointSize=point_size;
    _precision=precision;
}
    

FTFont*  OutlineFont::
createFontObj(void)
{
    return (FTFont*)(new FTGLOutlineFont);
}
    
// _FTGLOutlineFont
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PolygonFont

PolygonFont::
PolygonFont(const std::string&    font, 
            int                    point_size,
            double                precision):
VectorFont(font)
{
    if(init(font))
    {
    }
    _pointSize=point_size;
    _precision=precision;
}
    
PolygonFont::
PolygonFont(const char*    font, 
            int                    point_size,
            double                precision):
VectorFont(std::string(font))
{
    if(init(font))
    {
    }
    _pointSize=point_size;
    _precision=precision;
}
    
FTFont*  PolygonFont::
createFontObj(void)
{
    return (FTFont*)(new FTGLPolygonFont);
}
    

// PolygonFont
///////////////////////////////////////////////////////////////////////////////
