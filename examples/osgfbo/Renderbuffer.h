#ifndef OSG_RENDERBUFFER
#define OSG_RENDERBUFFER 1

#include <osg/Object>
#include <osg/buffered_value>
#include <osg/GL>

#ifndef GL_VERSION_1_4
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#endif

namespace osg
{

    class FBOExtensions;

    class Renderbuffer: public Object
    {
    public:
        Renderbuffer();
        Renderbuffer(int width, int height, GLenum internalFormat);
        Renderbuffer(const Renderbuffer &copy, const CopyOp &copyop = CopyOp::SHALLOW_COPY);

        META_Object(osg, Renderbuffer);

        inline int getWidth() const;
        inline int getHeight() const;
        inline void setWidth(int w);
        inline void setHeight(int h);
        inline void setSize(int w, int h);
        inline GLenum getInternalFormat() const;
        inline void setInternalFormat(GLenum format);

        GLuint getObjectID(unsigned int contextID, const FBOExtensions *ext) const;
        inline int compare(const Renderbuffer &rb) const;

    protected:
        virtual ~Renderbuffer() {}
        Renderbuffer &operator=(const Renderbuffer &) { return *this; }

        inline void dirtyAll() const;

    private:
        mutable buffered_value<GLuint> _objectID;
        mutable buffered_value<int> _dirty;
        GLenum _internalFormat;
        int _width;
        int _height;
    };

    // INLINE METHODS

    inline int Renderbuffer::getWidth() const
    {
        return _width;
    }

    inline int Renderbuffer::getHeight() const
    {
        return _height;
    }

    inline void Renderbuffer::setWidth(int w)
    {
        _width = w;
        dirtyAll();
    }

    inline void Renderbuffer::setHeight(int h)
    {
        _height = h;
        dirtyAll();
    }

    inline void Renderbuffer::setSize(int w, int h)
    {
        _width = w;
        _height = h;
        dirtyAll();
    }

    inline GLenum Renderbuffer::getInternalFormat() const
    {
        return _internalFormat;
    }

    inline void Renderbuffer::setInternalFormat(GLenum format)
    {
        _internalFormat = format;
        dirtyAll();
    }

    inline void Renderbuffer::dirtyAll() const
    {
        _dirty.setAllElementsTo(1);
    }

    inline int Renderbuffer::compare(const Renderbuffer &rb) const
    {
        if (&rb == this) return 0;
        if (_internalFormat < rb._internalFormat) return -1;
        if (_internalFormat > rb._internalFormat) return 1;
        if (_width < rb._width) return -1;
        if (_width > rb._width) return 1;
        if (_height < rb._height) return -1;
        if (_height > rb._height) return 1;
        return 0;
    }

}

#endif
