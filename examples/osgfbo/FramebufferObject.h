#ifndef OSG_FRAMEBUFFEROBJECT
#define OSG_FRAMEBUFFEROBJECT 1

#include <osg/Texture>
#include <osg/buffered_value>

#include "FramebufferAttachment.h"

namespace osg
{

    class FramebufferObject: public StateAttribute
    {
    public:
        typedef std::map<GLenum, FramebufferAttachment> AttachmentMap;

        FramebufferObject();
        FramebufferObject(const FramebufferObject &copy, const CopyOp &copyop = CopyOp::SHALLOW_COPY);

        META_StateAttribute(osg, FramebufferObject, (StateAttribute::Type)0x101010/*FRAMEBUFFEROBJECT*/);

        inline const AttachmentMap &getAttachmentMap() const;
        inline bool hasAttachment(GLenum attachment_point) const;
        inline const FramebufferAttachment& getAttachment(GLenum attachment_point) const;
        inline void setAttachment(GLenum attachment_point, const FramebufferAttachment &attachment);

        int compare(const StateAttribute &sa) const;
        void apply(State &state) const;

    protected:
        virtual ~FramebufferObject() {}
        FramebufferObject &operator=(const FramebufferObject &) { return *this; }

        inline void dirtyAll();

    private:        
        AttachmentMap _attachments;
        mutable buffered_value<int> _dirtyAttachmentList;
        mutable buffered_value<int> _unsupported;
        mutable buffered_value<GLuint> _fboID;
    };

    // INLINE METHODS

    inline const FramebufferObject::AttachmentMap &FramebufferObject::getAttachmentMap() const
    {
        return _attachments;
    }

    inline bool FramebufferObject::hasAttachment(GLenum attachment_point) const
    {
        return _attachments.find(attachment_point) != _attachments.end();
    }

    inline const FramebufferAttachment &FramebufferObject::getAttachment(GLenum attachment_point) const
    {
        return _attachments.find(attachment_point)->second;
    }

    inline void FramebufferObject::setAttachment(GLenum attachment_point, const FramebufferAttachment &attachment)
    {
        _attachments[attachment_point] = attachment;
        dirtyAll();
    }

    inline void FramebufferObject::dirtyAll()
    {
        _dirtyAttachmentList.setAllElementsTo(1);
    }

}

#endif
