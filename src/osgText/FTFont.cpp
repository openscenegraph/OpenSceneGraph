#include    "FTFace.h"
#include    "FTFont.h"
#include    "FTGlyphContainer.h"
#include    "FTGL.h"

#include <osg/DisplaySettings>

// mrn@changes
FTFont::FTFont():
    face(),
    numFaces(0),
    numGlyphs(0),
    err(0)
{


    _contextGlyphList.resize(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),NULL);

    pen.x = 0;
    pen.y = 0;
}


FTFont::~FTFont()
{
    Close();
}


bool FTFont::Open( const char* fontname )
{
    if( face.Open( fontname))
    {
        FT_Face* ftFace = face.Face();        
        numGlyphs = (*ftFace)->num_glyphs;
        
        return true;
    }
    else
    {
        err = face.Error();
        return false;
    }
}


// mrn@changes
void FTFont::Close()
{
    GlyphContextContainer::iterator    itr;
    for(itr=_contextGlyphList.begin();itr!=_contextGlyphList.end();itr++)
        delete *itr;
    _contextGlyphList.clear();
}

// mrn@changes
bool FTFont::FaceSize( const unsigned int size, const unsigned int res , unsigned int renderContext)
{
    charSize = face.Size( size, res);

    // check the context
    if (_contextGlyphList.size() <= renderContext) 
        _contextGlyphList.resize(renderContext,NULL);

    FTGlyphContainer*& glyphList=_contextGlyphList[renderContext];
    
    if( glyphList)
        delete glyphList;
    
    glyphList = new FTGlyphContainer( &face, numGlyphs);
    
    if( MakeGlyphList(renderContext))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool FTFont::Created(unsigned int renderContext)
{
    if(renderContext < _contextGlyphList.size())
        return (_contextGlyphList[renderContext] != NULL);
    else 
        return false;
}

bool FTFont::CharMap( FT_Encoding encoding)
{
    err = face.CharMap( encoding);
    return !err;
}


int    FTFont::Ascender() const
{
    return charSize.Ascender();
}


int    FTFont::Descender() const
{
    return charSize.Descender();
}


// mrn@changes
float FTFont::Advance( const wchar_t* string)
{
    // all are the same, a bit a hack
    FTGlyphContainer* glyphList=_contextGlyphList[0];

    const wchar_t* c = string; // wchar_t IS unsigned?
    float width = 0;

    while( *c)
    {
        width += glyphList->Advance( *c, *(c + 1));    
        ++c;
    }

    return width;
}


// mrn@changes
float FTFont::Advance( const char* string)
{
    // all are the same, a bit a hack
    FTGlyphContainer* glyphList=_contextGlyphList[0];

    const unsigned char* c = (unsigned char*)string; // This is ugly, what is the c++ way?
    float width = 0;

    while( *c)
    {
        width += glyphList->Advance( *c, *(c + 1));    
        ++c;
    }

    return width;
}


// mrn@changes
void FTFont::render( const char* string , unsigned int renderContext)
{
    FTGlyphContainer* glyphList=_contextGlyphList[renderContext];

    const unsigned char* c = (unsigned char*)string; // This is ugly, what is the c++ way?
    FT_Vector kernAdvance;
    pen.x = 0; pen.y = 0;

    while( *c)
    {
        kernAdvance = glyphList->render( *c, *(c + 1), pen);
        
        pen.x += kernAdvance.x;
        pen.y += kernAdvance.y;
        
        ++c;
    }
}


// mrn@changes
void FTFont::render( const wchar_t* string , unsigned int renderContext)
{
    FTGlyphContainer* glyphList=_contextGlyphList[renderContext];

    const wchar_t* c = string; // wchar_t IS unsigned?
    FT_Vector kernAdvance;
    pen.x = 0; pen.y = 0;

    while( *c)
    {
        kernAdvance = glyphList->render( *c, *(c + 1), pen);
        
        pen.x += kernAdvance.x;
        pen.y += kernAdvance.y;
        
        ++c;
    }
}
