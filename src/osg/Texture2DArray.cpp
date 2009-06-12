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
#include <osg/ImageSequence>
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
            _textureDepth(text._textureDepth),
            _numMipmapLevels(text._numMipmapLevels),
            _subloadCallback(text._subloadCallback)
{
    // copy all images by iterating through all of them
    for (int i=0; i < text._textureDepth; i++)
    {
        _images.push_back(copyop(text._images[i].get()));
        _modifiedCount.push_back(ImageModifiedCount());
    }
}

Texture2DArray::~Texture2DArray()
{
}

int Texture2DArray::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macro's below.
    COMPARE_StateAttribute_Types(Texture2DArray,sa)

    bool noImages = true;
    for (int n=0; n < _textureDepth; n++)
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

    return 0; // passed all the above comparison macro's, must be equal.
}

void Texture2DArray::setImage(unsigned int layer, Image* image)
{
    // check if the layer exceeds the texture depth
    if (static_cast<int>(layer) >= _textureDepth)
    {
        // print warning and do nothing
        notify(WARN)<<"Warning: Texture2DArray::setImage(..) failed, the given layer number is bigger then the size of the texture array."<<std::endl;
        return;
    }
    
    if (_images[layer] == image) return;

    unsigned numImageSequencesBefore = 0;
    for (unsigned int i=0; i<getNumImages(); ++i)
    {
        osg::ImageSequence* is = dynamic_cast<osg::ImageSequence*>(_images[i].get());
        if (is) ++numImageSequencesBefore;
    }

    // set image
    _images[layer] = image;
   _modifiedCount[layer].setAllElementsTo(0);

    // find out if we need to reset the update callback to handle the animation of ImageSequence
    unsigned numImageSequencesAfter = 0;
    for (unsigned int i=0; i<getNumImages(); ++i)
    {
        osg::ImageSequence* is = dynamic_cast<osg::ImageSequence*>(_images[i].get());
        if (is) ++numImageSequencesAfter;
    }

    if (numImageSequencesBefore>0)
    {
        if (numImageSequencesAfter==0)
        {
            setUpdateCallback(0);
            setDataVariance(osg::Object::STATIC);
        }
    }
    else if (numImageSequencesAfter>0)
    {
        setUpdateCallback(new ImageSequence::UpdateCallback());
        setDataVariance(osg::Object::DYNAMIC);
    }
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
    if (depth < _textureDepth)
    {
        _images.resize(depth);
        _modifiedCount.resize(depth);
    }
    
    // if we increase the array, then add new empty elements
    if (depth > _textureDepth)
    {
        _images.resize(depth, ref_ptr<Image>(0));
        _modifiedCount.resize(depth, ImageModifiedCount());
    }
        
    // resize the texture array
    _textureDepth = depth;
}

Image* Texture2DArray::getImage(unsigned int layer)
{
    return _images[layer].get();
}

const Image* Texture2DArray::getImage(unsigned int layer) const
{
    return _images[layer].get();
}

bool Texture2DArray::imagesValid() const
{
    if (_textureDepth < 1) return false;
    for (int n=0; n < _textureDepth; n++)
    {
        if (!_images[n].valid() || !_images[n]->data())
            return false;
    }
    return true;
}

void Texture2DArray::computeInternalFormat() const
{
    if (imagesValid()) computeInternalFormatWithImage(*_images[0]); 
    else computeInternalFormatType();
}


void Texture2DArray::apply(State& state) const
{
    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    // if not supported, then return
    if (!extensions->isTexture2DArraySupported() || !extensions->isTexture3DSupported())
    {
        notify(WARN)<<"Warning: Texture2DArray::apply(..) failed, 2D texture arrays are not support by OpenGL driver."<<std::endl;
        return;
    }
    
    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    // if we already have an texture object, then 
    if (textureObject != 0)
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
            // for each image of the texture array do
            for (GLsizei n=0; n < _textureDepth; n++)
            {
                osg::Image* image = _images[n].get();
                
                // if image content is modified, then upload it to the GPU memory
                if (image && getModifiedCount(n,contextID) != image->getModifiedCount())
                {
                    applyTexImage2DArray_subload(state, image, _textureWidth, _textureHeight, n, _internalFormat, _numMipmapLevels);
                    getModifiedCount(n,contextID) = image->getModifiedCount();
                }
            }
        }

    }
    
    // there is no texture object, but exists a subload callback, so use it to upload images
    else if (_subloadCallback.valid())
    {
        // generate texture (i.e. glGenTexture) and apply parameters
        _textureObjectBuffer[contextID] = textureObject = generateTextureObject(contextID, GL_TEXTURE_2D_ARRAY_EXT);
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
        textureObject = generateTextureObject(
                contextID,GL_TEXTURE_2D_ARRAY_EXT,_numMipmapLevels,_internalFormat,_textureWidth,_textureHeight,_textureDepth,0);
        
        // bind texture
        textureObject->bind();
        applyTexParameters(GL_TEXTURE_2D_ARRAY_EXT, state);

        // first we need to allocate the texture memory
        extensions->glTexImage3D( GL_TEXTURE_2D_ARRAY_EXT, 0, _internalFormat,
                     _textureWidth, _textureHeight, _textureDepth,
                     _borderWidth,
                     _sourceFormat ? _sourceFormat : _internalFormat,
                     _sourceType ? _sourceType : GL_UNSIGNED_BYTE,
                     0); 

        // now for each layer we upload it into the memory
        for (GLsizei n=0; n<_textureDepth; n++)
        {
            // if image is valid then upload it to the texture memory
            osg::Image* image = _images[n].get();
            if (image)
            {
                // now load the image data into the memory, this will also check if image do have valid properties
                applyTexImage2DArray_subload(state, image, _textureWidth, _textureHeight, n, _internalFormat, _numMipmapLevels);
                getModifiedCount(n,contextID) = image->getModifiedCount();
            }
        }
        textureObject->setAllocated(_numMipmapLevels,_internalFormat,_textureWidth,_textureHeight,_textureDepth,0);

        _textureObjectBuffer[contextID] = textureObject;
        
        // no idea what this for ;-)
        if (_unrefImageDataAfterApply && areAllTextureObjectsLoaded())
        {
            Texture2DArray* non_const_this = const_cast<Texture2DArray*>(this);
            for (int n=0; n<_textureDepth; n++)
            {                
                if (_images[n].valid() && _images[n]->getDataVariance()==STATIC)
                {
                    non_const_this->_images[n] = 0;
                }
            }
        }
        
    }
    
    // No images present, but dimensions are set. So create empty texture
    else if ( (_textureWidth > 0) && (_textureHeight > 0) && (_textureDepth > 0) && (_internalFormat!=0) )
    {
        // generate texture 
        _textureObjectBuffer[contextID] = textureObject = generateTextureObject(
                contextID, GL_TEXTURE_2D_ARRAY_EXT,_numMipmapLevels,_internalFormat,_textureWidth,_textureHeight,_textureDepth,0);
        
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

void Texture2DArray::applyTexImage2DArray_subload(State& state, Image* image, GLsizei inwidth, GLsizei inheight, GLsizei indepth, GLint inInternalFormat, GLsizei& numMipmapLevels) const
{
    // if we don't have a valid image we can't create a texture!
    if (!imagesValid())
        return;

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);    
    const Texture::Extensions* texExtensions = Texture::getExtensions(contextID,true);
    GLenum target = GL_TEXTURE_2D_ARRAY_EXT;
    
    // compute the internal texture format, this set the _internalFormat to an appropriate value.
    computeInternalFormat();

    // select the internalFormat required for the texture.
    // bool compressed = isCompressedInternalFormat(_internalFormat);
    bool compressed_image = isCompressedInternalFormat((GLenum)image->getPixelFormat());

    // if the required layer is exceeds the maximum allowed layer sizes
    if (indepth > extensions->maxLayerCount())
    {
        // we give a warning and do nothing
        notify(WARN)<<"Warning: Texture2DArray::applyTexImage2DArray_subload(..) the given layer number exceeds the maximum number of supported layers."<<std::endl;
        return;        
    }

    //Rescale if resize hint is set or NPOT not supported or dimensions exceed max size
    if( _resizeNonPowerOfTwoHint || !texExtensions->isNonPowerOfTwoTextureSupported(_min_filter)
        || inwidth > extensions->max2DSize()
        || inheight > extensions->max2DSize())
        image->ensureValidSizeForTexturing(extensions->max2DSize());

    // image size or format has changed, this is not allowed, hence return
    if (image->s()!=inwidth || 
        image->t()!=inheight || 
        image->getInternalTextureFormat()!=inInternalFormat ) 
    {
        notify(WARN)<<"Warning: Texture2DArray::applyTexImage2DArray_subload(..) given image do have wrong dimension or internal format."<<std::endl;
        return;        
    }    
    
    glPixelStorei(GL_UNPACK_ALIGNMENT,image->getPacking());

    // if no special mipmapping is required, then
    if( _min_filter == LINEAR || _min_filter == NEAREST )
    {
        numMipmapLevels = 1;

        // upload non-compressed image
        if (!compressed_image)
        {
            extensions->glTexSubImage3D( target, 0,
                                      0, 0, indepth,
                                      inwidth, inheight, 1,
                                      (GLenum)image->getPixelFormat(),
                                      (GLenum)image->getDataType(),
                                      image->data() );
        }
        
        // if we support compression and image is compressed, then
        else if (extensions->isCompressedTexImage3DSupported())
        {
            // notify(WARN)<<"glCompressedTexImage3D "<<inwidth<<", "<<inheight<<", "<<indepth<<std::endl;
            numMipmapLevels = 1;

            GLint blockSize, size;
            getCompressedSize(_internalFormat, inwidth, inheight, 1, blockSize,size);

            extensions->glCompressedTexSubImage3D(target, 0,
                0, 0, indepth,  
                inwidth, inheight, 1, 
                (GLenum)image->getPixelFormat(),
                size, 
                image->data());
        }

    // we want to use mipmapping, so enable it
    }else
    {
        // image does not provide mipmaps, so we have to create them
        if(!image->isMipmap())
        {
            notify(WARN)<<"Warning: Texture2DArray::applyTexImage2DArray_subload(..) automagic mipmap generation is currently not implemented. Check texture's min/mag filters."<<std::endl;

        // the image object does provide mipmaps, so upload the in the certain levels of a layer
        }else
        {
            numMipmapLevels = image->getNumMipmapLevels();

            int width  = image->s();
            int height = image->t();
            
            for( GLsizei k = 0 ; k < numMipmapLevels  && (width || height ) ;k++)
            {

                if (width == 0)
                    width = 1;
                if (height == 0)
                    height = 1;

                extensions->glTexSubImage3D( target, k, 0, 0, indepth,
                                          width, height, 1, 
                                          (GLenum)image->getPixelFormat(),
                                          (GLenum)image->getDataType(),
                                          image->getMipmapData(k));

                width >>= 1;
                height >>= 1;
            }
        }

    }
}


void Texture2DArray::copyTexSubImage2DArray(State& state, int xoffset, int yoffset, int zoffset, int x, int y, int width, int height )
{
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

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
        notify(WARN)<<"Warning: Texture2DArray::copyTexSubImage2DArray(..) failed, cannot not copy to a non existant texture."<<std::endl;
    }
}

void Texture2DArray::allocateMipmap(State& state) const
{
    const unsigned int contextID = state.getContextID();

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);
    
    if (textureObject && _textureWidth != 0 && _textureHeight != 0 && _textureDepth != 0)
    {
        const Extensions* extensions = getExtensions(contextID,true);
        // const Texture::Extensions* texExtensions = Texture::getExtensions(contextID,true);
        
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

            extensions->glTexImage3D( GL_TEXTURE_2D_ARRAY_EXT, k, _internalFormat,
                     width, height, _textureDepth, _borderWidth,
                     _sourceFormat ? _sourceFormat : _internalFormat,
                     _sourceType ? _sourceType : GL_UNSIGNED_BYTE, NULL);

            width >>= 1;
            height >>= 1;
        }
                
        // inform state that this texture is the current one bound.
        state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);        
    }
}

typedef buffered_value< ref_ptr<Texture2DArray::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

Texture2DArray::Extensions* Texture2DArray::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void Texture2DArray::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

Texture2DArray::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

Texture2DArray::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isTexture3DSupported = rhs._isTexture3DSupported;
    _isTexture2DArraySupported = rhs._isTexture2DArraySupported;
    
    _max2DSize = rhs._max2DSize;
    _maxLayerCount = rhs._maxLayerCount;
    
    _glTexImage3D = rhs._glTexImage3D;
    _glTexSubImage3D = rhs._glTexSubImage3D;
    _glCopyTexSubImage3D = rhs._glCopyTexSubImage3D;
    _glCompressedTexImage3D = rhs._glCompressedTexImage3D;
    _glCompressedTexSubImage3D = rhs._glCompressedTexSubImage3D;;
}

void Texture2DArray::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isTexture3DSupported)                 _isTexture3DSupported = false;
    if (!rhs._isTexture2DArraySupported)            _isTexture2DArraySupported = false;
    if (rhs._max2DSize<_max2DSize)                  _max2DSize = rhs._max2DSize;
    if (rhs._maxLayerCount<_maxLayerCount)          _maxLayerCount = rhs._maxLayerCount;

    if (!rhs._glTexImage3D)                         _glTexImage3D = 0;
    if (!rhs._glTexSubImage3D)                      _glTexSubImage3D = 0;
    if (!rhs._glCompressedTexImage3D)               _glTexImage3D = 0;
    if (!rhs._glCompressedTexSubImage3D)            _glTexSubImage3D = 0;
    if (!rhs._glCopyTexSubImage3D)                  _glCopyTexSubImage3D = 0;
}

void Texture2DArray::Extensions::setupGLExtensions(unsigned int contextID)
{
    _isTexture3DSupported = isGLExtensionSupported(contextID,"GL_EXT_texture3D");
    _isTexture2DArraySupported = isGLExtensionSupported(contextID,"GL_EXT_texture_array");

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_max2DSize);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS_EXT, &_maxLayerCount);

    setGLExtensionFuncPtr(_glTexImage3D, "glTexImage3D","glTexImage3DEXT");
    setGLExtensionFuncPtr(_glTexSubImage3D, "glTexSubImage3D","glTexSubImage3DEXT");
    setGLExtensionFuncPtr(_glCompressedTexImage3D, "glCompressedTexImage3D","glCompressedTexImage3DARB");
    setGLExtensionFuncPtr(_glCompressedTexSubImage3D, "glCompressedTexSubImage3D","glCompressedTexSubImage3DARB");
    setGLExtensionFuncPtr(_glCopyTexSubImage3D, "glCopyTexSubImage3D","glCopyTexSubImage3DEXT");
}

void Texture2DArray::Extensions::glTexImage3D( GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels) const
{
    if (_glTexImage3D)
    {
        _glTexImage3D( target, level, internalFormat, width, height, depth, border, format, type, pixels);
    }
    else
    {
        notify(WARN)<<"Error: glTexImage3D not supported by OpenGL driver"<<std::endl;
    }
}

void Texture2DArray::Extensions::glTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels) const
{
    if (_glTexSubImage3D)
    {
        _glTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    }
    else
    {
        notify(WARN)<<"Error: glTexSubImage3D not supported by OpenGL driver"<<std::endl;
    }
}

void Texture2DArray::Extensions::glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data) const
{
    if (_glCompressedTexImage3D)
    {
        _glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
    }
    else
    {
        notify(WARN)<<"Error: glCompressedTexImage3D not supported by OpenGL driver"<<std::endl;
    }
}

void Texture2DArray::Extensions::glCompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data ) const
{
    if (_glCompressedTexSubImage3D)
    {
        _glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
    }
    else
    {
        notify(WARN)<<"Error: glCompressedTexImage2D not supported by OpenGL driver"<<std::endl;
    }
}

void Texture2DArray::Extensions::glCopyTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height ) const
{
    if (_glCopyTexSubImage3D)
    {
        _glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
    }
    else
    {
        notify(WARN)<<"Error: glCopyTexSubImage3D not supported by OpenGL driver"<<std::endl;
    }
}

