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

// initial FBO support written by Marco Jez, June 2005.

#include <osg/FrameBufferObject>
#include <osg/State>
#include <osg/GLExtensions>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture2DMultisample>
#include <osg/Texture3D>
#include <osg/Texture2DArray>
#include <osg/TextureCubeMap>
#include <osg/TextureRectangle>
#include <osg/Notify>
#include <osg/Timer>

using namespace osg;

static buffered_object< ref_ptr<FBOExtensions> > s_extensions;

FBOExtensions* FBOExtensions::instance(unsigned contextID, bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new FBOExtensions(contextID);
    return s_extensions[contextID].get();
}

/**************************************************************************
 * FBOExtensions
 **************************************************************************/
#define LOAD_FBO_EXT(name)    setGLExtensionFuncPtr(name, (#name))

FBOExtensions::FBOExtensions(unsigned int contextID)
:   glBindRenderbufferEXT(0),
    glGenRenderbuffersEXT(0),
    glDeleteRenderbuffersEXT(0),
    glRenderbufferStorageEXT(0),
    glRenderbufferStorageMultisampleEXT(0),
    glRenderbufferStorageMultisampleCoverageNV(0),
    glBindFramebufferEXT(0),
    glDeleteFramebuffersEXT(0),
    glGenFramebuffersEXT(0),
    glCheckFramebufferStatusEXT(0),
    glFramebufferTexture1DEXT(0),
    glFramebufferTexture2DEXT(0),
    glFramebufferTexture3DEXT(0),
    glFramebufferTextureLayerEXT(0),
    glFramebufferRenderbufferEXT(0),
    glGenerateMipmapEXT(0),
    glBlitFramebufferEXT(0),
    _supported(false),
    _packed_depth_stencil_supported(false)
{
    if (!isGLExtensionSupported(contextID, "GL_EXT_framebuffer_object"))
        return;
    
    LOAD_FBO_EXT(glBindRenderbufferEXT);
    LOAD_FBO_EXT(glGenRenderbuffersEXT);
    LOAD_FBO_EXT(glDeleteRenderbuffersEXT);
    LOAD_FBO_EXT(glRenderbufferStorageEXT);
    LOAD_FBO_EXT(glBindFramebufferEXT);
    LOAD_FBO_EXT(glDeleteFramebuffersEXT);
    LOAD_FBO_EXT(glGenFramebuffersEXT);
    LOAD_FBO_EXT(glCheckFramebufferStatusEXT);
    LOAD_FBO_EXT(glFramebufferTexture1DEXT);
    LOAD_FBO_EXT(glFramebufferTexture2DEXT);
    LOAD_FBO_EXT(glFramebufferTexture3DEXT);
    LOAD_FBO_EXT(glFramebufferTextureLayerEXT);
    LOAD_FBO_EXT(glFramebufferRenderbufferEXT);
    LOAD_FBO_EXT(glGenerateMipmapEXT);

    _supported = 
        glBindRenderbufferEXT != 0 &&
        glDeleteRenderbuffersEXT != 0 &&
        glGenRenderbuffersEXT != 0 &&
        glRenderbufferStorageEXT != 0 &&
        glBindFramebufferEXT != 0 &&
        glDeleteFramebuffersEXT != 0 &&
        glGenFramebuffersEXT != 0 &&
        glCheckFramebufferStatusEXT != 0 &&
        glFramebufferTexture1DEXT != 0 &&
        glFramebufferTexture2DEXT != 0 &&
        glFramebufferTexture3DEXT != 0 &&
        glFramebufferRenderbufferEXT != 0 &&
        glGenerateMipmapEXT != 0;

    if (!isGLExtensionSupported(contextID, "GL_EXT_framebuffer_blit"))
        return;

    LOAD_FBO_EXT(glBlitFramebufferEXT);

    if (isGLExtensionSupported(contextID, "GL_EXT_framebuffer_multisample"))
    {
        LOAD_FBO_EXT(glRenderbufferStorageMultisampleEXT);
    }

    if (isGLExtensionSupported(contextID, "GL_NV_framebuffer_multisample_coverage"))
    {
        LOAD_FBO_EXT(glRenderbufferStorageMultisampleCoverageNV);
    }

    if (isGLExtensionSupported(contextID, "GL_EXT_packed_depth_stencil"))
    {
        _packed_depth_stencil_supported = true;
    }
}


/**************************************************************************
 * RenderBuffer
 **************************************************************************/

///////////////////////////////////////////////////////////////////////////
// static cache of glRenderbuffers flagged for deletion, which will actually
// be deleted in the correct GL context.

typedef std::list<GLuint> RenderBufferHandleList;
typedef osg::buffered_object<RenderBufferHandleList> DeletedRenderBufferCache;

static OpenThreads::Mutex    s_mutex_deletedRenderBufferCache;
static DeletedRenderBufferCache s_deletedRenderBufferCache;

void RenderBuffer::deleteRenderBuffer(unsigned int contextID, GLuint rb)
{
    if( rb )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedRenderBufferCache);

        // add glProgram to the cache for the appropriate context.
        s_deletedRenderBufferCache[contextID].push_back(rb);
    }
}

void RenderBuffer::flushDeletedRenderBuffers(unsigned int contextID,double /*currentTime*/, double& availableTime)
{
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    const FBOExtensions* extensions = FBOExtensions::instance(contextID,true);
    if(!extensions || !extensions->isSupported() ) return;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedRenderBufferCache);

        RenderBufferHandleList& pList = s_deletedRenderBufferCache[contextID];
        for(RenderBufferHandleList::iterator titr=pList.begin();
            titr!=pList.end() && elapsedTime<availableTime;
            )
        {
            extensions->glDeleteRenderbuffersEXT(1, &(*titr) );
            titr = pList.erase( titr );
            elapsedTime = timer.delta_s(start_tick,timer.tick());
        }
    }

    availableTime -= elapsedTime;
}

void RenderBuffer::discardDeletedRenderBuffers(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedRenderBufferCache);
    RenderBufferHandleList& pList = s_deletedRenderBufferCache[contextID];
    pList.clear();
}


RenderBuffer::RenderBuffer()
:    Object(),
    _internalFormat(GL_DEPTH_COMPONENT24),
    _width(512),
    _height(512),
    _samples(0),
    _colorSamples(0)
{
}

RenderBuffer::RenderBuffer(int width, int height, GLenum internalFormat, int samples, int colorSamples)
:    Object(),
    _internalFormat(internalFormat),
    _width(width),
    _height(height),
    _samples(samples),
    _colorSamples(colorSamples)
{
}

RenderBuffer::RenderBuffer(const RenderBuffer &copy, const CopyOp &copyop)
:    Object(copy, copyop),
    _internalFormat(copy._internalFormat),
    _width(copy._width),
    _height(copy._height),
    _samples(copy._samples),
    _colorSamples(copy._colorSamples)
{
}

RenderBuffer::~RenderBuffer()
{
    for(unsigned i=0; i<_objectID.size(); ++i)
    {
        if (_objectID[i]) deleteRenderBuffer(i, _objectID[i]);
    }
}

int RenderBuffer::getMaxSamples(unsigned int contextID, const FBOExtensions *ext)
{
    static osg::buffered_value<GLint> maxSamplesList;

    GLint& maxSamples = maxSamplesList[contextID];

    if (!maxSamples && ext->isMultisampleSupported())
    {
        glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);
    }

    return maxSamples;
}

GLuint RenderBuffer::getObjectID(unsigned int contextID, const FBOExtensions *ext) const
{
    GLuint &objectID = _objectID[contextID];

    int &dirty = _dirty[contextID];

    if (objectID == 0)
    {
        ext->glGenRenderbuffersEXT(1, &objectID);
        if (objectID == 0) 
            return 0;
        dirty = 1;
    }

    if (dirty)
    {
        // bind and configure
        ext->glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, objectID);

        // framebuffer_multisample_coverage specification requires that coverage
        // samples must be >= color samples.
        if (_samples < _colorSamples)
        {
            notify(WARN) << "Coverage samples must be greater than or equal to color samples."
                " Setting coverage samples equal to color samples." << std::endl;
            const_cast<RenderBuffer*>(this)->setSamples(_colorSamples);
        }

        if (_samples > 0 && ext->isMultisampleCoverageSupported())
        {
            int samples = minimum(_samples, getMaxSamples(contextID, ext));
            int colorSamples = minimum(_colorSamples, samples);

            ext->glRenderbufferStorageMultisampleCoverageNV(GL_RENDERBUFFER_EXT,
                samples, colorSamples, _internalFormat, _width, _height);
        }
        else if (_samples > 0 && ext->isMultisampleSupported())
        {
            int samples = minimum(_samples, getMaxSamples(contextID, ext));

            ext->glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT,
                samples, _internalFormat, _width, _height);
        }
        else
        {
            ext->glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, _internalFormat, _width, _height);
        }
        dirty = 0;
    }

    return objectID;
}

/**************************************************************************
 * FrameBufferAttachment
 **************************************************************************/

#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X  0x8515
#endif

struct FrameBufferAttachment::Pimpl
{
    enum TargetType
    {
        RENDERBUFFER,
        TEXTURE1D,
        TEXTURE2D,
        TEXTURE3D,
        TEXTURECUBE,
        TEXTURERECT,
        TEXTURE2DARRAY,
        TEXTURE2DMULTISAMPLE
    };
    
    TargetType targetType;
    ref_ptr<RenderBuffer> renderbufferTarget;
    ref_ptr<Texture> textureTarget;
    int cubeMapFace;
    int level;
    int zoffset;

    explicit Pimpl(TargetType ttype = RENDERBUFFER, int lev = 0)
    :    targetType(ttype),
        cubeMapFace(0),
        level(lev),
        zoffset(0)
    {
    }

    Pimpl(const Pimpl &copy)
    :    targetType(copy.targetType),
        renderbufferTarget(copy.renderbufferTarget),
        textureTarget(copy.textureTarget),
        cubeMapFace(copy.cubeMapFace),
        level(copy.level),
        zoffset(copy.zoffset)
    {
    }
};

FrameBufferAttachment::FrameBufferAttachment()
{
    _ximpl = new Pimpl;
}

FrameBufferAttachment::FrameBufferAttachment(const FrameBufferAttachment &copy)
{
    _ximpl = new Pimpl(*copy._ximpl);
}

FrameBufferAttachment::FrameBufferAttachment(RenderBuffer* target)
{
    _ximpl = new Pimpl(Pimpl::RENDERBUFFER);
    _ximpl->renderbufferTarget = target;
}

FrameBufferAttachment::FrameBufferAttachment(Texture1D* target, int level)
{
    _ximpl = new Pimpl(Pimpl::TEXTURE1D, level);
    _ximpl->textureTarget = target;
}

FrameBufferAttachment::FrameBufferAttachment(Texture2D* target, int level)
{
    _ximpl = new Pimpl(Pimpl::TEXTURE2D, level);
    _ximpl->textureTarget = target;
}

FrameBufferAttachment::FrameBufferAttachment(Texture2DMultisample* target, int level)
{
    _ximpl = new Pimpl(Pimpl::TEXTURE2DMULTISAMPLE, level);
    _ximpl->textureTarget = target;
}

FrameBufferAttachment::FrameBufferAttachment(Texture3D* target, int zoffset, int level)
{
    _ximpl = new Pimpl(Pimpl::TEXTURE3D, level);
    _ximpl->textureTarget = target;
    _ximpl->zoffset = zoffset;
}

FrameBufferAttachment::FrameBufferAttachment(Texture2DArray* target, int layer, int level)
{
    _ximpl = new Pimpl(Pimpl::TEXTURE2DARRAY, level);
    _ximpl->textureTarget = target;
    _ximpl->zoffset = layer;
}

FrameBufferAttachment::FrameBufferAttachment(TextureCubeMap* target, int face, int level)
{
    _ximpl = new Pimpl(Pimpl::TEXTURECUBE, level);
    _ximpl->textureTarget = target;
    _ximpl->cubeMapFace = face;
}

FrameBufferAttachment::FrameBufferAttachment(TextureRectangle* target)
{
    _ximpl = new Pimpl(Pimpl::TEXTURERECT);
    _ximpl->textureTarget = target;
}

FrameBufferAttachment::FrameBufferAttachment(Camera::Attachment& attachment)
{
    osg::Texture* texture = attachment._texture.get();
    
    if (texture)
    {
        osg::Texture1D* texture1D = dynamic_cast<osg::Texture1D*>(texture);
        if (texture1D)
        {
            _ximpl = new Pimpl(Pimpl::TEXTURE1D, attachment._level);
            _ximpl->textureTarget = texture1D;
            return;
        }

        osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(texture);
        if (texture2D)
        {
            _ximpl = new Pimpl(Pimpl::TEXTURE2D, attachment._level);
            _ximpl->textureTarget = texture2D;
            return;
        }

        osg::Texture2DMultisample* texture2DMS = dynamic_cast<osg::Texture2DMultisample*>(texture);
        if (texture2DMS)
        {
            _ximpl = new Pimpl(Pimpl::TEXTURE2DMULTISAMPLE, attachment._level);
            _ximpl->textureTarget = texture2DMS;
            return;
        }

        osg::Texture3D* texture3D = dynamic_cast<osg::Texture3D*>(texture);
        if (texture3D)
        {
            _ximpl = new Pimpl(Pimpl::TEXTURE3D, attachment._level);
            _ximpl->textureTarget = texture3D;
            _ximpl->zoffset = attachment._face;
            return;
        }
        
        osg::Texture2DArray* texture2DArray = dynamic_cast<osg::Texture2DArray*>(texture);
        if (texture2DArray)
        {
            _ximpl = new Pimpl(Pimpl::TEXTURE2DARRAY, attachment._level);
            _ximpl->textureTarget = texture2DArray;
            _ximpl->zoffset = attachment._face;
            return;
        }

        osg::TextureCubeMap* textureCubeMap = dynamic_cast<osg::TextureCubeMap*>(texture);
        if (textureCubeMap)
        {
            _ximpl = new Pimpl(Pimpl::TEXTURECUBE, attachment._level);
            _ximpl->textureTarget = textureCubeMap;
            _ximpl->cubeMapFace = attachment._face;
            return;
        }

        osg::TextureRectangle* textureRectangle = dynamic_cast<osg::TextureRectangle*>(texture);
        if (textureRectangle)
        {
            _ximpl = new Pimpl(Pimpl::TEXTURERECT);
            _ximpl->textureTarget = textureRectangle;
            return;
        }
    }

    osg::Image* image = attachment._image.get();
    if (image)
    {
        if (image->s()>0 && image->t()>0)
        {
            GLenum format = attachment._image->getInternalTextureFormat();
            if (format == 0)
                format = attachment._internalFormat;
            _ximpl = new Pimpl(Pimpl::RENDERBUFFER);
            _ximpl->renderbufferTarget = new osg::RenderBuffer(image->s(), image->t(), format);
        }
        else
        {
            osg::notify(osg::WARN)<<"Error: FrameBufferAttachment::FrameBufferAttachment(Camera::Attachment&) passed an empty osg::Image, image must be allocated first."<<std::endl;
        }
        return;
    }

    osg::notify(osg::WARN)<<"Error: FrameBufferAttachment::FrameBufferAttachment(Camera::Attachment&) passed an unrecognised Texture type."<<std::endl;
}


FrameBufferAttachment::~FrameBufferAttachment()
{
    delete _ximpl;
}

FrameBufferAttachment &FrameBufferAttachment::operator = (const FrameBufferAttachment &copy)
{
    delete _ximpl;
    _ximpl = new Pimpl(*copy._ximpl);
    return *this;
}

bool FrameBufferAttachment::isMultisample() const
{
    if (_ximpl->renderbufferTarget.valid())
    {
        return _ximpl->renderbufferTarget->getSamples() > 0;
    }

    return false;
}

void FrameBufferAttachment::createRequiredTexturesAndApplyGenerateMipMap(State &state, const FBOExtensions* ext) const
{
    unsigned int contextID = state.getContextID();

    // force compile texture if necessary
    Texture::TextureObject *tobj = 0;
    if (_ximpl->textureTarget.valid())
    {
        tobj = _ximpl->textureTarget->getTextureObject(contextID);
        if (!tobj || tobj->_id == 0)
        {
            _ximpl->textureTarget->compileGLObjects(state);
            tobj = _ximpl->textureTarget->getTextureObject(contextID);
        }
        if (!tobj || tobj->_id == 0)
            return;

        Texture::FilterMode minFilter = _ximpl->textureTarget->getFilter(Texture::MIN_FILTER);
        if (minFilter==Texture::LINEAR_MIPMAP_LINEAR || 
            minFilter==Texture::LINEAR_MIPMAP_NEAREST || 
            minFilter==Texture::NEAREST_MIPMAP_LINEAR || 
            minFilter==Texture::NEAREST_MIPMAP_NEAREST)
        {
            state.setActiveTextureUnit(0);
            state.applyTextureAttribute(0, _ximpl->textureTarget.get());
            ext->glGenerateMipmapEXT(_ximpl->textureTarget->getTextureTarget());
        }

    }
}

void FrameBufferAttachment::attach(State &state, GLenum target, GLenum attachment_point, const FBOExtensions* ext) const
{
    unsigned int contextID = state.getContextID();

    Texture::TextureObject *tobj = 0;
    if (_ximpl->textureTarget.valid())
    {
        tobj = _ximpl->textureTarget->getTextureObject(contextID);
        if (!tobj || tobj->_id == 0)
        {
            _ximpl->textureTarget->compileGLObjects(state);
            tobj = _ximpl->textureTarget->getTextureObject(contextID);

        }
        if (!tobj || tobj->_id == 0)
            return;
    }
    
    switch (_ximpl->targetType)
    {
    default:
    case Pimpl::RENDERBUFFER:
        ext->glFramebufferRenderbufferEXT(target, attachment_point, GL_RENDERBUFFER_EXT, _ximpl->renderbufferTarget->getObjectID(contextID, ext));
        break;
    case Pimpl::TEXTURE1D:
        ext->glFramebufferTexture1DEXT(target, attachment_point, GL_TEXTURE_1D, tobj->_id, _ximpl->level);
        break;
    case Pimpl::TEXTURE2D:
        ext->glFramebufferTexture2DEXT(target, attachment_point, GL_TEXTURE_2D, tobj->_id, _ximpl->level);
        break;
    case Pimpl::TEXTURE2DMULTISAMPLE:
        ext->glFramebufferTexture2DEXT(target, attachment_point, GL_TEXTURE_2D_MULTISAMPLE, tobj->_id, _ximpl->level);
        break;
    case Pimpl::TEXTURE3D:
        ext->glFramebufferTexture3DEXT(target, attachment_point, GL_TEXTURE_3D, tobj->_id, _ximpl->level, _ximpl->zoffset);
        break;
    case Pimpl::TEXTURE2DARRAY:
        ext->glFramebufferTextureLayerEXT(target, attachment_point, tobj->_id, _ximpl->level, _ximpl->zoffset);
        break;
    case Pimpl::TEXTURERECT:
        ext->glFramebufferTexture2DEXT(target, attachment_point, GL_TEXTURE_RECTANGLE, tobj->_id, 0);
        break;
    case Pimpl::TEXTURECUBE:
        ext->glFramebufferTexture2DEXT(target, attachment_point, GL_TEXTURE_CUBE_MAP_POSITIVE_X + _ximpl->cubeMapFace, tobj->_id, _ximpl->level);
        break;
    }
}

int FrameBufferAttachment::compare(const FrameBufferAttachment &fa) const
{
    if (&fa == this) return 0;
    if (_ximpl->targetType < fa._ximpl->targetType) return -1;
    if (_ximpl->targetType > fa._ximpl->targetType) return 1;
    if (_ximpl->renderbufferTarget.get() < fa._ximpl->renderbufferTarget.get()) return -1;
    if (_ximpl->renderbufferTarget.get() > fa._ximpl->renderbufferTarget.get()) return 1;
    if (_ximpl->textureTarget.get() < fa._ximpl->textureTarget.get()) return -1;
    if (_ximpl->textureTarget.get() > fa._ximpl->textureTarget.get()) return 1;
    if (_ximpl->cubeMapFace < fa._ximpl->cubeMapFace) return -1;
    if (_ximpl->cubeMapFace > fa._ximpl->cubeMapFace) return 1;
    if (_ximpl->level < fa._ximpl->level) return -1;
    if (_ximpl->level > fa._ximpl->level) return 1;
    if (_ximpl->zoffset < fa._ximpl->zoffset) return -1;
    if (_ximpl->zoffset > fa._ximpl->zoffset) return 1;
    return 0;
}

RenderBuffer* FrameBufferAttachment::getRenderBuffer()
{
    return _ximpl->renderbufferTarget.get();
}

Texture* FrameBufferAttachment::getTexture()
{
    return _ximpl->textureTarget.get();
}

const RenderBuffer* FrameBufferAttachment::getRenderBuffer() const
{
    return _ximpl->renderbufferTarget.get();
}

const Texture* FrameBufferAttachment::getTexture() const
{
    return _ximpl->textureTarget.get();
}

int FrameBufferAttachment::getCubeMapFace() const
{
    return _ximpl->cubeMapFace;
}

int FrameBufferAttachment::getTextureLevel() const
{
    return _ximpl->level;
}

int FrameBufferAttachment::getTexture3DZOffset() const
{
    return _ximpl->zoffset;
}

int FrameBufferAttachment::getTextureArrayLayer() const
{
    return _ximpl->zoffset;
}

/**************************************************************************
 * FrameBufferObject
 **************************************************************************/

///////////////////////////////////////////////////////////////////////////
// static cache of glRenderbuffers flagged for deletion, which will actually
// be deleted in the correct GL context.

typedef std::list<GLuint> FrameBufferObjectHandleList;
typedef osg::buffered_object<FrameBufferObjectHandleList> DeletedFrameBufferObjectCache;

static OpenThreads::Mutex    s_mutex_deletedFrameBufferObjectCache;
static DeletedFrameBufferObjectCache s_deletedFrameBufferObjectCache;

void FrameBufferObject::deleteFrameBufferObject(unsigned int contextID, GLuint rb)
{
    if( rb )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedFrameBufferObjectCache);

        // add glProgram to the cache for the appropriate context.
        s_deletedFrameBufferObjectCache[contextID].push_back(rb);
    }
}

void FrameBufferObject::flushDeletedFrameBufferObjects(unsigned int contextID,double /*currentTime*/, double& availableTime)
{
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    const FBOExtensions* extensions = FBOExtensions::instance(contextID,true);
    if(!extensions || !extensions->isSupported() ) return;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedFrameBufferObjectCache);

        FrameBufferObjectHandleList& pList = s_deletedFrameBufferObjectCache[contextID];
        for(FrameBufferObjectHandleList::iterator titr=pList.begin();
            titr!=pList.end() && elapsedTime<availableTime;
            )
        {
            extensions->glDeleteFramebuffersEXT(1, &(*titr) );
            titr = pList.erase( titr );
            elapsedTime = timer.delta_s(start_tick,timer.tick());
        }
    }

    availableTime -= elapsedTime;
}

void FrameBufferObject::discardDeletedFrameBufferObjects(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedFrameBufferObjectCache);
    FrameBufferObjectHandleList& pList = s_deletedFrameBufferObjectCache[contextID];

    pList.clear();
}



FrameBufferObject::FrameBufferObject()
:    StateAttribute()
{
}

FrameBufferObject::FrameBufferObject(const FrameBufferObject &copy, const CopyOp &copyop)
:    StateAttribute(copy, copyop),
    _attachments(copy._attachments),
    _drawBuffers(copy._drawBuffers)
{
}

FrameBufferObject::~FrameBufferObject()
{
    for(unsigned i=0; i<_fboID.size(); ++i)
    {
        if (_fboID[i]) deleteFrameBufferObject(i, _fboID[i]);
    }
}

void FrameBufferObject::setAttachment(BufferComponent attachment_point, const FrameBufferAttachment &attachment)
{
    _attachments[attachment_point] = attachment;

    updateDrawBuffers();
    dirtyAll();
}


GLenum FrameBufferObject::convertBufferComponentToGLenum(BufferComponent attachment_point) const
{
    switch(attachment_point)
    {
        case(Camera::DEPTH_BUFFER): return GL_DEPTH_ATTACHMENT_EXT;
        case(Camera::STENCIL_BUFFER): return GL_STENCIL_ATTACHMENT_EXT;
        case(Camera::COLOR_BUFFER): return GL_COLOR_ATTACHMENT0_EXT;
        default: return GLenum(GL_COLOR_ATTACHMENT0_EXT + (attachment_point-Camera::COLOR_BUFFER0));
    }
}

void FrameBufferObject::updateDrawBuffers()
{
    _drawBuffers.clear();

    // create textures and mipmaps before we bind the frame buffer object
    for (AttachmentMap::const_iterator i=_attachments.begin(); i!=_attachments.end(); ++i)
    {
        // setup draw buffers based on the attachment definition
        if (i->first >= Camera::COLOR_BUFFER0 && i->first <= Camera::COLOR_BUFFER15)
            _drawBuffers.push_back(convertBufferComponentToGLenum(i->first));
    }
}

void FrameBufferObject::apply(State &state) const
{
    apply(state, READ_DRAW_FRAMEBUFFER);
}

void FrameBufferObject::apply(State &state, BindTarget target) const
{
    unsigned int contextID = state.getContextID();

    if (_unsupported[contextID])
        return;
        
        
    FBOExtensions* ext = FBOExtensions::instance(contextID,true);
    if (!ext->isSupported())
    {
        _unsupported[contextID] = 1;
        notify(WARN) << "Warning: EXT_framebuffer_object is not supported" << std::endl;
        return;
    }

    if (_attachments.empty())
    {
        ext->glBindFramebufferEXT(target, 0);
        return;
    }

    int &dirtyAttachmentList = _dirtyAttachmentList[contextID];

    GLuint &fboID = _fboID[contextID];
    if (fboID == 0)
    {
        ext->glGenFramebuffersEXT(1, &fboID);
        if (fboID == 0)
        {
            notify(WARN) << "Warning: FrameBufferObject: could not create the FBO" << std::endl;
            return;
        }

        dirtyAttachmentList = 1;

    }

    if (dirtyAttachmentList)
    {
        // the set of of attachments appears to be thread sensitive, it shouldn't be because 
        // OpenGL FBO handles osg::FrameBufferObject has are multi-buffered...
        // so as a temporary fix will stick in a mutex to ensure that only one thread passes through here
        // at one time.
        static OpenThreads::Mutex s_mutex;
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex);

        // create textures and mipmaps before we bind the frame buffer object
        for (AttachmentMap::const_iterator i=_attachments.begin(); i!=_attachments.end(); ++i)
        {
            const FrameBufferAttachment &fa = i->second;
            fa.createRequiredTexturesAndApplyGenerateMipMap(state, ext);
        }

    }
    
   
    ext->glBindFramebufferEXT(target, fboID);

    // enable drawing buffers to render the result to fbo
    if (_drawBuffers.size() > 0)
    {
        GL2Extensions *gl2e = GL2Extensions::Get(state.getContextID(), true );
        if (gl2e)
        {
            gl2e->glDrawBuffers(_drawBuffers.size(), &(_drawBuffers[0]));
        }
    }

    if (dirtyAttachmentList)
    {
        for (AttachmentMap::const_iterator i=_attachments.begin(); i!=_attachments.end(); ++i)
        {
            const FrameBufferAttachment &fa = i->second;
            switch(i->first)
            {
                case(Camera::PACKED_DEPTH_STENCIL_BUFFER):
                    if (ext->isPackedDepthStencilSupported())
                    {
                        fa.attach(state, target, GL_DEPTH_ATTACHMENT_EXT, ext);
                        fa.attach(state, target, GL_STENCIL_ATTACHMENT_EXT, ext);
                    }
                    else
                    {
                        notify(WARN) << 
                            "Warning: FrameBufferObject: could not attach PACKED_DEPTH_STENCIL_BUFFER, "
                            "EXT_packed_depth_stencil is not supported !" << std::endl;
                    }
                    break;

                default:
                    fa.attach(state, target, convertBufferComponentToGLenum(i->first), ext);
                    break;
            }
        }        
        dirtyAttachmentList = 0;
    }

}

bool FrameBufferObject::isMultisample() const
{
    if (_attachments.size())
    {
        // If the FBO is correctly set up then all attachments will be either
        // multisampled or single sampled. Therefore we can just return the
        // result of the first attachment.
        return _attachments.begin()->second.isMultisample();
    }

    return false;
}

int FrameBufferObject::compare(const StateAttribute &sa) const
{
    COMPARE_StateAttribute_Types(FrameBufferObject, sa);
    COMPARE_StateAttribute_Parameter(_attachments.size());
    AttachmentMap::const_iterator i = _attachments.begin();
    AttachmentMap::const_iterator j = rhs._attachments.begin();
    for (; i!=_attachments.end(); ++i, ++j)
    {
        int cmp = i->second.compare(j->second);
        if (cmp != 0) return cmp;
    }
    return 0;
}
