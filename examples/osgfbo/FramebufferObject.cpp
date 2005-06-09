#include "FramebufferObject.h"
#include "FBOExtensions.h"
#include <osg/State>
#include <osg/Notify>

using namespace osg;

FramebufferObject::FramebufferObject()
:    StateAttribute()
{
}

FramebufferObject::FramebufferObject(const FramebufferObject &copy, const CopyOp &copyop)
:    StateAttribute(copy, copyop),
    _attachments(copy._attachments)
{
}

void FramebufferObject::apply(State &state) const
{
    unsigned int contextID = state.getContextID();

    if (_unsupported[contextID])
        return;

    FBOExtensions* ext = FBOExtensions::instance(contextID);
    if (!ext->isSupported())
    {
        _unsupported[contextID] = 1;
        notify(WARN) << "Warning: EXT_framebuffer_object is not supported" << std::endl;
        return;
    }

    if (_attachments.empty())
    {
        ext->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        return;
    }

    int &dirtyAttachmentList = _dirtyAttachmentList[contextID];

    GLuint &fboID = _fboID[contextID];
    if (fboID == 0)
    {
        ext->glGenFramebuffersEXT(1, &fboID);
        if (fboID == 0)
        {
            notify(WARN) << "Warning: FramebufferObject: could not create the FBO" << std::endl;
            return;
        }
        dirtyAttachmentList = 1;
    }

    ext->glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID);

    if (dirtyAttachmentList)
    {
        for (AttachmentMap::const_iterator i=_attachments.begin(); i!=_attachments.end(); ++i)
        {
            const FramebufferAttachment &fa = i->second;
            fa.attach(state, i->first, ext);
        }        
        dirtyAttachmentList = 0;
    }
}

int FramebufferObject::compare(const StateAttribute &sa) const
{
    COMPARE_StateAttribute_Types(FramebufferObject, sa);
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
