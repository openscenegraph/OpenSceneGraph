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
#include <osg/Texture1D>
#include <osg/State>
#include <osg/GLU>

typedef void (APIENTRY * MyCompressedTexImage1DArbProc) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);

using namespace osg;

Texture1D::Texture1D():
            _textureWidth(0),
            _numMipmapLevels(0)
{
}

Texture1D::Texture1D(const Texture1D& text,const CopyOp& copyop):
            Texture(text,copyop),
            _image(copyop(text._image.get())),
            _textureWidth(text._textureWidth),
            _numMipmapLevels(text._numMipmapLevels),
            _subloadCallback(text._subloadCallback)
{
}

Texture1D::~Texture1D()
{
}

int Texture1D::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Paramter macro's below.
    COMPARE_StateAttribute_Types(Texture1D,sa)

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

    // compare each paramter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_textureWidth)
    COMPARE_StateAttribute_Parameter(_subloadCallback)

    return 0; // passed all the above comparison macro's, must be equal.
}

void Texture1D::setImage(Image* image)
{
    // delete old texture objects.
    dirtyTextureObject();

    _image = image;
    _modifiedCount.setAllElementsTo(0);

}


void Texture1D::apply(State& state) const
{

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject != 0)
    {
        textureObject->bind();

        if (getTextureParameterDirty(state.getContextID())) applyTexParameters(GL_TEXTURE_1D,state);

        if (_subloadCallback.valid())
        {
            _subloadCallback->subload(*this,state);
        }
        else if (_image.valid() && getModifiedCount(contextID) != _image->getModifiedCount())
        {
            applyTexImage1D(GL_TEXTURE_1D,_image.get(),state, _textureWidth, _numMipmapLevels);

            // update the modified count to show that it is upto date.
            getModifiedCount(contextID) = _image->getModifiedCount();
        }

    }
    else if (_subloadCallback.valid())
    {

        // we don't have a applyTexImage1D_subload yet so can't reuse.. so just generate a new texture object.        
        _textureObjectBuffer[contextID] = textureObject = generateTextureObject(contextID,GL_TEXTURE_1D);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_1D,state);

        _subloadCallback->load(*this,state);

        textureObject->setAllocated(_numMipmapLevels,_internalFormat,_textureWidth,1,1,0);

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        //glBindTexture( GL_TEXTURE_1D, handle );

    }
    else if (_image.valid() && _image->data())
    {

        // we don't have a applyTexImage1D_subload yet so can't reuse.. so just generate a new texture object.        
        _textureObjectBuffer[contextID] = textureObject = generateTextureObject(contextID,GL_TEXTURE_1D);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_1D,state);

        applyTexImage1D(GL_TEXTURE_1D,_image.get(),state, _textureWidth, _numMipmapLevels);

        textureObject->setAllocated(_numMipmapLevels,_internalFormat,_textureWidth,1,1,0);

        // update the modified count to show that it is upto date.
        getModifiedCount(contextID) = _image->getModifiedCount();
    
        if (_unrefImageDataAfterApply && areAllTextureObjectsLoaded() && _image->getDataVariance()==STATIC)
        {
            Texture1D* non_const_this = const_cast<Texture1D*>(this);
            non_const_this->_image = 0;
        }
        
    }
    else if ( (_textureWidth!=0) && (_internalFormat!=0) )
    {
        _textureObjectBuffer[contextID] = textureObject = generateTextureObject(
                contextID,GL_TEXTURE_1D,_numMipmapLevels,_internalFormat,_textureWidth,1,1,0);
        
        textureObject->bind();

        applyTexParameters(GL_TEXTURE_1D,state);

        // no image present, but dimensions at set so lets create the texture
        glTexImage1D( GL_TEXTURE_1D, 0, _internalFormat,
                     _textureWidth, _borderWidth,
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
        glBindTexture( GL_TEXTURE_1D, 0 );
    }
}

void Texture1D::computeInternalFormat() const
{
    if (_image.valid()) computeInternalFormatWithImage(*_image); 
}

void Texture1D::applyTexImage1D(GLenum target, Image* image, State& state, GLsizei& inwidth, GLsizei& numMipmapLevels) const
{
    // if we don't have a valid image we can't create a texture!
    if (!image || !image->data())
        return;

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);


    // compute the internal texture format, this set the _internalFormat to an appropriate value.
    computeInternalFormat();

    // select the internalFormat required for the texture.
    bool compressed = isCompressedInternalFormat(_internalFormat);
    
    //Rescale if resize hint is set or NPOT not supported or dimension exceeds max size
    if( _resizeNonPowerOfTwoHint || !extensions->isNonPowerOfTwoTextureSupported(_min_filter) || inwidth > extensions->maxTextureSize() )
    {
        // this is not thread safe... should really create local image data and rescale to that as per Texture2D.
        image->ensureValidSizeForTexturing(extensions->maxTextureSize());
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT,image->getPacking());

    static MyCompressedTexImage1DArbProc glCompressedTexImage1D_ptr = 
        (MyCompressedTexImage1DArbProc)getGLExtensionFuncPtr("glCompressedTexImage1DARB");

    if( _min_filter == LINEAR || _min_filter == NEAREST )
    {
        if ( !compressed )
        {
            numMipmapLevels = 1;
            glTexImage1D( target, 0, _internalFormat,
                image->s(), _borderWidth,
                (GLenum)image->getPixelFormat(),
                (GLenum)image->getDataType(),
                image->data() );

        }
        else if(glCompressedTexImage1D_ptr)
        {
            numMipmapLevels = 1;
            GLint blockSize = ( _internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? 8 : 16 ); 
            GLint size = ((image->s()+3)/4)*((image->t()+3)/4)*blockSize;
            glCompressedTexImage1D_ptr(target, 0, _internalFormat, 
                  image->s(), _borderWidth, 
                  size, 
                  image->data());                

        }

    }
    else
    {
        if(!image->isMipmap())
        {

            numMipmapLevels = 1;

            gluBuild1DMipmaps( target, _internalFormat,
                image->s(),
                (GLenum)image->getPixelFormat(), (GLenum)image->getDataType(),
                image->data() );

        }
        else
        {
            numMipmapLevels = image->getNumMipmapLevels();

            int width  = image->s();

            if( !compressed )
            {
                for( GLsizei k = 0 ; k < numMipmapLevels  && width ;k++)
                {

                    glTexImage1D( target, k, _internalFormat,
                         width,_borderWidth,
                        (GLenum)image->getPixelFormat(),
                        (GLenum)image->getDataType(),
                        image->getMipmapData(k));

                    width >>= 1;
                }
            }
            else if(glCompressedTexImage1D_ptr)
            {
                GLint blockSize = ( _internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? 8 : 16 ); 
                GLint size = 0; 
                for( GLsizei k = 0 ; k < numMipmapLevels  && width ;k++)
                {

                    size = ((width+3)/4)*blockSize;
                    glCompressedTexImage1D_ptr(target, k, _internalFormat, 
                        width,  _borderWidth, size, image->getMipmapData(k));                

                    width >>= 1;
                }
            }
        }

    }

    inwidth = image->s();
}

void Texture1D::copyTexImage1D(State& state, int x, int y, int width)
{
    const unsigned int contextID = state.getContextID();

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject != 0)
    {
        if (width==(int)_textureWidth)
        {
            // we have a valid texture object which is the right size
            // so lets play clever and use copyTexSubImage1D instead.
            // this allows use to reuse the texture object and avoid
            // expensive memory allocations.
            copyTexSubImage1D(state,0 ,x, y, width);
            return;
        }
        // the relevent texture object is not of the right size so
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
    _min_filter = LINEAR;
    _mag_filter = LINEAR;

    _textureObjectBuffer[contextID] = textureObject = generateTextureObject(contextID,GL_TEXTURE_1D);

    textureObject->bind();


    applyTexParameters(GL_TEXTURE_1D,state);
    glCopyTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, x, y, width, 0 );

    _textureWidth = width;
    _numMipmapLevels = 1;
    
    textureObject->setAllocated(_numMipmapLevels,_internalFormat,_textureWidth,1,1,0);

    // inform state that this texture is the current one bound.
    state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);
}

void Texture1D::copyTexSubImage1D(State& state, int xoffset, int x, int y, int width)
{
    const unsigned int contextID = state.getContextID();

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject != 0)
    {

        textureObject->bind();

        // we have a valid image
        applyTexParameters(GL_TEXTURE_1D,state);
        glCopyTexSubImage1D( GL_TEXTURE_1D, 0, xoffset, x, y, width);

        // inform state that this texture is the current one bound.
        state.haveAppliedTextureAttribute(state.getActiveTextureUnit(), this);

    }
    else
    {
        // no texture object already exsits for this context so need to
        // create it upfront - simply call copyTexImage1D.
        copyTexImage1D(state,x,y,width);
    }
}
