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
#include <osg/GLExtensions>
#include <osg/Texture3D>
#include <osg/State>
#include <osg/GLU>
#include <osg/Notify>

#include <string.h>



using namespace osg;

Texture3D::Texture3D():
            _textureWidth(0),
            _textureHeight(0),
            _textureDepth(0),
            _numMimpmapLevels(0)
{
}

Texture3D::Texture3D(const Texture3D& text,const CopyOp& copyop):
            Texture(text,copyop),
            _image(copyop(text._image.get())),
            _textureWidth(text._textureWidth),
            _textureHeight(text._textureHeight),
            _textureDepth(text._textureDepth),
            _numMimpmapLevels(text._numMimpmapLevels),
            _subloadCallback(text._subloadCallback)
{
}

Texture3D::~Texture3D()
{
}

int Texture3D::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Paramter macro's below.
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

    int result = compareTexture(rhs);
    if (result!=0) return result;

    // compare each paramter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_textureWidth)
    COMPARE_StateAttribute_Parameter(_textureHeight)
    COMPARE_StateAttribute_Parameter(_textureDepth)
    COMPARE_StateAttribute_Parameter(_subloadCallback)

    return 0; // passed all the above comparison macro's, must be equal.
}

void Texture3D::setImage(Image* image)
{
    // delete old texture objects.
    dirtyTextureObject();

    _modifiedTag.setAllElementsTo(0);

    _image = image;
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

    // get the globj for the current contextID.
    GLuint& handle = getTextureObject(contextID);

    if (handle != 0)
    {

        glBindTexture( GL_TEXTURE_3D, handle );
        if (getTextureParameterDirty(state.getContextID())) applyTexParameters(GL_TEXTURE_3D,state);

        if (_subloadCallback.valid())
        {
            _subloadCallback->subload(*this,state);
        }
        else if (_image.get() && getModifiedTag(contextID) != _image->getModifiedTag())
        {
            applyTexImage3D(GL_TEXTURE_3D,_image.get(),state, _textureWidth, _textureHeight, _textureDepth,_numMimpmapLevels);

            // update the modified tag to show that it is upto date.
            getModifiedTag(contextID) = _image->getModifiedTag();
        }

    }
    else if (_subloadCallback.valid())
    {

        glGenTextures( 1L, (GLuint *)&handle );
        glBindTexture( GL_TEXTURE_3D, handle );

        applyTexParameters(GL_TEXTURE_3D,state);

        _subloadCallback->load(*this,state);

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        glBindTexture( GL_TEXTURE_3D, handle );

    }
    else if (_image.valid() && _image->data())
    {

        glGenTextures( 1L, (GLuint *)&handle );
        glBindTexture( GL_TEXTURE_3D, handle );

        applyTexParameters(GL_TEXTURE_3D,state);

        applyTexImage3D(GL_TEXTURE_3D,_image.get(),state, _textureWidth, _textureHeight, _textureDepth,_numMimpmapLevels);

        // update the modified tag to show that it is upto date.
        getModifiedTag(contextID) = _image->getModifiedTag();

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        glBindTexture( GL_TEXTURE_3D, handle );
        
    }
    else
    {
        glBindTexture( GL_TEXTURE_3D, 0 );
    }
}

void Texture3D::computeInternalFormat() const
{
    if (_image.valid()) computeInternalFormatWithImage(*_image); 
}

void Texture3D::applyTexImage3D(GLenum target, Image* image, State& state, GLsizei& inwidth, GLsizei& inheight, GLsizei& indepth, GLsizei& numMimpmapLevels) const
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
    if (compressed)
    {
        notify(WARN)<<"Warning::cannot currently use compressed format with 3D textures."<<std::endl;
        return;
    }    
    
    image->ensureValidSizeForTexturing(extensions->maxTexture3DSize());

    glPixelStorei(GL_UNPACK_ALIGNMENT,image->getPacking());

    if( _min_filter == LINEAR || _min_filter == NEAREST )
    {
        numMimpmapLevels = 1;
        extensions->glTexImage3D( target, 0, _internalFormat,
                                  image->s(), image->t(), image->r(), 0,
                                  (GLenum)image->getPixelFormat(),
                                  (GLenum)image->getDataType(),
                                  image->data() );
    }
    else
    {
        if(!image->isMipmap())
        {

            numMimpmapLevels = 1;

            extensions->gluBuild3DMipmaps( target, _internalFormat,
                                           image->s(),image->t(),image->r(),
                                           (GLenum)image->getPixelFormat(), (GLenum)image->getDataType(),
                                           image->data() );

        }
        else
        {
            numMimpmapLevels = image->getNumMipmapLevels();

            int width  = image->s();
            int height = image->t();
            int depth = image->r();

            for( GLsizei k = 0 ; k < numMimpmapLevels  && (width || height || depth) ;k++)
            {

                if (width == 0)
                    width = 1;
                if (height == 0)
                    height = 1;
                if (depth == 0)
                    depth = 1;

                extensions->glTexImage3D( target, k, _internalFormat,
                                          width, height, depth, 0,
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

    // get the globj for the current contextID.
    GLuint& handle = getTextureObject(contextID);
    const Extensions* extensions = getExtensions(contextID,true);
    
    if (handle)
    {

        // we have a valid image
        glBindTexture( GL_TEXTURE_3D, handle );
        applyTexParameters(GL_TEXTURE_3D,state);
        extensions->glCopyTexSubImage3D( GL_TEXTURE_3D, 0, xoffset,yoffset,zoffset, x, y, width, height);

        /* Redundant, delete later */
        glBindTexture( GL_TEXTURE_3D, handle );

        // inform state that this texture is the current one bound.
        state.haveAppliedAttribute(this);

    }
    else
    {
        notify(WARN)<<"Warning: Texture3D::copyTexSubImage3D(..) failed, cannot not copy to a non existant texture."<<std::endl;
    }
}

typedef buffered_value< ref_ptr<Texture3D::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

const Texture3D::Extensions* Texture3D::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions;
    return s_extensions[contextID].get();
}

void Texture3D::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

#ifndef GL_MAX_3D_TEXTURE_SIZE
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#endif

Texture3D::Extensions::Extensions()
{
    setupGLExtenions();
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
    if (!rhs._glCopyTexSubImage3D)                  _glCopyTexSubImage3D = 0;
    if (!rhs._gluBuild3DMipmaps)                    _gluBuild3DMipmaps = 0;
}

void Texture3D::Extensions::setupGLExtenions()
{
    _isTexture3DFast = isGLExtensionSupported("GL_EXT_texture3D");

    if (_isTexture3DFast) _isTexture3DSupported = true;
    else _isTexture3DSupported = strncmp((const char*)glGetString(GL_VERSION),"1.2",3)>=0;
    
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &_maxTexture3DSize);

    _glTexImage3D           = getGLExtensionFuncPtr("glTexImage3D","glTexImage3DEXT");;
    _glTexSubImage3D        = getGLExtensionFuncPtr("glTexSubImage3D","glTexSubImage3DEXT");
    _glCopyTexSubImage3D    = getGLExtensionFuncPtr("glCopyTexSubImage3D","glCopyTexSubImage3DEXT");
    _gluBuild3DMipmaps      = getGLExtensionFuncPtr("gluBuild3DMipmaps");

}

void Texture3D::Extensions::glTexImage3D( GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels) const
{
//    ::glTexImage3D( target, level, internalFormat, width, height, depth, border, format, type, pixels);
    if (_glTexImage3D)
    {
        typedef void (APIENTRY * GLTexImage3DProc)      ( GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
        ((GLTexImage3DProc)_glTexImage3D)( target, level, internalFormat, width, height, depth, border, format, type, pixels);
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
        typedef void (APIENTRY * GLTexSubImage3DProc)   ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
        ((GLTexSubImage3DProc)_glTexSubImage3D)( target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    }
    else
    {
        notify(WARN)<<"Error: glTexSubImage3D not supported by OpenGL driver"<<std::endl;
    }
}

void Texture3D::Extensions::glCopyTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height ) const
{
//    ::glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
    if (_glCopyTexSubImage3D)
    {
        typedef void (APIENTRY * GLCopyTexSubImageProc) ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height );
        ((GLCopyTexSubImageProc)_glCopyTexSubImage3D)(target, level, xoffset, yoffset, zoffset, x, y, width, height);
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
        typedef void (APIENTRY * GLUBuild3DMipMapsProc) ( GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *data);
        ((GLUBuild3DMipMapsProc)_gluBuild3DMipmaps)(target, internalFormat, width, height, depth, format, type, data);
    }
    else
    {
        notify(WARN)<<"Error: gluBuild3DMipmaps not supported by OpenGL driver"<<std::endl;
    }
}
