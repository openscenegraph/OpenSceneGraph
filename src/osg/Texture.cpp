#include <osg/GLExtensions>
#include <osg/Image>
#include <osg/Texture>
#include <osg/State>
#include <osg/Notify>
#include <osg/GLU>

using namespace osg;

#ifndef 
#define GL_TEXTURE_WRAP_R                 0x8072
#endif

Texture::Texture():
            _wrap_s(CLAMP),
            _wrap_t(CLAMP),
            _wrap_r(CLAMP),
            _min_filter(LINEAR_MIPMAP_LINEAR), // trilinear
            _mag_filter(LINEAR),
            _maxAnisotropy(1.0f),
            _borderColor(0.0, 0.0, 0.0, 0.0),
            _internalFormatMode(USE_IMAGE_DATA_FORMAT),
            _internalFormat(0)
{
}

Texture::Texture(const Texture& text,const CopyOp& copyop):
            StateAttribute(text,copyop),
            _wrap_s(text._wrap_s),
            _wrap_t(text._wrap_t),
            _wrap_r(text._wrap_r),
            _min_filter(text._min_filter),
            _mag_filter(text._mag_filter),
            _maxAnisotropy(text._maxAnisotropy),
            _borderColor(text._borderColor),
            _internalFormatMode(text._internalFormatMode),
            _internalFormat(text._internalFormat)
{
}

Texture::~Texture()
{
    // delete old texture objects.
    dirtyTextureObject();
}

int Texture::compareTexture(const Texture& rhs) const
{
    COMPARE_StateAttribute_Parameter(_wrap_s)
    COMPARE_StateAttribute_Parameter(_wrap_t)
    COMPARE_StateAttribute_Parameter(_wrap_r)
    COMPARE_StateAttribute_Parameter(_min_filter)
    COMPARE_StateAttribute_Parameter(_mag_filter)
    COMPARE_StateAttribute_Parameter(_maxAnisotropy)
    COMPARE_StateAttribute_Parameter(_internalFormatMode)
    COMPARE_StateAttribute_Parameter(_internalFormat)
    
    return 0;
}


void Texture::setWrap(WrapParameter which, WrapMode wrap)
{
    switch( which )
    {
        case WRAP_S : _wrap_s = wrap; dirtyTextureParameters(); break;
        case WRAP_T : _wrap_t = wrap; dirtyTextureParameters(); break;
        case WRAP_R : _wrap_r = wrap; dirtyTextureParameters(); break;
        default : notify(WARN)<<"Error: invalid 'which' passed Texture::setWrap("<<(unsigned int)which<<","<<(unsigned int)wrap<<")"<<std::endl; break;
    }
    
}


Texture::WrapMode Texture::getWrap(WrapParameter which) const
{
    switch( which )
    {
        case WRAP_S : return _wrap_s;
        case WRAP_T : return _wrap_t;
        case WRAP_R : return _wrap_r;
        default : notify(WARN)<<"Error: invalid 'which' passed Texture::getWrap(which)"<<std::endl; return _wrap_s;
    }
}


void Texture::setFilter(FilterParameter which, FilterMode filter)
{
    switch( which )
    {
        case MIN_FILTER : _min_filter = filter; dirtyTextureParameters(); break;
        case MAG_FILTER : _mag_filter = filter; dirtyTextureParameters(); break;
        default : notify(WARN)<<"Error: invalid 'which' passed Texture::setFilter("<<(unsigned int)which<<","<<(unsigned int)filter<<")"<<std::endl; break;
    }
}


Texture::FilterMode Texture::getFilter(FilterParameter which) const
{
    switch( which )
    {
        case MIN_FILTER : return _min_filter;
        case MAG_FILTER : return _mag_filter;
        default : notify(WARN)<<"Error: invalid 'which' passed Texture::getFilter(which)"<< std::endl; return _min_filter;
    }
}

void Texture::setMaxAnisotropy(float anis)
{
    if (_maxAnisotropy!=anis)
    {
        _maxAnisotropy = anis; 
        dirtyTextureParameters();
    }
}

/** Force a recompile on next apply() of associated OpenGL texture objects.*/
void Texture::dirtyTextureObject()
{
    for(uint i=0;i<_handleList.size();++i)
    {
        if (_handleList[i] != 0)
        {
            Texture::deleteTextureObject(i,_handleList[i]);
            _handleList[i] = 0;
        }
    }
}

void Texture::dirtyTextureParameters()
{
    for(uint i=0;i<_texParametersDirtyList.size();++i)
    {
        _texParametersDirtyList[i] = 1;
    }
}

void Texture::computeInternalFormatWithImage(osg::Image& image) const
{
    const uint contextID = 0; // state.getContextID();  // set to 0 right now, assume same paramters for each graphics context...
    const Extensions* extensions = getExtensions(contextID,true);

//    static bool s_ARB_Compression = isGLExtensionSupported("GL_ARB_texture_compression");
//    static bool s_S3TC_Compression = isGLExtensionSupported("GL_EXT_texture_compression_s3tc");

    GLint internalFormat = image.getInternalTextureFormat();
    switch(_internalFormatMode)
    {
        case(USE_IMAGE_DATA_FORMAT):
            internalFormat = image.getInternalTextureFormat();
            break;

        case(USE_ARB_COMPRESSION):
            if (extensions->isTextureCompressionARBSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(1): internalFormat = GL_COMPRESSED_ALPHA_ARB; break;
                    case(2): internalFormat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB; break;
                    case(3): internalFormat = GL_COMPRESSED_RGB_ARB; break;
                    case(4): internalFormat = GL_COMPRESSED_RGBA_ARB; break;
                    case(GL_RGB): internalFormat = GL_COMPRESSED_RGB_ARB; break;
                    case(GL_RGBA): internalFormat = GL_COMPRESSED_RGBA_ARB; break;
                    case(GL_ALPHA): internalFormat = GL_COMPRESSED_ALPHA_ARB; break;
                    case(GL_LUMINANCE): internalFormat = GL_COMPRESSED_LUMINANCE_ARB; break;
                    case(GL_LUMINANCE_ALPHA): internalFormat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB; break;
                    case(GL_INTENSITY): internalFormat = GL_COMPRESSED_INTENSITY_ARB; break;
                }
            }
            else internalFormat = image.getInternalTextureFormat();
            break;

        case(USE_S3TC_DXT1_COMPRESSION):
            if (extensions->isTextureCompressionS3TCSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(3):        internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(4):        internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
                    case(GL_RGB):   internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
                    default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            else internalFormat = image.getInternalTextureFormat();
            break;

        case(USE_S3TC_DXT3_COMPRESSION):
            if (extensions->isTextureCompressionS3TCSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(3):
                    case(GL_RGB):   internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(4):
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
                    default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            else internalFormat = image.getInternalTextureFormat();
            break;

        case(USE_S3TC_DXT5_COMPRESSION):
            if (extensions->isTextureCompressionS3TCSupported())
            {
                switch(image.getPixelFormat())
                {
                    case(3):
                    case(GL_RGB):   internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(4):
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
                    default:        internalFormat = image.getInternalTextureFormat(); break;
                }
            }
            else internalFormat = image.getInternalTextureFormat();
            break;

        case(USE_USER_DEFINED_FORMAT):
            internalFormat = _internalFormat;
            break;

    }
    
    _internalFormat = internalFormat;
}

bool Texture::isCompressedInternalFormat() const
{
    return isCompressedInternalFormat(getInternalFormat());
}

bool Texture::isCompressedInternalFormat(GLint internalFormat) const
{
    switch(internalFormat)
    {
        case(GL_COMPRESSED_ALPHA_ARB):
        case(GL_COMPRESSED_INTENSITY_ARB):
        case(GL_COMPRESSED_LUMINANCE_ALPHA_ARB):
        case(GL_COMPRESSED_LUMINANCE_ARB):
        case(GL_COMPRESSED_RGBA_ARB):
        case(GL_COMPRESSED_RGB_ARB):
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT):
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT):
            return true;
        default:
            return false;
    }
}

void Texture::applyTexParameters(GLenum target, State& state) const
{
    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const uint contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    WrapMode ws = _wrap_s, wt = _wrap_t, wr = _wrap_r;

    // GL_IBM_texture_mirrored_repeat, fall-back REPEAT
    if (!extensions->isTextureMirroredRepeatSupported())
    {
        if (ws == MIRROR)
            ws = REPEAT;
        if (wt == MIRROR)
            wt = REPEAT;
        if (wr == MIRROR)
            wr = REPEAT;
    }

    // GL_EXT_texture_edge_clamp, fall-back CLAMP
    if (!extensions->isTextureEdgeClampSupported())
    {
        if (ws == CLAMP_TO_EDGE)
            ws = CLAMP;
        if (wt == CLAMP_TO_EDGE)
            wt = CLAMP;
        if (wr == CLAMP_TO_EDGE)
            wr = CLAMP;
    }

    if(!extensions->isTextureBorderClampSupported())
    {
        if(ws == CLAMP_TO_BORDER)
            ws = CLAMP;
        if(wt == CLAMP_TO_BORDER)
            wt = CLAMP;
        if(wr == CLAMP_TO_BORDER)
            wr = CLAMP;
    }

    glTexParameteri( target, GL_TEXTURE_WRAP_S, ws );
    glTexParameteri( target, GL_TEXTURE_WRAP_T, wt );
    glTexParameteri( target, GL_TEXTURE_WRAP_R, wr );

    glTexParameteri( target, GL_TEXTURE_MIN_FILTER, _min_filter);
    glTexParameteri( target, GL_TEXTURE_MAG_FILTER, _mag_filter);

    if (_maxAnisotropy>1.0f && extensions->isTextureFilterAnisotropicSupported())
    {
        // note, GL_TEXTURE_MAX_ANISOTROPY_EXT will either be defined
        // by gl.h (or via glext.h) or by include/osg/Texture.
        glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, _maxAnisotropy);
    }

    if (extensions->isTextureBorderClampSupported())
    {
        glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, _borderColor.ptr());
    }

    getTextureParameterDirty(state.getContextID()) = false;

}

void Texture::applyTexImage2D(GLenum target, Image* image, State& state, GLsizei& inwidth, GLsizei& inheight,GLsizei& numMimpmapLevels) const
{
    // if we don't have a valid image we can't create a texture!
    if (!image || !image->data())
        return;

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const uint contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    // update the modified tag to show that it is upto date.
    getModifiedTag(contextID) = image->getModifiedTag();
    

    // compute the internal texture format, this set the _internalFormat to an appropriate value.
    computeInternalFormat();

    // select the internalFormat required for the texture.
    bool compressed = isCompressedInternalFormat(_internalFormat);
    
    image->ensureValidSizeForTexturing(extensions->maxTextureSize());

    glPixelStorei(GL_UNPACK_ALIGNMENT,image->getPacking());

    if( _min_filter == LINEAR || _min_filter == NEAREST )
    {
        if ( !compressed )
        {
            numMimpmapLevels = 1;
            glTexImage2D( target, 0, _internalFormat,
                image->s(), image->t(), 0,
                (GLenum)image->getPixelFormat(),
                (GLenum)image->getDataType(),
                image->data() );

        }
        else if (extensions->isCompressedTexImage2DSupported())
        {
            numMimpmapLevels = 1;
            GLint blockSize = ( _internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? 8 : 16 ); 
            GLint size = ((image->s()+3)/4)*((image->t()+3)/4)*blockSize;
            extensions->glCompressedTexImage2D(target, 0, _internalFormat, 
                                               image->s(), image->t(),0, 
                                               size, 
                                               image->data());                

        }

    }
    else
    {
        if(!image->isMipmap())
        {

            numMimpmapLevels = 1;

            gluBuild2DMipmaps( target, _internalFormat,
                image->s(),image->t(),
                (GLenum)image->getPixelFormat(), (GLenum)image->getDataType(),
                image->data() );

        }
        else
        {
            numMimpmapLevels = image->getNumMipmapLevels();

            int width  = image->s();
            int height = image->t();

            if( !compressed )
            {
                for( GLsizei k = 0 ; k < numMimpmapLevels  && (width || height) ;k++)
                {

                    if (width == 0)
                        width = 1;
                    if (height == 0)
                        height = 1;

                    glTexImage2D( target, k, _internalFormat,
                         width, height, 0,
                        (GLenum)image->getPixelFormat(),
                        (GLenum)image->getDataType(),
                        image->getMipmapData(k));

                    width >>= 1;
                    height >>= 1;
                }
            }
            else if (extensions->isCompressedTexImage2DSupported())
            {
                GLint blockSize = ( _internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? 8 : 16 ); 
                GLint size = 0; 
                for( GLsizei k = 0 ; k < numMimpmapLevels  && (width || height) ;k++)
                {
                    if (width == 0)
                        width = 1;
                    if (height == 0)
                        height = 1;

                    size = ((width+3)/4)*((height+3)/4)*blockSize;
                    extensions->glCompressedTexImage2D(target, k, _internalFormat, 
                                                       width, height, 0, size, image->getMipmapData(k));                

                    width >>= 1;
                    height >>= 1;
                }
            }
        }

    }

    inwidth = image->s();
    inheight = image->t();
    
}


///////////////////////////////////////////////////////////////////////////////////////////////
//  Static map to manage the deletion of texture objects are the right time.
//////////////////////////////////////////////////////////////////////////////////////////////
#include <map>
#include <set>

// static cache of deleted display lists which can only 
// by completely deleted once the appropriate OpenGL context
// is set.
typedef std::map<osg::uint,std::set<GLuint> > DeletedTextureObjectCache;
static DeletedTextureObjectCache s_deletedTextureObjectCache;


void Texture::deleteTextureObject(uint contextID,GLuint handle)
{
    if (handle!=0)
    {
        // insert the handle into the cache for the appropriate context.
        s_deletedTextureObjectCache[contextID].insert(handle);
    }
}


void Texture::flushDeletedTextureObjects(uint contextID)
{
    DeletedTextureObjectCache::iterator citr = s_deletedTextureObjectCache.find(contextID);
    if (citr!=s_deletedTextureObjectCache.end())
    {
        std::set<GLuint>& textureObjectSet = citr->second;
        for(std::set<GLuint>::iterator titr=textureObjectSet.begin();
                                     titr!=textureObjectSet.end();
                                     ++titr)
        {
            glDeleteTextures( 1L, &(*titr ));
        }
        s_deletedTextureObjectCache.erase(citr);
    }
}

void Texture::compile(State& state) const
{
    apply(state);
}


typedef buffered_value< ref_ptr<Texture::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

const Texture::Extensions* Texture::getExtensions(uint contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions;
    return s_extensions[contextID].get();
}

void Texture::setExtensions(uint contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

Texture::Extensions::Extensions()
{
    setupGLExtenions();
}

Texture::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isTextureFilterAnisotropicSupported = rhs._isTextureFilterAnisotropicSupported;
    _isTextureMirroredRepeatSupported = rhs._isTextureMirroredRepeatSupported;
    _isTextureEdgeClampSupported = rhs._isTextureEdgeClampSupported;
    _isTextureBorderClampSupported = rhs._isTextureBorderClampSupported;

    _isTextureCompressionARBSupported = rhs._isTextureCompressionARBSupported;
    _isTextureCompressionS3TCSupported = rhs._isTextureCompressionS3TCSupported;

    _maxTextureSize = rhs._maxTextureSize;

    _glCompressedTexImage2D = rhs._glCompressedTexImage2D;
}

void Texture::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isTextureFilterAnisotropicSupported) _isTextureFilterAnisotropicSupported = false;
    if (!rhs._isTextureMirroredRepeatSupported) _isTextureMirroredRepeatSupported = false;
    if (!rhs._isTextureEdgeClampSupported) _isTextureEdgeClampSupported = false;
    if (!rhs._isTextureBorderClampSupported) _isTextureBorderClampSupported = false;
    
    if (!rhs._isTextureCompressionARBSupported) _isTextureCompressionARBSupported = false;
    if (!rhs._isTextureCompressionS3TCSupported) _isTextureCompressionS3TCSupported = false;

    if (rhs._maxTextureSize<_maxTextureSize) _maxTextureSize = rhs._maxTextureSize;

    if (!rhs._glCompressedTexImage2D) _glCompressedTexImage2D = 0;
}

void Texture::Extensions::setupGLExtenions()
{
 
    _isTextureFilterAnisotropicSupported = isGLExtensionSupported("GL_EXT_texture_filter_anisotropic");
    _isTextureMirroredRepeatSupported = isGLExtensionSupported("GL_IBM_texture_mirrored_repeat");
    _isTextureEdgeClampSupported = isGLExtensionSupported("GL_EXT_texture_edge_clamp");
    _isTextureBorderClampSupported = isGLExtensionSupported("GL_ARB_texture_border_clamp");

    _isTextureCompressionARBSupported = isGLExtensionSupported("GL_ARB_texture_compression");
    _isTextureCompressionS3TCSupported = isGLExtensionSupported("GL_EXT_texture_compression_s3tc");

    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&_maxTextureSize);

    char *ptr;
    if( (ptr = getenv("OSG_MAX_TEXTURE_SIZE")) != 0)
    {
        GLint osg_max_size = atoi(ptr);

        if (osg_max_size<_maxTextureSize)
        {

            _maxTextureSize = osg_max_size;
        }

    }      

    _glCompressedTexImage2D = getGLExtensionFuncPtr("glCompressedTexImage2DARB");;

}

void Texture::Extensions::glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) const
{
    if (_glCompressedTexImage2D)
    {
        typedef void (APIENTRY * CompressedTexImage2DArbProc) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
        ((CompressedTexImage2DArbProc)_glCompressedTexImage2D)(target, level, internalformat, width, height, border, imageSize, data);
    }
    else
    {
        notify(WARN)<<"Error: glCompressedTexImage2D not supported by OpenGL driver"<<std::endl;
    }
    
}
