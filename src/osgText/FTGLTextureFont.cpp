#include    "FTGLTextureFont.h"
#include    "FTGlyphContainer.h"
#include    "FTTextureGlyph.h"

#include <osg/DisplaySettings>

inline GLuint NextPowerOf2( GLuint in)
{
     in -= 1;

     in |= in >> 16;
     in |= in >> 8;
     in |= in >> 4;
     in |= in >> 2;
     in |= in >> 1;

     return in + 1;
}


FTGLTextureFont::FTGLTextureFont()
:    maxTextSize(0),
    textureWidth(0),
    textureHeight(0),
    numTextures(1),
    textMem(0),
    glyphHeight(0),
    glyphWidth(0),
    padding(1)
{

    glContextTextureID.resize(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),0);
}

FTGLTextureFont::FTGLTextureFont(int textureSize)
:    maxTextSize(textureSize),
    textureWidth(0),
    textureHeight(0),
    numTextures(1),
    textMem(0),
    glyphHeight(0),
    glyphWidth(0),
    padding(1)
{
    glContextTextureID.resize(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),0);
}

FTGLTextureFont::~FTGLTextureFont()
{
    ContextTextureId::iterator itr;
    for(itr=glContextTextureID.begin();itr != glContextTextureID.end(); itr++)
    {
        if (*itr)
        {
            glDeleteTextures( numTextures, (const GLuint*)*itr);
            osgDelete [] *itr;
        }
    }
}


// mrn@changes
bool FTGLTextureFont::MakeGlyphList(unsigned int renderContext)
{
    // FTGlyphContainer* glyphList=_contextGlyphList[renderContext];

    // check the context
    if (glContextTextureID.size() <= renderContext) 
        glContextTextureID.resize(renderContext,0);
 
    unsigned long* glTextureID=glContextTextureID[renderContext];
    if(glTextureID)
        return true;
    else
    {
        glTextureID= osgNew unsigned long[16];
        memset(glTextureID,0,sizeof(unsigned long)*16);
        glContextTextureID[renderContext]=glTextureID;
    }

    if( !maxTextSize)
        glGetIntegerv( GL_MAX_TEXTURE_SIZE, (GLint*)&maxTextSize);
        
    glyphHeight = ( charSize.Height()) + padding;
    glyphWidth = ( charSize.Width()) + padding;
    
    GetSize();
    GLuint totalMem;

    if( textureHeight > (maxTextSize-padding*2))
    {
        numTextures = static_cast<int>( textureHeight / (maxTextSize-padding*2)) + 1;
        if( numTextures > 15) // FIXME
            numTextures = 15;
        
        GLsizei heightRemain = NextPowerOf2( textureHeight % (maxTextSize-padding*2));
        totalMem = ((maxTextSize * ( numTextures - 1)) + heightRemain) * textureWidth;

        glGenTextures( numTextures, (GLuint*)&glTextureID[0]);

        textMem = osgNew unsigned char[totalMem]; // GL_ALPHA texture;
        memset( textMem, 0, totalMem);
            
        unsigned int glyphNum = 0;
        unsigned char* currTextPtr = textMem;
        
        for( int x = 0; x < numTextures - 1; ++x)
        {
            glyphNum = FillGlyphs( glyphNum, glTextureID[x], textureWidth, maxTextSize, currTextPtr,renderContext);
            
            CreateTexture( glTextureID[x], textureWidth, maxTextSize, currTextPtr);
            
            currTextPtr += ( textureWidth * maxTextSize);
            ++glyphNum;
        }
        
        glyphNum = FillGlyphs( glyphNum, glTextureID[numTextures - 1], textureWidth, heightRemain, currTextPtr,renderContext);
        CreateTexture( glTextureID[numTextures - 1], textureWidth, heightRemain, currTextPtr);
    }
    else
    {

        textureHeight = NextPowerOf2( textureHeight+padding*2);
        totalMem = textureWidth * textureHeight;
        
        glGenTextures( numTextures, (GLuint*)&glTextureID[0]);

        textMem = osgNew unsigned char[totalMem]; // GL_ALPHA texture;
        memset( textMem, 0, totalMem);

        FillGlyphs( 0, glTextureID[0], textureWidth, textureHeight, textMem,renderContext);
        CreateTexture( glTextureID[0], textureWidth, textureHeight, textMem);
    }

    osgDelete [] textMem;
    return !err;
}

// mrn@changes
unsigned int FTGLTextureFont::FillGlyphs( unsigned int glyphStart, GLuint id, GLsizei width, GLsizei height, unsigned char* textdata, unsigned int renderContext)
{
    FTGlyphContainer* glyphList=_contextGlyphList[renderContext];

    int currentTextX = padding;
    int currentTextY = padding;// + padding;
    
    float currTextU = (float)padding / (float)width;
    float currTextV = (float)padding / (float)height;
    
    unsigned int n;
        
    for( n = glyphStart; n <= numGlyphs; ++n)
    {
        FT_Glyph* ftGlyph = face.Glyph( n, FT_LOAD_NO_HINTING);
        
        if( ftGlyph)
        {
            unsigned char* data = textdata + (( currentTextY * width) + currentTextX);
            
            currTextU = (float)currentTextX / (float)width;
            
            FTTextureGlyph* tempGlyph = osgNew FTTextureGlyph( *ftGlyph, id, data, width, height, currTextU, currTextV);
            glyphList->Add( tempGlyph);

            currentTextX += glyphWidth;
            if( currentTextX > ( width - glyphWidth))
            {
                currentTextY += glyphHeight;
                if( currentTextY > ( height - glyphHeight))
                    return n;
                    
                currentTextX = padding;
                currTextV = (float)currentTextY / (float)height;
            }
        }
        else
        {
            err = face.Error();
        }
    }


    return n;
}


void FTGLTextureFont::GetSize()
{
    //work out the max width. Most likely maxTextSize
    textureWidth = NextPowerOf2( (numGlyphs * glyphWidth) + padding*2);
    if( textureWidth > maxTextSize)
    {
        textureWidth = maxTextSize;
    }
    
    int h = static_cast<int>( (textureWidth-padding*2) / glyphWidth);
        
    textureHeight = (( numGlyphs / h) + 1) * glyphHeight;
}


// mrn@changes
void FTGLTextureFont::CreateTexture( GLuint id, GLsizei width, GLsizei height, unsigned char* data)
{
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1); //What does this do exactly?
    glBindTexture( GL_TEXTURE_2D, id);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
}


// mrn@changes
void FTGLTextureFont::render( const char* string, unsigned int renderContext)
{    
    glPushAttrib( GL_ENABLE_BIT | GL_HINT_BIT | GL_LINE_BIT | GL_PIXEL_MODE_BIT);
    
    glEnable(GL_BLEND);
     glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // GL_ONE
     
     FTFont::render( string,renderContext);

    glPopAttrib();
}


// mrn@changes
void FTGLTextureFont::render( const wchar_t* string, unsigned int renderContext)
{    
    glPushAttrib( GL_ENABLE_BIT | GL_HINT_BIT | GL_LINE_BIT | GL_PIXEL_MODE_BIT);
    
    glEnable(GL_BLEND);
     glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // GL_ONE
     
     FTFont::render( string,renderContext);
    
    glPopAttrib();
}

