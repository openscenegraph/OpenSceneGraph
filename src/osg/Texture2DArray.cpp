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
#include <osg/Texture2DArray>
#include <osg/State>
#include <osg/Notify>

#include <string.h>


using namespace osg;

Texture2DArray::Texture2DArray():
            _textureWidth(0),
            _textureHeight(0),
            _textureDepth(0),
            _numMipmapLevels(0)
{
}

Texture2DArray::Texture2DArray(const Texture2DArray& text,const CopyOp& copyop):
            Texture(text,copyop),
            _textureWidth(text._textureWidth),
            _textureHeight(text._textureHeight),
            _textureDepth(0),
            _numMipmapLevels(text._numMipmapLevels),
            _subloadCallback(text._subloadCallback)
{
    setTextureDepth(text._textureDepth);

    for(unsigned int i = 0; i<static_cast<unsigned int>(_images.size()); ++i)
    {
        setImage(i, copyop(text._images[i].get()));
    }
}

Texture2DArray::~Texture2DArray()
{
    for(unsigned int i = 0; i<static_cast<unsigned int>(_images.size()); ++i)
    {
        setImage(i, NULL);
    }
}

int Texture2DArray::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types(Texture2DArray,sa)

    if (_images.size()<rhs._images.size()) return -1;
    if (_images.size()>rhs._images.size()) return 1;

    bool noImages = true;
    for (unsigned int n=0; n < static_cast<unsigned int>(_images.size()); n++)
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
        int result = compareTextureObjects(rhs);
        if (result!=0) return result;
    }

    int result = compareTexture(rhs);
    if (result!=0) return result;

    // compare each parameter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_textureWidth)
    COMPARE_StateAttribute_Parameter(_textureHeight)
    COMPARE_StateAttribute_Parameter(_textureDepth)
    COMPARE_StateAttribute_Parameter(_subloadCallback)

    return 0; // passed all the above comparison macros, must be equal.
}

void Texture2DArray::setImage(unsigned int layer, Image* image)
{
    if (layer>= static_cast<unsigned int>(_images.size()))
    {
        // _images vector not large enough to contain layer so expand it.
        _images.resize(layer+1);
        _modifiedCount.resize(layer+1);
    }
    else
    {
        // do not need to replace already assigned images
        if (_images[layer] == image) return;
    }

    unsigned numImageRequireUpdateBefore = 0;
    for (unsigned int i=0; i<getNumImages(); ++i)
    {
        if (_images[i].valid() && _images[i]->requiresUpdateCall()) ++numImageRequireUpdateBefore;
    }

    if (_images[layer].valid())
    {
        _images[layer]->removeClient(this);
    }

    // set image
    _images[layer] = image;
    _modifiedCount[layer].setAllElementsTo(0);

    if (_images[layer].valid())
    {
        _images[layer]->addClient(this);
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

Image* Texture2DArray::getImage(unsigned int layer)
{
    return (layer<static_cast<unsigned int>(_images.size())) ? _images[layer].get() : 0;
}

const Image* Texture2DArray::getImage(unsigned int layer) const
{
    return (layer<static_cast<unsigned int>(_images.size())) ? _images[layer].get() : 0;
}

bool Texture2DArray::imagesValid() const
{
    if (_images.empty()) return false;

    for(Images::const_iterator itr = _images.begin();
        itr != _images.end();
        ++itr)
    {
        if (!(itr->valid()) || !((*itr)->valid())) return false;
    }
    return true;
}

void Texture2DArray::setTextureSize(int width, int height, int depth)
{
    // set dimensions
    _textureWidth = width;
    _textureHeight = height;
    setTextureDepth(depth);
}

void Texture2DArray::setTextureDepth(int depth)
{
    // if we decrease the number of layers, then delete non-used
    if (depth < static_cast<int>(_images.size()))
    {
        _images.resize(depth);
        _modifiedCount.resize(depth);
    }

    // resize the texture array
    _textureDepth = depth;
}

void Texture2DArray::computeInternalFormat() const
{
    if (imagesValid()) computeInternalFormatWithImage(*_images[0]);
    else computeInternalFormatType();
}

GLsizei Texture2DArray::computeTextureDepth() const
{
    GLsizei textureDepth = _textureDepth;
    if (textureDepth==0)
    {
        for(Images::const_iterator itr = _images.begin();
            itr != _images.end();
            ++itr)
        {
            const osg::Image* image = itr->get();
            if (image) textureDepth += image->r();
        }
    }
    return textureDepth;
}


void Texture2DArray::apply(State& state) const
{
    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();

    Texture::TextureObjectManager* tom = Texture::getTextureObjectManager(contextID).get();
    ElapsedTime elapsedTime(&(tom->getApplyTime()));
    tom->getNumberApplied()++;

    const GLExtensions* extensions = state.get<GLExtensions>();

    // if not supported, then return
    if (!extensions->isTexture2DArraySupported || !extensions->isTexture3DSupported)
    {
        OSG_WARN<<"Warning: Texture2DArray::apply(..) failed, 2D texture arrays are not support by OpenGL driver."<<std::endl;
        return;
    }

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    GLsizei textureDepth = computeTextureDepth();

    if (textureObject && textureDepth>0)
    {
        const osg::Image* image = (_images.size()>0) ? _images[0].get() : 0;
        if (image && getModifiedCount(0, contextID) != image->getModifiedCount())
        {
            // compute the internal texture format, this set the _internalFormat to an appropriate value.
            computeInternalFormat();

            GLsizei new_width, new_height, new_numMipmapLevels;

            // compute the dimensions of the texture.
            computeRequiredTextureDimensions(state, *image, new_width, new_height, new_numMipmapLevels);

            if (!textureObject->match(GL_TEXTURE_2D_ARRAY_EXT, new_numMipmapLevels, _internalFormat, new_width, new_height, textureDepth, _borderWidth))
            {
                Texture::releaseTextureObject(contextID, _textureObjectBuffer[contextID].get());
                _textureObjectBuffer[contextID] = 0;
                textureObject = 0;
            }
        }
    }

    // if we already have an texture object, then
    if (textureObject)
    {
        // bind texture object
        textureObject->bind();

        // if texture parameters changed, then reset them
        if (getTextureParameterDirty(state.getContextID())) applyTexParameters(GL_TEXTURE_2D_ARRAY_EXT,state);

        // if subload is specified, then use it to subload the images to GPU memory
        if (_subloadCallback.valid())
        {
            _subloadCallback->subload(*this,state);
        }
        else
        {
            GLsizei n = 0;
            for(Images::const_iterator itr = _images.begin();
                itr != _images.end();
                ++itr)
            {
                osg::Image* image = itr->get();
                if (image)
                {
                    if (getModifiedCount(n,contextID) != image->getModifiedCount())
                    {
                        applyTexImage2DArray_subload(state, image, n, _textureWidth, _textureHeight, image->r(), _internalFormat, _numMipmapLevels);
                        getModifiedCount(n,contextID) = image->getModifiedCount();
                    }
                    n += image->r();
                }
            }
            }

    }

    // there is no texture object, but exists a subload callback, so use it to upload images
    else if (_subloadCallback.valid())
    {
        // generate texture (i.e. glGenTexture) and apply parameters
        textureObject = generateAndAssignTextureObject(contextID, GL_TEXTURE_2D_ARRAY_EXT);
        textureObject->bind();
        applyTexParameters(GL_TEXTURE_2D_ARRAY_EXT, state);
        _subloadCallback->load(*this,state);
    }

    // nothing before, but we have valid images, so do manual upload and create texture object manually
    // TODO: we assume _images[0] is valid, however this may not be always the case
    //       some kind of checking for the first valid image is required (Art, may 2008)
    else if (imagesValid())
    {
        // compute the internal texture format, this set the _internalFormat to an appropriate value.
        computeInternalFormat();

        // compute the dimensions of the texture.
        computeRequiredTextureDimensions(state,*_images[0],_textureWidth, _textureHeight, _numMipmapLevels);

        // create texture object
        textureObject = generateAndAssignTextureObject(
                    contextID, GL_TEXTURE_2D_ARRAY_EXT,_numMipmapLevels, _internalFormat, _textureWidth, _textureHeight, textureDepth,0);

        // bind texture
        textureObject->bind();
        applyTexParameters(GL_TEXTURE_2D_ARRAY_EXT, state);

        // First we need to allocate the texture memory
        int sourceFormat = _sourceFormat ? _sourceFormat : _internalFormat;

        if( isCompressedInternalFormat( sourceFormat ) &&
            sourceFormat == _internalFormat &&
            extensions->isCompressedTexImage3DSupported() )
        {
            extensions->glCompressedTexImage3D( GL_TEXTURE_2D_ARRAY_EXT, 0, _internalFormat,
                     _textureWidth, _textureHeight, textureDepth, _borderWidth,
                     _images[0]->getImageSizeInBytes() * textureDepth,
                     0);
        }
        else
        {
            // Override compressed source format with safe GL_RGBA value which not generate error
            // We can safely do this as source format is not important when source data is NULL
            if( isCompressedInternalFormat( sourceFormat ) )
                sourceFormat = GL_RGBA;

            extensions->glTexImage3D( GL_TEXTURE_2D_ARRAY_EXT, 0, _internalFormat,
                     _textureWidth, _textureHeight, textureDepth, _borderWidth,
                     sourceFormat, _sourceType ? _sourceType : GL_UNSIGNED_BYTE,
                     0);
        }

        // For certain we have to manually allocate memory for mipmaps if images are compressed
        // if not allocated OpenGL will produce errors on mipmap upload.
        // I have not tested if this is necessary for plain texture formats but
        // common sense suggests its required as well.
        if( _min_filter != LINEAR && _min_filter != NEAREST && _images[0]->isMipmap() )
        {
            allocateMipmap( state );
        }

        GLsizei n = 0;
        for(Images::const_iterator itr = _images.begin();
            itr != _images.end();
            ++itr)
        {
            osg::Image* image = itr->get();
            if (image)
            {
                if (getModifiedCount(n,contextID) != image->getModifiedCount())
                {
                    applyTexImage2DArray_subload(state, image, n, _textureWidth, _textureHeight, image->r(), _internalFormat, _numMipmapLevels);
                    getModifiedCount(n,contextID) = image->getModifiedCount();
                }
                n += image->r();
            }
        }

        const GLExtensions* extensions = state.get<GLExtensions>();
        // source images have no mipmamps but we could generate them...
        if( _min_filter != LINEAR && _min_filter != NEAREST && !_images[0]->isMipmap() &&
            _useHardwareMipMapGeneration && extensions->isGenerateMipMapSupported )
        {
            _numMipmapLevels = Image::computeNumberOfMipmapLevels( _textureWidth, _textureHeight );
            generateMipmap( state );
        }

        textureObject->setAllocated(_numMipmapLevels, _internalFormat, _textureWidth, _textureHeight, textureDepth,0);

        // unref image data?
        if (isSafeToUnrefImageData(state))
        {
            Texture2DArray* non_const_this = const_cast<Texture2DArray*>(this);
            for(Images::iterator itr = non_const_this->_images.begin();
                itr != non_const_this->_images.end();
                ++itr)
            {
                osg::Image* image = itr->get();
                if (image && image->getDataVariance()==STATIC)
                {
                    *itr = NULL;
                }
            }
        }

    }

    // No images present, but dimensions are set. So create empty texture
    else if ( (_textureWidth > 0) && (_textureHeight > 0) && (_textureDepth > 0) && (_internalFormat!=0) )
    {
        // generate texture
        textureObject = generateAndAssignTextureObject(
                contextID, GL_TEXTURE_2D_ARRAY_EXT,_numMipmapLevels,_internalFormat, _textureWidth, _textureHeight, _textureDepth,0);

        textureObject->bind();
        applyTexParameters(GL_TEXTURE_2D_ARRAY_EXT,state);

        extensions->glTexImage3D( GL_TEXTURE_2D_ARRAY_EXT, 0, _internalFormat,
                     _textureWidth, _textureHeight, _textureDepth,
                     _borderWidth,
                     _sourceFormat ? _sourceFormat : _internalFormat,
                     _sourceType ? _sourceType : GL_UNSIGNED_BYTE,
                     0);

    }

    // nothing before, so just unbind the texture target
    else
    {
        glBindTexture( GL_TEXTURE_2D_ARRAY_EXT, 0 );
    }

    // if texture object is now valid and we have to allocate mipmap levels, then
    if (textureObject != 0 && _texMipmapGenerationDirtyList[contextID])
    {
        generateMipmap(state);
    }
}


void Texture2DArray::applyTexImage2DArray_subload(State& state, Image* image, GLsizei layer, GLsizei inwidth, GLsizei inheight, GLsizei indepth, GLint inInternalFormat, GLsizei& numMipmapLevels) const
{
    // if we don't have a valid image we can't create a texture!
    if (!imagesValid())
        return;

    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const GLExtensions* extensions = state.get<GLExtensions>();
    GLenum target = GL_TEXTURE_2D_ARRAY_EXT;

    // compute the internal texture format, this set the _internalFormat to an appropriate value.
    computeInternalFormat();

    // select the internalFormat required for the texture.
    // bool compressed = isCompressedInternalFormat(_internalFormat);
    bool compressed_image = isCompressedInternalFormat((GLenum)image->getPixelFormat());

    // if the required layer is exceeds the maximum allowed layer sizes
    if (indepth > extensions->maxLayerCount)
    {
        // we give a warning and do nothing
        OSG_WARN<<"Warning: Texture2DArray::applyTexImage2DArray_subload(..) the given layer number exceeds the maximum number of supported layers."<<std::endl;
        return;
    }

    //Rescale if resize hint is set or NPOT not supported or dimensions exceed max size
    if( _resizeNonPowerOfTwoHint || !extensions->isNonPowerOfTwoTextureSupported(_min_filter)
        || inwidth > extensions->max2DSize
        || inheight > extensions->max2DSize)
        image->ensureValidSizeForTexturing(extensions->max2DSize);

    // image size or format has changed, this is not allowed, hence return
    if (image->s()!=inwidth ||
        image->t()!=inheight ||
        image->getInternalTextureFormat()!=inInternalFormat )
    {
        OSG_WARN<<"Warning: Texture2DArray::applyTexImage2DArray_subload(..) given image do have wrong dimension or internal format."<<std::endl;
        return;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT,image->getPacking());
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
    glPixelStorei(GL_UNPACK_ROW_LENGTH,image->getRowLength());
#endif

    bool useHardwareMipmapGeneration =
        !image->isMipmap() && _useHardwareMipMapGeneration && extensions->isGenerateMipMapSupported;

    // if no special mipmapping is required, then
    if( _min_filter == LINEAR || _min_filter == NEAREST || useHardwareMipmapGeneration )
    {
        if( _min_filter == LINEAR || _min_filter == NEAREST )
            numMipmapLevels = 1;
        else //Hardware Mipmap Generation
            numMipmapLevels = image->getNumMipmapLevels();

        // upload non-compressed image
        if ( !compressed_image )
        {
            extensions->glTexSubImage3D( target, 0,
                                      0, 0, layer,
                                      inwidth, inheight, indepth,
                                      (GLenum)image->getPixelFormat(),
                                      (GLenum)image->getDataType(),
                                      image->data() );
        }

        // if we support compression and image is compressed, then
        else if (extensions->isCompressedTexImage3DSupported())
        {
            // OSG_WARN<<"glCompressedTexImage3D "<<inwidth<<", "<<inheight<<", "<<indepth<<std::endl;

            GLint blockSize, size;
            getCompressedSize(_internalFormat, inwidth, inheight, 1, blockSize,size);

            extensions->glCompressedTexSubImage3D(target, 0,
                0, 0, layer,
                inwidth, inheight, indepth,
                (GLenum)image->getPixelFormat(),
                size,
                image->data());
        }

    // we want to use mipmapping, so enable it
    }
    else
    {
        // image does not provide mipmaps, so we have to create them
        if( !image->isMipmap() )
        {
            numMipmapLevels = 1;
            OSG_WARN<<"Warning: Texture2DArray::applyTexImage2DArray_subload(..) mipmap layer not passed, and auto mipmap generation turned off or not available. Check texture's min/mag filters & hardware mipmap generation."<<std::endl;

            // the image object does provide mipmaps, so upload the in the certain levels of a layer
        }
        else
        {
            numMipmapLevels = image->getNumMipmapLevels();

            int width  = image->s();
            int height = image->t();

            if( !compressed_image )
            {

                for( GLsizei k = 0 ; k < numMipmapLevels  && (width || height ) ;k++)
                {
                    if (width == 0)
                       width = 1;
                    if (height == 0)
                       height = 1;

                    extensions->glTexSubImage3D( target, k, 0, 0, layer,
                                              width, height, indepth,
                                              (GLenum)image->getPixelFormat(),
                                              (GLenum)image->getDataType(),
                                               image->getMipmapData(k));

                    width >>= 1;
                    height >>= 1;
                }
            }
            else if (extensions->isCompressedTexImage3DSupported())
            {
                GLint blockSize,size;
                for( GLsizei k = 0 ; k < numMipmapLevels  && (width || height) ;k++)
                {
                    if (width == 0)
                        width = 1;
                    if (height == 0)
                        height = 1;

                    getCompressedSize(image->getInternalTextureFormat(), width, height, indepth, blockSize,size);

//                    state.checkGLErrors("before extensions->glCompressedTexSubImage3D(");

                    extensions->glCompressedTexSubImage3D(target, k, 0, 0, layer,
                                                       width, height, indepth,
                                                       (GLenum)image->getPixelFormat(),
                                                       size,
                                                       image->getMipmapData(k));

//                    state.checkGLErrors("after extensions->glCompressedTexSubImage3D(");

                    width >>= 1;
                    height >>= 1;
                }
            }
        }

    }
}


void Texture2DArray::copyTexSubImage2DArray(State& state, int xoffset, int yoffset, int zoffset, int x, int y, int width, int height )
{
    const unsigned int contextID = state.getContextID();
    const GLExtensions* extensions = state.get<GLExtensions>();

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    // if texture object is valid
    if (textureObject != 0)
    {
        textureObject->bind();

        applyTexParameters(GL_TEXTURE_2D_ARRAY_EXT,state);
        extensions->glCopyTexSubImage3D( GL_TEXTURE_2D_ARRAY_EXT, 0, xoffset,yoffset,zoffset, x, y, width, height);

        // inform state that this texture is the current one bound.
        state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);

    }
    else
    {
        OSG_WARN<<"Warning: Texture2DArray::copyTexSubImage2DArray(..) failed, cannot not copy to a non existent texture."<<std::endl;
    }
}

void Texture2DArray::allocateMipmap(State& state) const
{
    const unsigned int contextID = state.getContextID();

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    GLsizei textureDepth = computeTextureDepth();

    if (textureObject && _textureWidth != 0 && _textureHeight != 0 && textureDepth != 0)
    {
        const GLExtensions* extensions = state.get<GLExtensions>();

        int safeSourceFormat = _sourceFormat ? _sourceFormat : _internalFormat;

        // Make sure source format does not contain compressed formats value (like DXT3)
        // they are invalid when passed to glTexImage3D source format parameter
        if( isCompressedInternalFormat( safeSourceFormat ) )
        {
            if( safeSourceFormat != _internalFormat || !extensions->isCompressedTexImage3DSupported() )
                safeSourceFormat = GL_RGBA;
        }

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

            if( isCompressedInternalFormat(safeSourceFormat) )
            {
                int size = 0, blockSize = 0;

                getCompressedSize( _internalFormat, width, height, textureDepth, blockSize, size);

                extensions->glCompressedTexImage3D( GL_TEXTURE_2D_ARRAY_EXT, k, _internalFormat,
                                                    width, height, _textureDepth, _borderWidth,
                                                    size,
                                                    NULL);
            }
            else
            {
                extensions->glTexImage3D( GL_TEXTURE_2D_ARRAY_EXT, k, _internalFormat,
                     width, height, textureDepth, _borderWidth,
                     safeSourceFormat, _sourceType ? _sourceType : GL_UNSIGNED_BYTE,
                     NULL);
            }

            width >>= 1;
            height >>= 1;
        }

        // inform state that this texture is the current one bound.
        state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);
    }
}
