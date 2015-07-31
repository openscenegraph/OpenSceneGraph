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
#include <osg/GL>
#include <osg/Notify>
#include <osg/Math>
#include <osg/buffered_value>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

#include <string>
#include <vector>
#include <set>

#if defined(WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif // WIN32_LEAN_AND_MEAN
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif // NOMINMAX
    #include <windows.h>
#elif defined(__APPLE__)
    // The NS*Symbol* stuff found in <mach-o/dyld.h> is deprecated.
    // Since 10.3 (Panther) OS X has provided the dlopen/dlsym/dlclose
    // family of functions under <dlfcn.h>. Since 10.4 (Tiger), Apple claimed
    // the dlfcn family was significantly faster than the NS*Symbol* family.
    // Since 'deprecated' needs to be taken very seriously with the
    // coming of 10.5 (Leopard), it makes sense to use the dlfcn family when possible.
    #include <AvailabilityMacros.h>
    #if !defined(MAC_OS_X_VERSION_10_3) || (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_3)
        #define USE_APPLE_LEGACY_NSSYMBOL
        #include <mach-o/dyld.h>
    #else
        #include <dlfcn.h>
    #endif
#else
    #include <dlfcn.h>
#endif

using namespace osg;

typedef std::set<std::string>  ExtensionSet;
static osg::buffered_object<ExtensionSet> s_glExtensionSetList;
static osg::buffered_object<std::string> s_glRendererList;
static osg::buffered_value<int> s_glInitializedList;

static osg::buffered_object<ExtensionSet> s_gluExtensionSetList;
static osg::buffered_object<std::string> s_gluRendererList;
static osg::buffered_value<int> s_gluInitializedList;

float osg::getGLVersionNumber()
{
    // needs to be extended to do proper things with subversions like 1.5.1, etc.
    char *versionstring   = (char*) glGetString( GL_VERSION );
    if (!versionstring) return 0.0;

    return (findAsciiToFloat(versionstring));
}

bool osg::isExtensionInExtensionString(const char *extension, const char *extensionString)
{
    const char *startOfWord = extensionString;
    const char *endOfWord;
    while ((endOfWord = strchr(startOfWord,' ')) != 0)
    {
        if (strncmp(extension, startOfWord, endOfWord - startOfWord) == 0)
            return true;
        startOfWord = endOfWord+1;
    }
    if (*startOfWord && strcmp(extension, startOfWord) == 0)
        return true;

   return false;
}

bool osg::isGLExtensionSupported(unsigned int contextID, const char *extension)
{
    return osg::isGLExtensionOrVersionSupported(contextID, extension, FLT_MAX);
}

bool osg::isGLExtensionOrVersionSupported(unsigned int contextID, const char *extension, float requiredGLVersion)
{
    ExtensionSet& extensionSet = s_glExtensionSetList[contextID];
    std::string& rendererString = s_glRendererList[contextID];

    // first check to see if GL version number of recent enough.
    bool result = requiredGLVersion <= osg::getGLVersionNumber();

    if (!result)
    {
        // if not already set up, initialize all the per graphic context values.
        if (!s_glInitializedList[contextID])
        {
            s_glInitializedList[contextID] = 1;

            // set up the renderer
            const GLubyte* renderer = glGetString(GL_RENDERER);
            rendererString = renderer ? (const char*)renderer : "";

            // get the extension list from OpenGL.
            GLint numExt = 0;
            #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
            if( osg::getGLVersionNumber() >= 3.0 )
            {
                // OpenGL 3.0 adds the concept of indexed strings and
                // deprecates calls to glGetString( GL_EXTENSIONS ), which
                // will now generate GL_INVALID_ENUM.

                // Get extensions using new indexed string interface.

                typedef const GLubyte * GL_APIENTRY PFNGLGETSTRINGIPROC( GLenum, GLuint );
                PFNGLGETSTRINGIPROC* glGetStringi = 0;
                setGLExtensionFuncPtr( glGetStringi, "glGetStringi");

                if( glGetStringi != NULL )
                {
                    #  ifndef GL_NUM_EXTENSIONS
                    #    define GL_NUM_EXTENSIONS 0x821D
                    #  endif
                    glGetIntegerv( GL_NUM_EXTENSIONS, &numExt );
                    int idx;
                    for( idx=0; idx<numExt; idx++ )
                    {
                        extensionSet.insert( std::string( (char*)( glGetStringi( GL_EXTENSIONS, idx ) ) ) );
                    }
                }
                else
                {
                    OSG_WARN << "isGLExtensionOrVersionSupported: Can't obtain glGetStringi function pointer." << std::endl;
                }
            }
            #endif

            // No extensions found so far, so try with glGetString
            if (numExt == 0)
            {
                // Get extensions using GL1/2 interface.

                const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
                if (extensions==NULL) return false;

                // insert the ' ' delimiated extensions words into the extensionSet.
                const char *startOfWord = extensions;
                const char *endOfWord;
                while ((endOfWord = strchr(startOfWord,' '))!=NULL)
                {
                    extensionSet.insert(std::string(startOfWord,endOfWord));
                    startOfWord = endOfWord+1;
                }
                if (*startOfWord!=0) extensionSet.insert(std::string(startOfWord));
            }

    #if defined(WIN32) && (defined(OSG_GL1_AVAILABLE) || defined(OSG_GL2_AVAILABLE) || defined(OSG_GL3_AVAILABLE))

            // add WGL extensions to the list

            typedef const char* WINAPI WGLGETEXTENSIONSSTRINGARB(HDC);
            WGLGETEXTENSIONSSTRINGARB* wglGetExtensionsStringARB = 0;
            setGLExtensionFuncPtr(wglGetExtensionsStringARB, "wglGetExtensionsStringARB");

            typedef const char* WINAPI WGLGETEXTENSIONSSTRINGEXT();
            WGLGETEXTENSIONSSTRINGEXT* wglGetExtensionsStringEXT = 0;
            setGLExtensionFuncPtr(wglGetExtensionsStringEXT, "wglGetExtensionsStringEXT");

            const char* wglextensions = 0;

            if (wglGetExtensionsStringARB)
            {
                HDC dc = wglGetCurrentDC();
                wglextensions = wglGetExtensionsStringARB(dc);
            }
            else if (wglGetExtensionsStringEXT)
            {
                wglextensions = wglGetExtensionsStringEXT();
            }

            if (wglextensions)
            {
                const char* startOfWord = wglextensions;
                const char* endOfWord;
                while ((endOfWord = strchr(startOfWord, ' ')))
                {
                    extensionSet.insert(std::string(startOfWord, endOfWord));
                    startOfWord = endOfWord+1;
                }
                if (*startOfWord != 0) extensionSet.insert(std::string(startOfWord));
            }

    #endif

            OSG_NOTIFY(INFO)<<"OpenGL extensions supported by installed OpenGL drivers are:"<<std::endl;
            for(ExtensionSet::iterator itr=extensionSet.begin();
                itr!=extensionSet.end();
                ++itr)
            {
                OSG_NOTIFY(INFO)<<"    "<<*itr<<std::endl;
            }

        }

        // true if extension found in extensionSet.
        result = extensionSet.find(extension)!=extensionSet.end();
    }

    // now see if extension is in the extension disabled list
    bool extensionDisabled = false;
    if (result)
    {

        const std::string& disableString = getGLExtensionDisableString();
        if (!disableString.empty())
        {

            std::string::size_type pos=0;
            while ( pos!=std::string::npos && (pos=disableString.find(extension,pos))!=std::string::npos )
            {
                std::string::size_type previousColon = disableString.find_last_of(':',pos);
                std::string::size_type previousSemiColon = disableString.find_last_of(';',pos);

                std::string renderer = "";
                if (previousColon!=std::string::npos)
                {
                    if (previousSemiColon==std::string::npos) renderer = disableString.substr(0,previousColon);
                    else if (previousSemiColon<previousColon) renderer = disableString.substr(previousSemiColon+1,previousColon-previousSemiColon-1);
                }

                if (!renderer.empty())
                {

                    // remove leading spaces if they exist.
                    std::string::size_type leadingSpaces = renderer.find_first_not_of(' ');
                    if (leadingSpaces==std::string::npos) renderer = ""; // nothing but spaces
                    else if (leadingSpaces!=0) renderer.erase(0,leadingSpaces);

                    // remove trailing spaces if they exist.
                    std::string::size_type trailingSpaces = renderer.find_last_not_of(' ');
                    if (trailingSpaces!=std::string::npos) renderer.erase(trailingSpaces+1,std::string::npos);

                }

                if (renderer.empty())
                {
                    extensionDisabled = true;
                    break;
                }

                if (rendererString.find(renderer)!=std::string::npos)
                {
                    extensionDisabled = true;
                    break;

                }

                // move the position in the disable string along so that the same extension is found multiple times
                ++pos;
            }

        }
    }

    if (result)
    {
        if (!extensionDisabled)
        {
            OSG_NOTIFY(INFO)<<"OpenGL extension '"<<extension<<"' is supported."<<std::endl;
        }
        else
        {
            OSG_NOTIFY(INFO)<<"OpenGL extension '"<<extension<<"' is supported by OpenGL\ndriver but has been disabled by osg::getGLExtensionDisableString()."<<std::endl;
        }
    }
    else
    {
        OSG_NOTIFY(INFO)<<"OpenGL extension '"<<extension<<"' is not supported."<<std::endl;
    }


    return result && !extensionDisabled;
}

void osg::setGLExtensionDisableString(const std::string& disableString)
{
    getGLExtensionDisableString() = disableString;
}

std::string& osg::getGLExtensionDisableString()
{
    static const char* envVar = getenv("OSG_GL_EXTENSION_DISABLE");
    static std::string s_GLExtensionDisableString(envVar?envVar:"Nothing defined");

    return s_GLExtensionDisableString;
}

OSG_INIT_SINGLETON_PROXY(GLExtensionDisableStringInitializationProxy, osg::getGLExtensionDisableString())

#ifdef OSG_GL_LIBRARY_STATIC

    #include "GLStaticLibrary.h"

    void* osg::getGLExtensionFuncPtr(const char *funcName)
    {
        return GLStaticLibrary::getProcAddress(funcName);
    }

#else

    void* osg::getGLExtensionFuncPtr(const char *funcName)
    {
        // OSG_NOTICE<<"osg::getGLExtensionFuncPtr("<<funcName<<")"<<std::endl;
    #if defined(__ANDROID__)
        #if defined(OSG_GLES1_AVAILABLE)
            static void *handle = dlopen("libGLESv1_CM.so", RTLD_NOW);
        #elif defined(OSG_GLES2_AVAILABLE)
            static void *handle = dlopen("libGLESv2.so", RTLD_NOW);
        #endif
        return dlsym(handle, funcName);

    #elif defined(WIN32)

        #if defined(OSG_GLES2_AVAILABLE)
            static HMODULE hmodule = GetModuleHandle(TEXT("libGLESv2.dll"));
            return convertPointerType<void*, PROC>(GetProcAddress(hmodule, funcName));
        #elif defined(OSG_GLES1_AVAILABLE)
            static HMODULE hmodule = GetModuleHandleA(TEXT("libgles_cm.dll"));
            return convertPointerType<void*, PROC>(GetProcAddress(hmodule, funcName));
        #else
            return convertPointerType<void*, PROC>(wglGetProcAddress(funcName));
        #endif

    #elif defined(__APPLE__)

        #if defined(USE_APPLE_LEGACY_NSSYMBOL)
            std::string temp( "_" );
            temp += funcName;    // Mac OS X prepends an underscore on function names
            if ( NSIsSymbolNameDefined( temp.c_str() ) )
            {
                NSSymbol symbol = NSLookupAndBindSymbol( temp.c_str() );
                return NSAddressOfSymbol( symbol );
            } else
                return NULL;
        #else
            // I am uncertain of the correct and ideal usage of dlsym here.
            // On the surface, it would seem that the FreeBSD implementation
            // would be the ideal one to copy, but ELF and Mach-o are different
            // and Apple's man page says the following about using RTLD_DEFAULT:
            // "This can be a costly search and should be avoided."
            // The documentation mentions nothing about passing in 0 so I must
            // assume the behavior is undefined.
            // So I could try copying the Sun method which I think all this
            // actually originated from.

            // return dlsym( RTLD_DEFAULT, funcName );
            static void *handle = dlopen((const char *)0L, RTLD_LAZY);
            return dlsym(handle, funcName);
        #endif

    #elif defined (__sun)

        static void *handle = dlopen((const char *)0L, RTLD_LAZY);
        return dlsym(handle, funcName);

    #elif defined (__sgi)

        static void *handle = dlopen((const char *)0L, RTLD_LAZY);
        return dlsym(handle, funcName);

    #elif defined (__FreeBSD__)

        return dlsym( RTLD_DEFAULT, funcName );

    #elif defined (__linux__)

        typedef void (*__GLXextFuncPtr)(void);
        typedef __GLXextFuncPtr (*GetProcAddressARBProc)(const char*);

        #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
        static GetProcAddressARBProc s_glXGetProcAddressARB = convertPointerType<GetProcAddressARBProc, void*>(dlsym(0, "glXGetProcAddressARB"));
        if (s_glXGetProcAddressARB)
        {
            return convertPointerType<void*, __GLXextFuncPtr>((s_glXGetProcAddressARB)(funcName));
        }
        #endif

        return dlsym(0, funcName);

    #elif defined (__QNX__)

        return dlsym(RTLD_DEFAULT, funcName);

    #else // all other unixes

        return dlsym(0, funcName);

    #endif
    }
#endif

///////////////////////////////////////////////////////////////////////////
// Static array of percontext osg::GLExtensions instances

typedef osg::buffered_object< osg::ref_ptr<GLExtensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

GLExtensions* GLExtensions::Get(unsigned int contextID, bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized)
            s_extensions[contextID] = new GLExtensions(contextID);

    return s_extensions[contextID].get();
}

void GLExtensions::Set(unsigned int contextID, GLExtensions* extensions)
{
    s_extensions[contextID] = extensions;
}

///////////////////////////////////////////////////////////////////////////
// Extension function pointers for OpenGL v2.x

GLExtensions::GLExtensions(unsigned int contextID)
{
    const char* version = (const char*) glGetString( GL_VERSION );
    if (!version)
    {
        OSG_NOTIFY(osg::FATAL)<<"Error: OpenGL version test failed, requires valid graphics context."<<std::endl;
        return;
    }

    glVersion = findAsciiToFloat( version );
    glslLanguageVersion = 0.0f;

    bool shadersBuiltIn = OSG_GLES2_FEATURES || OSG_GL3_FEATURES;

    isShaderObjectsSupported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_shader_objects");
    isVertexShaderSupported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_vertex_shader");
    isFragmentShaderSupported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_fragment_shader");
    isLanguage100Supported = shadersBuiltIn || osg::isGLExtensionSupported(contextID,"GL_ARB_shading_language_100");
    isGeometryShader4Supported = osg::isGLExtensionSupported(contextID,"GL_EXT_geometry_shader4");
    isGpuShader4Supported = osg::isGLExtensionSupported(contextID,"GL_EXT_gpu_shader4");
    areTessellationShadersSupported = osg::isGLExtensionSupported(contextID, "GL_ARB_tessellation_shader");
    isUniformBufferObjectSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_uniform_buffer_object");
    isGetProgramBinarySupported = osg::isGLExtensionSupported(contextID,"GL_ARB_get_program_binary");
    isGpuShaderFp64Supported = osg::isGLExtensionSupported(contextID,"GL_ARB_gpu_shader_fp64");
    isShaderAtomicCountersSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_shader_atomic_counters");

    isRectangleSupported = OSG_GL3_FEATURES ||
                           isGLExtensionSupported(contextID,"GL_ARB_texture_rectangle") ||
                           isGLExtensionSupported(contextID,"GL_EXT_texture_rectangle") ||
                           isGLExtensionSupported(contextID,"GL_NV_texture_rectangle");

    isCubeMapSupported = OSG_GLES2_FEATURES || OSG_GL3_FEATURES ||
                          isGLExtensionSupported(contextID,"GL_ARB_texture_cube_map") ||
                          isGLExtensionSupported(contextID,"GL_EXT_texture_cube_map") ||
                          (glVersion >= 1.3f);

    isClipControlSupported = isGLExtensionSupported(contextID,"GL_ARB_clip_control") ||
                             (glVersion >= 4.5f);


    isGlslSupported = ( glVersion >= 2.0f ) ||
                      ( isShaderObjectsSupported &&
                        isVertexShaderSupported &&
                        isFragmentShaderSupported &&
                        isLanguage100Supported );

    if( isGlslSupported )
    {
        // If glGetString raises an error, assume initial release "1.00"
        while(glGetError() != GL_NO_ERROR) {}        // reset error flag

        const char* langVerStr = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        if( (glGetError() == GL_NO_ERROR) && langVerStr )
        {
            glslLanguageVersion = (findAsciiToFloat(langVerStr));
        }
        else
            glslLanguageVersion = 1.0f;
    }

    OSG_INFO
            << "glVersion=" << glVersion << ", "
            << "isGlslSupported=" << (isGlslSupported ? "YES" : "NO") << ", "
            << "glslLanguageVersion=" << glslLanguageVersion
            << std::endl;


    setGLExtensionFuncPtr(glDrawBuffers, "glDrawBuffers", "glDrawBuffersARB");
    setGLExtensionFuncPtr(glAttachShader, "glAttachShader", "glAttachObjectARB");
    setGLExtensionFuncPtr(glBindAttribLocation, "glBindAttribLocation", "glBindAttribLocationARB");
    setGLExtensionFuncPtr(glCompileShader, "glCompileShader", "glCompileShaderARB");
    setGLExtensionFuncPtr(glCreateProgram, "glCreateProgram", "glCreateProgramObjectARB");
    setGLExtensionFuncPtr(glCreateShader, "glCreateShader", "glCreateShaderObjectARB");
    setGLExtensionFuncPtr(glDeleteProgram, "glDeleteProgram");
    setGLExtensionFuncPtr(glDeleteShader, "glDeleteShader");
    setGLExtensionFuncPtr(glDetachShader, "glDetachShader", "glDetachObjectARB");
    setGLExtensionFuncPtr(glDisableVertexAttribArray, "glDisableVertexAttribArray");
    setGLExtensionFuncPtr(glEnableVertexAttribArray, "glEnableVertexAttribArray");
    setGLExtensionFuncPtr(glGetActiveAttrib, "glGetActiveAttrib", "glGetActiveAttribARB");
    setGLExtensionFuncPtr(glGetActiveUniform, "glGetActiveUniform", "glGetActiveUniformARB");
    setGLExtensionFuncPtr(glGetAttachedShaders, "glGetAttachedShaders", "glGetAttachedObjectsARB");
    setGLExtensionFuncPtr(glGetAttribLocation, "glGetAttribLocation", "glGetAttribLocationARB");
    setGLExtensionFuncPtr(glGetProgramiv, "glGetProgramiv");
    setGLExtensionFuncPtr(glGetProgramInfoLog, "glGetProgramInfoLog");
    setGLExtensionFuncPtr(glGetShaderiv, "glGetShaderiv");
    setGLExtensionFuncPtr(glGetShaderInfoLog, "glGetShaderInfoLog");
    setGLExtensionFuncPtr(glGetShaderSource, "glGetShaderSource", "glGetShaderSourceARB");
    setGLExtensionFuncPtr(glGetUniformLocation, "glGetUniformLocation", "glGetUniformLocationARB");
    setGLExtensionFuncPtr(glGetUniformfv, "glGetUniformfv", "glGetUniformfvARB");
    setGLExtensionFuncPtr(glGetUniformiv, "glGetUniformiv", "glGetUniformivARB");
    setGLExtensionFuncPtr(glGetVertexAttribdv, "glGetVertexAttribdv");
    setGLExtensionFuncPtr(glGetVertexAttribfv, "glGetVertexAttribfv");
    setGLExtensionFuncPtr(glGetVertexAttribiv, "glGetVertexAttribiv");
    setGLExtensionFuncPtr(glGetVertexAttribPointerv, "glGetVertexAttribPointerv");
    setGLExtensionFuncPtr(glIsProgram, "glIsProgram");
    setGLExtensionFuncPtr(glIsShader, "glIsShader");
    setGLExtensionFuncPtr(glLinkProgram, "glLinkProgram", "glLinkProgramARB");
    setGLExtensionFuncPtr(glShaderSource, "glShaderSource", "glShaderSourceARB");
    setGLExtensionFuncPtr(glUseProgram, "glUseProgram", "glUseProgramObjectARB");
    setGLExtensionFuncPtr(glUniform1f, "glUniform1f", "glUniform1fARB");
    setGLExtensionFuncPtr(glUniform2f, "glUniform2f", "glUniform2fARB");
    setGLExtensionFuncPtr(glUniform3f, "glUniform3f", "glUniform3fARB");
    setGLExtensionFuncPtr(glUniform4f, "glUniform4f", "glUniform4fARB");
    setGLExtensionFuncPtr(glUniform1i, "glUniform1i", "glUniform1iARB");
    setGLExtensionFuncPtr(glUniform2i, "glUniform2i", "glUniform2iARB");
    setGLExtensionFuncPtr(glUniform3i, "glUniform3i", "glUniform3iARB");
    setGLExtensionFuncPtr(glUniform4i, "glUniform4i", "glUniform4iARB");
    setGLExtensionFuncPtr(glUniform1fv, "glUniform1fv", "glUniform1fvARB");
    setGLExtensionFuncPtr(glUniform2fv, "glUniform2fv", "glUniform2fvARB");
    setGLExtensionFuncPtr(glUniform3fv, "glUniform3fv", "glUniform3fvARB");
    setGLExtensionFuncPtr(glUniform4fv, "glUniform4fv", "glUniform4fvARB");
    setGLExtensionFuncPtr(glUniform1iv, "glUniform1iv", "glUniform1ivARB");
    setGLExtensionFuncPtr(glUniform2iv, "glUniform2iv", "glUniform2ivARB");
    setGLExtensionFuncPtr(glUniform3iv, "glUniform3iv", "glUniform3ivARB");
    setGLExtensionFuncPtr(glUniform4iv, "glUniform4iv", "glUniform4ivARB");
    setGLExtensionFuncPtr(glUniformMatrix2fv, "glUniformMatrix2fv", "glUniformMatrix2fvARB");
    setGLExtensionFuncPtr(glUniformMatrix3fv, "glUniformMatrix3fv", "glUniformMatrix3fvARB");
    setGLExtensionFuncPtr(glUniformMatrix4fv, "glUniformMatrix4fv", "glUniformMatrix4fvARB");
    setGLExtensionFuncPtr(glValidateProgram, "glValidateProgram", "glValidateProgramARB");
    setGLExtensionFuncPtr(glVertexAttrib1d, "glVertexAttrib1d");
    setGLExtensionFuncPtr(glVertexAttrib1dv, "glVertexAttrib1dv");
    setGLExtensionFuncPtr(glVertexAttrib1f, "glVertexAttrib1f");
    setGLExtensionFuncPtr(glVertexAttrib1fv, "glVertexAttrib1fv");
    setGLExtensionFuncPtr(glVertexAttrib1s, "glVertexAttrib1s");
    setGLExtensionFuncPtr(glVertexAttrib1sv, "glVertexAttrib1sv");
    setGLExtensionFuncPtr(glVertexAttrib2d, "glVertexAttrib2d");
    setGLExtensionFuncPtr(glVertexAttrib2dv, "glVertexAttrib2dv");
    setGLExtensionFuncPtr(glVertexAttrib2f, "glVertexAttrib2f");
    setGLExtensionFuncPtr(glVertexAttrib2fv, "glVertexAttrib2fv");
    setGLExtensionFuncPtr(glVertexAttrib2s, "glVertexAttrib2s");
    setGLExtensionFuncPtr(glVertexAttrib2sv, "glVertexAttrib2sv");
    setGLExtensionFuncPtr(glVertexAttrib3d, "glVertexAttrib3d");
    setGLExtensionFuncPtr(glVertexAttrib3dv, "glVertexAttrib3dv");
    setGLExtensionFuncPtr(glVertexAttrib3f, "glVertexAttrib3f");
    setGLExtensionFuncPtr(glVertexAttrib3fv, "glVertexAttrib3fv");
    setGLExtensionFuncPtr(glVertexAttrib3s, "glVertexAttrib3s");
    setGLExtensionFuncPtr(glVertexAttrib3sv, "glVertexAttrib3sv");
    setGLExtensionFuncPtr(glVertexAttrib4Nbv, "glVertexAttrib4Nbv");
    setGLExtensionFuncPtr(glVertexAttrib4Niv, "glVertexAttrib4Niv");
    setGLExtensionFuncPtr(glVertexAttrib4Nsv, "glVertexAttrib4Nsv");
    setGLExtensionFuncPtr(glVertexAttrib4Nub, "glVertexAttrib4Nub");
    setGLExtensionFuncPtr(glVertexAttrib4Nubv, "glVertexAttrib4Nubv");
    setGLExtensionFuncPtr(glVertexAttrib4Nuiv, "glVertexAttrib4Nuiv");
    setGLExtensionFuncPtr(glVertexAttrib4Nusv, "glVertexAttrib4Nusv");
    setGLExtensionFuncPtr(glVertexAttrib4bv, "glVertexAttrib4bv");
    setGLExtensionFuncPtr(glVertexAttrib4d, "glVertexAttrib4d");
    setGLExtensionFuncPtr(glVertexAttrib4dv, "glVertexAttrib4dv");
    setGLExtensionFuncPtr(glVertexAttrib4f, "glVertexAttrib4f");
    setGLExtensionFuncPtr(glVertexAttrib4fv, "glVertexAttrib4fv");
    setGLExtensionFuncPtr(glVertexAttrib4iv, "glVertexAttrib4iv");
    setGLExtensionFuncPtr(glVertexAttrib4s, "glVertexAttrib4s");
    setGLExtensionFuncPtr(glVertexAttrib4sv, "glVertexAttrib4sv");
    setGLExtensionFuncPtr(glVertexAttrib4ubv, "glVertexAttrib4ubv");
    setGLExtensionFuncPtr(glVertexAttrib4uiv, "glVertexAttrib4uiv");
    setGLExtensionFuncPtr(glVertexAttrib4usv, "glVertexAttrib4usv");
    setGLExtensionFuncPtr(glVertexAttribPointer, "glVertexAttribPointer");
    setGLExtensionFuncPtr(glVertexAttribDivisor, "glVertexAttribDivisor");

    // v1.5-only ARB entry points, in case they're needed for fallback
    setGLExtensionFuncPtr(glGetInfoLogARB, "glGetInfoLogARB");
    setGLExtensionFuncPtr(glGetObjectParameterivARB, "glGetObjectParameterivARB");
    setGLExtensionFuncPtr(glDeleteObjectARB, "glDeleteObjectARB");
    setGLExtensionFuncPtr(glGetHandleARB, "glGetHandleARB");

    // GL 2.1
    setGLExtensionFuncPtr(glUniformMatrix2x3fv,  "glUniformMatrix2x3fv" );
    setGLExtensionFuncPtr(glUniformMatrix3x2fv,  "glUniformMatrix3x2fv" );
    setGLExtensionFuncPtr(glUniformMatrix2x4fv,  "glUniformMatrix2x4fv" );
    setGLExtensionFuncPtr(glUniformMatrix4x2fv,  "glUniformMatrix4x2fv" );
    setGLExtensionFuncPtr(glUniformMatrix3x4fv,  "glUniformMatrix3x4fv" );
    setGLExtensionFuncPtr(glUniformMatrix4x3fv,  "glUniformMatrix4x3fv" );

    // ARB_clip_control
    setGLExtensionFuncPtr(glClipControl, "glClipControl");

    // EXT_geometry_shader4
    setGLExtensionFuncPtr(glProgramParameteri,  "glProgramParameteri", "glProgramParameteriEXT" );

    // ARB_tesselation_shader
    setGLExtensionFuncPtr(glPatchParameteri, "glPatchParameteri" );
    setGLExtensionFuncPtr(glPatchParameterfv, "glPatchParameterfv");

    // EXT_gpu_shader4
    setGLExtensionFuncPtr(glGetUniformuiv,  "glGetUniformuiv", "glGetUniformuivEXT" );
    setGLExtensionFuncPtr(glBindFragDataLocation,  "glBindFragDataLocation", "glBindFragDataLocationEXT" );
    setGLExtensionFuncPtr(glGetFragDataLocation,  "glGetFragDataLocation", "glGetFragDataLocationEXT" );
    setGLExtensionFuncPtr(glUniform1ui,  "glUniform1ui", "glUniform1uiEXT" );
    setGLExtensionFuncPtr(glUniform2ui,  "glUniform2ui", "glUniform2uiEXT" );
    setGLExtensionFuncPtr(glUniform3ui,  "glUniform3ui", "glUniform3uiEXT" );
    setGLExtensionFuncPtr(glUniform4ui,  "glUniform4ui", "glUniform4uiEXT" );
    setGLExtensionFuncPtr(glUniform1uiv,  "glUniform1uiv", "glUniform1uivEXT" );
    setGLExtensionFuncPtr(glUniform2uiv,  "glUniform2uiv", "glUniform2uivEXT" );
    setGLExtensionFuncPtr(glUniform3uiv,  "glUniform3uiv", "glUniform3uivEXT" );
    setGLExtensionFuncPtr(glUniform4uiv,  "glUniform4uiv", "glUniform4uivEXT" );
    // ARB_uniform_buffer_object
    setGLExtensionFuncPtr(glGetUniformIndices, "glGetUniformIndices");
    setGLExtensionFuncPtr(glGetActiveUniformsiv, "glGetActiveUniformsiv");
    setGLExtensionFuncPtr(glGetActiveUniformName, "glGetActiveUniformName");
    setGLExtensionFuncPtr(glGetUniformBlockIndex, "glGetUniformBlockIndex");
    setGLExtensionFuncPtr(glGetActiveUniformBlockiv, "glGetActiveUniformBlockiv");
    setGLExtensionFuncPtr(glGetActiveUniformBlockName, "glGetActiveUniformBlockName");
    setGLExtensionFuncPtr(glUniformBlockBinding, "glUniformBlockBinding");

    // ARB_get_program_binary
    setGLExtensionFuncPtr(glGetProgramBinary, "glGetProgramBinary");
    setGLExtensionFuncPtr(glProgramBinary, "glProgramBinary");

    // ARB_gpu_shader_fp64
    setGLExtensionFuncPtr(glUniform1d, "glUniform1d" );
    setGLExtensionFuncPtr(glUniform2d, "glUniform2d" );
    setGLExtensionFuncPtr(glUniform3d, "glUniform3d" );
    setGLExtensionFuncPtr(glUniform4d, "glUniform4d" );
    setGLExtensionFuncPtr(glUniform1dv, "glUniform1dv" );
    setGLExtensionFuncPtr(glUniform2dv, "glUniform2dv" );
    setGLExtensionFuncPtr(glUniform3dv, "glUniform3dv" );
    setGLExtensionFuncPtr(glUniform4dv, "glUniform4dv" );
    setGLExtensionFuncPtr(glUniformMatrix2dv, "glUniformMatrix2dv" );
    setGLExtensionFuncPtr(glUniformMatrix3dv, "glUniformMatrix3dv" );
    setGLExtensionFuncPtr(glUniformMatrix4dv, "glUniformMatrix4dv" );
    setGLExtensionFuncPtr(glUniformMatrix2x3dv,  "glUniformMatrix2x3dv" );
    setGLExtensionFuncPtr(glUniformMatrix3x2dv,  "glUniformMatrix3x2dv" );
    setGLExtensionFuncPtr(glUniformMatrix2x4dv,  "glUniformMatrix2x4dv" );
    setGLExtensionFuncPtr(glUniformMatrix4x2dv,  "glUniformMatrix4x2dv" );
    setGLExtensionFuncPtr(glUniformMatrix3x4dv,  "glUniformMatrix3x4dv" );
    setGLExtensionFuncPtr(glUniformMatrix4x3dv,  "glUniformMatrix4x3dv" );

    // ARB_shader_atomic_counters
    setGLExtensionFuncPtr(glGetActiveAtomicCounterBufferiv,  "glGetActiveAtomicCounterBufferiv" );

    // ARB_compute_shader
    setGLExtensionFuncPtr(glDispatchCompute,  "glDispatchCompute" );

    setGLExtensionFuncPtr(glMemoryBarrier,  "glMemoryBarrier", "glMemoryBarrierEXT" );

    // BufferObject extensions
    setGLExtensionFuncPtr(glGenBuffers, "glGenBuffers","glGenBuffersARB");
    setGLExtensionFuncPtr(glBindBuffer, "glBindBuffer","glBindBufferARB");
    setGLExtensionFuncPtr(glBufferData, "glBufferData","glBufferDataARB");
    setGLExtensionFuncPtr(glBufferSubData, "glBufferSubData","glBufferSubDataARB");
    setGLExtensionFuncPtr(glDeleteBuffers, "glDeleteBuffers","glDeleteBuffersARB");
    setGLExtensionFuncPtr(glIsBuffer, "glIsBuffer","glIsBufferARB");
    setGLExtensionFuncPtr(glGetBufferSubData, "glGetBufferSubData","glGetBufferSubDataARB");
    setGLExtensionFuncPtr(glMapBuffer, "glMapBuffer","glMapBufferARB");
    setGLExtensionFuncPtr(glMapBufferRange,  "glMapBufferRange" );
    setGLExtensionFuncPtr(glUnmapBuffer, "glUnmapBuffer","glUnmapBufferARB");
    setGLExtensionFuncPtr(glGetBufferParameteriv, "glGetBufferParameteriv","glGetBufferParameterivARB");
    setGLExtensionFuncPtr(glGetBufferPointerv, "glGetBufferPointerv","glGetBufferPointervARB");
    setGLExtensionFuncPtr(glBindBufferRange, "glBindBufferRange");
    setGLExtensionFuncPtr(glBindBufferBase,  "glBindBufferBase", "glBindBufferBaseEXT", "glBindBufferBaseNV" );
    setGLExtensionFuncPtr(glTexBuffer, "glTexBuffer","glTexBufferARB" );

    isPBOSupported = OSG_GL3_FEATURES || osg::isGLExtensionSupported(contextID,"GL_ARB_pixel_buffer_object");
    isUniformBufferObjectSupported = osg::isGLExtensionSupported(contextID, "GL_ARB_uniform_buffer_object");
    isTBOSupported = osg::isGLExtensionSupported(contextID,"GL_ARB_texture_buffer_object");
    isVAOSupported = osg::isGLExtensionSupported(contextID, "GL_ARB_vertex_array_object");
    isTransformFeedbackSupported = osg::isGLExtensionSupported(contextID, "GL_ARB_transform_feedback2");

    // BlendFunc extensions
    isBlendFuncSeparateSupported = OSG_GLES2_FEATURES || OSG_GL3_FEATURES ||
                                    osg::isGLExtensionSupported(contextID, "GL_EXT_blend_func_separate") ||
                                    (glVersion >= 1.4f);

    setGLExtensionFuncPtr(glBlendFuncSeparate, "glBlendFuncSeparate", "glBlendFuncSeparateEXT");

    setGLExtensionFuncPtr(glBlendFunci, "glBlendFunci", "glBlendFunciARB");
    setGLExtensionFuncPtr(glBlendFuncSeparatei, "glBlendFuncSeparatei", "glBlendFuncSeparateiARB");


    // Vertex Array extensions
    isSecondaryColorSupported = isGLExtensionSupported(contextID,"GL_EXT_secondary_color");
    isFogCoordSupported = isGLExtensionSupported(contextID,"GL_EXT_fog_coord");
    isMultiTexSupported = isGLExtensionSupported(contextID,"GL_ARB_multitexture");
    isOcclusionQuerySupported = osg::isGLExtensionSupported(contextID, "GL_NV_occlusion_query" );
    isARBOcclusionQuerySupported = OSG_GL3_FEATURES || osg::isGLExtensionSupported(contextID, "GL_ARB_occlusion_query" );

    isTimerQuerySupported = osg::isGLExtensionSupported(contextID, "GL_EXT_timer_query" );
    isARBTimerQuerySupported = osg::isGLExtensionSupported(contextID, "GL_ARB_timer_query");

    setGLExtensionFuncPtr(glFogCoordfv, "glFogCoordfv","glFogCoordfvEXT");
    setGLExtensionFuncPtr(glSecondaryColor3ubv, "glSecondaryColor3ubv","glSecondaryColor3ubvEXT");
    setGLExtensionFuncPtr(glSecondaryColor3fv, "glSecondaryColor3fv","glSecondaryColor3fvEXT");
    setGLExtensionFuncPtr(glMultiTexCoord1f, "glMultiTexCoord1f","glMultiTexCoord1fARB");
    setGLExtensionFuncPtr(glMultiTexCoord1fv, "glMultiTexCoord1fv","glMultiTexCoord1fvARB");
    setGLExtensionFuncPtr(glMultiTexCoord2fv, "glMultiTexCoord2fv","glMultiTexCoord2fvARB");
    setGLExtensionFuncPtr(glMultiTexCoord3fv, "glMultiTexCoord3fv","glMultiTexCoord3fvARB");
    setGLExtensionFuncPtr(glMultiTexCoord4fv, "glMultiTexCoord4fv","glMultiTexCoord4fvARB");
    setGLExtensionFuncPtr(glMultiTexCoord1d, "glMultiTexCoord1d","glMultiTexCoorddfARB");
    setGLExtensionFuncPtr(glMultiTexCoord2dv, "glMultiTexCoord2dv","glMultiTexCoord2dvARB");
    setGLExtensionFuncPtr(glMultiTexCoord3dv, "glMultiTexCoord3dv","glMultiTexCoord3dvARB");
    setGLExtensionFuncPtr(glMultiTexCoord4dv, "glMultiTexCoord4dv","glMultiTexCoord4dvARB");

    setGLExtensionFuncPtr(glVertexAttrib1s, "glVertexAttrib1s","glVertexAttrib1sARB");
    setGLExtensionFuncPtr(glVertexAttrib1f, "glVertexAttrib1f","glVertexAttrib1fARB");
    setGLExtensionFuncPtr(glVertexAttrib1d, "glVertexAttrib1d","glVertexAttrib1dARB");
    setGLExtensionFuncPtr(glVertexAttrib1fv, "glVertexAttrib1fv","glVertexAttrib1fvARB");
    setGLExtensionFuncPtr(glVertexAttrib2fv, "glVertexAttrib2fv","glVertexAttrib2fvARB");
    setGLExtensionFuncPtr(glVertexAttrib3fv, "glVertexAttrib3fv","glVertexAttrib3fvARB");
    setGLExtensionFuncPtr(glVertexAttrib4fv, "glVertexAttrib4fv","glVertexAttrib4fvARB");
    setGLExtensionFuncPtr(glVertexAttrib2dv, "glVertexAttrib2dv","glVertexAttrib2dvARB");
    setGLExtensionFuncPtr(glVertexAttrib3dv, "glVertexAttrib3dv","glVertexAttrib3dvARB");
    setGLExtensionFuncPtr(glVertexAttrib4dv, "glVertexAttrib4dv","glVertexAttrib4dvARB");
    setGLExtensionFuncPtr(glVertexAttrib4ubv, "glVertexAttrib4ubv","glVertexAttrib4ubvARB");
    setGLExtensionFuncPtr(glVertexAttrib4Nubv, "glVertexAttrib4Nubv","glVertexAttrib4NubvARB");

    setGLExtensionFuncPtr(glGenBuffers, "glGenBuffers","glGenBuffersARB");
    setGLExtensionFuncPtr(glBindBuffer, "glBindBuffer","glBindBufferARB");
    setGLExtensionFuncPtr(glBufferData, "glBufferData","glBufferDataARB");
    setGLExtensionFuncPtr(glBufferSubData, "glBufferSubData","glBufferSubDataARB");
    setGLExtensionFuncPtr(glDeleteBuffers, "glDeleteBuffers","glDeleteBuffersARB");
    setGLExtensionFuncPtr(glIsBuffer, "glIsBuffer","glIsBufferARB");
    setGLExtensionFuncPtr(glGetBufferSubData, "glGetBufferSubData","glGetBufferSubDataARB");
    setGLExtensionFuncPtr(glMapBuffer, "glMapBuffer","glMapBufferARB");
    setGLExtensionFuncPtr(glUnmapBuffer, "glUnmapBuffer","glUnmapBufferARB");
    setGLExtensionFuncPtr(glGetBufferParameteriv, "glGetBufferParameteriv","glGetBufferParameterivARB");
    setGLExtensionFuncPtr(glGetBufferPointerv, "glGetBufferPointerv","glGetBufferPointervARB");

    setGLExtensionFuncPtr(glGenOcclusionQueries, "glGenOcclusionQueries","glGenOcclusionQueriesNV");
    setGLExtensionFuncPtr(glDeleteOcclusionQueries, "glDeleteOcclusionQueries","glDeleteOcclusionQueriesNV");
    setGLExtensionFuncPtr(glIsOcclusionQuery, "glIsOcclusionQuery","_glIsOcclusionQueryNV");
    setGLExtensionFuncPtr(glBeginOcclusionQuery, "glBeginOcclusionQuery","glBeginOcclusionQueryNV");
    setGLExtensionFuncPtr(glEndOcclusionQuery, "glEndOcclusionQuery","glEndOcclusionQueryNV");
    setGLExtensionFuncPtr(glGetOcclusionQueryiv, "glGetOcclusionQueryiv","glGetOcclusionQueryivNV");
    setGLExtensionFuncPtr(glGetOcclusionQueryuiv, "glGetOcclusionQueryuiv","glGetOcclusionQueryuivNV");

    setGLExtensionFuncPtr(glGenQueries, "glGenQueries", "glGenQueriesARB");
    setGLExtensionFuncPtr(glDeleteQueries, "glDeleteQueries", "glDeleteQueriesARB");
    setGLExtensionFuncPtr(glIsQuery, "glIsQuery", "glIsQueryARB");
    setGLExtensionFuncPtr(glBeginQuery, "glBeginQuery", "glBeginQueryARB");
    setGLExtensionFuncPtr(glEndQuery, "glEndQuery", "glEndQueryARB");
    setGLExtensionFuncPtr(glGetQueryiv, "glGetQueryiv", "glGetQueryivARB");
    setGLExtensionFuncPtr(glGetQueryObjectiv, "glGetQueryObjectiv","glGetQueryObjectivARB");
    setGLExtensionFuncPtr(glGetQueryObjectuiv, "glGetQueryObjectuiv","glGetQueryObjectuivARB");
    setGLExtensionFuncPtr(glGetQueryObjectui64v, "glGetQueryObjectui64v","glGetQueryObjectui64vEXT");
    setGLExtensionFuncPtr(glQueryCounter, "glQueryCounter");
    setGLExtensionFuncPtr(glGetInteger64v, "glGetInteger64v");


    // SampleMaski functionality
    isTextureMultisampleSupported = isGLExtensionSupported(contextID, "GL_ARB_texture_multisample");
    isOpenGL32upported = (glVersion >= 3.2f);

    // function pointers
    setGLExtensionFuncPtr(glSampleMaski, "glSampleMaski");
    // protect against buggy drivers (maybe not necessary)
    isSampleMaskiSupported = glSampleMaski!=0;



    // old styple Vertex/Fragment Programs
    isVertexProgramSupported = isGLExtensionSupported(contextID,"GL_ARB_vertex_program");
    isFragmentProgramSupported = isGLExtensionSupported(contextID,"GL_ARB_fragment_program");

    setGLExtensionFuncPtr(glBindProgram,"glBindProgramARB");
    setGLExtensionFuncPtr(glGenPrograms, "glGenProgramsARB");
    setGLExtensionFuncPtr(glDeletePrograms, "glDeleteProgramsARB");
    setGLExtensionFuncPtr(glProgramString, "glProgramStringARB");
    setGLExtensionFuncPtr(glProgramLocalParameter4fv, "glProgramLocalParameter4fvARB");



    // Texture extensions
    const char* renderer = (const char*) glGetString(GL_RENDERER);
    std::string rendererString(renderer ? renderer : "");

    bool radeonHardwareDetected = (rendererString.find("Radeon")!=std::string::npos || rendererString.find("RADEON")!=std::string::npos);
    bool fireGLHardwareDetected = (rendererString.find("FireGL")!=std::string::npos || rendererString.find("FIREGL")!=std::string::npos);

    bool builtInSupport = OSG_GLES2_FEATURES || OSG_GL3_FEATURES;

    isMultiTexturingSupported = builtInSupport || OSG_GLES1_FEATURES ||
                                 isGLExtensionOrVersionSupported( contextID,"GL_ARB_multitexture", 1.3f) ||
                                 isGLExtensionOrVersionSupported(contextID,"GL_EXT_multitexture", 1.3f);

    isTextureFilterAnisotropicSupported = isGLExtensionSupported(contextID,"GL_EXT_texture_filter_anisotropic");
    isTextureSwizzleSupported = isGLExtensionSupported(contextID,"GL_ARB_texture_swizzle");
    isTextureCompressionARBSupported = builtInSupport || isGLExtensionOrVersionSupported(contextID,"GL_ARB_texture_compression", 1.3f);
    isTextureCompressionS3TCSupported = isGLExtensionSupported(contextID,"GL_EXT_texture_compression_s3tc") || isGLExtensionSupported(contextID, "GL_S3_s3tc");
    isTextureCompressionPVRTC2BPPSupported = isGLExtensionSupported(contextID,"GL_IMG_texture_compression_pvrtc");
    isTextureCompressionPVRTC4BPPSupported = isTextureCompressionPVRTC2BPPSupported;//covered by same extension
    isTextureCompressionETCSupported = isGLExtensionSupported(contextID,"GL_OES_compressed_ETC1_RGB8_texture");
    isTextureCompressionETC2Supported = isGLExtensionSupported(contextID,"GL_ARB_ES3_compatibility");
    isTextureCompressionRGTCSupported = isGLExtensionSupported(contextID,"GL_EXT_texture_compression_rgtc");
    isTextureCompressionPVRTCSupported = isGLExtensionSupported(contextID,"GL_IMG_texture_compression_pvrtc");

    isTextureMirroredRepeatSupported = builtInSupport ||
                                       isGLExtensionOrVersionSupported(contextID,"GL_IBM_texture_mirrored_repeat", 1.4f) ||
                                       isGLExtensionOrVersionSupported(contextID,"GL_ARB_texture_mirrored_repeat", 1.4f);

    isTextureEdgeClampSupported = builtInSupport ||
                                   isGLExtensionOrVersionSupported(contextID,"GL_EXT_texture_edge_clamp", 1.2f) ||
                                   isGLExtensionOrVersionSupported(contextID,"GL_SGIS_texture_edge_clamp", 1.2f);


    isTextureBorderClampSupported = OSG_GL3_FEATURES || ((OSG_GL1_FEATURES || OSG_GL2_FEATURES) && isGLExtensionOrVersionSupported(contextID,"GL_ARB_texture_border_clamp", 1.3f));
    isGenerateMipMapSupported = builtInSupport || isGLExtensionOrVersionSupported(contextID,"GL_SGIS_generate_mipmap", 1.4f);
    preferGenerateMipmapSGISForPowerOfTwo = (radeonHardwareDetected||fireGLHardwareDetected) ? false : true;
    isTextureMultisampledSupported = isGLExtensionSupported(contextID,"GL_ARB_texture_multisample");
    isShadowSupported = OSG_GL3_FEATURES || isGLExtensionSupported(contextID,"GL_ARB_shadow");
    isShadowAmbientSupported = isGLExtensionSupported(contextID,"GL_ARB_shadow_ambient");
    isClientStorageSupported = isGLExtensionSupported(contextID,"GL_APPLE_client_storage");
    isNonPowerOfTwoTextureNonMipMappedSupported = builtInSupport || isGLExtensionOrVersionSupported(contextID,"GL_ARB_texture_non_power_of_two", 2.0) || isGLExtensionSupported(contextID,"GL_APPLE_texture_2D_limited_npot");
    isNonPowerOfTwoTextureMipMappedSupported = builtInSupport || isNonPowerOfTwoTextureNonMipMappedSupported;
    isTextureIntegerEXTSupported = OSG_GL3_FEATURES || isGLExtensionSupported(contextID, "GL_EXT_texture_integer");

    if (rendererString.find("GeForce FX")!=std::string::npos)
    {
        isNonPowerOfTwoTextureMipMappedSupported = false;
        OSG_INFO<<"Disabling _isNonPowerOfTwoTextureMipMappedSupported for GeForce FX hardware."<<std::endl;
    }

    maxTextureSize=0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTextureSize);

    char *ptr;
    if( (ptr = getenv("OSG_MAX_TEXTURE_SIZE")) != 0)
    {
        GLint osg_max_size = atoi(ptr);

        if (osg_max_size<maxTextureSize)
        {

            maxTextureSize = osg_max_size;
        }
    }

    setGLExtensionFuncPtr(glTexStorage2D,"glTexStorage2D","glTexStorage2DARB");
    setGLExtensionFuncPtr(glCompressedTexImage2D,"glCompressedTexImage2D","glCompressedTexImage2DARB");
    setGLExtensionFuncPtr(glCompressedTexSubImage2D,"glCompressedTexSubImage2D","glCompressedTexSubImage2DARB");
    setGLExtensionFuncPtr(glGetCompressedTexImage,"glGetCompressedTexImage","glGetCompressedTexImageARB");;
    setGLExtensionFuncPtr(glTexImage2DMultisample, "glTexImage2DMultisample", "glTexImage2DMultisampleARB");

    setGLExtensionFuncPtr(glTexParameterIiv, "glTexParameterIiv", "glTexParameterIivARB");
    setGLExtensionFuncPtr(glTexParameterIuiv, "glTexParameterIuiv", "glTexParameterIuivARB");


    if (glTexParameterIiv == NULL) setGLExtensionFuncPtr(glTexParameterIiv, "glTexParameterIivEXT");
    if (glTexParameterIuiv == NULL) setGLExtensionFuncPtr(glTexParameterIuiv, "glTexParameterIuivEXT");

    setGLExtensionFuncPtr(glBindImageTexture, "glBindImageTexture", "glBindImageTextureARB");

    isTextureMaxLevelSupported = (glVersion >= 1.2f);

    isTextureStorageEnabled = isTexStorage2DSupported();
    if ( (ptr = getenv("OSG_GL_TEXTURE_STORAGE"))  != 0 && isTexStorage2DSupported())
    {
        if (strcmp(ptr,"OFF")==0 || strcmp(ptr,"DISABLE")==0 ) isTextureStorageEnabled = false;
        else isTextureStorageEnabled = true;
    }


    // Texture3D extensions
    isTexture3DFast = OSG_GL3_FEATURES || isGLExtensionSupported(contextID,"GL_EXT_texture3D");

    if (isTexture3DFast) isTexture3DSupported = true;
    else isTexture3DSupported = (glVersion >= 1.2f);

    maxTexture3DSize = 0;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxTexture3DSize);

    setGLExtensionFuncPtr(glTexImage3D, "glTexImage3D","glTexImage3DEXT");
    setGLExtensionFuncPtr(glTexSubImage3D, "glTexSubImage3D","glTexSubImage3DEXT");
    setGLExtensionFuncPtr(glCompressedTexImage3D, "glCompressedTexImage3D","glCompressedTexImage3DARB");
    setGLExtensionFuncPtr(glCompressedTexSubImage3D, "glCompressedTexSubImage3D","glCompressedTexSubImage3DARB");
    setGLExtensionFuncPtr(glCopyTexSubImage3D, "glCopyTexSubImage3D","glCopyTexSubImage3DEXT");


    // Texture2DArray extensions
    isTexture2DArraySupported = OSG_GL3_FEATURES || isGLExtensionSupported(contextID,"GL_EXT_texture_array");

    max2DSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max2DSize);
    maxLayerCount = 0;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS_EXT, &maxLayerCount);

    // Blending
    isBlendColorSupported = OSG_GLES2_FEATURES || OSG_GL3_FEATURES ||
                            isGLExtensionSupported(contextID,"GL_EXT_blend_color") ||
                            (glVersion >= 1.2f);

    setGLExtensionFuncPtr(glBlendColor, "glBlendColor", "glBlendColorEXT");

    bool bultInSupport = OSG_GLES2_FEATURES || OSG_GL3_FEATURES;
    isBlendEquationSupported = bultInSupport ||
        isGLExtensionSupported(contextID, "GL_EXT_blend_equation") ||
        (glVersion >= 1.2f);


    isBlendEquationSeparateSupported = bultInSupport ||
        isGLExtensionSupported(contextID, "GL_EXT_blend_equation_separate") ||
        (glVersion >= 2.0f);


    isSGIXMinMaxSupported = isGLExtensionSupported(contextID, "GL_SGIX_blend_alpha_minmax");
    isLogicOpSupported = isGLExtensionSupported(contextID, "GL_EXT_blend_logic_op");

    setGLExtensionFuncPtr(glBlendEquation, "glBlendEquation", "glBlendEquationEXT");
    setGLExtensionFuncPtr(glBlendEquationSeparate, "glBlendEquationSeparate", "glBlendEquationSeparateEXT");

    setGLExtensionFuncPtr(glBlendEquationi, "glBlendEquationi", "glBlendEquationiARB");
    setGLExtensionFuncPtr(glBlendEquationSeparatei, "glBlendEquationSeparatei", "glBlendEquationSeparateiARB");


    // glEnablei/glDisabli
    setGLExtensionFuncPtr(glEnablei, "glEnablei");
    setGLExtensionFuncPtr(glDisablei, "glDisablei");


    // Stencil`
    isStencilWrapSupported = isGLExtensionOrVersionSupported(contextID, "GL_EXT_stencil_wrap", 1.4f);
    isStencilTwoSidedSupported = isGLExtensionSupported(contextID, "GL_EXT_stencil_two_side");
    isOpenGL20Supported = (glVersion >= 2.0f);
    isSeparateStencilSupported = isGLExtensionSupported(contextID, "GL_ATI_separate_stencil");

    // function pointers
    setGLExtensionFuncPtr(glActiveStencilFace, "glActiveStencilFaceEXT");
    setGLExtensionFuncPtr(glStencilOpSeparate, "glStencilOpSeparate", "glStencilOpSeparateATI");
    setGLExtensionFuncPtr(glStencilMaskSeparate, "glStencilMaskSeparate");
    setGLExtensionFuncPtr(glStencilFuncSeparate, "glStencilFuncSeparate", "glStencilFuncSeparateATI");
    setGLExtensionFuncPtr(glStencilFuncSeparateATI, "glStencilFuncSeparateATI");


    // Color Mask
    setGLExtensionFuncPtr(glColorMaski, "glColorMaski", "glColorMaskiARB");


    // ClampColor
    isClampColorSupported = OSG_GL3_FEATURES ||
                             isGLExtensionSupported(contextID,"GL_ARB_color_buffer_float") ||
                             (glVersion >= 2.0f);

    setGLExtensionFuncPtr(glClampColor, "glClampColor", "glClampColorARB");


    // PrimitiveRestartIndex
    setGLExtensionFuncPtr(glPrimitiveRestartIndex, "glPrimitiveRestartIndex", "glPrimitiveRestartIndexNV");


    // Point
    isPointParametersSupported = OSG_GL3_FEATURES || (glVersion >= 1.4f)  ||
                                  isGLExtensionSupported(contextID,"GL_ARB_point_parameters") ||
                                  isGLExtensionSupported(contextID,"GL_EXT_point_parameters") ||
                                  isGLExtensionSupported(contextID,"GL_SGIS_point_parameters");


    isPointSpriteSupported = OSG_GL3_FEATURES || isGLExtensionSupported(contextID, "GL_ARB_point_sprite") || isGLExtensionSupported(contextID, "GL_OES_point_sprite") || isGLExtensionSupported(contextID, "GL_NV_point_sprite");
    isPointSpriteCoordOriginSupported = OSG_GL3_FEATURES || (glVersion >= 2.0f);


    setGLExtensionFuncPtr(glPointParameteri, "glPointParameteri", "glPointParameteriARB");
    if (!glPointParameteri) setGLExtensionFuncPtr(glPointParameteri, "glPointParameteriEXT", "glPointParameteriSGIS");

    setGLExtensionFuncPtr(glPointParameterf, "glPointParameterf", "glPointParameterfARB");
    if (!glPointParameterf) setGLExtensionFuncPtr(glPointParameterf, "glPointParameterfEXT", "glPointParameterfSGIS");

    setGLExtensionFuncPtr(glPointParameterfv, "glPointParameterfv", "glPointParameterfvARB");
    if (!glPointParameterfv) setGLExtensionFuncPtr(glPointParameterfv, "glPointParameterfvEXT", "glPointParameterfvSGIS");


    // Multisample
    isMultisampleSupported = OSG_GLES2_FEATURES || OSG_GL3_FEATURES || isGLExtensionSupported(contextID,"GL_ARB_multisample");
    isMultisampleFilterHintSupported = isGLExtensionSupported(contextID, "GL_NV_multisample_filter_hint");

    setGLExtensionFuncPtr(glSampleCoverage, "glSampleCoverageARB");


    // FrameBufferObject
    setGLExtensionFuncPtr(glBindRenderbuffer, "glBindRenderbuffer", "glBindRenderbufferEXT", "glBindRenderbufferOES");
    setGLExtensionFuncPtr(glDeleteRenderbuffers, "glDeleteRenderbuffers", "glDeleteRenderbuffersEXT", "glDeleteRenderbuffersOES");
    setGLExtensionFuncPtr(glGenRenderbuffers, "glGenRenderbuffers", "glGenRenderbuffersEXT", "glGenRenderbuffersOES");
    setGLExtensionFuncPtr(glRenderbufferStorage, "glRenderbufferStorage", "glRenderbufferStorageEXT", "glRenderbufferStorageOES");
    setGLExtensionFuncPtr(glRenderbufferStorageMultisample, "glRenderbufferStorageMultisample", "glRenderbufferStorageMultisampleEXT", "glRenderbufferStorageMultisampleOES");
    setGLExtensionFuncPtr(glRenderbufferStorageMultisampleCoverageNV, "glRenderbufferStorageMultisampleCoverageNV");
    setGLExtensionFuncPtr(glBindFramebuffer, "glBindFramebuffer", "glBindFramebufferEXT", "glBindFramebufferOES");
    setGLExtensionFuncPtr(glDeleteFramebuffers, "glDeleteFramebuffers", "glDeleteFramebuffersEXT", "glDeleteFramebuffersOES");
    setGLExtensionFuncPtr(glGenFramebuffers, "glGenFramebuffers", "glGenFramebuffersEXT", "glGenFramebuffersOES");
    setGLExtensionFuncPtr(glCheckFramebufferStatus, "glCheckFramebufferStatus", "glCheckFramebufferStatusEXT", "glCheckFramebufferStatusOES");

    setGLExtensionFuncPtr(glFramebufferTexture1D, "glFramebufferTexture1D", "glFramebufferTexture1DEXT", "glFramebufferTexture1DOES");
    setGLExtensionFuncPtr(glFramebufferTexture2D, "glFramebufferTexture2D", "glFramebufferTexture2DEXT", "glFramebufferTexture2DOES");
    setGLExtensionFuncPtr(glFramebufferTexture3D, "glFramebufferTexture3D", "glFramebufferTexture3DEXT", "glFramebufferTexture3DOES");
    setGLExtensionFuncPtr(glFramebufferTexture, "glFramebufferTexture", "glFramebufferTextureEXT", "glFramebufferTextureOES");
    setGLExtensionFuncPtr(glFramebufferTextureLayer, "glFramebufferTextureLayer", "glFramebufferTextureLayerEXT", "glFramebufferTextureLayerOES");
    setGLExtensionFuncPtr(glFramebufferTextureFace,  "glFramebufferTextureFace", "glFramebufferTextureFaceEXT", "glFramebufferTextureFaceOES" );
    setGLExtensionFuncPtr(glFramebufferRenderbuffer, "glFramebufferRenderbuffer", "glFramebufferRenderbufferEXT", "glFramebufferRenderbufferOES");

    setGLExtensionFuncPtr(glGenerateMipmap, "glGenerateMipmap", "glGenerateMipmapEXT", "glGenerateMipmapOES");
    setGLExtensionFuncPtr(glBlitFramebuffer, "glBlitFramebuffer", "glBlitFramebufferEXT", "glBlitFramebufferOES");
    setGLExtensionFuncPtr(glGetRenderbufferParameteriv, "glGetRenderbufferParameteriv", "glGetRenderbufferParameterivEXT", "glGetRenderbufferParameterivOES");

    isFrameBufferObjectSupported =
        glBindRenderbuffer != 0 &&
        glDeleteRenderbuffers != 0 &&
        glGenRenderbuffers != 0 &&
        glRenderbufferStorage != 0 &&
        glBindFramebuffer != 0 &&
        glDeleteFramebuffers != 0 &&
        glGenFramebuffers != 0 &&
        glCheckFramebufferStatus != 0 &&
        glFramebufferTexture2D != 0 &&
        glFramebufferRenderbuffer != 0 &&
        glGenerateMipmap != 0 &&
        glGetRenderbufferParameteriv != 0 &&
    ( OSG_GLES1_FEATURES || isGLExtensionOrVersionSupported(contextID, "GL_EXT_framebuffer_object",3.0f) );
      

    isPackedDepthStencilSupported = OSG_GL3_FEATURES ||
        (isGLExtensionSupported(contextID, "GL_EXT_packed_depth_stencil")) ||
        (isGLExtensionSupported(contextID, "GL_OES_packed_depth_stencil"));


    // Sync
    osg::setGLExtensionFuncPtr(glFenceSync, "glFenceSync");
    osg::setGLExtensionFuncPtr(glIsSync, "glIsSync");
    osg::setGLExtensionFuncPtr(glDeleteSync, "glDeleteSync");
    osg::setGLExtensionFuncPtr(glClientWaitSync, "glClientWaitSync");
    osg::setGLExtensionFuncPtr(glWaitSync, "glWaitSync");
    osg::setGLExtensionFuncPtr(glGetSynciv, "glGetSynciv");


    // Transform Feeedback
    osg::setGLExtensionFuncPtr(glBeginTransformFeedback, "glBeginTransformFeedback", "glBeginTransformFeedbackEXT");
    osg::setGLExtensionFuncPtr(glEndTransformFeedback, "glEndTransformFeedback", "glEndTransformFeedbackEXT");
    osg::setGLExtensionFuncPtr(glTransformFeedbackVaryings, "glTransformFeedbackVaryings", "glTransformFeedbackVaryingsEXT");
    osg::setGLExtensionFuncPtr(glGetTransformFeedbackVarying, "glGetTransformFeedbackVarying", "glGetTransformFeedbackVaryingEXT");
    osg::setGLExtensionFuncPtr(glBindTransformFeedback, "glBindTransformFeedback");
    osg::setGLExtensionFuncPtr(glDeleteTransformFeedbacks, "glDeleteTransformFeedbacks");
    osg::setGLExtensionFuncPtr(glGenTransformFeedbacks, "glGenTransformFeedbacks");
    osg::setGLExtensionFuncPtr(glIsTransformFeedback, "glIsTransformFeedback");
    osg::setGLExtensionFuncPtr(glPauseTransformFeedback, "glPauseTransformFeedback");
    osg::setGLExtensionFuncPtr(glResumeTransformFeedback, "glResumeTransformFeedback");
    osg::setGLExtensionFuncPtr(glDrawTransformFeedback, "glDrawTransformFeedback");
    osg::setGLExtensionFuncPtr(glDrawTransformFeedbackStream, "glDrawTransformFeedbackStream");
    osg::setGLExtensionFuncPtr(glDrawTransformFeedbackInstanced, "glDrawTransformFeedbackInstanced");
    osg::setGLExtensionFuncPtr(glDrawTransformFeedbackStreamInstanced, "glDrawTransformFeedbackStreamInstanced");
    osg::setGLExtensionFuncPtr(glCreateTransformFeedbacks, "glCreateTransformFeedbacks");
    osg::setGLExtensionFuncPtr(glTransformFeedbackBufferBase, "glTransformFeedbackBufferBase");
    osg::setGLExtensionFuncPtr(glTransformFeedbackBufferRange, "glTransformFeedbackBufferRange");
    osg::setGLExtensionFuncPtr(glGetTransformFeedbackiv, "glGetTransformFeedbackiv");
    osg::setGLExtensionFuncPtr(glGetTransformFeedbacki_v, "glGetTransformFeedbacki_v");
    osg::setGLExtensionFuncPtr(glGetTransformFeedbacki64_v, "glGetTransformFeedbacki64_v");

    //Vertex Array Object
    osg::setGLExtensionFuncPtr(glGenVertexArrays,"glGenVertexArrays");
    osg::setGLExtensionFuncPtr(glBindVertexArray,"glBindVertexArray");
    osg::setGLExtensionFuncPtr(glDeleteVertexArrays,"glDeleteVertexArrays");
    osg::setGLExtensionFuncPtr(glIsVertexArray,"glIsVertexArray");
    
}



///////////////////////////////////////////////////////////////////////////
// C++-friendly convenience methods

GLuint GLExtensions::getCurrentProgram() const
{
    if( glVersion >= 2.0f )
    {
        // GLSL as GL v2.0 core functionality
        GLint result = 0;
        glGetIntegerv( GL_CURRENT_PROGRAM, &result );
        return static_cast<GLuint>(result);
    }
    else if (glGetHandleARB)
    {
        // fallback for GLSL as GL v1.5 ARB extension
#ifndef GL_PROGRAM_OBJECT_ARB
#define GL_PROGRAM_OBJECT_ARB 0x8B40
#endif
        return glGetHandleARB( GL_PROGRAM_OBJECT_ARB );
    }
    else
    {
        OSG_WARN<<"Warning GLExtensions::getCurrentProgram not supported"<<std::endl;;
        return 0;
    }
}


bool GLExtensions::getProgramInfoLog( GLuint program, std::string& result ) const
{
    GLsizei bufLen = 0;        // length of buffer to allocate
    GLsizei strLen = 0;        // strlen GL actually wrote to buffer

    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &bufLen );
    if( bufLen > 1 )
    {
        GLchar* infoLog = new GLchar[bufLen];
        glGetProgramInfoLog( program, bufLen, &strLen, infoLog );
        if( strLen > 0 ) result = reinterpret_cast<char*>(infoLog);
        delete [] infoLog;
    }
    return (strLen > 0);
}


bool GLExtensions::getShaderInfoLog( GLuint shader, std::string& result ) const
{
    GLsizei bufLen = 0;        // length of buffer to allocate
    GLsizei strLen = 0;        // strlen GL actually wrote to buffer

    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &bufLen );
    if( bufLen > 1 )
    {
        GLchar* infoLog = new GLchar[bufLen];
        glGetShaderInfoLog( shader, bufLen, &strLen, infoLog );
        if( strLen > 0 ) result = reinterpret_cast<char*>(infoLog);
        delete [] infoLog;
    }
    return (strLen > 0);
}


bool GLExtensions::getAttribLocation( const char* attribName, GLuint& location ) const
{
    // is there an active GLSL program?
    GLuint program = getCurrentProgram();
    if( glIsProgram(program) == GL_FALSE ) return false;

    // has that program been successfully linked?
    GLint linked = GL_FALSE;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if( linked == GL_FALSE ) return false;

    // is there such a named attribute?
    GLint loc = glGetAttribLocation( program, reinterpret_cast<const GLchar*>(attribName) );
    if( loc < 0 ) return false;

    location = loc;
    return true;
}


bool GLExtensions::getFragDataLocation( const char* fragDataName, GLuint& location ) const
{
    // is there an active GLSL program?
    GLuint program = getCurrentProgram();
    if( glIsProgram(program) == GL_FALSE ) return false;

    // has that program been successfully linked?
    GLint linked = GL_FALSE;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if( linked == GL_FALSE ) return false;

    // check if supported
    if (glGetFragDataLocation == NULL) return false;

    // is there such a named attribute?
    GLint loc = glGetFragDataLocation( program, reinterpret_cast<const GLchar*>(fragDataName) );
    if( loc < 0 ) return false;

    location = loc;
    return true;
}

