#include "FBOExtensions.h"
#include <osg/GLExtensions>

using namespace osg;

#define LOAD_FBO_EXT(name)    name = (T##name *)getGLExtensionFuncPtr(#name);

FBOExtensions::FBOExtensions(unsigned int contextID)
:    _supported(false)
{
    if (!isGLExtensionSupported(contextID, "GL_EXT_framebuffer_object"))
        return;
    
    LOAD_FBO_EXT(glBindRenderbufferEXT);
    LOAD_FBO_EXT(glGenRenderbuffersEXT);
    LOAD_FBO_EXT(glRenderbufferStorageEXT);
    LOAD_FBO_EXT(glBindFramebufferEXT);
    LOAD_FBO_EXT(glGenFramebuffersEXT);
    LOAD_FBO_EXT(glCheckFramebufferStatusEXT);
    LOAD_FBO_EXT(glFramebufferTexture1DEXT);
    LOAD_FBO_EXT(glFramebufferTexture2DEXT);
    LOAD_FBO_EXT(glFramebufferTexture3DEXT);
    LOAD_FBO_EXT(glFramebufferRenderbufferEXT);
    LOAD_FBO_EXT(glGenerateMipmapEXT);

    _supported = 
        glBindRenderbufferEXT != 0 &&
        glGenRenderbuffersEXT != 0 &&
        glRenderbufferStorageEXT != 0 &&
        glBindFramebufferEXT != 0 &&
        glGenFramebuffersEXT != 0 &&
        glCheckFramebufferStatusEXT != 0 &&
        glFramebufferTexture1DEXT != 0 &&
        glFramebufferTexture2DEXT != 0 &&
        glFramebufferTexture3DEXT != 0 &&
        glFramebufferRenderbufferEXT != 0 &&
        glGenerateMipmapEXT != 0;
}
