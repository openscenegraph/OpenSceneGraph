#include "FramebufferAttachment.h"
#include "Renderbuffer.h"
#include "FBOExtensions.h"
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TextureCubeMap>
#include <osg/TextureRectangle>
#include <osg/State>

using namespace osg;

#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X  0x8515
#endif

struct FramebufferAttachment::Pimpl
{
    enum TargetType
    {
        RENDERBUFFER,
        TEXTURE1D,
        TEXTURE2D,
        TEXTURE3D,
        TEXTURECUBE,
        TEXTURERECT
    };
    
    TargetType targetType;
    ref_ptr<Renderbuffer> renderbufferTarget;
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

FramebufferAttachment::FramebufferAttachment()
{
    _ximpl = new Pimpl;
}

FramebufferAttachment::FramebufferAttachment(const FramebufferAttachment &copy)
{
    _ximpl = new Pimpl(*copy._ximpl);
}

FramebufferAttachment::FramebufferAttachment(Renderbuffer* target)
{
    _ximpl = new Pimpl(Pimpl::RENDERBUFFER);
    _ximpl->renderbufferTarget = target;
}

FramebufferAttachment::FramebufferAttachment(Texture1D* target, int level)
{
    _ximpl = new Pimpl(Pimpl::TEXTURE1D, level);
    _ximpl->textureTarget = target;
}

FramebufferAttachment::FramebufferAttachment(Texture2D* target, int level)
{
    _ximpl = new Pimpl(Pimpl::TEXTURE2D, level);
    _ximpl->textureTarget = target;
}

FramebufferAttachment::FramebufferAttachment(Texture3D* target, int level, int zoffset)
{
    _ximpl = new Pimpl(Pimpl::TEXTURE3D, level);
    _ximpl->textureTarget = target;
    _ximpl->zoffset = zoffset;
}

FramebufferAttachment::FramebufferAttachment(TextureCubeMap* target, int face, int level)
{
    _ximpl = new Pimpl(Pimpl::TEXTURECUBE, level);
    _ximpl->textureTarget = target;
    _ximpl->cubeMapFace = face;
}

FramebufferAttachment::FramebufferAttachment(TextureRectangle* target)
{
    _ximpl = new Pimpl(Pimpl::TEXTURERECT);
    _ximpl->textureTarget = target;
}

FramebufferAttachment::~FramebufferAttachment()
{
    delete _ximpl;
}

FramebufferAttachment &FramebufferAttachment::operator = (const FramebufferAttachment &copy)
{
    delete _ximpl;
    _ximpl = new Pimpl(*copy._ximpl);
    return *this;
}

void FramebufferAttachment::attach(State &state, GLenum attachment_point, const FBOExtensions* ext) const
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
    }

    switch (_ximpl->targetType)
    {
    default:
    case Pimpl::RENDERBUFFER:
        ext->glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, attachment_point, GL_RENDERBUFFER_EXT, _ximpl->renderbufferTarget->getObjectID(contextID, ext));
        break;
    case Pimpl::TEXTURE1D:
        ext->glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT, attachment_point, GL_TEXTURE_1D, tobj->_id, _ximpl->level);
        break;
    case Pimpl::TEXTURE2D:
        ext->glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment_point, GL_TEXTURE_2D, tobj->_id, _ximpl->level);
        break;
    case Pimpl::TEXTURE3D:
        ext->glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachment_point, GL_TEXTURE_3D, tobj->_id, _ximpl->level, _ximpl->zoffset);
        break;
    case Pimpl::TEXTURERECT:
        ext->glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment_point, GL_TEXTURE_RECTANGLE, tobj->_id, 0);
        break;
    case Pimpl::TEXTURECUBE:
        ext->glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment_point, GL_TEXTURE_CUBE_MAP_POSITIVE_X + _ximpl->cubeMapFace, tobj->_id, _ximpl->level);
        break;
    }
}

int FramebufferAttachment::compare(const FramebufferAttachment &fa) const
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
