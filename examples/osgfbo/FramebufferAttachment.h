#ifndef OSG_FRAMEBUFFERATTACHMENT
#define OSG_FRAMEBUFFERATTACHMENT 1

#include <osg/Object>
#include <osg/GL>

namespace osg
{

    class FBOExtensions;

    class Renderbuffer;
    class Texture1D;
    class Texture2D;
    class Texture3D;
    class TextureCubeMap;
    class TextureRectangle;

    class FramebufferAttachment
    {
    public:
        FramebufferAttachment();
        FramebufferAttachment(const FramebufferAttachment &copy);

        explicit FramebufferAttachment(Renderbuffer* target);
        explicit FramebufferAttachment(Texture1D* target, int level = 0);
        explicit FramebufferAttachment(Texture2D* target, int level = 0);
        explicit FramebufferAttachment(Texture3D* target, int zoffset, int level = 0);
        explicit FramebufferAttachment(TextureCubeMap* target, int face, int level = 0);
        explicit FramebufferAttachment(TextureRectangle* target);
        
        ~FramebufferAttachment();

        FramebufferAttachment &operator = (const FramebufferAttachment &copy);        

        void attach(State &state, GLenum attachment_point, const FBOExtensions* ext) const;
        int compare(const FramebufferAttachment &fa) const;

    private:
        // use the Pimpl idiom to avoid dependency from
        // all Texture* headers
        struct Pimpl;
        Pimpl* _ximpl;
    };

}

#endif
