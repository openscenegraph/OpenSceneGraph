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
#include <osg/TextureRectangle>
#include <osg/State>
#include <osg/GLU>
#include <osg/Notify>

#include <osg/Timer>

#ifndef GL_UNPACK_CLIENT_STORAGE_APPLE
#define GL_UNPACK_CLIENT_STORAGE_APPLE    0x85B2
#endif

#ifndef GL_APPLE_vertex_array_range
#define GL_VERTEX_ARRAY_RANGE_APPLE       0x851D
#define GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE 0x851E
#define GL_VERTEX_ARRAY_STORAGE_HINT_APPLE 0x851F
#define GL_VERTEX_ARRAY_RANGE_POINTER_APPLE 0x8521
#define GL_STORAGE_CACHED_APPLE           0x85BE
#define GL_STORAGE_SHARED_APPLE           0x85BF
#endif

// #define DO_TIMING

using namespace osg;

TextureRectangle::TextureRectangle():
    _textureWidth(0),
    _textureHeight(0)
{
    setWrap(WRAP_S, CLAMP);
    setWrap(WRAP_T, CLAMP);

    setFilter(MIN_FILTER, LINEAR);
    setFilter(MAG_FILTER, LINEAR);
}

TextureRectangle::TextureRectangle(Image* image):
            _textureWidth(0),
            _textureHeight(0)
{
    setWrap(WRAP_S, CLAMP);
    setWrap(WRAP_T, CLAMP);

    setFilter(MIN_FILTER, LINEAR);
    setFilter(MAG_FILTER, LINEAR);

    setImage(image);
}

TextureRectangle::TextureRectangle(const TextureRectangle& text,const CopyOp& copyop):
    Texture(text,copyop),
    _textureWidth(text._textureWidth),
    _textureHeight(text._textureHeight),
    _subloadCallback(text._subloadCallback)
{
    setImage(copyop(text._image.get()));
}

TextureRectangle::~TextureRectangle()
{
    setImage(NULL);
}

int TextureRectangle::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types(TextureRectangle,sa)

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
    COMPARE_StateAttribute_Parameter(_subloadCallback)

    return 0; // passed all the above comparison macros, must be equal.
}

void TextureRectangle::setImage(Image* image)
{
    if (_image == image) return;

    if (_image.valid())
    {
        _image->removeClient(this);

        if (_image->requiresUpdateCall())
        {
            setUpdateCallback(0);
            setDataVariance(osg::Object::STATIC);
        }
    }

    // delete old texture objects.
    dirtyTextureObject();

    _image = image;

    if (_image.valid())
    {
        _image->addClient(this);

        if (_image->requiresUpdateCall())
        {
            setUpdateCallback(new Image::UpdateCallback());
            setDataVariance(osg::Object::DYNAMIC);
        }
    }
}


void TextureRectangle::apply(State& state) const
{
    if (!state.get<GLExtensions>()->isRectangleSupported)
    {
        OSG_WARN<<"Warning: TextureRectangle::apply(..) failed, texture rectangle is not support by your OpenGL drivers."<<std::endl;
        return;
    }

    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();

    Texture::TextureObjectManager* tom = Texture::getTextureObjectManager(contextID).get();
    ElapsedTime elapsedTime(&(tom->getApplyTime()));
    tom->getNumberApplied()++;


    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject)
    {
        if (_image.valid() && getModifiedCount(contextID) != _image->getModifiedCount())
        {
            // compute the internal texture format, this set the _internalFormat to an appropriate value.
            computeInternalFormat();

            GLsizei new_width, new_height, new_numMipmapLevels;

            // compute the dimensions of the texture.
            computeRequiredTextureDimensions(state, *_image, new_width, new_height, new_numMipmapLevels);

            if (!textureObject->match(GL_TEXTURE_RECTANGLE, new_numMipmapLevels, _internalFormat, new_width, new_height, 1, _borderWidth))
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

        if (getTextureParameterDirty(state.getContextID()))
            applyTexParameters(GL_TEXTURE_RECTANGLE, state);

        if (_subloadCallback.valid())
        {
            _subloadCallback->subload(*this, state);
        }
        else if (_image.valid() && getModifiedCount(contextID) != _image->getModifiedCount())
        {
            applyTexImage_subload(GL_TEXTURE_RECTANGLE, _image.get(), state, _textureWidth, _textureHeight, _internalFormat);

            // update the modified count to show that it is up to date.
            getModifiedCount(contextID) = _image->getModifiedCount();
        }
    }
    else if (_subloadCallback.valid())
    {
        // we don't have a applyTexImage1D_subload yet so can't reuse.. so just generate a new texture object.
        textureObject = generateAndAssignTextureObject(contextID,GL_TEXTURE_RECTANGLE);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_RECTANGLE, state);

        _subloadCallback->load(*this, state);

        textureObject->setAllocated(1,_internalFormat,_textureWidth,_textureHeight,1,0);

        // in theory the following line is redundant, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        //glBindTexture(GL_TEXTURE_RECTANGLE, handle);
    }
    else if (_image.valid() && _image->data())
    {


        // keep the image around at least till we go out of scope.
        osg::ref_ptr<osg::Image> image = _image;

        // compute the internal texture format, this set the _internalFormat to an appropriate value.
        computeInternalFormat();

        _textureWidth = image->s();
        _textureHeight = image->t();

        textureObject = generateAndAssignTextureObject(
                contextID,GL_TEXTURE_RECTANGLE,1,_internalFormat,_textureWidth,_textureHeight,1,0);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_RECTANGLE, state);

        if (textureObject->isAllocated())
        {
            applyTexImage_subload(GL_TEXTURE_RECTANGLE, _image.get(), state, _textureWidth, _textureHeight, _internalFormat);
        }
        else
        {
            applyTexImage_load(GL_TEXTURE_RECTANGLE, _image.get(), state, _textureWidth, _textureHeight);
            textureObject->setAllocated(true);
        }

        // unref image data?
        if (isSafeToUnrefImageData(state) && _image->getDataVariance()==STATIC)
        {
            TextureRectangle* non_const_this = const_cast<TextureRectangle*>(this);
            non_const_this->_image = NULL;
        }
    }
    else if ( (_textureWidth!=0) && (_textureHeight!=0) && (_internalFormat!=0) )
    {
        textureObject = generateAndAssignTextureObject(
                contextID,GL_TEXTURE_RECTANGLE,0,_internalFormat,_textureWidth,_textureHeight,1,0);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_RECTANGLE,state);

        // no image present, but dimensions at set so lets create the texture
        glTexImage2D( GL_TEXTURE_RECTANGLE, 0, _internalFormat,
                     _textureWidth, _textureHeight, _borderWidth,
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
        glBindTexture(GL_TEXTURE_RECTANGLE, 0);
    }
}

void TextureRectangle::applyTexImage_load(GLenum target, Image* image, State& state, GLsizei& inwidth, GLsizei& inheight) const
{
    // if we don't have a valid image we can't create a texture!
    if (!image || !image->data())
        return;

    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    const GLExtensions* extensions = state.get<GLExtensions>();

    // update the modified count to show that it is up to date.
    getModifiedCount(contextID) = image->getModifiedCount();

    // compute the internal texture format, sets _internalFormat.
    computeInternalFormat();

    glPixelStorei(GL_UNPACK_ALIGNMENT, image->getPacking());
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
    glPixelStorei(GL_UNPACK_ROW_LENGTH,image->getRowLength());
#endif

    bool useClientStorage = extensions->isClientStorageSupported && getClientStorageHint();
    if (useClientStorage)
    {
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE,GL_TRUE);

        #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
            glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_PRIORITY,0.0f);
        #endif

        #ifdef GL_TEXTURE_STORAGE_HINT_APPLE
            glTexParameteri(target, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_CACHED_APPLE);
        #endif
    }

    const unsigned char* dataPtr = image->data();
    GLBufferObject* pbo = image->getOrCreateGLBufferObject(contextID);
    if (pbo)
    {
        state.bindPixelBufferObject(pbo);
        dataPtr = reinterpret_cast<unsigned char*>(pbo->getOffset(image->getBufferIndex()));
    }

    if(isCompressedInternalFormat(_internalFormat) && extensions->isCompressedTexImage2DSupported())
    {
        extensions->glCompressedTexImage2D(target, 0, _internalFormat,
          image->s(), image->t(), 0,
          image->getImageSizeInBytes(),
          dataPtr);
    }
    else
    {
        glTexImage2D(target, 0, _internalFormat,
          image->s(), image->t(), 0,
          (GLenum)image->getPixelFormat(),
          (GLenum)image->getDataType(),
          dataPtr );
    }


    if (pbo)
    {
        state.unbindPixelBufferObject();
    }

    inwidth = image->s();
    inheight = image->t();

    if (useClientStorage)
    {
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE,GL_FALSE);
    }
}

void TextureRectangle::applyTexImage_subload(GLenum target, Image* image, State& state, GLsizei& inwidth, GLsizei& inheight, GLint& inInternalFormat) const
{
    // if we don't have a valid image we can't create a texture!
    if (!image || !image->data())
        return;

    if (image->s()!=inwidth || image->t()!=inheight || image->getInternalTextureFormat()!=inInternalFormat)
    {
        applyTexImage_load(target, image, state, inwidth, inheight);
        return;
    }


    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    const GLExtensions* extensions = state.get<GLExtensions>();


    // update the modified count to show that it is up to date.
    getModifiedCount(contextID) = image->getModifiedCount();

    // compute the internal texture format, sets _internalFormat.
    computeInternalFormat();

    glPixelStorei(GL_UNPACK_ALIGNMENT, image->getPacking());
    unsigned int rowLength = image->getRowLength();

#ifdef DO_TIMING
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    OSG_NOTICE<<"TextureRectangle::apply pixelFormat = "<<std::hex<<image->getPixelFormat()<<std::dec<<std::endl;
#endif
    const unsigned char* dataPtr = image->data();
    GLBufferObject* pbo = image->getOrCreateGLBufferObject(contextID);
    if (pbo)
    {
        state.bindPixelBufferObject(pbo);
        dataPtr = reinterpret_cast<unsigned char*>(pbo->getOffset(image->getBufferIndex()));
        rowLength = 0;
#ifdef DO_TIMING
        OSG_NOTICE<<"after PBO "<<osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick())<<"ms"<<std::endl;
#endif
    }

#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
    glPixelStorei(GL_UNPACK_ROW_LENGTH,rowLength);
#endif

    if(isCompressedInternalFormat(_internalFormat) && extensions->isCompressedTexSubImage2DSupported())
    {
        extensions->glCompressedTexSubImage2D(target, 0,
          0,0,
          image->s(), image->t(),
          (GLenum)image->getPixelFormat(),
          (GLenum)image->getDataType(),
          dataPtr);
    }
    else
    {
        glTexSubImage2D(target, 0,
          0,0,
          image->s(), image->t(),
          (GLenum)image->getPixelFormat(),
          (GLenum)image->getDataType(),
          dataPtr);
    }

    if (pbo)
    {
        state.unbindPixelBufferObject();
    }

#ifdef DO_TIMING
    OSG_NOTICE<<"glTexSubImage2D "<<osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick())<<"ms"<<std::endl;
#endif
}

void TextureRectangle::computeInternalFormat() const
{
    if (_image.valid()) computeInternalFormatWithImage(*_image);
    else computeInternalFormatType();
}

void TextureRectangle::copyTexImage2D(State& state, int x, int y, int width, int height )
{
    const unsigned int contextID = state.getContextID();

    if (_internalFormat==0) _internalFormat=GL_RGBA;

    // get the globj for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject)
    {
        if (width==(int)_textureWidth && height==(int)_textureHeight)
        {
            // we have a valid texture object which is the right size
            // so lets play clever and use copyTexSubImage2D instead.
            // this allows use to reuse the texture object and avoid
            // expensive memory allocations.
            copyTexSubImage2D(state,0 ,0, x, y, width, height);
            return;
        }
        // the relevant texture object is not of the right size so
        // needs to been deleted
        // remove previously bound textures.
        dirtyTextureObject();
        // note, dirtyTextureObject() dirties all the texture objects for
        // this texture, is this right?  Perhaps we should dirty just the
        // one for this context.  Note sure yet will leave till later.
        // RO July 2001.
    }


    // remove any previously assigned images as these are nolonger valid.
    _image = NULL;

    // switch off mip-mapping.
    //
    textureObject = generateAndAssignTextureObject(contextID,GL_TEXTURE_RECTANGLE);

    textureObject->bind();

    applyTexParameters(GL_TEXTURE_RECTANGLE,state);

//    if (hardwareMipMapOn) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_TRUE);

    glCopyTexImage2D( GL_TEXTURE_RECTANGLE, 0, _internalFormat, x, y, width, height, 0 );

//    if (hardwareMipMapOn) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_FALSE);


    _textureWidth = width;
    _textureHeight = height;
//    _numMipmapLevels = 1;

    textureObject->setAllocated(1,_internalFormat,_textureWidth,_textureHeight,1,0);


    // inform state that this texture is the current one bound.
    state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);
}

void TextureRectangle::copyTexSubImage2D(State& state, int xoffset, int yoffset, int x, int y, int width, int height )
{
    const unsigned int contextID = state.getContextID();

    if (_internalFormat==0) _internalFormat=GL_RGBA;

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject)
    {
        // we have a valid image
        textureObject->bind();

        applyTexParameters(GL_TEXTURE_RECTANGLE,state);

//        if (hardwareMipMapOn) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_TRUE);

        glCopyTexSubImage2D( GL_TEXTURE_RECTANGLE, 0, xoffset, yoffset, x, y, width, height);

//        if (hardwareMipMapOn) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_FALSE);

        // inform state that this texture is the current one bound.
        state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);

    }
    else
    {
        // no texture object already exsits for this context so need to
        // create it upfront - simply call copyTexImage2D.
        copyTexImage2D(state,x,y,width,height);
    }
}

void TextureRectangle::allocateMipmap(State&) const
{
    OSG_NOTICE<<"Warning: TextureRectangle::allocateMipmap(State&) called eroneously, GL_TEXTURE_RECTANGLE does not support mipmapping."<<std::endl;
}

