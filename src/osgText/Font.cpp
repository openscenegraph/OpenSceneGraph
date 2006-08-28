;
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
#include <osg/ApplicationUsage>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osg/GLU>

using namespace osgText;
using namespace std;

static osg::ApplicationUsageProxy Font_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_TEXT_INCREMENTAL_SUBLOADING <type>","ON | OFF");


std::string osgText::findFontFile(const std::string& str)
{
    // try looking in OSGFILEPATH etc first for fonts.
    std::string filename = osgDB::findDataFile(str);
    if (!filename.empty()) return filename;


    static osgDB::FilePathList s_FontFilePath;
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
    #if defined(WIN32)
        osgDB::convertStringPathIntoFilePathList(
            ".;C:/winnt/fonts;C:/windows/fonts",
            s_FontFilePath);

        char *ptr;
        if ((ptr = getenv( "windir" )))
        {
            std::string winFontPath = ptr;
            winFontPath += "\\fonts";
            s_FontFilePath.push_back(winFontPath);
        }
    #else
        osgDB::convertStringPathIntoFilePathList(
            ".:/usr/share/fonts/ttf:/usr/share/fonts/ttf/western:/usr/share/fonts/ttf/decoratives",
            s_FontFilePath);
    #endif
    }

    filename = osgDB::findFileInPath(str,s_FontFilePath);
    if (!filename.empty()) return filename;

    // Try filename without pathname, if it has a path
    filename = osgDB::getSimpleFileName(str);
    if(filename!=str)
    {
        filename = osgDB::findFileInPath(filename,s_FontFilePath);
        if (!filename.empty()) return filename;
    }
    else
    {
        filename = osgText::findFontFile(std::string("fonts/")+filename);
        if (!filename.empty()) return filename;
    }

    // Not found, return empty string
    osg::notify(osg::WARN)<<"Warning: font file \""<<str<<"\" not found."<<std::endl;    
    return std::string();
}

osgText::Font* osgText::readFontFile(const std::string& filename)
{
    if (filename=="") return 0;

    std::string foundFile = findFontFile(filename);
    if (foundFile.empty()) return 0;
    
    osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
    options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_OBJECTS);

    osg::Object* object = osgDB::readObjectFile(foundFile, options.get());

    // if the object is a font then return it.
    osgText::Font* font = dynamic_cast<osgText::Font*>(object);
    if (font) return font;

    // otherwise if the object has zero references then delete it by doing another unref().
    if (object && object->referenceCount()==0) object->unref();
    return 0;
}

osgText::Font* osgText::readFontStream(std::istream& stream)
{
    osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
    options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_OBJECTS);

    // there should be a better way to get the FreeType ReaderWriter by name...
    osgDB::ReaderWriter *reader = osgDB::Registry::instance()->getReaderWriterForExtension("ttf");
    if (reader == 0) return 0;
    osgDB::ReaderWriter::ReadResult rr = reader->readObject(stream, options.get());
    if (rr.error())
    {
        osg::notify(osg::WARN) << rr.message() << std::endl;
        return 0;
    }
    if (!rr.validObject()) return 0;
    
    osg::Object *object = rr.takeObject();

    // if the object is a font then return it.
    osgText::Font* font = dynamic_cast<osgText::Font*>(object);
    if (font) return font;

    // otherwise if the object has zero references then delete it by doing another unref().
    if (object && object->referenceCount()==0) object->unref();
    return 0;
}


Font::Font(FontImplementation* implementation):
    _width(16),
    _height(16),
    _margin(2),
    _textureWidthHint(512),
    _textureHeightHint(512),
    _minFilterHint(osg::Texture::LINEAR_MIPMAP_LINEAR),
    _magFilterHint(osg::Texture::LINEAR)
{
    setImplementation(implementation);

    _texenv = new osg::TexEnv;
    _stateset = new osg::StateSet;
    _stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
}

Font::~Font()
{
    if (_implementation.valid()) _implementation->_facade = 0;
}

void Font::setImplementation(FontImplementation* implementation)
{
    if (_implementation.valid()) _implementation->_facade = 0;
    _implementation = implementation;
    if (_implementation.valid()) _implementation->_facade = this;
}

Font::FontImplementation* Font::getImplementation()
{
    return _implementation.get();
}

const Font::FontImplementation* Font::getImplementation() const
{
    return _implementation.get();
}

std::string Font::getFileName() const
{
    if (_implementation.valid()) return _implementation->getFileName();
    return "";
}

void Font::setFontResolution(unsigned int width, unsigned int height)
{
    if (_implementation.valid()) _implementation->setFontResolution(width, height);
}

unsigned int Font::getFontWidth() const
{
    return _width;
}

unsigned int Font::getFontHeight() const
{
    return _height;
}

void Font::setGlyphImageMargin(unsigned int margin)
{
    _margin = margin;
}

unsigned int Font::getGlyphImageMargin() const
{
    return _margin;
}

void Font::setTextureSizeHint(unsigned int width,unsigned int height)
{
    _textureWidthHint = width;
    _textureHeightHint = height;
}

unsigned int Font::getTextureWidthHint() const
{
    return _textureWidthHint;
}

unsigned int Font::getTextureHeightHint() const
{
    return _textureHeightHint;
}    


void Font::setMinFilterHint(osg::Texture::FilterMode mode)
{
    _minFilterHint = mode;
}

osg::Texture::FilterMode Font::getMinFilterHint() const
{
    return _minFilterHint;
}

/** Set the magnification texture filter to use when creating the texture to store the glyph images when rendering.
  * Note, this doesn't affect already created Texture Glhph's.*/
void Font::setMagFilterHint(osg::Texture::FilterMode mode)
{
    _magFilterHint = mode;
}

osg::Texture::FilterMode Font::getMagFilterHint() const
{
    return _magFilterHint;
}


Font::Glyph* Font::getGlyph(unsigned int charcode)
{
    SizeGlyphMap::iterator itr = _sizeGlyphMap.find(SizePair(_width,_height));
    if (itr!=_sizeGlyphMap.end())
    {
        GlyphMap& glyphmap = itr->second;    
        GlyphMap::iterator gitr = glyphmap.find(charcode);
        if (gitr!=glyphmap.end()) return gitr->second.get();
    }

    if (_implementation.valid()) return _implementation->getGlyph(charcode);
    else return 0;
}

void Font::releaseGLObjects(osg::State* state) const
{
  for(GlyphTextureList::const_iterator itr=_glyphTextureList.begin();
    itr!=_glyphTextureList.end();
    ++itr)
  {
    (*itr)->releaseGLObjects(state);
  }
}

osg::Vec2 Font::getKerning(unsigned int leftcharcode,unsigned int rightcharcode, KerningType kerningType)
{
    if (_implementation.valid()) return _implementation->getKerning(leftcharcode,rightcharcode,kerningType);
    else return osg::Vec2(0.0f,0.0f);
}

bool Font::hasVertical() const
{
    if (_implementation.valid()) return _implementation->hasVertical();
    else return false;
}



void Font::addGlyph(unsigned int width, unsigned int height, unsigned int charcode, Glyph* glyph)
{

    //cout << "charcode "<<(char)charcode<<"  "<<&_glyphTextureList<<endl;

    _sizeGlyphMap[SizePair(width,height)][charcode]=glyph;
    
    
    int posX=0,posY=0;
    
    GlyphTexture* glyphTexture = 0;
    for(GlyphTextureList::iterator itr=_glyphTextureList.begin();
        itr!=_glyphTextureList.end() && !glyphTexture;
        ++itr)
    {
        if ((*itr)->getSpaceForGlyph(glyph,posX,posY)) glyphTexture = itr->get();
    }
    
    if (glyphTexture)
    {
        //cout << "    found space for texture "<<glyphTexture<<" posX="<<posX<<" posY="<<posY<<endl;
    }
    
    if (!glyphTexture)
    {
        
        glyphTexture = new GlyphTexture;
        
        static int numberOfTexturesAllocated = 0;
        ++numberOfTexturesAllocated;

        osg::notify(osg::INFO)<< "   Font " << this<< ", numberOfTexturesAllocated "<<numberOfTexturesAllocated<<std::endl;

        // reserve enough space for the glyphs.
        glyphTexture->setGlyphImageMargin(_margin);
        glyphTexture->setTextureSize(_textureWidthHint,_textureHeightHint);
        glyphTexture->setFilter(osg::Texture::MIN_FILTER,_minFilterHint);
        glyphTexture->setFilter(osg::Texture::MAG_FILTER,_magFilterHint);
        glyphTexture->setMaxAnisotropy(8);
        
        _glyphTextureList.push_back(glyphTexture);
        
        if (!glyphTexture->getSpaceForGlyph(glyph,posX,posY))
        {
            osg::notify(osg::WARN)<<"Warning: unable to allocate texture big enough for glyph"<<std::endl;
            return;
        }

    }    
    
    // add the glyph into the texture.
    glyphTexture->addGlyph(glyph,posX,posY);
    
}


Font::GlyphTexture::GlyphTexture():
    _margin(2),
    _usedY(0),
    _partUsedX(0),
    _partUsedY(0)
{
}

Font::GlyphTexture::~GlyphTexture() 
{
}

// return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.
int Font::GlyphTexture::compare(const osg::StateAttribute& rhs) const
{
    if (this<&rhs) return -1;
    else if (this>&rhs) return 1;
    return 0;
}


bool Font::GlyphTexture::getSpaceForGlyph(Glyph* glyph, int& posX, int& posY)
{
        
    int width = glyph->s()+2*_margin;
    int height = glyph->t()+2*_margin;

    // first check box (_partUsedX,_usedY) to (width,height)
    if (width <= (getTextureWidth()-_partUsedX) &&
        height <= (getTextureHeight()-_usedY))
    {
        // can fit in existing row.

        // record the position in which the texture will be stored.
        posX = _partUsedX+_margin;
        posY = _usedY+_margin;        

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

        posX = _partUsedX+_margin;
        posY = _usedY+_margin;        

        // move used markers on.
        _partUsedX += width;
        if (_usedY+height>_partUsedY) _partUsedY = _usedY+height;
        
        return true;
    }

    // doesn't fit into glyph.
    return false;
}

void Font::GlyphTexture::addGlyph(Glyph* glyph, int posX, int posY)
{

    _glyphs.push_back(glyph);
    for(unsigned int i=0;i<_glyphsToSubload.size();++i)
    {
        _glyphsToSubload[i].push_back(glyph);
    }

    // set up the details of where to place glyph's image in the texture.
    glyph->setTexture(this);
    glyph->setTexturePosition(posX,posY);
    unsigned int sizeAdjustment = 0; // was 1.
    glyph->setMinTexCoord(osg::Vec2((float)(posX+_margin)/(float)(getTextureWidth()-sizeAdjustment),(float)(posY+_margin)/(float)(getTextureHeight()-sizeAdjustment)));
    glyph->setMaxTexCoord(osg::Vec2((float)(posX+glyph->s()-_margin)/(float)(getTextureWidth()-sizeAdjustment),(float)(posY+glyph->t()-_margin)/(float)(getTextureHeight()-sizeAdjustment)));
}

void Font::GlyphTexture::apply(osg::State& state) const
{
    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();

    if (contextID>=_glyphsToSubload.size())
    {
        // graphics context is beyond the number of glyphsToSubloads, so
        // we must now copy the glyph list across, this is a potential
        // threading issue though is multiple applies are happening the
        // same time on this object - to avoid this condition number of
        // graphics contexts should be set before create text.
        for(unsigned int i=_glyphsToSubload.size();i<=contextID;++i)
        {
            GlyphPtrList& glyphPtrs = _glyphsToSubload[i];
            for(GlyphRefList::const_iterator itr=_glyphs.begin();
                itr!=_glyphs.end();
                ++itr)
            {
                glyphPtrs.push_back(itr->get());
            }
        }
    }


    const Extensions* extensions = getExtensions(contextID,true);
    bool generateMipMapSupported = extensions->isGenerateMipMapSupported();

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);
    
    bool newTextureObject = (textureObject == 0);

    if (newTextureObject)
    {
        
        // being bound for the first time, need to allocate the texture

        _textureObjectBuffer[contextID] = textureObject = osg::Texture::generateTextureObject(
                contextID,GL_TEXTURE_2D,1,GL_ALPHA,getTextureWidth(), getTextureHeight(),1,0);

        textureObject->bind();


        applyTexParameters(GL_TEXTURE_2D,state);

        
        // need to look at generate mip map extension if mip mapping required.
        switch(_min_filter)
        {
        case NEAREST_MIPMAP_NEAREST:
        case NEAREST_MIPMAP_LINEAR:
        case LINEAR_MIPMAP_NEAREST:
        case LINEAR_MIPMAP_LINEAR:
            if (generateMipMapSupported)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
            }
            else glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, LINEAR);
            break;
        default:
            // not mip mapping so no problems.
            break;
        }
               
        // allocate the texture memory.
        glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA,
                getTextureWidth(), getTextureHeight(), 0,
                GL_ALPHA,
                GL_UNSIGNED_BYTE,
                0 );
    
    }
    else
    {
        // reuse texture by binding.
        textureObject->bind();
        
        if (getTextureParameterDirty(contextID))
        {
            applyTexParameters(GL_TEXTURE_2D,state);
        }


    }
    
    static const GLubyte* s_renderer = 0;
    static bool s_subloadAllGlyphsTogether = false;
    if (!s_renderer)
    {
        s_renderer = glGetString(GL_RENDERER);
        osg::notify(osg::INFO)<<"glGetString(GL_RENDERER)=="<<s_renderer<<std::endl;
        if (strstr((const char*)s_renderer,"IMPACT")!=0)
        {
            // we're running on an Octane, so need to work around its
            // subloading bugs by loading all at once.
            s_subloadAllGlyphsTogether = true;
        }
        
        const char* str = getenv("OSG_TEXT_INCREMENTAL_SUBLOADING");
        if (str && (strcmp(str,"OFF")==0 || strcmp(str,"Off")==0 || strcmp(str,"Off")==0))
        {
            s_subloadAllGlyphsTogether = true;
        }
    }


    // now subload the glyphs that are outstanding for this graphics context.
    GlyphPtrList& glyphsWereSubloading = _glyphsToSubload[contextID];

    if (!glyphsWereSubloading.empty() || newTextureObject)
    {
    
        if (!s_subloadAllGlyphsTogether)
        {
            if (newTextureObject)
            {
                for(GlyphRefList::const_iterator itr=_glyphs.begin();
                    itr!=_glyphs.end();
                    ++itr)
                {
                    (*itr)->subload();
                }
            }
            else // just subload the new entries.
            {            
                // default way of subloading as required.
                //std::cout<<"subloading"<<std::endl;
                for(GlyphPtrList::iterator itr=glyphsWereSubloading.begin();
                    itr!=glyphsWereSubloading.end();
                    ++itr)
                {
                    (*itr)->subload();
                }
            }
            
            // clear the list since we have now subloaded them.
            glyphsWereSubloading.clear();
            
        }
        else
        {
            osg::notify(osg::INFO)<<"osgText::Font loading all glyphs as a single subload."<<std::endl;

            // Octane has bugs in OGL driver which mean that subloads smaller
            // than 32x32 produce errors, and also cannot handle general alignment,
            // so to get round this copy all glyphs into a temporary image and
            // then subload the whole lot in one go.

            int tsize = getTextureHeight() * getTextureWidth();
            unsigned char *local_data = new unsigned char[tsize];
            memset( local_data, 0L, tsize);

            for(GlyphRefList::const_iterator itr=_glyphs.begin();
                itr!=_glyphs.end();
                ++itr)
            {
                //(*itr)->subload();

                // Rather than subloading to graphics, we'll write the values
                // of the glyphs into some intermediate data and subload the
                // whole thing at the end
                for( int t = 0; t < (*itr)->t(); t++ )
                {
                    for( int s = 0; s < (*itr)->s(); s++ )
                    {
                        int sindex = (t*(*itr)->s()+s);
                        int dindex =  
                            ((((*itr)->getTexturePositionY()+t) * getTextureWidth()) +
                            ((*itr)->getTexturePositionX()+s));

                        const unsigned char *sptr = &(*itr)->data()[sindex];
                        unsigned char *dptr       = &local_data[dindex];

                        (*dptr)   = (*sptr);
                    }
                }
            }

            // clear the list since we have now subloaded them.
            glyphsWereSubloading.clear();

            // Subload the image once
            glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 
                    getTextureWidth(),
                    getTextureHeight(),
                    GL_ALPHA, GL_UNSIGNED_BYTE, local_data );

            delete [] local_data;

        }
    }
    else
    {
        //std::cout << "no need to subload "<<std::endl;
    }



//     if (generateMipMapTurnedOn)
//     {
//         glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_FALSE);
//     }


}

// all the methods in Font::Glyph have been made non inline because VisualStudio6.0 is STUPID, STUPID, STUPID PILE OF JUNK.
Font::Glyph::Glyph() {}
Font::Glyph::~Glyph() {}

unsigned int Font::Glyph::getGlyphCode() const { return _glyphCode; }

// void Font::Glyph::setFont(Font* font) { _font = font; }
// Font* Font::Glyph::getFont() const { return _font; }

void Font::Glyph::setHorizontalBearing(const osg::Vec2& bearing) {  _horizontalBearing=bearing; }
const osg::Vec2& Font::Glyph::getHorizontalBearing() const { return _horizontalBearing; }

void Font::Glyph::setHorizontalAdvance(float advance) { _horizontalAdvance=advance; }
float Font::Glyph::getHorizontalAdvance() const { return _horizontalAdvance; }

void Font::Glyph::setVerticalBearing(const osg::Vec2& bearing) {  _verticalBearing=bearing; }
const osg::Vec2& Font::Glyph::getVerticalBearing() const { return _verticalBearing; }

void Font::Glyph::setVerticalAdvance(float advance) {  _verticalAdvance=advance; }
float Font::Glyph::getVerticalAdvance() const { return _verticalAdvance; }

void Font::Glyph::setTexture(GlyphTexture* texture) { _texture = texture; }
Font::GlyphTexture* Font::Glyph::getTexture() { return _texture; }
const Font::GlyphTexture* Font::Glyph::getTexture() const { return _texture; }

void Font::Glyph::setTexturePosition(int posX,int posY) { _texturePosX = posX; _texturePosY = posY; }
int Font::Glyph::getTexturePositionX() const { return _texturePosX; }
int Font::Glyph::getTexturePositionY() const { return _texturePosY; }

void Font::Glyph::setMinTexCoord(const osg::Vec2& coord) { _minTexCoord=coord; }
const osg::Vec2& Font::Glyph::getMinTexCoord() const { return _minTexCoord; }

void Font::Glyph::setMaxTexCoord(const osg::Vec2& coord) { _maxTexCoord=coord; }
const osg::Vec2& Font::Glyph::getMaxTexCoord() const { return _maxTexCoord; }

void Font::Glyph::subload() const
{
    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        osg::notify(osg::WARN)<<"before Font::Glyph::subload(): detected OpenGL error '"<<gluErrorString(errorNo)<<std::endl;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT,getPacking());

    glTexSubImage2D(GL_TEXTURE_2D,0,
                    _texturePosX,_texturePosY,
                    s(),t(),
                    (GLenum)getPixelFormat(),
                    (GLenum)getDataType(),
                    data());
                    
    errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {

        osg::notify(osg::WARN)<<"after Font::Glyph::subload() : detected OpenGL error '"<<gluErrorString(errorNo)<<"'"<<std::endl;
        osg::notify(osg::WARN)<< "\tglTexSubImage2D(0x"<<hex<<GL_TEXTURE_2D<<dec<<" ,"<<0<<"\t"<<std::endl<<
                                 "\t                "<<_texturePosX<<" ,"<<_texturePosY<<std::endl<<
                                 "\t                "<<s()<<" ,"<<t()<<std::endl<<hex<<
                                 "\t                0x"<<(GLenum)getPixelFormat()<<std::endl<<
                                 "\t                0x"<<(GLenum)getDataType()<<std::endl<<
                                 "\t                0x"<<(unsigned long)data()<<");"<<dec<<std::endl;
    }                    
}

void Font::Glyph::draw(osg::State& state) const
{
    GLuint& globj = _globjList[state.getContextID()];

    // call the globj if already set otherwise compile and execute.
    if( globj != 0 )
    {
        glCallList( globj );
    }
    else 
    {
        globj = glGenLists( 1 );
        glNewList( globj, GL_COMPILE_AND_EXECUTE );

        glPixelStorei(GL_UNPACK_ALIGNMENT,getPacking());
        glDrawPixels(s(), t(),
                     (GLenum)getPixelFormat(),
                     (GLenum)getDataType(),
                     data() );

        glEndList();
    }
}
