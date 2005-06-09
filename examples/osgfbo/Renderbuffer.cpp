#include "Renderbuffer.h"
#include "FBOExtensions.h"

using namespace osg;

Renderbuffer::Renderbuffer()
:    Object(),
    _width(512),
    _height(512),
    _internalFormat(GL_DEPTH_COMPONENT24)
{
}

Renderbuffer::Renderbuffer(int width, int height, GLenum internalFormat)
:    Object(),
    _width(width),
    _height(height),
    _internalFormat(internalFormat)
{
}

Renderbuffer::Renderbuffer(const Renderbuffer &copy, const CopyOp &copyop)
:    Object(copy, copyop),
    _width(copy._width),
    _height(copy._height),
    _internalFormat(copy._internalFormat)
{
}

GLuint Renderbuffer::getObjectID(unsigned int contextID, const FBOExtensions *ext) const
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
        ext->glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, _internalFormat, _width, _height);
        dirty = 0;
    }

    return objectID;
}
