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
#include <osg/Image>
#include <osg/Texture>
#include <osg/State>
#include <osg/Notify>
#include <osg/GLU>
#include <osg/Timer>
#include <osg/ApplicationUsage>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

using namespace osg;

#ifndef GL_TEXTURE_WRAP_R
#define GL_TEXTURE_WRAP_R                 0x8072
#endif

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

ApplicationUsageProxy Texture_e0(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_MAX_TEXTURE_SIZE","Set the maximum size of textures.");

unsigned int Texture::s_numberTextureReusedLastInLastFrame = 0;
unsigned int Texture::s_numberNewTextureInLastFrame = 0;
unsigned int Texture::s_numberDeletedTextureInLastFrame = 0;

unsigned int s_minimumNumberOfTextureObjectsToRetainInCache = 0;
void Texture::setMinimumNumberOfTextureObjectsToRetainInCache(unsigned int minimum)
{
    s_minimumNumberOfTextureObjectsToRetainInCache = minimum;
}

unsigned int Texture::getMinimumNumberOfTextureObjectsToRetainInCache()
{
    return s_minimumNumberOfTextureObjectsToRetainInCache;
}

class TextureObjectManager : public osg::Referenced
{
public:

    TextureObjectManager():
        _expiryDelay(0.0)
    {
        // printf("Constructing TextureObjectManager\n");
    }

    ~TextureObjectManager()
    {
        // printf("Destructing TextureObjectManager\n");
    }

    virtual Texture::TextureObject* generateTextureObject(unsigned int contextID,GLenum target);

    virtual Texture::TextureObject* generateTextureObject(unsigned int contextID,
                                                 GLenum    target,
                                                 GLint     numMipmapLevels,
                                                 GLenum    internalFormat,
                                                 GLsizei   width,
                                                 GLsizei   height,
                                                 GLsizei   depth,
                                                 GLint     border);

    virtual Texture::TextureObject* reuseTextureObject(unsigned int contextID,
                                              GLenum    target,
                                              GLint     numMipmapLevels,
                                              GLenum    internalFormat,
                                              GLsizei   width,
                                              GLsizei   height,
                                              GLsizei   depth,
                                              GLint     border);

    inline Texture::TextureObject* reuseOrGenerateTextureObject(unsigned int contextID,
                                              GLenum    target,
                                              GLint     numMipmapLevels,
                                              GLenum    internalFormat,
                                              GLsizei   width,
                                              GLsizei   height,
                                              GLsizei   depth,
                                              GLint     border)
    {
        Texture::TextureObject* to = reuseTextureObject(contextID,target,numMipmapLevels,internalFormat,width,height,depth,border);
        if (to) return to;
        else return generateTextureObject(contextID,target,numMipmapLevels,internalFormat,width,height,depth,border);
    }                                                      

    void addTextureObjects(Texture::TextureObjectListMap& toblm);

    void addTextureObjectsFrom(Texture& texture);

    void flushAllTextureObjects(unsigned int contextID);

    void flushTextureObjects(unsigned int contextID,double currentTime, double& availableTime);

    void setExpiryDelay(double expiryDelay) { _expiryDelay = expiryDelay; }

    double getExpiryDelay() const { return _expiryDelay; }

    /** How long to keep unused texture objects before deletion. */
    double                  _expiryDelay;

    Texture::TextureObjectListMap    _textureObjectListMap;

    // mutex to keep access serialized.
    OpenThreads::Mutex      _mutex;
};

Texture::TextureObject* TextureObjectManager::generateTextureObject(unsigned int /*contextID*/,GLenum target)
{
    GLuint id;
    glGenTextures( 1L, &id );

    return new Texture::TextureObject(id,target);
}

static int s_number = 0;

Texture::TextureObject* TextureObjectManager::generateTextureObject(unsigned int /*contextID*/,
                                                                             GLenum    target,
                                                                             GLint     numMipmapLevels,
                                                                             GLenum    internalFormat,
                                                                             GLsizei   width,
                                                                             GLsizei   height,
                                                                             GLsizei   depth,
                                                                             GLint     border)
{
    ++s_number;
    ++Texture::s_numberNewTextureInLastFrame;
    // notify(NOTICE)<<"creating new texture object "<<s_number<<std::endl;

    // no useable texture object found so return 0
    GLuint id;
    glGenTextures( 1L, &id );

    return new Texture::TextureObject(id,target,numMipmapLevels,internalFormat,width,height,depth,border);
}

Texture::TextureObject* TextureObjectManager::reuseTextureObject(unsigned int contextID,
                                                                             GLenum    target,
                                                                             GLint     numMipmapLevels,
                                                                             GLenum    internalFormat,
                                                                             GLsizei   width,
                                                                             GLsizei   height,
                                                                             GLsizei   depth,
                                                                             GLint     border)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    Texture::TextureObjectList& tol = _textureObjectListMap[contextID];
    for(Texture::TextureObjectList::iterator itr = tol.begin();
        itr != tol.end();
        ++itr)
    {
        if ((*itr)->match(target,numMipmapLevels,internalFormat,width,height,depth,border))
        {
            // found usable texture object.
            Texture::TextureObject* textureObject = (*itr).release();
            tol.erase(itr);
            
            // notify(NOTICE)<<"reusing texture object "<<std::endl;
            
            ++Texture::s_numberTextureReusedLastInLastFrame;

            return textureObject;
        }
    }
    
    return 0;
}

    

void TextureObjectManager::addTextureObjects(Texture::TextureObjectListMap& toblm)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    for(unsigned int i=0; i< toblm.size(); ++i)
    {
        Texture::TextureObjectList& tol = _textureObjectListMap[i];
        tol.insert(tol.end(),toblm[i].begin(),toblm[i].end());
    }
}

void TextureObjectManager::addTextureObjectsFrom(Texture& texture)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    texture.takeTextureObjects(_textureObjectListMap);
}

void TextureObjectManager::flushAllTextureObjects(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    Texture::TextureObjectList& tol = _textureObjectListMap[contextID];

    for(Texture::TextureObjectList::iterator itr=tol.begin();
        itr!=tol.end();
        ++itr)
    {
        glDeleteTextures( 1L, &((*itr)->_id));
    }
    tol.clear();
}

void TextureObjectManager::flushTextureObjects(unsigned int contextID,double currentTime, double& availableTime)
{
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    unsigned int numObjectsDeleted = 0;
    unsigned int maxNumObjectsToDelete = 4;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    unsigned int numTexturesDeleted = 0;
    {    
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        Texture::TextureObjectList& tol = _textureObjectListMap[contextID];

        // reset the time of any uninitialized objects.
        Texture::TextureObjectList::iterator itr;
        for(itr=tol.begin();
            itr!=tol.end();
            ++itr)
        {
            if ((*itr)->_timeStamp==0.0) (*itr)->_timeStamp=currentTime;
        }

        double expiryTime = currentTime-_expiryDelay;

        for(itr=tol.begin();
            itr!=tol.end() && elapsedTime<availableTime && tol.size()>s_minimumNumberOfTextureObjectsToRetainInCache && numObjectsDeleted<maxNumObjectsToDelete;
            )
        {
            if ((*itr)->_timeStamp<=expiryTime)
            {
                --s_number;
                ++Texture::s_numberDeletedTextureInLastFrame;

                glDeleteTextures( 1L, &((*itr)->_id));
                itr = tol.erase(itr);
                ++numTexturesDeleted;
                ++numObjectsDeleted;
            }
            else
            {
                ++itr;
            }
            elapsedTime = timer.delta_s(start_tick,timer.tick());
        }
    }
    elapsedTime = timer.delta_s(start_tick,timer.tick());
    // if (numTexturesDeleted) notify(osg::NOTICE)<<"Number of Texture's deleted = "<<numTexturesDeleted<<" new total "<<s_number<<" elapsed time"<<elapsedTime<<std::endl;
        
    availableTime -= elapsedTime;
}


static TextureObjectManager* getTextureObjectManager()
{
    static ref_ptr<TextureObjectManager> s_textureObjectManager = new TextureObjectManager;
    return s_textureObjectManager.get();
}


Texture::TextureObject* Texture::generateTextureObject(unsigned int contextID,GLenum target)
{
    if (getTextureObjectManager()) return getTextureObjectManager()->generateTextureObject(contextID,target);
    else return 0;
}

Texture::TextureObject* Texture::generateTextureObject(unsigned int contextID,
                                             GLenum    target,
                                             GLint     numMipmapLevels,
                                             GLenum    internalFormat,
                                             GLsizei   width,
                                             GLsizei   height,
                                             GLsizei   depth,
                                             GLint     border)
{
    if (getTextureObjectManager())    
        return getTextureObjectManager()->reuseOrGenerateTextureObject(contextID,
                                             target,
                                             numMipmapLevels,
                                             internalFormat,
                                             width,
                                             height,
                                             depth,
                                             border);
    else
        return 0;
}                                             

void Texture::flushAllDeletedTextureObjects(unsigned int contextID)
{
    if (getTextureObjectManager()) getTextureObjectManager()->flushAllTextureObjects(contextID);
}

void Texture::flushDeletedTextureObjects(unsigned int contextID,double currentTime, double& availbleTime)
{
    if (getTextureObjectManager()) getTextureObjectManager()->flushTextureObjects(contextID, currentTime, availbleTime);
}

Texture::Texture():
            _wrap_s(CLAMP),
            _wrap_t(CLAMP),
            _wrap_r(CLAMP),
            _min_filter(LINEAR_MIPMAP_LINEAR), // trilinear
            _mag_filter(LINEAR),
            _maxAnisotropy(1.0f),
            _useHardwareMipMapGeneration(true),
            _unrefImageDataAfterApply(false),
            _clientStorageHint(false),
            _resizeNonPowerOfTwoHint(true),
            _borderColor(0.0, 0.0, 0.0, 0.0),
            _borderWidth(0),
            _internalFormatMode(USE_IMAGE_DATA_FORMAT),
            _internalFormat(0),
            _sourceFormat(0),
            _sourceType(0),
            _use_shadow_comparison(false),
            _shadow_compare_func(LEQUAL),
            _shadow_texture_mode(LUMINANCE),
            _shadow_ambient(0)
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
            _useHardwareMipMapGeneration(text._useHardwareMipMapGeneration),
            _unrefImageDataAfterApply(text._unrefImageDataAfterApply),
            _clientStorageHint(text._clientStorageHint),
            _resizeNonPowerOfTwoHint(text._resizeNonPowerOfTwoHint),
            _borderColor(text._borderColor),
            _borderWidth(text._borderWidth),
            _internalFormatMode(text._internalFormatMode),
            _internalFormat(text._internalFormat),
            _sourceFormat(text._sourceFormat),
            _sourceType(text._sourceType),
            _use_shadow_comparison(text._use_shadow_comparison),
            _shadow_compare_func(text._shadow_compare_func),
            _shadow_texture_mode(text._shadow_texture_mode),
            _shadow_ambient(text._shadow_ambient)
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
    COMPARE_StateAttribute_Parameter(_useHardwareMipMapGeneration)
    COMPARE_StateAttribute_Parameter(_internalFormatMode)

    // only compare _internalFomat is it has alrady been set in both lhs, and rhs
    if (_internalFormat!=0 && rhs._internalFormat!=0)
    {
        COMPARE_StateAttribute_Parameter(_internalFormat)
    }

    COMPARE_StateAttribute_Parameter(_sourceFormat)
    COMPARE_StateAttribute_Parameter(_sourceType)

    COMPARE_StateAttribute_Parameter(_use_shadow_comparison)
    COMPARE_StateAttribute_Parameter(_shadow_compare_func)
    COMPARE_StateAttribute_Parameter(_shadow_texture_mode)
    COMPARE_StateAttribute_Parameter(_shadow_ambient)

    COMPARE_StateAttribute_Parameter(_unrefImageDataAfterApply)
    COMPARE_StateAttribute_Parameter(_clientStorageHint)
    COMPARE_StateAttribute_Parameter(_resizeNonPowerOfTwoHint)

    return 0;
}

int Texture::compareTextureObjects(const Texture& rhs) const
{
    if (_textureObjectBuffer.size()<rhs._textureObjectBuffer.size()) return -1; 
    if (rhs._textureObjectBuffer.size()<_textureObjectBuffer.size()) return 1;
    for(unsigned int i=0; i<_textureObjectBuffer.size(); ++i)
    {
        if (_textureObjectBuffer[i] < rhs._textureObjectBuffer[i]) return -1;
        else if (rhs._textureObjectBuffer[i] < _textureObjectBuffer[i]) return 1;
    }
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
    if (getTextureObjectManager()) getTextureObjectManager()->addTextureObjectsFrom(*this);
}

void Texture::takeTextureObjects(Texture::TextureObjectListMap& toblm)
{
    for(unsigned int i = 0; i<_textureObjectBuffer.size();++i)
    {
        if (_textureObjectBuffer[i].valid()) 
        {
            //notify(INFO) << "releasing texure "<<toblm[i].size()<<std::endl;
            toblm[i].push_back(_textureObjectBuffer[i]);
        }
    }
    _textureObjectBuffer.setAllElementsTo(0);
}

void Texture::dirtyTextureParameters()
{
    for(unsigned int i=0;i<_texParametersDirtyList.size();++i)
    {
        _texParametersDirtyList[i] = 1;
    }
}

void Texture::computeInternalFormatWithImage(const osg::Image& image) const
{
    GLint internalFormat = image.getInternalTextureFormat();

    if (_internalFormatMode==USE_IMAGE_DATA_FORMAT)
    {
        internalFormat = image.getInternalTextureFormat();
    }
    else if (_internalFormatMode==USE_USER_DEFINED_FORMAT)
    {
        internalFormat = _internalFormat;
    }
    else
    {

        const unsigned int contextID = 0; // state.getContextID();  // set to 0 right now, assume same paramters for each graphics context...
        const Extensions* extensions = getExtensions(contextID,true);

        switch(_internalFormatMode)
        {
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
        default:
            break;
        }
    }
    
    _internalFormat = internalFormat;
    
    //osg::notify(osg::NOTICE)<<"Internal format="<<std::hex<<internalFormat<<std::dec<<std::endl;
}

bool Texture::isCompressedInternalFormat() const
{
    return isCompressedInternalFormat(getInternalFormat());
}

bool Texture::isCompressedInternalFormat(GLint internalFormat)
{
    switch(internalFormat)
    {
        case(GL_COMPRESSED_ALPHA_ARB):
        case(GL_COMPRESSED_INTENSITY_ARB):
        case(GL_COMPRESSED_LUMINANCE_ALPHA_ARB):
        case(GL_COMPRESSED_LUMINANCE_ARB):
        case(GL_COMPRESSED_RGBA_ARB):
        case(GL_COMPRESSED_RGB_ARB):
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT):
            return true;
        default:
            return false;
    }
}

void Texture::getCompressedSize(GLenum internalFormat, GLint width, GLint height, GLint depth, GLint& blockSize, GLint& size)
{
    if (internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT || internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
        blockSize = 8;
    else if (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT || internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
        blockSize = 16;
    else
    {
        notify(WARN)<<"Texture::getCompressedSize(...) : cannot compute correct size of compressed format ("<<internalFormat<<") returning 0."<<std::endl;
        blockSize = 0;
    }
         
    size = ((width+3)/4)*((height+3)/4)*depth*blockSize;        
}

void Texture::applyTexParameters(GLenum target, State& state) const
{
    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
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
    
    if (target!=GL_TEXTURE_1D) glTexParameteri( target, GL_TEXTURE_WRAP_T, wt );
    
    if (target==GL_TEXTURE_3D) glTexParameteri( target, GL_TEXTURE_WRAP_R, wr );

    glTexParameteri( target, GL_TEXTURE_MIN_FILTER, _min_filter);
    glTexParameteri( target, GL_TEXTURE_MAG_FILTER, _mag_filter);

    if (extensions->isTextureFilterAnisotropicSupported())
    {
        // note, GL_TEXTURE_MAX_ANISOTROPY_EXT will either be defined
        // by gl.h (or via glext.h) or by include/osg/Texture.
        glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, _maxAnisotropy);
    }

    if (extensions->isTextureBorderClampSupported())
    {
        glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, _borderColor.ptr());
    }

    if (extensions->isShadowSupported() && target == GL_TEXTURE_2D)
    {
        if (_use_shadow_comparison)
        {
            glTexParameteri(target, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
            glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC_ARB, _shadow_compare_func);
            glTexParameteri(target, GL_DEPTH_TEXTURE_MODE_ARB, _shadow_texture_mode);

            // if ambient value is 0 - it is default behaviour of GL_ARB_shadow
            // no need for GL_ARB_shadow_ambient in this case
            if (extensions->isShadowAmbientSupported() && _shadow_ambient > 0)
            {
                glTexParameterf(target, TEXTURE_COMPARE_FAIL_VALUE_ARB, _shadow_ambient);
            }
        }
        else 
        {
            glTexParameteri(target, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
        }
    }

    getTextureParameterDirty(state.getContextID()) = false;

}

void Texture::computeRequiredTextureDimensions(State& state, const osg::Image& image,GLsizei& inwidth, GLsizei& inheight,GLsizei& numMipmapLevels) const
{
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    int width,height;

    if( !_resizeNonPowerOfTwoHint && extensions->isNonPowerOfTwoTextureSupported(_min_filter) )
    {
        width = image.s();
        height = image.t();
    }
    else
    {
        width = Image::computeNearestPowerOfTwo(image.s()-2*_borderWidth)+2*_borderWidth;
        height = Image::computeNearestPowerOfTwo(image.t()-2*_borderWidth)+2*_borderWidth;
    }

    // cap the size to what the graphics hardware can handle.
    if (width>extensions->maxTextureSize()) width = extensions->maxTextureSize();
    if (height>extensions->maxTextureSize()) height = extensions->maxTextureSize();

    inwidth = width;
    inheight = height;
    
    bool useHardwareMipMapGeneration = !image.isMipmap() && _useHardwareMipMapGeneration && extensions->isGenerateMipMapSupported();

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
        for( ; (width || height) ;++numMipmapLevels)
        {

            if (width == 0)
                width = 1;
            if (height == 0)
                height = 1;

            width >>= 1;
            height >>= 1;
        }    
    }
}

bool Texture::areAllTextureObjectsLoaded() const
{
    for(unsigned int i=0;i<DisplaySettings::instance()->getMaxNumberOfGraphicsContexts();++i)
    {
        if (_textureObjectBuffer[i]==0) return false;
    }
    return true;
}


void Texture::applyTexImage2D_load(State& state, GLenum target, const Image* image, GLsizei inwidth, GLsizei inheight,GLsizei numMipmapLevels) const
{
    // if we don't have a valid image we can't create a texture!
    if (!image || !image->data())
        return;

#ifdef DO_TIMING
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    osg::notify(osg::NOTICE)<<"glTexImage2D pixelFormat = "<<std::hex<<image->getPixelFormat()<<std::dec<<std::endl;
#endif

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);
    
    bool generateMipMapSupported = extensions->isGenerateMipMapSupported();

    // select the internalFormat required for the texture.
    bool compressed_image = isCompressedInternalFormat((GLenum)image->getPixelFormat());

    // If the texture's internal format is a compressed type, then the
    // user is requesting that the graphics card compress the image if it's
    // not already compressed. However, if the image is not a multiple of 
    // four in each dimension the subsequent calls to glTexSubImage* will
    // fail. Revert to uncompressed format in this case.
    if (isCompressedInternalFormat(_internalFormat) &&
        (((inwidth >> 2) << 2) != inwidth ||
         ((inheight >> 2) << 2) != inheight))
    {
        osg::notify(osg::NOTICE)<<"Received a request to compress an image, but image size is not a multiple of four ("<<inwidth<<"x"<<inheight<<"). Reverting to uncompressed.\n";
        switch(_internalFormat)
        {
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGB: _internalFormat = GL_RGB; break;
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            case GL_COMPRESSED_RGBA: _internalFormat = GL_RGBA; break;
            case GL_COMPRESSED_ALPHA: _internalFormat = GL_ALPHA; break;
            case GL_COMPRESSED_LUMINANCE: _internalFormat = GL_LUMINANCE; break;
            case GL_COMPRESSED_LUMINANCE_ALPHA: _internalFormat = GL_LUMINANCE_ALPHA; break;
            case GL_COMPRESSED_INTENSITY: _internalFormat = GL_INTENSITY; break;
        }
    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT,image->getPacking());
    
    bool useClientStorage = extensions->isClientStorageSupported() && getClientStorageHint();
    if (useClientStorage)
    {
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE,GL_TRUE);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_PRIORITY,0.0f);
        
        #ifdef GL_TEXTURE_STORAGE_HINT_APPLE    
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_CACHED_APPLE);
        #endif
    }
    
    unsigned char* data = (unsigned char*)image->data();
 

    bool needImageRescale = inwidth!=image->s() || inheight!=image->t();
    if (needImageRescale)
    {

        // resize the image to power of two.
        
        if (image->isMipmap())
        {
            notify(WARN)<<"Warning:: Mipmapped osg::Image not a power of two, cannot apply to texture."<<std::endl;
            return;
        }
        else if (compressed_image)
        {
            notify(WARN)<<"Warning:: Compressed osg::Image not a power of two, cannot apply to texture."<<std::endl;
            return;
        }
        
        unsigned int newTotalSize = osg::Image::computeRowWidthInBytes(inwidth,image->getPixelFormat(),image->getDataType(),image->getPacking())*inheight;
        data = new unsigned char [newTotalSize];
        
        if (!data)
        {
            notify(WARN)<<"Warning:: Not enough memory to resize image, cannot apply to texture."<<std::endl;
            return;
        }

        if (!image->getFileName().empty()) notify(NOTICE) << "Scaling image '"<<image->getFileName()<<"' from ("<<image->s()<<","<<image->t()<<") to ("<<inwidth<<","<<inheight<<")"<<std::endl;
        else notify(NOTICE) << "Scaling image from ("<<image->s()<<","<<image->t()<<") to ("<<inwidth<<","<<inheight<<")"<<std::endl;

        // rescale the image to the correct size.
        glPixelStorei(GL_PACK_ALIGNMENT,image->getPacking());
        gluScaleImage(image->getPixelFormat(),
                      image->s(),image->t(),image->getDataType(),image->data(),
                      inwidth,inheight,image->getDataType(),data);
        
    }    

    unsigned char* dataMinusOffset = 0;
    unsigned char* dataPlusOffset = 0;

    const PixelBufferObject* pbo = image->getPixelBufferObject();
    if (pbo && pbo->isPBOSupported(contextID) && !needImageRescale)
    {
        pbo->compileBuffer(state);
        pbo->bindBuffer(contextID);
        dataMinusOffset = data;
        dataPlusOffset = reinterpret_cast<unsigned char*>(pbo->offset());
#ifdef DO_TIMING
        osg::notify(osg::NOTICE)<<"after PBO "<<osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick())<<"ms"<<std::endl;
#endif
    }
    else
    {
        pbo = 0;
    }

    bool useHardwareMipMapGeneration = !image->isMipmap() && _useHardwareMipMapGeneration && generateMipMapSupported;
    
    if( _min_filter == LINEAR || _min_filter == NEAREST || useHardwareMipMapGeneration)
    {
        bool hardwareMipMapOn = false;
        if (_min_filter != LINEAR && _min_filter != NEAREST) 
        {
            if (useHardwareMipMapGeneration) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
            hardwareMipMapOn = true;
        }

        if ( !compressed_image)
        {
            numMipmapLevels = 1;

            glTexImage2D( target, 0, _internalFormat,
                inwidth, inheight, _borderWidth,
                (GLenum)image->getPixelFormat(),
                (GLenum)image->getDataType(),
                data -dataMinusOffset+dataPlusOffset);

        }
        else if (extensions->isCompressedTexImage2DSupported())
        {
            numMipmapLevels = 1;

            GLint blockSize, size;
            getCompressedSize(_internalFormat, inwidth, inheight, 1, blockSize,size);

            extensions->glCompressedTexImage2D(target, 0, _internalFormat, 
                inwidth, inheight,0, 
                size, 
                data-dataMinusOffset+dataPlusOffset);                
        }

        if (hardwareMipMapOn) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_FALSE);
    }
    else
    {
        // we require mip mapping.
        if(image->isMipmap())
        {

            // image is mip mapped so we take the mip map levels from the image.
        
            numMipmapLevels = image->getNumMipmapLevels();

            int width  = inwidth;
            int height = inheight;

            if( !compressed_image )
            {
                for( GLsizei k = 0 ; k < numMipmapLevels  && (width || height) ;k++)
                {

                    if (width == 0)
                        width = 1;
                    if (height == 0)
                        height = 1;

                    glTexImage2D( target, k, _internalFormat,
                         width, height, _borderWidth,
                        (GLenum)image->getPixelFormat(),
                        (GLenum)image->getDataType(),
                        image->getMipmapData(k)-dataMinusOffset+dataPlusOffset);

                    width >>= 1;
                    height >>= 1;
                }
            }
            else if (extensions->isCompressedTexImage2DSupported())
            {
                GLint blockSize, size;

                for( GLsizei k = 0 ; k < numMipmapLevels  && (width || height) ;k++)
                {
                    if (width == 0)
                        width = 1;
                    if (height == 0)
                        height = 1;

                    getCompressedSize(_internalFormat, width, height, 1, blockSize,size);
                    
                    extensions->glCompressedTexImage2D(target, k, _internalFormat, 
                                                       width, height, _borderWidth, 
                                                       size, image->getMipmapData(k)-dataMinusOffset+dataPlusOffset);                

                    width >>= 1;
                    height >>= 1;
                }
            }
        }
        else
        {
        
            if ( !compressed_image)
            {
                numMipmapLevels = 0;

                gluBuild2DMipmaps( target, _internalFormat,
                    inwidth,inheight,
                    (GLenum)image->getPixelFormat(), (GLenum)image->getDataType(),
                    data -dataMinusOffset+dataPlusOffset);

                int width  = image->s();
                int height = image->t();
                for( numMipmapLevels = 0 ; (width || height) ; ++numMipmapLevels)
                {
                    width >>= 1;
                    height >>= 1;
                }
            }
            else 
            {
                notify(WARN)<<"Warning:: Compressed image cannot be mip mapped"<<std::endl;
            }

        }

    }

    if (pbo)
    {
        pbo->unbindBuffer(contextID);
    }
#ifdef DO_TIMING
    static double s_total_time = 0.0;
    double delta_time = osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick());
    s_total_time += delta_time;
    osg::notify(osg::NOTICE)<<"glTexImage2D "<<delta_time<<"ms  total "<<s_total_time<<"ms"<<std::endl;
#endif

    if (needImageRescale)
    {
        // clean up the resized image.
        delete [] data;
    }
    
    if (useClientStorage)
    {
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE,GL_FALSE);
    }
}



void Texture::applyTexImage2D_subload(State& state, GLenum target, const Image* image, GLsizei inwidth, GLsizei inheight, GLint inInternalFormat, GLint numMipmapLevels) const
{
    // if we don't have a valid image we can't create a texture!
    if (!image || !image->data())
        return;

    // image size has changed so we have to re-load the image from scratch.
    if (image->s()!=inwidth || image->t()!=inheight || image->getInternalTextureFormat()!=inInternalFormat ) 
    {
        applyTexImage2D_load(state, target, image, inwidth, inheight,numMipmapLevels); 
        return;
    }
    // else image size the same as when loaded so we can go ahead and subload
    
    // If the texture's internal format is a compressed type, then the
    // user is requesting that the graphics card compress the image if it's
    // not already compressed. However, if the image is not a multiple of 
    // four in each dimension the subsequent calls to glTexSubImage* will
    // fail. Revert to uncompressed format in this case.
    if (isCompressedInternalFormat(_internalFormat) &&
        (((inwidth >> 2) << 2) != inwidth ||
         ((inheight >> 2) << 2) != inheight))
    {
        applyTexImage2D_load(state, target, image, inwidth, inheight, numMipmapLevels);
        return;
    }

#ifdef DO_TIMING
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    osg::notify(osg::NOTICE)<<"glTexSubImage2D pixelFormat = "<<std::hex<<image->getPixelFormat()<<std::dec<<std::endl;
#endif
   

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);
    
    bool generateMipMapSupported = extensions->isGenerateMipMapSupported();

    // select the internalFormat required for the texture.
    bool compressed_image = isCompressedInternalFormat((GLenum)image->getPixelFormat());
    
    glPixelStorei(GL_UNPACK_ALIGNMENT,image->getPacking());
    
    unsigned char* data = (unsigned char*)image->data();
 

    bool needImageRescale = inwidth!=image->s() || inheight!=image->t();
    if (needImageRescale)
    {

        // resize the image to power of two.
        
        if (image->isMipmap())
        {
            notify(WARN)<<"Warning:: Mipmapped osg::Image not a power of two, cannot apply to texture."<<std::endl;
            return;
        }
        else if (compressed_image)
        {
            notify(WARN)<<"Warning:: Compressed osg::Image not a power of two, cannot apply to texture."<<std::endl;
            return;
        }
        
        unsigned int newTotalSize = osg::Image::computeRowWidthInBytes(inwidth,image->getPixelFormat(),image->getDataType(),image->getPacking())*inheight;
        data = new unsigned char [newTotalSize];
        
        if (!data)
        {
            notify(WARN)<<"Warning:: Not enough memory to resize image, cannot apply to texture."<<std::endl;
            return;
        }

        if (!image->getFileName().empty()) notify(NOTICE) << "Scaling image '"<<image->getFileName()<<"' from ("<<image->s()<<","<<image->t()<<") to ("<<inwidth<<","<<inheight<<")"<<std::endl;
        else notify(NOTICE) << "Scaling image from ("<<image->s()<<","<<image->t()<<") to ("<<inwidth<<","<<inheight<<")"<<std::endl;

        // rescale the image to the correct size.
        glPixelStorei(GL_PACK_ALIGNMENT,image->getPacking());
        gluScaleImage(image->getPixelFormat(),
                      image->s(),image->t(),image->getDataType(),image->data(),
                      inwidth,inheight,image->getDataType(),data);
        
    }    

    bool useHardwareMipMapGeneration = !image->isMipmap() && _useHardwareMipMapGeneration && generateMipMapSupported;
    
    unsigned char* dataMinusOffset=0;
    unsigned char* dataPlusOffset=0;

    const PixelBufferObject* pbo = image->getPixelBufferObject();
    if (pbo && pbo->isPBOSupported(contextID) && !needImageRescale)
    {
        pbo->compileBuffer(state);
        pbo->bindBuffer(contextID);
        dataMinusOffset = data;
        dataPlusOffset = reinterpret_cast<unsigned char*>(pbo->offset());
#ifdef DO_TIMING
        osg::notify(osg::NOTICE)<<"after PBO "<<osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick())<<"ms"<<std::endl;
#endif
    }
    else
    {
        pbo = 0;
    }

    if( _min_filter == LINEAR || _min_filter == NEAREST || useHardwareMipMapGeneration)
    {
    
        bool hardwareMipMapOn = false;
        if (_min_filter != LINEAR && _min_filter != NEAREST) 
        {
            if (useHardwareMipMapGeneration) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
            hardwareMipMapOn = true;
        }
        

        if (!compressed_image)
        {
            glTexSubImage2D( target, 0, 
                0, 0,
                inwidth, inheight,
                (GLenum)image->getPixelFormat(),
                (GLenum)image->getDataType(),
                data - dataMinusOffset + dataPlusOffset);

        }
        else if (extensions->isCompressedTexImage2DSupported())
        {        
            GLint blockSize,size;
            getCompressedSize(image->getInternalTextureFormat(), inwidth, inheight, 1, blockSize,size);

            extensions->glCompressedTexSubImage2D(target, 0, 
                0,0,
                inwidth, inheight,
                (GLenum)image->getPixelFormat(),
                size,
                data - dataMinusOffset + dataPlusOffset );                
        }

        if (hardwareMipMapOn) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS,GL_FALSE);
    }
    else
    {
        if (image->isMipmap())
        {
            numMipmapLevels = image->getNumMipmapLevels();

            int width  = inwidth;
            int height = inheight;

            if( !compressed_image )
            {
                for( GLsizei k = 0 ; k < numMipmapLevels  && (width || height) ;k++)
                {

                    if (width == 0)
                        width = 1;
                    if (height == 0)
                        height = 1;

                    glTexSubImage2D( target, k, 
                        0, 0,
                        width, height,
                        (GLenum)image->getPixelFormat(),
                        (GLenum)image->getDataType(),
                        image->getMipmapData(k) - dataMinusOffset + dataPlusOffset);

                    width >>= 1;
                    height >>= 1;
                }
            }
            else if (extensions->isCompressedTexImage2DSupported())
            {
                GLint blockSize,size;
                for( GLsizei k = 0 ; k < numMipmapLevels  && (width || height) ;k++)
                {
                    if (width == 0)
                        width = 1;
                    if (height == 0)
                        height = 1;

                    getCompressedSize(image->getInternalTextureFormat(), width, height, 1, blockSize,size);

                    //state.checkGLErrors("before extensions->glCompressedTexSubImage2D(");

                    extensions->glCompressedTexSubImage2D(target, k,  
                                                       0, 0,
                                                       width, height, 
                                                       (GLenum)image->getPixelFormat(),
                                                       size,
                                                        image->getMipmapData(k) - dataMinusOffset + dataPlusOffset);                

                    //state.checkGLErrors("after extensions->glCompressedTexSubImage2D(");

                    width >>= 1;
                    height >>= 1;
                }

            }
        }
        else
        {
            //notify(WARN)<<"Warning:: cannot subload mip mapped texture from non mipmapped image."<<std::endl;
            applyTexImage2D_load(state, target, image, inwidth, inheight,numMipmapLevels); 
            return;
        }
    }
    
    if (pbo)
    {
        pbo->unbindBuffer(contextID);
    }
#ifdef DO_TIMING
    osg::notify(osg::NOTICE)<<"glTexSubImage2D "<<osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick())<<"ms"<<std::endl;
#endif

    if (needImageRescale)
    {
        // clean up the resized image.
        delete [] data;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
//  Static map to manage the deletion of texture objects are the right time.
//////////////////////////////////////////////////////////////////////////////////////////////
#include <map>
#include <set>


void Texture::compileGLObjects(State& state) const
{
    apply(state);
}

void Texture::releaseGLObjects(State* state) const
{
    if (!state) const_cast<Texture*>(this)->dirtyTextureObject();
    else
    {
        unsigned int contextID = state->getContextID();
        if (_textureObjectBuffer[contextID].valid() && getTextureObjectManager()) 
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getTextureObjectManager()->_mutex);
            
            getTextureObjectManager()->_textureObjectListMap[contextID].push_back(_textureObjectBuffer[contextID]);
            _textureObjectBuffer[contextID] = 0;
        }
    }
}

typedef buffered_value< ref_ptr<Texture::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

Texture::Extensions* Texture::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void Texture::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

Texture::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

Texture::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isMultiTexturingSupported = rhs._isMultiTexturingSupported;
    _isTextureFilterAnisotropicSupported = rhs._isTextureFilterAnisotropicSupported;
    _isTextureCompressionARBSupported = rhs._isTextureCompressionARBSupported;
    _isTextureCompressionS3TCSupported = rhs._isTextureCompressionS3TCSupported;
    _isTextureMirroredRepeatSupported = rhs._isTextureMirroredRepeatSupported;
    _isTextureEdgeClampSupported = rhs._isTextureEdgeClampSupported;
    _isTextureBorderClampSupported = rhs._isTextureBorderClampSupported;
    _isGenerateMipMapSupported = rhs._isGenerateMipMapSupported;

    _maxTextureSize = rhs._maxTextureSize;

    _glCompressedTexImage2D = rhs._glCompressedTexImage2D;

    _isShadowSupported = rhs._isShadowSupported;
    _isShadowAmbientSupported = rhs._isShadowAmbientSupported;

    _isClientStorageSupported = rhs._isClientStorageSupported;

    _isNonPowerOfTwoTextureMipMappedSupported = rhs._isNonPowerOfTwoTextureMipMappedSupported;
    _isNonPowerOfTwoTextureNonMipMappedSupported = rhs._isNonPowerOfTwoTextureNonMipMappedSupported;
}

void Texture::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isMultiTexturingSupported) _isMultiTexturingSupported = false;
    
    if (!rhs._isTextureFilterAnisotropicSupported) _isTextureFilterAnisotropicSupported = false;
    if (!rhs._isTextureMirroredRepeatSupported) _isTextureMirroredRepeatSupported = false;
    if (!rhs._isTextureEdgeClampSupported) _isTextureEdgeClampSupported = false;
    if (!rhs._isTextureBorderClampSupported) _isTextureBorderClampSupported = false;
    
    if (!rhs._isTextureCompressionARBSupported) _isTextureCompressionARBSupported = false;
    if (!rhs._isTextureCompressionS3TCSupported) _isTextureCompressionS3TCSupported = false;
    
    if (!rhs._isGenerateMipMapSupported) _isGenerateMipMapSupported = false;

    if (rhs._maxTextureSize<_maxTextureSize) _maxTextureSize = rhs._maxTextureSize;
    if (rhs._numTextureUnits<_numTextureUnits) _numTextureUnits = rhs._numTextureUnits;

    if (!rhs._glCompressedTexImage2D) _glCompressedTexImage2D = 0;
    if (!rhs._glCompressedTexSubImage2D) _glCompressedTexSubImage2D = 0;
    if (!rhs._glGetCompressedTexImage) _glGetCompressedTexImage = 0;

    if (!rhs._isShadowSupported) _isShadowSupported = false;
    if (!rhs._isShadowAmbientSupported) _isShadowAmbientSupported = false;
    
    if (!rhs._isClientStorageSupported) _isClientStorageSupported = false;

    if (!rhs._isNonPowerOfTwoTextureMipMappedSupported) _isNonPowerOfTwoTextureMipMappedSupported = false;
    if (!rhs._isNonPowerOfTwoTextureNonMipMappedSupported) _isNonPowerOfTwoTextureNonMipMappedSupported = false;
}

void Texture::Extensions::setupGLExtensions(unsigned int contextID)
{
    const char* version = (const char*) glGetString( GL_VERSION );
    if (!version)
    {
        osg::notify(osg::FATAL)<<"Error: In Texture::Extensions::setupGLExtensions(..) OpenGL version test failed, requires valid graphics context."<<std::endl;
        return;
    }
    

    float glVersion = atof( (const char *)version );
    const char* renderer = (const char*) glGetString(GL_RENDERER);
    std::string rendererString(renderer ? renderer : "");
    
    _isMultiTexturingSupported = ( glVersion >= 1.3 ) ||
                                 isGLExtensionSupported(contextID,"GL_ARB_multitexture") ||
                                 isGLExtensionSupported(contextID,"GL_EXT_multitexture");
    _isTextureFilterAnisotropicSupported = isGLExtensionSupported(contextID,"GL_EXT_texture_filter_anisotropic");
    _isTextureCompressionARBSupported = ( glVersion >= 1.3 ) ||
                                        isGLExtensionSupported(contextID,"GL_ARB_texture_compression");
    _isTextureCompressionS3TCSupported = isGLExtensionSupported(contextID,"GL_EXT_texture_compression_s3tc");
    _isTextureMirroredRepeatSupported = ( glVersion >= 1.4 ) || 
                                    isGLExtensionSupported(contextID,"GL_IBM_texture_mirrored_repeat") ||
                                    isGLExtensionSupported(contextID,"GL_ARB_texture_mirrored_repeat");
    _isTextureEdgeClampSupported = ( glVersion >= 1.2 ) ||
                                    isGLExtensionSupported(contextID,"GL_EXT_texture_edge_clamp") || 
                                    isGLExtensionSupported(contextID,"GL_SGIS_texture_edge_clamp");
    _isTextureBorderClampSupported = ( glVersion >= 1.3 ) ||
                                    isGLExtensionSupported(contextID,"GL_ARB_texture_border_clamp");
    _isGenerateMipMapSupported = (strncmp((const char*)glGetString(GL_VERSION),"1.4",3)>=0) ||
                                  isGLExtensionSupported(contextID,"GL_SGIS_generate_mipmap");
    _isShadowSupported = isGLExtensionSupported(contextID,"GL_ARB_shadow");
    _isShadowAmbientSupported = isGLExtensionSupported(contextID,"GL_ARB_shadow_ambient");

    _isClientStorageSupported = isGLExtensionSupported(contextID,"GL_APPLE_client_storage");

    _isNonPowerOfTwoTextureNonMipMappedSupported = ( glVersion >= 2.0 ) ||
                                       isGLExtensionSupported(contextID,"GL_ARB_texture_non_power_of_two");

    _isNonPowerOfTwoTextureMipMappedSupported = _isNonPowerOfTwoTextureNonMipMappedSupported;
    
    if (rendererString.find("Radeon")!=std::string::npos || rendererString.find("RADEON")!=std::string::npos)
    {
        _isNonPowerOfTwoTextureMipMappedSupported = false;
        osg::notify(osg::INFO)<<"Disabling _isNonPowerOfTwoTextureMipMappedSupported for ATI hardware."<<std::endl;
    }

    if (rendererString.find("GeForce FX")!=std::string::npos)
    {
        _isNonPowerOfTwoTextureMipMappedSupported = false;
        osg::notify(osg::INFO)<<"Disabling _isNonPowerOfTwoTextureMipMappedSupported for GeForce FX hardware."<<std::endl;
    }

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

    if( _isMultiTexturingSupported )
    {
       glGetIntegerv(GL_MAX_TEXTURE_UNITS,&_numTextureUnits);
    }
    else
    {
       _numTextureUnits = 1;
    }

    _glCompressedTexImage2D = getGLExtensionFuncPtr("glCompressedTexImage2D","glCompressedTexImage2DARB");
    _glCompressedTexSubImage2D = getGLExtensionFuncPtr("glCompressedTexSubImage2D","glCompressedTexSubImage2DARB");
    _glGetCompressedTexImage = getGLExtensionFuncPtr("glGetCompressedTexImage","glGetCompressedTexImageARB");;
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

void Texture::Extensions::glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) const
{
    if (_glCompressedTexSubImage2D)
    {
        typedef void (APIENTRY * CompressedTexSubImage2DArbProc) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
        ((CompressedTexSubImage2DArbProc)_glCompressedTexSubImage2D)(target, level, xoffset, yoffset, width, height, format, imageSize, data);
    }
    else
    {
        notify(WARN)<<"Error: glCompressedTexImage2D not supported by OpenGL driver"<<std::endl;
    }
    
}
void Texture::Extensions::glGetCompressedTexImage(GLenum target, GLint level, GLvoid *data) const
{
    if (_glGetCompressedTexImage)
    {
        typedef void (APIENTRY * GetCompressedTexImageArbProc) (GLenum target, GLint level, GLvoid *data);
        ((GetCompressedTexImageArbProc)_glGetCompressedTexImage)(target, level, data);
    }
    else
    {
        notify(WARN)<<"Error: glGetCompressedTexImage not supported by OpenGL driver"<<std::endl;
    }
}

