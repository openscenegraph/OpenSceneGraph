#include <osg/GLExtensions>
#include <osg/Image>
#include <osg/Texture>
#include <osg/State>
#include <osg/Notify>
#include <osg/GLU>

typedef void (APIENTRY * MyCompressedTexImage2DArbProc) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);

using namespace osg;


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
//     _handleList.resize(DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),0);
//     _modifiedTag.resize(DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),0);
//     _texParametersDirtyList.resize(DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),true);
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
//     _handleList.resize(DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),0);
//     _modifiedTag.resize(DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),0);
//     _texParametersDirtyList.resize(DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),true);
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
        _texParametersDirtyList[i] = 0;
    }
}

void Texture::computeInternalFormatWithImage(osg::Image& image) const
{
    static bool s_ARB_Compression = isGLExtensionSupported("GL_ARB_texture_compression");
    static bool s_S3TC_Compression = isGLExtensionSupported("GL_EXT_texture_compression_s3tc");

    GLint internalFormat = image.getInternalTextureFormat();
    switch(_internalFormatMode)
    {
        case(USE_IMAGE_DATA_FORMAT):
            internalFormat = image.getInternalTextureFormat();
            break;

        case(USE_ARB_COMPRESSION):
            if (s_ARB_Compression)
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
            if (s_S3TC_Compression)
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
            if (s_S3TC_Compression)
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
            if (s_S3TC_Compression)
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
    WrapMode ws = _wrap_s, wt = _wrap_t;

    // GL_IBM_texture_mirrored_repeat, fall-back REPEAT
    static bool s_mirroredSupported = isGLExtensionSupported("GL_IBM_texture_mirrored_repeat");
    if (!s_mirroredSupported)
    {
        if (ws == MIRROR)
            ws = REPEAT;
        if (wt == MIRROR)
            wt = REPEAT;
    }

    // GL_EXT_texture_edge_clamp, fall-back CLAMP
    static bool s_edgeClampSupported = isGLExtensionSupported("GL_EXT_texture_edge_clamp");
    if (!s_edgeClampSupported)
    {
        if (ws == CLAMP_TO_EDGE)
            ws = CLAMP;
        if (wt == CLAMP_TO_EDGE)
            wt = CLAMP;
    }

    static bool s_borderClampSupported = isGLExtensionSupported("GL_ARB_texture_border_clamp");
    if(!s_borderClampSupported)
    {
        if(ws == CLAMP_TO_BORDER)
            ws = CLAMP;
        if(wt == CLAMP_TO_BORDER)
            wt = CLAMP;
    }

    glTexParameteri( target, GL_TEXTURE_WRAP_S, ws );
    glTexParameteri( target, GL_TEXTURE_WRAP_T, wt );

    glTexParameteri( target, GL_TEXTURE_MIN_FILTER, _min_filter);
    glTexParameteri( target, GL_TEXTURE_MAG_FILTER, _mag_filter);

    if (_maxAnisotropy>1.0f)
    {
        // check for support for anisotropic filter,
        // note since this is static varible it is intialised
        // only on the first time entering this code block,
        // is then never reevaluated on subsequent calls.
        static bool s_anisotropicSupported =
            isGLExtensionSupported("GL_EXT_texture_filter_anisotropic");

        if (s_anisotropicSupported)
        {
            // note, GL_TEXTURE_MAX_ANISOTROPY_EXT will either be defined
            // by gl.h (or via glext.h) or by include/osg/Texture.
            glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, _maxAnisotropy);
        }
    }

    if (s_borderClampSupported)
    {
        glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, _borderColor.ptr());
    }

    getTextureParameterDity(state.getContextID()) = false;

}

void Texture::applyTexImage2D(GLenum target, Image* image, State& state, GLsizei& inwidth, GLsizei& inheight,GLsizei& numMimpmapLevels) const
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

    static MyCompressedTexImage2DArbProc glCompressedTexImage2D_ptr = 
        (MyCompressedTexImage2DArbProc)getGLExtensionFuncPtr("glCompressedTexImage2DARB");

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
        else if(glCompressedTexImage2D_ptr)
        {
            numMimpmapLevels = 1;
            GLint blockSize = ( _internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? 8 : 16 ); 
            GLint size = ((image->s()+3)/4)*((image->t()+3)/4)*blockSize;
            glCompressedTexImage2D_ptr(target, 0, _internalFormat, 
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
            else if(glCompressedTexImage2D_ptr)
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
                    glCompressedTexImage2D_ptr(target, k, _internalFormat, 
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
        std::set<uint>& textureObjectSet = citr->second;
        for(std::set<uint>::iterator titr=textureObjectSet.begin();
                                     titr!=textureObjectSet.end();
                                     ++titr)
        {
            glDeleteTextures( 1L, &(*titr ));
        }
        s_deletedTextureObjectCache.erase(citr);
    }
}


GLint Texture::getMaxTextureSize()
{
    static GLint s_maxTextureSize = 0;
    if (s_maxTextureSize == 0)
    {
    
        glGetIntegerv(GL_MAX_TEXTURE_SIZE,&s_maxTextureSize);
        notify(INFO) << "GL_MAX_TEXTURE_SIZE "<<s_maxTextureSize<<std::endl;
        
        char *ptr;
        if( (ptr = getenv("OSG_MAX_TEXTURE_SIZE")) != 0)
        {
            GLint osg_max_size = atoi(ptr);
            
            notify(INFO) << "OSG_MAX_TEXTURE_SIZE "<<osg_max_size<<std::endl;
            
            if (osg_max_size<s_maxTextureSize)
            {
                
                s_maxTextureSize = osg_max_size;
            }
            
        }      
        notify(INFO) << "Selected max texture size "<<s_maxTextureSize<<std::endl;
    }
    return s_maxTextureSize;
}

void Texture::compile(State& state) const
{
    apply(state);
}
