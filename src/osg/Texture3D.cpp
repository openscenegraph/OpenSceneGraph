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
#include <osg/Texture3D>
#include <osg/State>
#include <osg/ImageSequence>
#include <osg/GLU>
#include <osg/Notify>

#include <string.h>



using namespace osg;

Texture3D::Texture3D():
            _textureWidth(0),
            _textureHeight(0),
            _textureDepth(0),
            _numMipmapLevels(0)
{
}


Texture3D::Texture3D(Image* image):
            _textureWidth(0),
            _textureHeight(0),
            _textureDepth(0),
            _numMipmapLevels(0)
{
    setImage(image);
}

Texture3D::Texture3D(const Texture3D& text,const CopyOp& copyop):
            Texture(text,copyop),
            _image(copyop(text._image.get())),
            _textureWidth(text._textureWidth),
            _textureHeight(text._textureHeight),
            _textureDepth(text._textureDepth),
            _numMipmapLevels(text._numMipmapLevels),
            _subloadCallback(text._subloadCallback)
{
}

Texture3D::~Texture3D()
{
}

int Texture3D::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macro's below.
    COMPARE_StateAttribute_Types(Texture3D,sa)

    if (_image!=rhs._image) // smart pointer comparison.
    {
        if (_image.valid())
        {
            if (rhs._image.valid())
            {
                int result = _image->compare(*rhs._image);
                if (result!=0) return result;
            }
            else
            {
                return 1; // valid lhs._image is greater than null. 
            }
        }
        else if (rhs._image.valid()) 
        {
            return -1; // valid rhs._image is greater than null. 
        }
    }

    if (!_image && !rhs._image)
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
    COMPARE_StateAttribute_Parameter(_textureDepth)
    COMPARE_StateAttribute_Parameter(_subloadCallback)

    return 0; // passed all the above comparison macro's, must be equal.
}

void Texture3D::setImage(Image* image)
{
    if (_image == image) return;

    if (dynamic_cast<osg::ImageSequence*>(_image.get()))
    {
        setUpdateCallback(0);
        setDataVariance(osg::Object::STATIC);
    }

    // delete old texture objects.
    dirtyTextureObject();

    _modifiedCount.setAllElementsTo(0);

    _image = image;
    
    if (dynamic_cast<osg::ImageSequence*>(_image.get()))
    {
        setUpdateCallback(new ImageSequence::UpdateCallback());
        setDataVariance(osg::Object::DYNAMIC);
    }
}

void Texture3D::computeRequiredTextureDimensions(State& state, const osg::Image& image,GLsizei& inwidth, GLsizei& inheight,GLsizei& indepth, GLsizei& numMipmapLevels) const
{
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);
    const Texture::Extensions* texExtensions = Texture::getExtensions(contextID,true);

    int width,height,depth;

    if( !_resizeNonPowerOfTwoHint && texExtensions->isNonPowerOfTwoTextureSupported(_min_filter) )
    {
        width = image.s();
        height = image.t();
        depth = image.r();
    }
    else
    {
        width = Image::computeNearestPowerOfTwo(image.s()-2*_borderWidth)+2*_borderWidth;
        height = Image::computeNearestPowerOfTwo(image.t()-2*_borderWidth)+2*_borderWidth;
        depth = Image::computeNearestPowerOfTwo(image.r()-2*_borderWidth)+2*_borderWidth;
    }

    // cap the size to what the graphics hardware can handle.
    if (width>extensions->maxTexture3DSize()) width = extensions->maxTexture3DSize();
    if (height>extensions->maxTexture3DSize()) height = extensions->maxTexture3DSize();
    if (depth>extensions->maxTexture3DSize()) depth = extensions->maxTexture3DSize();
    
    inwidth = width;
    inheight = height;
    indepth = depth;
    
    bool useHardwareMipMapGeneration = !image.isMipmap() && _useHardwareMipMapGeneration && texExtensions->isGenerateMipMapSupported();

    if( _min_filter == LINEAR || _min_filter == NEAREST || useHardwareMipMapGeneration )
    {
        numMipmapLevels = 1;
    }
    else if( image.isMipmap() )
    {
        numMipmapLevels = image.getNumMipmapLevels();
    }
    else
    {
        numMipmapLevels = 0;
        for( ; (width || height || depth) ;++numMipmapLevels)
        {

            if (width == 0)
                width = 1;
            if (height == 0)
                height = 1;
            if (depth == 0)
                depth = 1;

            width >>= 1;
            height >>= 1;
            depth >>= 1;
        }    
    }
}

void Texture3D::apply(State& state) const
{

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    
    const Extensions* extensions = getExtensions(contextID,true);
                                        
    if (!extensions->isTexture3DSupported())
    {
        notify(WARN)<<"Warning: Texture3D::apply(..) failed, 3D texturing is not support by OpenGL driver."<<std::endl;
        return;
    }

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);
    
    if (textureObject)
    {
        // we have a valid image
        textureObject->bind();

        if (getTextureParameterDirty(state.getContextID())) applyTexParameters(GL_TEXTURE_3D,state);

        if (_subloadCallback.valid())
        {
            _subloadCallback->subload(*this,state);
        }
        else if (_image.get() && getModifiedCount(contextID) != _image->getModifiedCount())
        {
           computeRequiredTextureDimensions(state,*_image,_textureWidth, _textureHeight, _textureDepth,_numMipmapLevels);

            applyTexImage3D(GL_TEXTURE_3D,_image.get(),state, _textureWidth, _textureHeight, _textureDepth,_numMipmapLevels);

            // update the modified count to show that it is upto date.
            getModifiedCount(contextID) = _image->getModifiedCount();
        }

    }
    else if (_subloadCallback.valid())
    {

        _textureObjectBuffer[contextID] = textureObject = generateTextureObject(contextID,GL_TEXTURE_3D);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_3D,state);

        _subloadCallback->load(*this,state);

        textureObject->setAllocated(_numMipmapLevels,_internalFormat,_textureWidth,_textureHeight,_textureDepth,0);

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        //glBindTexture( GL_TEXTURE_3D, handle );

    }
    else if (_image.valid() && _image->data())
    {

        // compute the internal texture format, this set the _internalFormat to an appropriate value.
        computeInternalFormat();

        // compute the dimensions of the texture.
        computeRequiredTextureDimensions(state,*_image,_textureWidth, _textureHeight, _textureDepth,_numMipmapLevels);

        textureObject = generateTextureObject(contextID,GL_TEXTURE_3D);

        textureObject->bind();


        applyTexParameters(GL_TEXTURE_3D,state);

        applyTexImage3D(GL_TEXTURE_3D,_image.get(),state, _textureWidth, _textureHeight, _textureDepth,_numMipmapLevels);

        textureObject->setAllocated(_numMipmapLevels,_internalFormat,_textureWidth,_textureHeight,_textureDepth,0);

        // update the modified count to show that it is upto date.
        getModifiedCount(contextID) = _image->getModifiedCount();

        _textureObjectBuffer[contextID] = textureObject;

        if (_unrefImageDataAfterApply && areAllTextureObjectsLoaded() && _image->getDataVariance()==STATIC)
        {
            Texture3D* non_const_this = const_cast<Texture3D*>(this);
            non_const_this->_image = 0;
        }

    }
    else if ( (_textureWidth!=0) && (_textureHeight!=0) && (_textureDepth!=0) && (_internalFormat!=0) )
    {
        _textureObjectBuffer[contextID] = textureObject = generateTextureObject(
                contextID,GL_TEXTURE_3D,_numMipmapLevels,_internalFormat,_textureWidth,_textureHeight,_textureDepth,0);
        
        textureObject->bind();

        applyTexParameters(GL_TEXTURE_3D,state);

        // no image present, but dimensions at set so lets create the texture
        extensions->glTexImage3D( GL_TEXTURE_3D, 0, _internalFormat,
                     _textureWidth, _textureHeight, _textureDepth,
                     _borderWidth,
                     _sourceFormat ? _sourceFormat : _internalFormat,
                     _sourceType ? _sourceType : GL_UNSIGNED_BYTE,
                     0);                
                     
        if (_readPBuffer.valid())
        {
            _readPBuffer->bindPBufferToTexture(GL_FRONT);
        }
        
    }
    else
    {
        glBindTexture( GL_TEXTURE_3D, 0 );
    }
    
    // if texture object is now valid and we have to allocate mipmap levels, then
    if (textureObject != 0 && _texMipmapGenerationDirtyList[contextID])
    {
        generateMipmap(state);
    }
}

void Texture3D::computeInternalFormat() const
{
    if (_image.valid()) computeInternalFormatWithImage(*_image); 
    else computeInternalFormatType();
}

void Texture3D::applyTexImage3D(GLenum target, Image* image, State& state, GLsizei& inwidth, GLsizei& inheight, GLsizei& indepth, GLsizei& numMipmapLevels) const
{
    // if we don't have a valid image we can't create a texture!
    if (!image || !image->data())
        return;

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);    
    const Texture::Extensions* texExtensions = Texture::getExtensions(contextID,true);

    // compute the internal texture format, this set the _internalFormat to an appropriate value.
    computeInternalFormat();

    // select the internalFormat required for the texture.
    bool compressed = isCompressedInternalFormat(_internalFormat);
    bool compressed_image = isCompressedInternalFormat((GLenum)image->getPixelFormat());

    if (compressed)
    {
        //notify(WARN)<<"Warning::cannot currently use compressed format with 3D textures."<<std::endl;
        //return;
    }    
    
    //Rescale if resize hint is set or NPOT not supported or dimensions exceed max size
    if( _resizeNonPowerOfTwoHint || !texExtensions->isNonPowerOfTwoTextureSupported(_min_filter)
        || inwidth > extensions->maxTexture3DSize()
        || inheight > extensions->maxTexture3DSize()
        || indepth > extensions->maxTexture3DSize() )
        image->ensureValidSizeForTexturing(extensions->maxTexture3DSize());

    glPixelStorei(GL_UNPACK_ALIGNMENT,image->getPacking());

    bool useHardwareMipMapGeneration = !image->isMipmap() && _useHardwareMipMapGeneration && texExtensions->isGenerateMipMapSupported();

    if( _min_filter == LINEAR || _min_filter == NEAREST || useHardwareMipMapGeneration )
    {
        bool hardwareMipMapOn = false;
        if (_min_filter != LINEAR && _min_filter != NEAREST) 
        {
            if (useHardwareMipMapGeneration) glTexParameteri(GL_TEXTURE_3D, GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
            hardwareMipMapOn = true;
        }

        numMipmapLevels = 1;

        if (!compressed_image)
        {
            extensions->glTexImage3D( target, 0, _internalFormat,
                                      inwidth, inheight, indepth,
                                      _borderWidth,
                                      (GLenum)image->getPixelFormat(),
                                      (GLenum)image->getDataType(),
                                      image->data() );
        }
        else if (extensions->isCompressedTexImage3DSupported())
        {
            // notify(WARN)<<"glCompressedTexImage3D "<<inwidth<<", "<<inheight<<", "<<indepth<<std::endl;
            numMipmapLevels = 1;

            GLint blockSize, size;
            getCompressedSize(_internalFormat, inwidth, inheight, indepth, blockSize,size);

            extensions->glCompressedTexImage3D(target, 0, _internalFormat, 
                inwidth, inheight, indepth, 
                _borderWidth,
                size, 
                image->data());
        }

        if (hardwareMipMapOn) glTexParameteri(GL_TEXTURE_3D, GL_GENERATE_MIPMAP_SGIS,GL_FALSE);
    }
    else
    {
        if(!image->isMipmap())
        {

            numMipmapLevels = 1;

            extensions->gluBuild3DMipmaps( target, _internalFormat,
                                           image->s(),image->t(),image->r(),
                                           (GLenum)image->getPixelFormat(), (GLenum)image->getDataType(),
                                           image->data() );

        }
        else
        {
            numMipmapLevels = image->getNumMipmapLevels();

            int width  = image->s();
            int height = image->t();
            int depth = image->r();

            for( GLsizei k = 0 ; k < numMipmapLevels  && (width || height || depth) ;k++)
            {

                if (width == 0)
                    width = 1;
                if (height == 0)
                    height = 1;
                if (depth == 0)
                    depth = 1;

                extensions->glTexImage3D( target, k, _internalFormat,
                                          width, height, depth, _borderWidth,
                                          (GLenum)image->getPixelFormat(),
                                          (GLenum)image->getDataType(),
                                          image->getMipmapData(k));

                width >>= 1;
                height >>= 1;
                depth >>= 1;
            }
        }

    }

    inwidth  = image->s();
    inheight = image->t();
    indepth  = image->r();
    
}

void Texture3D::copyTexSubImage3D(State& state, int xoffset, int yoffset, int zoffset, int x, int y, int width, int height )
{
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject != 0)
    {
        textureObject->bind();

        applyTexParameters(GL_TEXTURE_3D,state);
        extensions->glCopyTexSubImage3D( GL_TEXTURE_3D, 0, xoffset,yoffset,zoffset, x, y, width, height);

        /* Redundant, delete later */
        //glBindTexture( GL_TEXTURE_3D, handle );

        // inform state that this texture is the current one bound.
        state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);

    }
    else
    {
        notify(WARN)<<"Warning: Texture3D::copyTexSubImage3D(..) failed, cannot not copy to a non existant texture."<<std::endl;
    }
}

void Texture3D::allocateMipmap(State& state) const
{
    const unsigned int contextID = state.getContextID();

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);
    
    if (textureObject && _textureWidth != 0 && _textureHeight != 0 && _textureDepth != 0)
    {
        const Extensions* extensions = getExtensions(contextID,true);
    
        // bind texture
        textureObject->bind();

        // compute number of mipmap levels
        int width = _textureWidth;
        int height = _textureHeight;
        int depth = _textureDepth;
        int numMipmapLevels = Image::computeNumberOfMipmapLevels(width, height, depth);

        // we do not reallocate the level 0, since it was already allocated
        width >>= 1;
        height >>= 1;
        depth >>= 1;
                
        for( GLsizei k = 1; k < numMipmapLevels  && (width || height || depth); k++)
        {
            if (width == 0)
                width = 1;
            if (height == 0)
                height = 1;
            if (depth == 0)
                depth = 1;

            extensions->glTexImage3D( GL_TEXTURE_3D, k, _internalFormat,
                     width, height, depth, _borderWidth,
                     _sourceFormat ? _sourceFormat : _internalFormat,
                     _sourceType ? _sourceType : GL_UNSIGNED_BYTE, NULL);

            width >>= 1;
            height >>= 1;
            depth >>= 1;
        }
                
        // inform state that this texture is the current one bound.
        state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);        
    }
}

typedef buffered_value< ref_ptr<Texture3D::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

Texture3D::Extensions* Texture3D::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void Texture3D::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

#ifndef GL_MAX_3D_TEXTURE_SIZE
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#endif

Texture3D::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

Texture3D::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isTexture3DSupported = rhs._isTexture3DSupported;
    _isTexture3DFast = rhs._isTexture3DFast;
    _maxTexture3DSize = rhs._maxTexture3DSize;

    _glTexImage3D = rhs._glTexImage3D;
    _glTexSubImage3D = rhs._glTexSubImage3D;
    _glCopyTexSubImage3D = rhs._glCopyTexSubImage3D;
    _gluBuild3DMipmaps = rhs._gluBuild3DMipmaps;
}

void Texture3D::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isTexture3DSupported)                 _isTexture3DSupported = false;
    if (!rhs._isTexture3DFast)                      _isTexture3DFast = false;
    if (rhs._maxTexture3DSize<_maxTexture3DSize)    _maxTexture3DSize = rhs._maxTexture3DSize;

    if (!rhs._glTexImage3D)                         _glTexImage3D = 0;
    if (!rhs._glTexSubImage3D)                      _glTexSubImage3D = 0;
    if (!rhs._glCompressedTexImage3D)               _glTexImage3D = 0;
    if (!rhs._glCompressedTexSubImage3D)            _glTexSubImage3D = 0;
    if (!rhs._glCopyTexSubImage3D)                  _glCopyTexSubImage3D = 0;
    if (!rhs._gluBuild3DMipmaps)                    _gluBuild3DMipmaps = 0;
}

void Texture3D::Extensions::setupGLExtensions(unsigned int contextID)
{
    _isTexture3DFast = isGLExtensionSupported(contextID,"GL_EXT_texture3D");

    if (_isTexture3DFast) _isTexture3DSupported = true;
    else _isTexture3DSupported = strncmp((const char*)glGetString(GL_VERSION),"1.2",3)>=0;
    
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &_maxTexture3DSize);

    setGLExtensionFuncPtr(_glTexImage3D,"glTexImage3D","glTexImage3DEXT");
    setGLExtensionFuncPtr(_glTexSubImage3D,"glTexSubImage3D","glTexSubImage3DEXT");
    setGLExtensionFuncPtr(_glCompressedTexImage3D,"glCompressedTexImage3D","glCompressedTexImage3DARB");
    setGLExtensionFuncPtr(_glCompressedTexSubImage3D,"glCompressedTexSubImage3D","glCompressedTexSubImage3DARB");
    setGLExtensionFuncPtr(_glCopyTexSubImage3D,"glCopyTexSubImage3D","glCopyTexSubImage3DEXT");
    setGLExtensionFuncPtr(_gluBuild3DMipmaps,"gluBuild3DMipmaps");

}

void Texture3D::Extensions::glTexImage3D( GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels) const
{
//    ::glTexImage3D( target, level, internalFormat, width, height, depth, border, format, type, pixels);
    if (_glTexImage3D)
    {
        _glTexImage3D( target, level, internalFormat, width, height, depth, border, format, type, pixels);
    }
    else
    {
        notify(WARN)<<"Error: glTexImage3D not supported by OpenGL driver"<<std::endl;
    }
}

void Texture3D::Extensions::glTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels) const
{
//    ::glTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    if (_glTexSubImage3D)
    {
        _glTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    }
    else
    {
        notify(WARN)<<"Error: glTexSubImage3D not supported by OpenGL driver"<<std::endl;
    }
}

void Texture3D::Extensions::glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data) const
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

void Texture3D::Extensions::glCompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data ) const
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

void Texture3D::Extensions::glCopyTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height ) const
{
//    ::glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
    if (_glCopyTexSubImage3D)
    {
        _glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
    }
    else
    {
        notify(WARN)<<"Error: glCopyTexSubImage3D not supported by OpenGL driver"<<std::endl;
    }
}

void Texture3D::Extensions::gluBuild3DMipmaps( GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *data) const
{
//    ::gluBuild3DMipmaps(target, internalFormat, width, height, depth, format, type, data);
    if (_gluBuild3DMipmaps)
    {
        _gluBuild3DMipmaps(target, internalFormat, width, height, depth, format, type, data);
    }
    else
    {
        notify(WARN)<<"Error: gluBuild3DMipmaps not supported by OpenGL driver"<<std::endl;
    }
}
