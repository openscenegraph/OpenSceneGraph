#include <osg/GLExtensions>
#include <osg/Texture1D>
#include <osg/State>
#include <osg/GLU>

typedef void (APIENTRY * MyCompressedTexImage1DArbProc) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);

using namespace osg;

Texture1D::Texture1D():
            _textureWidth(0),
            _numMimpmapLevels(0)
{
}

Texture1D::Texture1D(const Texture1D& text,const CopyOp& copyop):
            TextureBase(text,copyop),
            _image(copyop(text._image.get())),
            _textureWidth(text._textureWidth),
            _numMimpmapLevels(text._numMimpmapLevels),
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
                if (_image->getFileName()<rhs._image->getFileName()) return -1;
                else if (_image->getFileName()>rhs._image->getFileName()) return 1;;
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

    int result = compareTextureBase(rhs);
    if (result!=0) return result;

    // compare each paramter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_textureWidth)
    COMPARE_StateAttribute_Parameter(_subloadCallback)

    return 0; // passed all the above comparison macro's, must be equal.
}

void Texture1D::setImage(Image* image)
{
    // delete old texture objects.
    for(TextureNameList::iterator itr=_handleList.begin();
                               itr!=_handleList.end();
                               ++itr)
    {
        if (*itr != 0)
        {
            // contact global texture object handler to delete texture objects
            // in appropriate context.
            // glDeleteTextures( 1L, (const GLuint *)itr );
            *itr = 0;
        }
    }

    _image = image;
}


void Texture1D::apply(State& state) const
{

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const uint contextID = state.getContextID();

    // get the globj for the current contextID.
    GLuint& handle = getTextureObject(contextID);

    if (handle != 0)
    {

        glBindTexture( GL_TEXTURE_1D, handle );
        if (_texParamtersDirty) applyTexParameters(GL_TEXTURE_1D,state);

        if (_subloadCallback.valid())
        {
            _subloadCallback->subload(*this,state);
        }

    }
    else if (_subloadCallback.valid())
    {

        glGenTextures( 1L, (GLuint *)&handle );
        glBindTexture( GL_TEXTURE_1D, handle );

        applyTexParameters(GL_TEXTURE_1D,state);

        _subloadCallback->load(*this,state);

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        glBindTexture( GL_TEXTURE_1D, handle );

    }
    else if (_image.valid() && _image->data())
    {

        glGenTextures( 1L, (GLuint *)&handle );
        glBindTexture( GL_TEXTURE_1D, handle );

        applyTexParameters(GL_TEXTURE_1D,state);

        applyTexImage1D(GL_TEXTURE_1D,_image.get(),state, _textureWidth, _numMimpmapLevels);

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        glBindTexture( GL_TEXTURE_1D, handle );
        
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

void Texture1D::applyTexImage1D(GLenum target, Image* image, State& state, GLsizei& inwidth, GLsizei& numMimpmapLevels) const
{
    // if we don't have a valid image we can't create a texture!
    if (!image || !image->data())
        return;

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const uint contextID = state.getContextID();

    // update the modified tag to show that it is upto date.
    getModifiedTag(contextID) = image->getModifiedTag();
    

    // compute the internal texture format, this set the _internalFormat to an appropriate value.
    computeInternalFormat();

    // select the internalFormat required for the texture.
    bool compressed = isCompressedInternalFormat(_internalFormat);
    
    image->ensureValidSizeForTexturing();

    glPixelStorei(GL_UNPACK_ALIGNMENT,image->getPacking());

    static MyCompressedTexImage1DArbProc glCompressedTexImage1D_ptr = 
        (MyCompressedTexImage1DArbProc)getGLExtensionFuncPtr("glCompressedTexImage1DARB");

    if( _min_filter == LINEAR || _min_filter == NEAREST )
    {
        if ( !compressed )
        {
            numMimpmapLevels = 1;
            glTexImage1D( target, 0, _internalFormat,
                image->s(), 0,
                (GLenum)image->getPixelFormat(),
                (GLenum)image->getDataType(),
                image->data() );

        }
        else if(glCompressedTexImage1D_ptr)
        {
            numMimpmapLevels = 1;
            GLint blockSize = ( _internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? 8 : 16 ); 
            GLint size = ((image->s()+3)/4)*((image->t()+3)/4)*blockSize;
            glCompressedTexImage1D_ptr(target, 0, _internalFormat, 
                  image->s(), 0, 
                  size, 
                  image->data());                

        }

    }
    else
    {
        if(!image->isMipmap())
        {

            numMimpmapLevels = 1;

            gluBuild1DMipmaps( target, _internalFormat,
                image->s(),
                (GLenum)image->getPixelFormat(), (GLenum)image->getDataType(),
                image->data() );

        }
        else
        {
            numMimpmapLevels = image->getNumMipmapLevels();

            int width  = image->s();

            if( !compressed )
            {
                for( GLsizei k = 0 ; k < numMimpmapLevels  && width ;k++)
                {

                    glTexImage1D( target, k, _internalFormat,
                         width,0,
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
                for( GLsizei k = 0 ; k < numMimpmapLevels  && width ;k++)
                {

                    size = ((width+3)/4)*blockSize;
                    glCompressedTexImage1D_ptr(target, k, _internalFormat, 
                        width,  0, size, image->getMipmapData(k));                

                    width >>= 1;
                }
            }
        }

    }

    inwidth = image->s();
}

void Texture1D::copyTexImage1D(State& state, int x, int y, int width)
{
    const uint contextID = state.getContextID();

    // get the globj for the current contextID.
    GLuint& handle = getTextureObject(contextID);
    
    if (handle)
    {
        if (width==(int)_textureWidth)
        {
            // we have a valid texture object which is the right size
            // so lets play clever and use copyTexSubImage2D instead.
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

    // Get a new 2d texture handle.
    glGenTextures( 1, &handle );

    glBindTexture( GL_TEXTURE_1D, handle );
    applyTexParameters(GL_TEXTURE_1D,state);
    glCopyTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, x, y, width, 0 );

    _textureWidth = width;
    
    // inform state that this texture is the current one bound.
    state.haveAppliedAttribute(this);
}

void Texture1D::copyTexSubImage1D(State& state, int xoffset, int x, int y, int width)
{
    const uint contextID = state.getContextID();

    // get the globj for the current contextID.
    GLuint& handle = getTextureObject(contextID);
    
    if (handle)
    {

        // we have a valid image
        glBindTexture( GL_TEXTURE_1D, handle );
        applyTexParameters(GL_TEXTURE_1D,state);
        glCopyTexSubImage1D( GL_TEXTURE_1D, 0, xoffset, x, y, width);

        /* Redundant, delete later */
        glBindTexture( GL_TEXTURE_1D, handle );

        // inform state that this texture is the current one bound.
        state.haveAppliedAttribute(this);

    }
    else
    {
        // no texture object already exsits for this context so need to
        // create it upfront - simply call copyTexImage1D.
        copyTexImage1D(state,x,y,width);
    }
}
