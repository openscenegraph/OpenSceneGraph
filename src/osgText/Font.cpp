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

// define the default paths to look for fonts.
// note delimator is : for unix, ; for windows.
#if defined(__linux) || defined(__FreeBSD__) || defined (__sgi) || defined (__DARWIN_OSX__)
    static char* s_FontFilePath = ".:/usr/share/fonts/ttf:/usr/share/fonts/ttf/western:/usr/share/fonts/ttf/decoratives";
#elif defined(WIN32)
    static char* s_FontFilePath = ".;C:/windows/fonts";
#else
    static char* s_FontFilePath = ".:";
#endif

std::string findFontFile(const std::string& str)
{
    // try looking in OSGFILEPATH etc first for fonts.
    char* filename = osgDB::findFile(str.c_str());
    if (filename) return std::string(filename);

    // else fallback into the standard font file paths.
    if (s_FontFilePath)
    {
        filename = osgDB::findFileInPath(str.c_str(),s_FontFilePath);
        if (filename) return std::string(filename);
    }
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

bool  Font::
open(const char* font)
 { return open(std::string(font)); }

bool Font::
create(osg::State& state,int pointSize,const unsigned int res)
{
    _pointSize=pointSize;
    _res=res;

    return create(state);
}

bool  Font::
create(osg::State& state)
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

void Font::
output(osg::State& state,const char* text)
{
    if(_created)
        _font->render(text,state.getContextID());
    else
        create(state,_pointSize);
}

void  Font::
clear()
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
