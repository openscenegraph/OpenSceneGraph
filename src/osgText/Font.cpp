/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

#include <osg/State>
#include <osg/Notify>
#include <osgDB/ReadFile>
#include <osg/GLU>

using namespace osgText;

osgText::Font* osgText::readFontFile(const std::string& filename)
{
    osg::Object* object = osgDB::readObjectFile(filename);
    
    // if the object is a font then return it.
    osgText::Font* font = dynamic_cast<osgText::Font*>(object);
    if (font) return font;
    
    // otherwise if the object has zero references then delete it by doing another unref().
    if (object && object->referenceCount()==0) object->unref();
    return 0;
}


Font::Font():
    _width(16),
    _height(16)
{
}

Font::~Font()
{
}

void Font::addGlyph(unsigned int charcode, Glyph* glyph)
{
    _glyphMap[charcode]=glyph;
    
    
    int posX=0,posY=0;
    
    GlyphTexture* glyphTexture = 0;
    for(GlyphTextureList::iterator itr=_glyphTextureList.begin();
        itr!=_glyphTextureList.end() && !glyphTexture;
        ++itr)
    {
        if ((*itr)->getSpaceForGlyph(glyph,posX,posY)) glyphTexture = itr->get();
    }
    
    if (!glyphTexture)
    {
        //std::cout<<"Creating new GlyphTexture & StateSet"<<std::endl;
        
        osg::StateSet* stateset = new osg::StateSet;
        _stateSetList.push_back(stateset);

        glyphTexture = new GlyphTexture;
        
        // reserve enough space for the glyphs.
        glyphTexture->setTextureSize(256,256);
        glyphTexture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
        //glyphTexture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
        //glyphTexture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::NEAREST);
        glyphTexture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
        glyphTexture->setMaxAnisotropy(8);
        
        _glyphTextureList.push_back(glyphTexture);
        
        glyphTexture->setStateSet(stateset);
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        stateset->setTextureAttributeAndModes(0,glyphTexture,osg::StateAttribute::ON);

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
    _stateset(0),
    _usedY(0),
    _partUsedX(0),
    _partUsedY(0)
{
}

Font::GlyphTexture::~GlyphTexture() 
{
}

bool Font::GlyphTexture::getSpaceForGlyph(Glyph* glyph, int& posX, int& posY)
{

    int margin = 2;
        
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
    glyph->setMinTexCoord(osg::Vec2((float)posX/((float)getTextureWidth()-1.0f),(float)posY/((float)getTextureHeight()-1.0f)));
    glyph->setMaxTexCoord(osg::Vec2((float)(posX+glyph->s())/((float)getTextureWidth()-1.0f),(float)(posY+glyph->t())/((float)getTextureHeight()-1.0f)));
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
            _glyphsToSubload[i] = _glyphs;
        }
    }


    // get the globj for the current contextID.
    GLuint& handle = getTextureObject(contextID);

    if (handle == 0)
    {
        // being bound for the first time, need to allocate the texture
        glGenTextures( 1L, (GLuint *)&handle );
        glBindTexture( GL_TEXTURE_2D, handle );

        applyTexParameters(GL_TEXTURE_2D,state);
        
        //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
        
        // allocate the texture memory.
        glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA,
                getTextureWidth(), getTextureHeight(), 0,
                GL_LUMINANCE_ALPHA,
                GL_UNSIGNED_BYTE,
                0 );
    
    }
    else
    {
        // reuse texture by binding.
        glBindTexture( GL_TEXTURE_2D, handle );
        if (getTextureParameterDirty(contextID))
            applyTexParameters(GL_TEXTURE_2D,state);

    }
    
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_TRUE);

    // now subload the glyphs that are outstanding for this graphics context.
    GlyphList& glyphsWereSubloading = _glyphsToSubload[contextID];

    if (!glyphsWereSubloading.empty())
    {

        for(GlyphList::iterator itr=glyphsWereSubloading.begin();
            itr!=glyphsWereSubloading.end();
            ++itr)
        {
            (*itr)->subload();
        }
        
        // clear the list since we have now subloaded them.
        glyphsWereSubloading.clear();
    }
    else
    {
        //std::cout << "no need to subload "<<std::endl;
    }
}

Font::Glyph::Glyph() {}
Font::Glyph::~Glyph() {}

void Font::Glyph::subload()
{
    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        osg::notify(osg::WARN)<<"before: detected OpenGL error '"<<gluErrorString(errorNo)<<std::endl;
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
        std::cout << "  "<<GL_TEXTURE_2D<<"\t"<<0<<"\t"<<
                    _texturePosX<<"\t"<<_texturePosY<<"\t"<<
                    s()<<"\t"<<t()<<"\t"<<
                    (GLenum)getPixelFormat()<<"\t"<<
                    (GLenum)getDataType()<<"\t"<<
                    (int)(*data())<<std::endl;

        osg::notify(osg::WARN)<<"after: detected OpenGL error '"<<gluErrorString(errorNo)<<std::endl;
    }                    
}
