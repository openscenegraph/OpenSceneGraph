#include <osg/GLExtensions>
#include <osg/Texture>

#ifdef TEXTURE_USE_DEPRECATED_API

#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <osg/ref_ptr>
#include <osg/Image>
#include <osg/State>
#include <osg/Notify>

#include <osg/GLU>

using namespace osg;

Texture::Texture():
            _textureWidth(0),
            _textureHeight(0),
            _numMimpmapLevels(0),
            _subloadMode(OFF),
            _subloadTextureOffsetX(0),
            _subloadTextureOffsetY(0),
            _subloadImageOffsetX(0),
            _subloadImageOffsetY(0),
            _subloadImageWidth(0),
            _subloadImageHeight(0)
{
}

Texture::Texture(const Texture& text,const CopyOp& copyop):
            TextureBase(text,copyop),
            _image(copyop(text._image.get())),
            _textureWidth(text._textureWidth),
            _textureHeight(text._textureHeight),
            _numMimpmapLevels(text._numMimpmapLevels),
            _subloadMode(text._subloadMode),
            _subloadTextureOffsetX(text._subloadTextureOffsetX),
            _subloadTextureOffsetY(text._subloadTextureOffsetY),
            _subloadImageOffsetX(text._subloadImageOffsetX),
            _subloadImageOffsetY(text._subloadImageOffsetY),
            _subloadImageWidth(text._subloadImageWidth),
            _subloadImageHeight(text._subloadImageHeight),
            _subloadCallback(text._subloadCallback)
{}

Texture::~Texture()
{
}

int Texture::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Paramter macro's below.
    COMPARE_StateAttribute_Types(Texture,sa)

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
    COMPARE_StateAttribute_Parameter(_textureHeight)
    COMPARE_StateAttribute_Parameter(_subloadMode)
    COMPARE_StateAttribute_Parameter(_subloadTextureOffsetX)
    COMPARE_StateAttribute_Parameter(_subloadTextureOffsetY)
    COMPARE_StateAttribute_Parameter(_subloadImageOffsetX)
    COMPARE_StateAttribute_Parameter(_subloadImageOffsetY)
    COMPARE_StateAttribute_Parameter(_subloadImageWidth)
    COMPARE_StateAttribute_Parameter(_subloadImageHeight)

    return 0; // passed all the above comparison macro's, must be equal.
}

void Texture::setImage(Image* image)
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

void Texture::apply(State& state) const
{

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const uint contextID = state.getContextID();

    // get the globj for the current contextID.
    GLuint& handle = getTextureObject(contextID);

    if (handle != 0)
    {
        if (_subloadMode == OFF)
        {
            glBindTexture( GL_TEXTURE_2D, handle );
            if (_texParametersDirty) applyTexParameters(GL_TEXTURE_2D,state);
        }
        else  if (_image.valid() && _image->data())
        {
            glBindTexture( GL_TEXTURE_2D, handle );
            if (_texParametersDirty) applyTexParameters(GL_TEXTURE_2D,state);

            uint& modifiedTag = getModifiedTag(contextID);
            if (_subloadMode == AUTO ||
                (_subloadMode == IF_DIRTY && modifiedTag != _image->getModifiedTag()))
            {
                glPixelStorei(GL_UNPACK_ROW_LENGTH,_image->s());

                glTexSubImage2D(GL_TEXTURE_2D, 0,
                                _subloadTextureOffsetX, _subloadTextureOffsetY,
                                (_subloadImageWidth>0)?_subloadImageWidth:_image->s(), (_subloadImageHeight>0)?_subloadImageHeight:_image->t(),
                                (GLenum) _image->getPixelFormat(), (GLenum) _image->getDataType(),
                                _image->data(_subloadImageOffsetX,_subloadImageOffsetY));

                glPixelStorei(GL_UNPACK_ROW_LENGTH,0);

                // update the modified flag to show that the image has been loaded.
                modifiedTag = _image->getModifiedTag();
            }
            else if (_subloadMode == USE_CALLBACK)
            {
                _subloadCallback->subload(GL_TEXTURE_2D,*this,state);
            }
            
        }
    }
    else if (_image.valid() && _image->data())
    {

        glGenTextures( 1L, (GLuint *)&handle );
        glBindTexture( GL_TEXTURE_2D, handle );

        applyTexParameters(GL_TEXTURE_2D,state);
        applyTexImage2D(GL_TEXTURE_2D,_image.get(),state, _textureWidth, _textureHeight, _numMimpmapLevels);

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        glBindTexture( GL_TEXTURE_2D, handle );

    }
    else
    {
        glBindTexture( GL_TEXTURE_2D, 0 );
    }
}

void Texture::computeInternalFormat() const
{
    if (_image.valid()) computeInternalFormatWithImage(*_image); 
}


void Texture::copyTexImage2D(State& state, int x, int y, int width, int height )
{
    const uint contextID = state.getContextID();

    // get the globj for the current contextID.
    GLuint& handle = getTextureObject(contextID);
    
    if (handle)
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

    glBindTexture( GL_TEXTURE_2D, handle );
    applyTexParameters(GL_TEXTURE_2D,state);
    glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, x, y, width, height, 0 );


    _textureWidth = width;
    _textureHeight = height;

    // inform state that this texture is the current one bound.
    state.haveAppliedAttribute(this);
}

void Texture::copyTexSubImage2D(State& state, int xoffset, int yoffset, int x, int y, int width, int height )
{
    const uint contextID = state.getContextID();

    // get the globj for the current contextID.
    GLuint& handle = getTextureObject(contextID);
    
    if (handle)
    {

        // we have a valid image
        glBindTexture( GL_TEXTURE_2D, handle );
        applyTexParameters(GL_TEXTURE_2D,state);
        glCopyTexSubImage2D( GL_TEXTURE_2D, 0, xoffset,yoffset, x, y, width, height);

        /* Redundant, delete later */
        glBindTexture( GL_TEXTURE_2D, handle );

        // inform state that this texture is the current one bound.
        state.haveAppliedAttribute(this);

    }
    else
    {
        // no texture object already exsits for this context so need to
        // create it upfront - simply call copyTexImage2D.
        copyTexImage2D(state,x,y,width,height);
    }
}

#endif // USE_DEPRECATED_API
