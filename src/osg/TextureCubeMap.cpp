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
#include <osg/GLExtensions>
#include <osg/ref_ptr>
#include <osg/Image>
#include <osg/State>
#include <osg/TextureCubeMap>
#include <osg/Notify>

#include <osg/GLU>


using namespace osg;

static GLenum faceTarget[6] =
{
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};


TextureCubeMap::TextureCubeMap():
            _textureWidth(0),
            _textureHeight(0),
            _numMipmapLevels(0)
{
    setUseHardwareMipMapGeneration(false);
}

TextureCubeMap::TextureCubeMap(const TextureCubeMap& text,const CopyOp& copyop):
            Texture(text,copyop),
            _textureWidth(text._textureWidth),
            _textureHeight(text._textureHeight),
            _numMipmapLevels(text._numMipmapLevels),
            _subloadCallback(text._subloadCallback)
{
    setImage(0, copyop(text._images[0].get()));
    setImage(1, copyop(text._images[1].get()));
    setImage(2, copyop(text._images[2].get()));
    setImage(3, copyop(text._images[3].get()));
    setImage(4, copyop(text._images[4].get()));
    setImage(5, copyop(text._images[5].get()));
}

TextureCubeMap::~TextureCubeMap()
{
    setImage(0, NULL);
    setImage(1, NULL);
    setImage(2, NULL);
    setImage(3, NULL);
    setImage(4, NULL);
    setImage(5, NULL);
}

int TextureCubeMap::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types(TextureCubeMap,sa)

    bool noImages = true;
    for (int n=0; n<6; n++)
    {
        if (noImages && _images[n].valid()) noImages = false;
        if (noImages && rhs._images[n].valid()) noImages = false;

        if (_images[n]!=rhs._images[n]) // smart pointer comparison.
        {
            if (_images[n].valid())
            {
                if (rhs._images[n].valid())
                {
                    int result = _images[n]->compare(*rhs._images[n]);
                    if (result!=0) return result;
                }
                else
                {
                    return 1; // valid lhs._image is greater than null.
                }
            }
            else if (rhs._images[n].valid())
            {
                return -1; // valid rhs._image is greater than null.
            }
        }
    }

    if (noImages)
    {
        // no image attached to either Texture2D
        // but could these textures already be downloaded?
        // check the _textureObjectBuffer to see if they have been
        // downloaded

        int result = compareTextureObjects(rhs);
        if (result!=0) return result;
    }

    int result = compareTexture(rhs);
    if (result!=0) return result;

    // compare each parameter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_textureWidth)
    COMPARE_StateAttribute_Parameter(_textureHeight)
    COMPARE_StateAttribute_Parameter(_subloadCallback)

    return 0; // passed all the above comparison macros, must be equal.
}


void TextureCubeMap::setImage(unsigned int face, Image* image)
{
    if (_images[face] == image) return;

    unsigned numImageRequireUpdateBefore = 0;
    for (unsigned int i=0; i<getNumImages(); ++i)
    {
        if (_images[i].valid() && _images[i]->requiresUpdateCall()) ++numImageRequireUpdateBefore;
    }

    if (_images[face].valid())
    {
        _images[face]->removeClient(this);
    }

    _images[face] = image;
    _modifiedCount[face].setAllElementsTo(0);

    if (_images[face].valid())
    {
        _images[face]->addClient(this);
    }

    // find out if we need to reset the update callback to handle the animation of image
    unsigned numImageRequireUpdateAfter = 0;
    for (unsigned int i=0; i<getNumImages(); ++i)
    {
        if (_images[i].valid() && _images[i]->requiresUpdateCall()) ++numImageRequireUpdateAfter;
    }

    if (numImageRequireUpdateBefore>0)
    {
        if (numImageRequireUpdateAfter==0)
        {
            setUpdateCallback(0);
            setDataVariance(osg::Object::STATIC);
        }
    }
    else if (numImageRequireUpdateAfter>0)
    {
        setUpdateCallback(new Image::UpdateCallback());
        setDataVariance(osg::Object::DYNAMIC);
    }
}

Image* TextureCubeMap::getImage(unsigned int face)
{
    return _images[face].get();
}

const Image* TextureCubeMap::getImage(unsigned int face) const
{
    return _images[face].get();
}

bool TextureCubeMap::imagesValid() const
{
    for (int n=0; n<6; n++)
    {
        if (!_images[n].valid() || !_images[n]->data())
            return false;
    }
    return true;
}

void TextureCubeMap::computeInternalFormat() const
{
    if (imagesValid()) computeInternalFormatWithImage(*_images[0]);
    else computeInternalFormatType();
}

void TextureCubeMap::apply(State& state) const
{
    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();

    Texture::TextureObjectManager* tom = Texture::getTextureObjectManager(contextID).get();
    ElapsedTime elapsedTime(&(tom->getApplyTime()));
    tom->getNumberApplied()++;

    const Extensions* extensions = getExtensions(contextID,true);

    if (!extensions->isCubeMapSupported())
        return;

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject)
    {
        const osg::Image* image = _images[0].get();
        if (image && getModifiedCount(0, contextID) != image->getModifiedCount())
        {
            // compute the internal texture format, this set the _internalFormat to an appropriate value.
            computeInternalFormat();

            GLsizei new_width, new_height, new_numMipmapLevels;

            // compute the dimensions of the texture.
            computeRequiredTextureDimensions(state, *image, new_width, new_height, new_numMipmapLevels);

            if (!textureObject->match(GL_TEXTURE_CUBE_MAP, new_numMipmapLevels, _internalFormat, new_width, new_height, 1, _borderWidth))
            {
                Texture::releaseTextureObject(contextID, _textureObjectBuffer[contextID].get());
                _textureObjectBuffer[contextID] = 0;
                textureObject = 0;
            }
        }
    }

    if (textureObject)
    {
        textureObject->bind();

        if (getTextureParameterDirty(state.getContextID())) applyTexParameters(GL_TEXTURE_CUBE_MAP,state);

        if (_subloadCallback.valid())
        {
            _subloadCallback->subload(*this,state);
        }
        else
        {
            for (int n=0; n<6; n++)
            {
                const osg::Image* image = _images[n].get();
                if (image && getModifiedCount((Face)n,contextID) != image->getModifiedCount())
                {
                    applyTexImage2D_subload( state, faceTarget[n], _images[n].get(), _textureWidth, _textureHeight, _internalFormat, _numMipmapLevels);
                    getModifiedCount((Face)n,contextID) = image->getModifiedCount();
                }
            }
        }

    }
    else if (_subloadCallback.valid())
    {
        _textureObjectBuffer[contextID] = textureObject = generateTextureObject(this, contextID,GL_TEXTURE_CUBE_MAP);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_CUBE_MAP,state);

        _subloadCallback->load(*this,state);

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        //glBindTexture( GL_TEXTURE_CUBE_MAP, handle );

    }
    else if (imagesValid())
    {

        // compute the internal texture format, this set the _internalFormat to an appropriate value.
        computeInternalFormat();

        // compute the dimensions of the texture.
        computeRequiredTextureDimensions(state,*_images[0],_textureWidth, _textureHeight, _numMipmapLevels);

        // cubemap textures must have square dimensions
        if( _textureWidth != _textureHeight )
        {
            _textureWidth = _textureHeight = minimum( _textureWidth , _textureHeight );
        }

        textureObject = generateTextureObject(
                this, contextID,GL_TEXTURE_CUBE_MAP,_numMipmapLevels,_internalFormat,_textureWidth,_textureHeight,1,0);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_CUBE_MAP,state);

        for (int n=0; n<6; n++)
        {
            const osg::Image* image = _images[n].get();
            if (image)
            {
                if (textureObject->isAllocated())
                {
                    applyTexImage2D_subload( state, faceTarget[n], image, _textureWidth, _textureHeight, _internalFormat, _numMipmapLevels);
                }
                else
                {
                    applyTexImage2D_load( state, faceTarget[n], image, _textureWidth, _textureHeight, _numMipmapLevels);
                }
                getModifiedCount((Face)n,contextID) = image->getModifiedCount();
            }


        }

        _textureObjectBuffer[contextID] = textureObject;

        // unref image data?
        if (isSafeToUnrefImageData(state))
        {
            TextureCubeMap* non_const_this = const_cast<TextureCubeMap*>(this);
            for (int n=0; n<6; n++)
            {
                if (_images[n].valid() && _images[n]->getDataVariance()==STATIC)
                {
                    non_const_this->_images[n] = NULL;
                }
            }
        }

    }
    else if ( (_textureWidth!=0) && (_textureHeight!=0) && (_internalFormat!=0) )
    {
        _textureObjectBuffer[contextID] = textureObject = generateTextureObject(
                this, contextID,GL_TEXTURE_CUBE_MAP,_numMipmapLevels,_internalFormat,_textureWidth,_textureHeight,1,0);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_CUBE_MAP,state);

        for (int n=0; n<6; n++)
        {
            // no image present, but dimensions at set so less create the texture
            glTexImage2D( faceTarget[n], 0, _internalFormat,
                         _textureWidth, _textureHeight, _borderWidth,
                         _sourceFormat ? _sourceFormat : _internalFormat,
                         _sourceType ? _sourceType : GL_UNSIGNED_BYTE,
                         0);
        }

    }
    else
    {
        glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
    }

    // if texture object is now valid and we have to allocate mipmap levels, then
    if (textureObject != 0 && _texMipmapGenerationDirtyList[contextID])
    {
        generateMipmap(state);
    }
}

void TextureCubeMap::copyTexSubImageCubeMap(State& state, int face, int xoffset, int yoffset, int x, int y, int width, int height )
{
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    if (!extensions->isCubeMapSupported())
        return;

    if (_internalFormat==0) _internalFormat=GL_RGBA;

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (!textureObject)
    {

        if (_textureWidth==0) _textureWidth = width;
        if (_textureHeight==0) _textureHeight = height;

        // create texture object.
        apply(state);

        textureObject = getTextureObject(contextID);

        if (!textureObject)
        {
            // failed to create texture object
            OSG_NOTICE<<"Warning : failed to create TextureCubeMap texture obeject, copyTexSubImageCubeMap abondoned."<<std::endl;
            return;
        }

    }

    GLenum target = faceTarget[face];

    if (textureObject)
    {
        // we have a valid image
        textureObject->bind();

        applyTexParameters(GL_TEXTURE_CUBE_MAP, state);

        bool needHardwareMipMap = (_min_filter != LINEAR && _min_filter != NEAREST);
        bool hardwareMipMapOn = false;
        if (needHardwareMipMap)
        {
            hardwareMipMapOn = isHardwareMipmapGenerationEnabled(state);

            if (!hardwareMipMapOn)
            {
                // have to switch off mip mapping
                OSG_NOTICE<<"Warning: TextureCubeMap::copyTexImage2D(,,,,) switch off mip mapping as hardware support not available."<<std::endl;
                _min_filter = LINEAR;
            }
        }

        GenerateMipmapMode mipmapResult = mipmapBeforeTexImage(state, hardwareMipMapOn);

        glCopyTexSubImage2D( target , 0, xoffset, yoffset, x, y, width, height);

        mipmapAfterTexImage(state, mipmapResult);

        // inform state that this texture is the current one bound.
        state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);

    }
}

void TextureCubeMap::allocateMipmap(State& state) const
{
    const unsigned int contextID = state.getContextID();

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject && _textureWidth != 0 && _textureHeight != 0)
    {
        // bind texture
        textureObject->bind();

        // compute number of mipmap levels
        int width = _textureWidth;
        int height = _textureHeight;
        int numMipmapLevels = Image::computeNumberOfMipmapLevels(width, height);

        // we do not reallocate the level 0, since it was already allocated
        width >>= 1;
        height >>= 1;

        for( GLsizei k = 1; k < numMipmapLevels  && (width || height); k++)
        {
            if (width == 0)
                width = 1;
            if (height == 0)
                height = 1;

            for (int n=0; n<6; n++)
            {
                glTexImage2D( faceTarget[n], k, _internalFormat,
                            width, height, _borderWidth,
                            _sourceFormat ? _sourceFormat : _internalFormat,
                            _sourceType ? _sourceType : GL_UNSIGNED_BYTE,
                            0);
            }

            width >>= 1;
            height >>= 1;
        }

        // inform state that this texture is the current one bound.
        state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);
    }
}

typedef buffered_value< ref_ptr<TextureCubeMap::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

TextureCubeMap::Extensions* TextureCubeMap::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void TextureCubeMap::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

TextureCubeMap::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

TextureCubeMap::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isCubeMapSupported = rhs._isCubeMapSupported;
}

void TextureCubeMap::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isCubeMapSupported) _isCubeMapSupported = false;
}

void TextureCubeMap::Extensions::setupGLExtensions(unsigned int contextID)
{
    _isCubeMapSupported = OSG_GLES2_FEATURES || OSG_GL3_FEATURES ||
                          isGLExtensionSupported(contextID,"GL_ARB_texture_cube_map") ||
                          isGLExtensionSupported(contextID,"GL_EXT_texture_cube_map") ||
                          strncmp((const char*)glGetString(GL_VERSION),"1.3",3)>=0;;
}
